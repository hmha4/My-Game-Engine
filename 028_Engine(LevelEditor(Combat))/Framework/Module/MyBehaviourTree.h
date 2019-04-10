#pragma once

namespace Xml
{
	class XMLDocument;
	class XMLElement;
}

// --------------------------------------------------------------------- //
//	This class represents each node int the behavior tree
// --------------------------------------------------------------------- //
class Node
{
public:
	Node() {}
	virtual ~Node() {}

	virtual bool Invoke() = 0;
	virtual void Remove() = 0;

	wstring name;
	wstring type;

};

class Task : public Node
{
public:
	Task(function<bool(void)> func) : func(func)
	{
		name = L"Task";
		type = L"Task";
	}

	void Func(function<bool(void)> func) { this->func = func; }

	virtual bool Invoke() override
	{
		return func();
	}

	virtual void Remove() override
	{

	}

private:
	function<bool(void)> func;
};

// --------------------------------------------------------------------- //
//	This type of node follows composite pattern, 
//	containing a list of other nodes
// --------------------------------------------------------------------- //
class CompositeNode : public Node
{
public:
	vector<Node *> &GetChildren() { return children; }	//	Cannot change children neither after return nor inside the function
	void AddChild(Node * child) { children.emplace_back(child); }

	//TODO: find out what && is
	void AddChildren(initializer_list<Node*>&& newChildren)			//	initializer_list<T> : ���� ���� ����Ʈ
	{
		for (Node * child : newChildren)
			AddChild(child);
	}


protected:
	vector<Node *> ChildrenShuffled() const
	{
		vector<Node *> temp = children;
		random_shuffle(temp.begin(), temp.end());
		return temp;
	}

private:
	vector<Node *> children;
};

// --------------------------------------------------------------------- //
//	These types of nodes are control flow nodes
//	Control flow ����� ������ �� ��� �Ʒ��� �ִ� 
//	���� Ʈ��(�귣ġ�� �Ҹ�)�� ���� ǥ���Ǵ� �ൿ(���� �½�ũ)�� �����ϴ� ���Դϴ�.
// --------------------------------------------------------------------- //

//	Selector : Run until one child returns true
class Selector : public CompositeNode
{
public:
	Selector()
	{
		type = L"Selector";
		name = L"Selector";
	}

	virtual bool Invoke() override
	{
		for (Node * child : GetChildren())
		{
			if (child->Invoke() == true)
				return true;
		}

		return false;
	}

	virtual void Remove() override
	{
		for (Node * child : GetChildren())
		{
			child->Remove();
			SAFE_DELETE(child);
		}
		GetChildren().clear();
	}
};

class RandomSelector : public CompositeNode
{
public:
	RandomSelector()
	{
		type = L"Selector";
		name = L"RandomSelector";
	}
	virtual bool Invoke() override
	{
		for (Node * child : ChildrenShuffled())
		{
			if (child->Invoke() == true)
				return true;
		}

		return false;
	}

	virtual void Remove() override
	{
		for (Node * child : GetChildren())
		{
			child->Remove();
			SAFE_DELETE(child);
		}
		GetChildren().clear();
	}
};

//	Sequence : Run until one child returns false
class Sequence : public CompositeNode
{
public:
	Sequence()
	{
		type = L"Sequence";
		name = L"Sequence";
	}

	virtual bool Invoke() override
	{
		for (Node * child : GetChildren())
		{
			if (child->Invoke() == false)
				return false;
		}

		return true;
	}

	virtual void Remove() override
	{
		for (Node * child : GetChildren())
		{
			child->Remove();
			SAFE_DELETE(child);
		}
		GetChildren().clear();
	}
};

// --------------------------------------------------------------------- //
//	Decorator
//	Function is either to transform the result it receives from 
//	its child node's status, to terminate the child, or repeat
//	processing of the child, depending on the type of decorator node
// --------------------------------------------------------------------- //
class DecoratorNode : public Node
{
public:
	void SetChild(Node * newChild)
	{
		child = newChild;
	}

	Node * GetChild() const { return child; }
protected:
	void DeleteChild() { SAFE_DELETE(child); }

private:
	Node * child;	//	Only one child is allowed
};

