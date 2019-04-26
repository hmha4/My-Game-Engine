#include "Framework.h"
#include "MyBehaviourTree.h"
#include "Utilities/Xml.h"

MyBehaviourTree::MyBehaviourTree()
	: name(L"")
{
	root = new Root();
	treeId = 0;
}

MyBehaviourTree::~MyBehaviourTree()
{
	root->Remove();
	SAFE_DELETE(root);
}

bool MyBehaviourTree::Update() const
{
	return root->Invoke();
}

static ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
void MyBehaviourTree::ImguiRender()
{
	ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiSetCond_FirstUseEver);
	if (!ImGui::Begin("Create Behavior Tree", &MyGui::show_app_behaviour, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoMove))
	{
		ImGui::End();
		return;
	}

	//왼쪽면에 노드의 리스트를 보여준다
	bool open_context_menu = false; // 리스트에서도 마우스오른쪽 누르면 메뉴나오게
	int node_hovered_in_list = -1;  // 리스트에서 마우스로 노드위치에 올릴때
	int node_hovered_in_scene = -1; // 씬에서 마우스로 노드위치에 올릴때

	ImGui::BeginChild("[Node List]", ImVec2(100, 0), true);
	{
		ImGui::Text("Nodes");
		ImGui::Separator();
		if (ImGui::Button("Clear All"))
		{
			nodes.clear();
			links.clear();
			root->Remove();
		}

		for (int nodeIndex = 0; nodeIndex < nodes.size(); nodeIndex++)
		{
			ImGuiNode* node = &nodes[nodeIndex];

			//Selectable쓰려면 푸쉬아이디써야함
			ImGui::PushID(node->ID);
			if (ImGui::Selectable(node->Name, node->ID == node_selected))
				node_selected = node->ID;

			//리스트에서 해당노드위에 마우스를 올려놓았을때
			if (ImGui::IsItemHovered())
			{
				node_hovered_in_list = node->ID;
				open_context_menu |= ImGui::IsMouseClicked(1);
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginGroup();
	{
		const float NODE_SLOT_RADIUS = 4.0f;			//슬롯동그라미크기
		const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

		// 중앙켄버스 생성
		if (ImGui::Button("Save Behavior Tree"))
			SaveBehavior();

		ImGui::SameLine();
		ImGui::Checkbox("Show grid", &show_grid);
		float i = ImGui::GetWindowContentRegionWidth();
		ImGui::SameLine(i - 280);
		ImGui::PushItemWidth(160);
		if (ImGui::BeginCombo("", "Required Functions"))
		{
			for (pair<wstring, function<int(void)>> map : functionMap)
			{
				string temp = String::ToString(map.first);
				ImGui::Selectable(temp.c_str());
			}
			ImGui::EndCombo();
		}

		ImGui::Spacing();
		ImGui::Text("Center Position : (%.2f, %.2f)", scrolling.x, scrolling.y);

		//그려지는 노드의 크기및 색상
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, IM_COL32(60, 60, 70, 200));

		//중앙켄버스 영역과 노드의 내부속성 너비
		ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
		ImGui::PushItemWidth(120.0f);

		//중앙켄버스 영역에서 포지션을 구하기위함
		ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;

		//켄버스안에 노드나 모양들 그리기위함
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		// Display grid
		if (show_grid)
		{
			ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
			float GRID_SZ = 64.0f;
			ImVec2 win_pos = ImGui::GetCursorScreenPos();
			ImVec2 canvas_sz = ImGui::GetWindowSize();
			for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
				draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
			for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
				draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
		}

		// Display links
		draw_list->ChannelsSplit(2);
		draw_list->ChannelsSetCurrent(0); // Background

		for (int j = 0; j < links.Size; j++)
		{
			NodeLink* link = &links[j];
			if (link->isMovingList) // 노드간선이 연결안됬을때
			{
				ImGuiNode* node_inp = &nodes[link->FromChildIdx];
				ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link->FromChildSlot);
				ImVec2 p2 = ImGui::GetIO().MousePos;
				draw_list->AddBezierCurve(p1, p1 + ImVec2(0, 50), p2 + ImVec2(0, -50), p2, IM_COL32(200, 200, 100, 255), 3.0f);
			}
			else
			{
				ImGuiNode* node_inp = &nodes[link->FromChildIdx];
				ImGuiNode* node_out = &nodes[link->ToChildIdx];
				ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link->FromChildSlot);
				ImVec2 p2 = offset + node_out->GetInputSlotPos(link->ToChildSlot);
				draw_list->AddBezierCurve(p1, p1 + ImVec2(0, 50), p2 + ImVec2(0, -50), p2, IM_COL32(200, 200, 100, 255), 3.0f);
			}
		}

		// Display nodes
		for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
		{
			ImGuiNode* node = &nodes[node_idx];
			ImGui::PushID(node->ID);
			ImVec2 node_rect_min = offset + node->Pos;

			// Display node contents first
			draw_list->ChannelsSetCurrent(1); // Foreground
			bool old_any_active = ImGui::IsAnyItemActive();
			ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);

			ImGui::BeginGroup(); // Lock horizontal position
			{
				if (node->nType == ImGuiNode::NodeType::Root)
				{
					ImGui::ColorButton("Root", ImColor(150, 150, 150, 255), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoLabel, ImVec2(20, 20));
					ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 1, 1, 1), "Root");
				}
				if (node->nType == ImGuiNode::NodeType::Selector)
				{
					ImGui::ColorButton("Selector", ImColor(150, 150, 150, 255), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoLabel, ImVec2(20, 20));
					ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 1, 1, 1), "Selector");

					ImGui::TextColored(ImVec4(1, 1, 1, 1), node->Name);
				}
				if (node->nType == ImGuiNode::NodeType::Sequence)
				{
					ImGui::ColorButton("Sequence", ImColor(150, 150, 150, 255), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoLabel, ImVec2(20, 20));
					ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 1, 1, 1), "Sequence");

					ImGui::TextColored(ImVec4(1, 1, 1, 1), node->Name);
				}
				if (node->nType == ImGuiNode::NodeType::Decorator)
				{
					ImGui::ColorButton("Decorator", ImColor(60, 150, 60, 255), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoLabel, ImVec2(20, 20));
					ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 1, 1, 1), "Decorator");

					ImGui::TextColored(ImVec4(1, 1, 1, 1), node->Name);
				}
				if (node->nType == ImGuiNode::NodeType::Condition)
				{
					string temp = ENUM_TO_STRING(node->nType);
					string name = node->Name;

					ImGui::ColorButton(temp.append("##" + name).c_str(), ImColor(60, 60, 150, 255), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoLabel, ImVec2(20, 20));
					ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 1, 1, 1), "Condition");

					ImGui::TextColored(ImVec4(1, 1, 1, 1), node->Name);
					ImGui::Spacing();

					
					if (name.find("Function") != string::npos)
					{
						ImGui::Spacing();
						ImGui::Checkbox("##Function", &node->check);
						ImGui::SameLine();
						if (node->check == true)
							ImGui::TextColored(ImVec4(1, 1, 1, 1), "TRUE");
						else ImGui::TextColored(ImVec4(1, 1, 1, 1), "FALSE");
					}
					else if (name.find("Numeric") != string::npos)
					{
						ImGui::Spacing();
						ImGui::TextColored(ImVec4(1, 1, 1, 1), "Value");
						ImGui::InputInt("##Numeric", &node->value); 

						ImGui::Spacing();
						ImGui::TextColored(ImVec4(1, 1, 1, 1), "CompareType");

						ImGui::RadioButton("##Less", (int*)&node->cType, 0);
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(1, 1, 1, 1), "Less"); ImGui::SameLine();
						ImGui::RadioButton("##Greater", (int*)&node->cType, 1);
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(1, 1, 1, 1), "Greater"); ImGui::SameLine();
						ImGui::RadioButton("##Equal", (int*)&node->cType, 2);
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(1, 1, 1, 1), "Equal"); ImGui::SameLine();
						ImGui::RadioButton("##!Equal", (int*)&node->cType, 3);
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(1, 1, 1, 1), "!Equal");
					}
				}
				if (node->nType == ImGuiNode::NodeType::Task)
				{
					ImGui::ColorButton("Task", ImColor(100, 60, 150, 255), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoLabel, ImVec2(20, 20));
					ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 1, 1, 1), "Task");

					ImGui::TextColored(ImVec4(1, 1, 1, 1), node->Name);
				}
			}
			ImGui::EndGroup();

			// Save the size of what we have emitted and whether any of the widgets are being used
			bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
			node->Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
			ImVec2 node_rect_max = node_rect_min + node->Size;

			// Display node box
			draw_list->ChannelsSetCurrent(0); // Background
			ImGui::SetCursorScreenPos(node_rect_min);
			ImGui::InvisibleButton("node", node->Size);
			if (ImGui::IsItemHovered())
			{
				node_hovered_in_scene = node->ID;
				open_context_menu |= ImGui::IsMouseClicked(1);

				if (links.size() >= 1)
				{
					if (ImGui::IsMouseClicked(0) && links[links.size() - 1].isMovingList)
					{

						links[links.size() - 1].isMovingList = false;
						links[links.size() - 1].ToChildIdx = node->ID;

						node->parentIdx = nodes[links[links.size() - 1].FromChildIdx].ID;
					}
				}
			}

			bool node_moving_active = ImGui::IsItemActive();
			if (node_widgets_active || node_moving_active)
				node_selected = node->ID;
			if (node_moving_active && ImGui::IsMouseDragging(0))
				node->Pos = node->Pos + ImGui::GetIO().MouseDelta;

			if (node->nType == ImGuiNode::NodeType::Root)
			{
				ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? IM_COL32(150, 150, 150, 255) : IM_COL32(60, 60, 60, 255);
				draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
				draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(150, 150, 150, 255), 4.0f);
			}
			if (node->nType == ImGuiNode::NodeType::Selector)
			{
				ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? IM_COL32(150, 150, 150, 255) : IM_COL32(60, 60, 60, 255);
				draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
				draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(150, 150, 150, 255), 4.0f);
			}
			if (node->nType == ImGuiNode::NodeType::Sequence)
			{
				ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? IM_COL32(150, 150, 150, 255) : IM_COL32(60, 60, 60, 255);
				draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
				draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(150, 150, 150, 255), 4.0f);
			}
			if (node->nType == ImGuiNode::NodeType::Decorator)
			{
				ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? IM_COL32(60, 150, 60, 255) : IM_COL32(35, 80, 35, 255);
				draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
				draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(60, 150, 60, 255), 4.0f);
			}
			if (node->nType == ImGuiNode::NodeType::Condition)
			{
				ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? IM_COL32(60, 60, 150, 255) : IM_COL32(35, 35, 80, 255);
				draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
				draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(60, 60, 150, 255), 4.0f);
			}
			if (node->nType == ImGuiNode::NodeType::Task)
			{
				ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? IM_COL32(100, 60, 150, 255) : IM_COL32(60, 30, 80, 255);
				draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
				draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(100, 60, 150, 255), 4.0f);
			}

			for (int slot_idx = 0; slot_idx < node->FromChildCount; slot_idx++)
				draw_list->AddCircleFilled(offset + node->GetInputSlotPos(slot_idx), NODE_SLOT_RADIUS, IM_COL32(200, 200, 200, 150));
			for (int slot_idx = 0; slot_idx < node->ToChildCount; slot_idx++)
				draw_list->AddCircleFilled(offset + node->GetOutputSlotPos(slot_idx), NODE_SLOT_RADIUS, IM_COL32(200, 200, 200, 150));

			ImGui::PopID();
		}
		draw_list->ChannelsMerge();

		// Open context menu
		if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
		{
			node_selected = node_hovered_in_list = node_hovered_in_scene = -1;
			open_context_menu = true;
		}
		if (open_context_menu)
		{
			ImGui::OpenPopup("context_menu");
			if (node_hovered_in_list != -1)
				node_selected = node_hovered_in_list;
			if (node_hovered_in_scene != -1)
				node_selected = node_hovered_in_scene;
		}

		// Draw context menu
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
		if (ImGui::BeginPopup("context_menu"))
		{
			ImGuiNode* node = node_selected != -1 ? &nodes[node_selected] : NULL;
			ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;

			if (node)
			{
				ImGui::InputText("Name", node->Name, sizeof(node->Name));
				ImGui::Separator();

				if ((node->nType == ImGuiNode::NodeType::Root ||
					node->nType == ImGuiNode::NodeType::Decorator) &&
					node->ToChildCount >= 1)
				{

				}
				else if(node->nType != ImGuiNode::NodeType::Condition)
				{
					if (ImGui::MenuItem("Add Child"))
					{
						node->ToChildCount++;
						links.push_back(NodeLink(node->ID, node->ToChildCount - 1));
						links[links.size() - 1].isMovingList = true;

					}
				}
				
				if (ImGui::MenuItem("Delete"))
				{
					ImVector<ImGuiNode>::iterator iIter = nodes.begin();

					for (; iIter != nodes.end(); iIter++)
					{
						if (iIter->ID == node->ID)
						{
							nodes.erase(iIter);
							break;
						}
					}
					ImVector<NodeLink>::iterator lIter = links.begin();

					for (; lIter != links.end(); lIter++)
					{
						if (lIter->FromChildIdx == node->ID ||
							lIter->ToChildIdx == node->ID)
						{
							nodes[lIter->FromChildIdx].ToChildCount--;
							links.erase(lIter);
							break;
						}
					}
				}
			}
			else
			{
				if (ImGui::BeginMenu("Add"))
				{
					if (ImGui::MenuItem("Root"))
					{
						ImGuiNode node(nodes.Size, "Root", scene_pos);
						node.nType = ImGuiNode::NodeType::Root;

						nodes.push_back(node);
					}
					if (ImGui::BeginMenu("Selector"))
					{
						ImGuiNode node;

						if (ImGui::MenuItem("Selector"))
						{
							node = ImGuiNode(nodes.Size, "Selector", scene_pos);
							node.nType = ImGuiNode::NodeType::Selector;
							nodes.push_back(node);
						}
						if (ImGui::MenuItem("RandomSelector"))
						{
							node = ImGuiNode(nodes.Size, "RandomSelector", scene_pos);
							node.nType = ImGuiNode::NodeType::Selector;
							nodes.push_back(node);
						}

						ImGui::EndMenu();
					}

					if (ImGui::MenuItem("Sequence"))
					{
						ImGuiNode node(nodes.Size, "Sequence", scene_pos);
						node.nType = ImGuiNode::NodeType::Sequence;

						nodes.push_back(node);
					}
					if (ImGui::BeginMenu("Decorator"))
					{
						ImGuiNode node;

						if (ImGui::MenuItem("Inverter"))
						{
							node = ImGuiNode(nodes.Size, "Inverter", scene_pos);
							node.nType = ImGuiNode::NodeType::Decorator;
							nodes.push_back(node);
						}
						if (ImGui::MenuItem("Succeeder"))
						{
							node = ImGuiNode(nodes.Size, "Succeeder", scene_pos);
							node.nType = ImGuiNode::NodeType::Decorator;
							nodes.push_back(node);
						}
						if (ImGui::MenuItem("Failer"))
						{
							node = ImGuiNode(nodes.Size, "Failer", scene_pos);
							node.nType = ImGuiNode::NodeType::Decorator;
							nodes.push_back(node);
						}
						if (ImGui::MenuItem("Repeater"))
						{
							node = ImGuiNode(nodes.Size, "Repeater", scene_pos);
							node.nType = ImGuiNode::NodeType::Decorator;
							nodes.push_back(node);
						}
						if (ImGui::MenuItem("RepeatUntilFail"))
						{
							node = ImGuiNode(nodes.Size, "RepeatUntilFail", scene_pos);
							node.nType = ImGuiNode::NodeType::Decorator;
							nodes.push_back(node);
						}

						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Condition"))
					{
						ImGuiNode node;

						if (ImGui::MenuItem("Function"))
						{
							node = ImGuiNode(nodes.Size, "Function", scene_pos);
							node.nType = ImGuiNode::NodeType::Condition;
							nodes.push_back(node);
						}
						if (ImGui::MenuItem("Numeric"))
						{
							node = ImGuiNode(nodes.Size, "Numeric", scene_pos);
							node.nType = ImGuiNode::NodeType::Condition;
							nodes.push_back(node);
						}

						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Task"))
					{
						ImGuiNode node(nodes.Size, "Task", scene_pos);
						node.nType = ImGuiNode::NodeType::Task;

						nodes.push_back(node);
					}
					ImGui::EndMenu();
				}
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		// Scrolling
		if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
			scrolling = scrolling + ImGui::GetIO().MouseDelta;

		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);
	}
	ImGui::EndGroup();

	ImGui::End();
}

void MyBehaviourTree::SetRootChild(Node * rootChild)
{
	if (root == NULL) false;

	root->SetChild(rootChild);
}

void MyBehaviourTree::InputFunctions(initializer_list<wstring> && name, initializer_list<function<int(void)>>&& functions)
{
	initializer_list<wstring>::iterator iter = name.begin();

	for (function<int(void)> func : functions)
	{
		functionMap[(*iter)] = func;
		iter++;
	}
}


Node * MyBehaviourTree::CreateTask(wstring name)
{
	Task * task = new Task(functionMap[name]);
	task->name = name;
	task->type = L"Task";

	Node * node = dynamic_cast<Node*>(task);
	if (node != NULL)
		return node;

	return nullptr;
}

Node * MyBehaviourTree::CreateSelector(wstring name)
{
	Node * node = NULL;
	if (name == L"Selector")
	{
		Selector * selector = new Selector();

		node = dynamic_cast<Node *>(selector);
	}
	else if (name == L"RandomSelector")
	{
		RandomSelector * selector = new RandomSelector();

		node = dynamic_cast<Node *>(selector);
	}

	if (node != NULL)
		return node;

	return nullptr;
}

Node * MyBehaviourTree::CreateSequence(wstring name)
{
	Node * node = NULL;

	Sequence * sequence = new Sequence();

	node = dynamic_cast<Node *>(sequence);

	if (node != NULL)
		return node;

	return nullptr;
}

Node * MyBehaviourTree::CreateDecorator(wstring name)
{
	Node * node = NULL;

	if (name == L"Root")
	{
		Root * deco = new Root();
		node = dynamic_cast<Node *>(deco);
	}
	else if (name == L"Inverter")
	{
		Inverter * deco = new Inverter();
		node = dynamic_cast<Node *>(deco);
	}
	else if (name == L"Succeeder")
	{
		Succeeder * deco = new Succeeder();
		node = dynamic_cast<Node *>(deco);
	}
	else if (name == L"Failer")
	{
		Failer * deco = new Failer();
		node = dynamic_cast<Node *>(deco);
	}
	else if (name == L"Repeater")
	{
		Repeater * deco = new Repeater();
		node = dynamic_cast<Node *>(deco);
	}
	else if (name == L"RepeatUntilFail")
	{
		RepeatUntilFail * deco = new RepeatUntilFail();
		node = dynamic_cast<Node *>(deco);
	}

	if (node != NULL)
		return node;

	return nullptr;
}

Node * MyBehaviourTree::CreateTFCondition(wstring name, bool check)
{
	Node * node = NULL;

	TFCondition * condition = new TFCondition(functionMap[name], check);
	condition->name = name;

	node = dynamic_cast<Node *>(condition);

	if (node != NULL)
		return node;

	return nullptr;
}

Node * MyBehaviourTree::CreateNumericCondition(wstring name, int val, ECompareType type)
{
	Node * node = NULL;

	NumericCondition * condition = new NumericCondition(functionMap[name], val, type);
	condition->name = name;

	node = dynamic_cast<Node *>(condition);

	if (node != NULL)
		return node;

	return nullptr;
}

void MyBehaviourTree::SaveBehavior()
{
	root->Remove();
	vector<Node*> saveNodes;

	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].nType == ImGuiNode::NodeType::Root)
		{
			wstring name = String::ToWString(nodes[i].Name);
			root->name = name;

			saveNodes.push_back(root);
		}
		else if (nodes[i].nType == ImGuiNode::NodeType::Sequence)
		{
			wstring name = String::ToWString(nodes[i].Name);

			saveNodes.push_back(CreateSequence(name));
		}
		else if (nodes[i].nType == ImGuiNode::NodeType::Selector)
		{
			wstring name = String::ToWString(nodes[i].Name);

			saveNodes.push_back(CreateSelector(name));
		}
		else if (nodes[i].nType == ImGuiNode::NodeType::Decorator)
		{
			wstring name = String::ToWString(nodes[i].Name);

			saveNodes.push_back(CreateDecorator(name));
		}
		else if (nodes[i].nType == ImGuiNode::NodeType::Condition)
		{
			wstring name = String::ToWString(nodes[i].Name);

			if (name.find(L"Function") != wstring::npos)
				saveNodes.push_back(CreateTFCondition(name, nodes[i].check));
			else if (name.find(L"Numeric") != wstring::npos)
				saveNodes.push_back(CreateNumericCondition(name, nodes[i].value, nodes[i].cType));
		}
		else if (nodes[i].nType == ImGuiNode::NodeType::Task)
		{
			wstring name = String::ToWString(nodes[i].Name);

			saveNodes.push_back(CreateTask(name));
		}
	}


	for (size_t i = 0; i < saveNodes.size(); i++)
	{
		for (size_t j = 0; j < saveNodes.size(); j++)
		{
			if (i == j)continue;

			if (i == nodes[j].parentIdx)
			{
				if (saveNodes[i]->type == L"Decorator")
				{
					DecoratorNode * decorator = dynamic_cast<DecoratorNode*>(saveNodes[i]);
					if (decorator != NULL)
						decorator->SetChild(saveNodes[j]);
				}
				else if (saveNodes[i]->type == L"Selector" || saveNodes[i]->type == L"Sequence")
				{
					CompositeNode * composite = dynamic_cast<CompositeNode*>(saveNodes[i]);
					if (composite != NULL)
						composite->AddChild(saveNodes[j]);
				}
			}
		}
	}
}