class Root : public DecoratorNode
{
public:
	Root()
	{
		type = L"Decorator";
		name = L"Root";
		SetChild(NULL);
	}
	//friend class BehaviourTree;
	virtual bool Invoke() override
	{
		if (GetChild() != NULL)
			return GetChild()->Invoke();
		else
			return false;
	}
	virtual void Remove() override
	{
		if (GetChild() != NULL)
			GetChild()->Remove();

		DeleteChild();
	}
};

//	Inverts the result of the child. A child fails and it will 
//	return success to its parent, or a child succeeds and it 
//	will return failure to the parent.
class Inverter : public DecoratorNode
{
public:
	Inverter()
	{
		type = L"Decorator";
		name = L"Inverter";
		SetChild(NULL);
	}

	virtual bool Invoke() override { return !GetChild()->Invoke(); }
	virtual void Remove() override
	{
		if (GetChild() != NULL)
			GetChild()->Remove();

		DeleteChild();
	}
};

//	A succeeder will always return success, irrespective of what
//	the child node actually returned. These are useful in cases 
//	where you want to process a branch of a tree where a failure 
//	is expected or anticipated, but you don��t want to abandon
//	processing of a sequence that branch sits on.
class Succeeder : public DecoratorNode
{
public:
	Succeeder()
	{
		type = L"Decorator";
		name = L"Succeeder";
		SetChild(NULL);
	}
	virtual bool Invoke() override { GetChild()->Invoke();  return true; }
	virtual void Remove() override
	{
		if (GetChild() != NULL)
			GetChild()->Remove();

		DeleteChild();
	}
};

//	The opposite of a Succeeder, always returning fail.
//	Note that this can be achieved also by using an 
//	Inverter and setting its child to a Succeeder.
class Failer : public DecoratorNode
{
public:
	Failer()
	{
		type = L"Decorator";
		name = L"Failer";
		SetChild(NULL);
	}
	virtual bool Invoke() override { GetChild()->Invoke();  return false; }
	virtual void Remove() override
	{
		if (GetChild() != NULL)
			GetChild()->Remove();

		DeleteChild();
	}
};

//	A repeater will reprocess its child node each time its child returns 
//	a result. These are often used at the very base of the tree, 
//	to make the tree to run continuously. Repeaters may optionally
//	run their children a set number of times before returning to their parent.
class Repeater : public DecoratorNode
{
public:
	Repeater(int num = -1) : numRepeats(num)
	{
		type = L"Decorator";
		name = L"Repeater";
		SetChild(NULL);
	}  // By default, never terminate.
	virtual bool Invoke() override
	{
		if (numRepeats == -1)
			while (true) GetChild()->Invoke();
		else {
			for (int i = 0; i < numRepeats - 1; i++)
				GetChild()->Invoke();
			return GetChild()->Invoke();
		}
	}
	virtual void Remove() override
	{
		if (GetChild() != NULL)
			GetChild()->Remove();

		DeleteChild();
	}
private:
	int numRepeats;
};

//	Like a repeater, these decorators will continue to reprocess 
//	their child. That is until the child finally returns a failure, 
//	at which point the repeater will return success to its parent.
class RepeatUntilFail : public DecoratorNode
{
public:
	RepeatUntilFail()
	{
		type = L"Decorator";
		name = L"RepeatUntilFail";
		SetChild(NULL);
	}
	virtual bool Invoke() override
	{
		while (GetChild()->Invoke() == true) {}
		return true;
	}
	virtual void Remove() override
	{
		if (GetChild() != NULL)
			GetChild()->Remove();

		DeleteChild();
	}
};

// --------------------------------------------------------------------- //
//	Condition
// --------------------------------------------------------------------- //
class TFCondition : public Node
{
public:
	TFCondition(function<bool(void)> func, bool check) : func(func), check(check)
	{
		type = L"Condition";
		name = L"Function";
	}

	bool Check() { return check; }
	void Func(function<bool(void)> func) { this->func = func; }

	virtual bool Invoke() override
	{
		if (func() == check)
			return true;
		else
			return false;
	}
	virtual void Remove() override { }
private:
	function<bool(void)> func;
	bool check;
};


enum class ECompareType
{
	ELess, EGreater, EEqual, ENotEqual,
};

class NumericCondition : public Node
{
public:
	NumericCondition(function<int(void)> func, int val, ECompareType type) : func(func), val(val), cType(type)
	{
		this->type = L"Condition";
		this->name = L"Numeric";
	}

	int Val() { return val; }
	int CType() { return (int)cType; }
	void Func(function<int(void)> func) { this->func = func; }