void MyBehaviourTree::GetImguiTreeNode(Node * node, int curIdx, int parentIdx)
{
	int nodeId = curIdx;
	ImVec2 nodePos = { 0,0 };
	int toChildCount = 0;
	vector<Node*>::iterator nIter;

	CompositeNode * composite = dynamic_cast<CompositeNode*>(node);
	if (composite != NULL)
	{
		for (nIter = composite->GetChildren().begin(); nIter != composite->GetChildren().end(); nIter++)
			toChildCount++;
	}
	DecoratorNode * decorator = dynamic_cast<DecoratorNode*>(node);
	if (decorator != NULL)
		toChildCount++;

	// 자기자신부터 넣어준다
	ImGuiNode imguiNode;

	
	nodePos.x += 100 * curIdx;
	nodePos.y += 100 * parentIdx;

	imguiNode.ID = nodeId;
	imguiNode.Pos = nodePos;
	imguiNode.FromChildCount = 1;
	imguiNode.ToChildCount = toChildCount;
	imguiNode.parentIdx = parentIdx;

	strncpy(imguiNode.Name, String::ToString(node->name).c_str(), 31);
	imguiNode.Name[31] = 0;

	if (node->name == L"Root")
	{
		imguiNode.nType = ImGuiNode::NodeType::Root;
	}
	else if (node->name == L"Selector" || node->name == L"RandomSelector")
	{
		imguiNode.nType = ImGuiNode::NodeType::Selector;
	}
	else if (node->name == L"Sequence")
	{
		imguiNode.nType = ImGuiNode::NodeType::Sequence;
	}
	else if (node->name == L"Inverter" || node->name == L"Succeeder" || node->name == L"Failer" || node->name == L"Repeater" || node->name == L"RepeatUntilFail")
	{
		imguiNode.nType = ImGuiNode::NodeType::Decorator;
	}
	else if (node->name.find(L"Function") != wstring::npos || node->name.find(L"Numeric") != wstring::npos)
	{
		imguiNode.nType = ImGuiNode::NodeType::Condition;

		TFCondition * func = dynamic_cast<TFCondition*>(node);
		NumericCondition * numeric = dynamic_cast<NumericCondition*>(node);
		if (func != NULL)
		{
			imguiNode.check = func->Check();
			func->Func(functionMap[func->name]);
		}

		if (numeric != NULL)
		{
			imguiNode.cType = (ECompareType)numeric->CType();
			imguiNode.value = numeric->Val();
			numeric->Func(functionMap[numeric->name]);
		}
	}
	else
	{
		imguiNode.nType = ImGuiNode::NodeType::Task;
		Task * task = dynamic_cast<Task*>(node);
		if (task != NULL)
			task->Func(functionMap[task->name]);
	}

	nodes[nodeId] = imguiNode;
	treeId++;

	if (nodes[nodeId].nType == ImGuiNode::NodeType::Task) return;
	if (nodes[nodeId].nType == ImGuiNode::NodeType::Condition) return;

	if (composite != NULL)
	{
		for (nIter = composite->GetChildren().begin(); nIter != composite->GetChildren().end(); nIter++)
			GetImguiTreeNode((*nIter), treeId, curIdx);
	}
	if (decorator != NULL)
	{
		Node * node = decorator->GetChild();
		if (node != NULL)
			GetImguiTreeNode(node, treeId, curIdx);
	}
}

void MyBehaviourTree::GetImguiLink(int index, int parent)
{
	ImGuiNode node = nodes[index];
	if (node.ToChildCount == 0)
		return;

	for (int i = 0; i < node.ToChildCount; i++)
	{
		NodeLink link;
		link.FromChildIdx = parent + 1;
		link.ToChildSlot = 0;
		link.ToChildIdx = links.size() + 1;
		link.FromChildSlot = i;


		links.push_back(link);
		GetImguiLink(links.size(), links.size()-1);
	}
}

void MyBehaviourTree::SaveTree(Xml::XMLElement * parent, Xml::XMLElement * element, Xml::XMLDocument*document, Node*node)
{
	//맨처음 루트노드
	static Xml::XMLElement*sElement = element;

	sElement = document->NewElement(String::ToString(node->type).c_str());
	sElement->SetText(String::ToString(node->name).c_str());
	parent->LinkEndChild(sElement);

	sElement = document->NewElement("ChildCount");
	CompositeNode * composite = dynamic_cast<CompositeNode*>(node);
	if (composite != NULL)
		sElement->SetText(composite->GetChildren().size());
	DecoratorNode * decorator = dynamic_cast<DecoratorNode*>(node);
	if (decorator != NULL)
		sElement->SetText(1);
	if (node->type == L"Task" || node->type == L"Condition")
		sElement->SetText(0);
	
	parent->LinkEndChild(sElement);

	//if (node->type == L"Task" || node->type == L"Condition")
	//	return;

	sElement = document->NewElement("Child");
	parent->LinkEndChild(sElement);
	vector<Node*>::iterator nIter;

	if (composite != NULL)
	{
		for (nIter = composite->GetChildren().begin(); nIter != composite->GetChildren().end(); nIter++)
		{
			Xml::XMLElement * childElement = NULL;
			childElement = document->NewElement("Type");
			childElement->SetText(String::ToString((*nIter)->type).c_str());
			sElement->LinkEndChild(childElement);
			childElement = document->NewElement("Name");
			childElement->SetText(String::ToString((*nIter)->name).c_str());
			sElement->LinkEndChild(childElement);
			if ((*nIter)->name.find(L"Function") != wstring::npos)
			{
				TFCondition * TF = dynamic_cast<TFCondition*>((*nIter));
				if (TF != NULL)
				{
					childElement = document->NewElement("Check");
					childElement->SetText((int)TF->Check());
					sElement->LinkEndChild(childElement);
				}
			}
			if ((*nIter)->name.find(L"Numeric") != wstring::npos)
			{
				NumericCondition * numeric = dynamic_cast<NumericCondition*>((*nIter));
				if (numeric != NULL)
				{
					childElement = document->NewElement("Value");
					childElement->SetText(numeric->Val());
					sElement->LinkEndChild(childElement);
					childElement = document->NewElement("CompareType");
					childElement->SetText(numeric->CType());
					sElement->LinkEndChild(childElement);
				}
			}
		}

		for (nIter = composite->GetChildren().begin(); nIter != composite->GetChildren().end(); nIter++)
		{
			SaveTree(parent, sElement, document, (*nIter));
		}
	}

	if (decorator != NULL)
	{
		Node * node = decorator->GetChild();

		Xml::XMLElement * childElement = NULL;
		childElement = document->NewElement("Type");
		childElement->SetText(String::ToString(node->type).c_str());
		sElement->LinkEndChild(childElement);
		childElement = document->NewElement("Name");
		childElement->SetText(String::ToString(node->name).c_str());
		sElement->LinkEndChild(childElement);
		if (node->name.find(L"Function") != wstring::npos)
		{
			TFCondition * TF = dynamic_cast<TFCondition*>(node);
			if (TF != NULL)
			{
				childElement = document->NewElement("Check");
				childElement->SetText((int)TF->Check());
				sElement->LinkEndChild(childElement);
			}
		}
		if (node->name.find(L"Numeric") != wstring::npos)
		{
			NumericCondition * numeric = dynamic_cast<NumericCondition*>(node);
			if (numeric != NULL)
			{
				childElement = document->NewElement("Value");
				childElement->SetText(numeric->Val());
				sElement->LinkEndChild(childElement);
				childElement = document->NewElement("CompareType");
				childElement->SetText(numeric->CType());
				sElement->LinkEndChild(childElement);
			}
		}
		

		SaveTree(parent, sElement, document, node);
	}
}