	virtual bool Invoke() override
	{
		bool result = false;
		int objVal = func();

		switch (cType)
		{
		case ECompareType::ELess:
			result = objVal < val;
			break;
		case ECompareType::EGreater:
			result = objVal > val;
			break;
		case ECompareType::EEqual:
			result = objVal == val;
			break;
		case ECompareType::ENotEqual:
			result = objVal != val;
			break;
		}

		return result;
	}
	virtual void Remove() override { }
private:
	function<int(void)> func;
	int val;
	ECompareType cType;
};

struct NodeLink
{
	int FromChildIdx, FromChildSlot;
	int ToChildIdx, ToChildSlot;
	int sibCount = 0;

	bool isMovingList = false;

	NodeLink() {}
	NodeLink(int fromIdx, int fromSlot)
	{
		FromChildIdx = fromIdx;
		FromChildSlot = fromSlot;
		ToChildIdx = 0;
		ToChildSlot = 0;
	}
};

struct ImGuiNode
{
	enum NodeType
	{
		Root,
		Selector,
		Sequence,
		Decorator,
		Condition,
		Task
	} nType;

	bool check;
	int value;
	ECompareType cType;

	int ID;				//	�������̵�
	char Name[32];		//	���(�ൿ)�̸�
	ImVec2 Pos;			//	��ġ
	ImVec2 Size;		//	������
	int FromChildCount;	//	������ �����ڽİ���
	int ToChildCount;
	int parentIdx;

	ImGuiNode() {}
	ImGuiNode(int id, char*name, const ImVec2& pos)
		: ID(id)
		, Pos(pos)
		, FromChildCount(1)
		, ToChildCount(0)
		, parentIdx(-1)
		, check(false)
		, value(0)
		, cType(ECompareType::EGreater)
	{
		strncpy(Name, name, 31);
		Name[31] = 0;
	}

	void TFConditionVar(bool val) {
		check = val;
	}
	void NumericConditionVar(int val, ECompareType type)
	{
		value = val;
		cType = type;
	}

	ImVec2 GetInputSlotPos(int slot_no) const
	{
		return ImVec2(Pos.x + Size.x * ((float)slot_no + 1) / ((float)FromChildCount + 1), Pos.y);
	}

	ImVec2 GetOutputSlotPos(int slot_no) const // �ش縵ũ������ �����ǹ�ȯ
	{
		return ImVec2(Pos.x + Size.x * ((float)slot_no + 1) / ((float)ToChildCount + 1), Pos.y + Size.y);
	}
};

class MyBehaviourTree
{
public:
	MyBehaviourTree();
	~MyBehaviourTree();

	bool Update() const;
	void ImguiRender();

	void SetRootChild(Node * rootChild);
	void InputFunctions
	(
		initializer_list<wstring>&& name,
		initializer_list<function<int(void)>>&& functions
	);

	wstring& Name() { return name; }
	Root * GetRoot() { return root; }
	// xml�� �����ϴ� �Լ�
	void SaveTree(Xml::XMLElement * parent, Xml::XMLElement * element, Xml::XMLDocument*document, Node*node);
	// ��ü���� �����̹�Ʈ�� �д��Լ�
	void ReadBehaviorTree(Xml::XMLElement * element, Node*node);
	void BindNodes();
private:
	Node * CreateTask(wstring name);
	Node *CreateSelector(wstring name);
	Node *CreateSequence(wstring name);
	Node *CreateDecorator(wstring name);
	Node *CreateTFCondition(wstring name, bool check);
	Node *CreateNumericCondition(wstring name, int val, ECompareType type);

private:
	// ������ �����ϴ��Լ�
	void SaveBehavior();
	// ��Ʈ�������� �ӱ��̳��� �о���� �Լ�
	void GetImguiTreeNode(Node *node, int curIdx, int parentIdx, int parentSibCount);
	void GetImguiLink();
	void GetImguiLink(int parent, int index);


private:
	Root * root;
	map<wstring, function<int(void)>> functionMap;
	wstring name;
private:
	ImVector<ImGuiNode> nodes;
	ImVector<NodeLink> links;	// �θ�->�ڽ������� ����
	ImVec2 scrolling = ImVec2(0.0f, 0.0f);
	bool show_grid = true;
	int node_selected = -1;	//	������ �����̵�
	int treeId;

	class Xml::XMLElement * readBehavior;
};