void MyBehaviourTree::ReadBehaviorTree(Xml::XMLElement * element, Node * node)
{
	readBehavior = element;
	
	wstring nodeName = String::ToWString(readBehavior->GetText());

	readBehavior = readBehavior->NextSiblingElement(); // 자식갯수
	CompositeNode * composite = dynamic_cast<CompositeNode*>(node);
	DecoratorNode * decorator = dynamic_cast<DecoratorNode*>(node);
	int childCount = 0;
	if (decorator != NULL || composite != NULL)
		childCount = (int)readBehavior->FloatText();

	readBehavior = readBehavior->NextSiblingElement(); // 자식

	Xml::XMLElement * childElement = readBehavior->FirstChildElement();

	if (childElement == NULL)
	{
		readBehavior = readBehavior->NextSiblingElement();
		return;
	}

	Node * temp = NULL;
	for (int i = 0; i < childCount; i++)
	{
		wstring childType = String::ToWString(childElement->GetText());
		childElement = childElement->NextSiblingElement();
		wstring childName = String::ToWString(childElement->GetText());
		childElement = childElement->NextSiblingElement();

		if (childType == L"Selector")
			temp = CreateSelector(childName);
		else if (childType == L"Sequence")
			temp = CreateSequence(childName);
		else if (childType == L"Decorator")
			temp = CreateDecorator(childName);
		else if (childName.find(L"Function") != wstring::npos)
		{
			bool check = (bool)childElement->FloatText();
			childElement = childElement->NextSiblingElement();

			temp = CreateTFCondition(childName, check);
		}
		else if (childName.find(L"Numeric") != wstring::npos)
		{
			int val = (int)childElement->FloatText();
			childElement = childElement->NextSiblingElement();
			int cType = (int)childElement->FloatText();
			childElement = childElement->NextSiblingElement();

			temp = CreateNumericCondition(childName, val, (ECompareType)cType);
		}
		else
			temp = CreateTask(childName);

		if (composite != NULL)
			composite->AddChild(temp);
		if (decorator != NULL)
			decorator->SetChild(temp);
	}

	readBehavior = readBehavior->NextSiblingElement(); // 노드이름

	vector<Node*>::iterator nIter;

	if (composite != NULL)
	{
		for (nIter = composite->GetChildren().begin(); nIter != composite->GetChildren().end(); nIter++)
			ReadBehaviorTree(readBehavior, (*nIter));
	}
	if (decorator != NULL)
	{
		Node * node = decorator->GetChild();
		ReadBehaviorTree(readBehavior, node);
	}
}

void MyBehaviourTree::BindNodes()
{
	nodes.resize(30);
	GetImguiTreeNode(root, 0, -1);
	GetImguiLink(0, -1);

	nodes.resize(links[links.size() - 1].ToChildIdx + 1);

	treeId = 0;
}
