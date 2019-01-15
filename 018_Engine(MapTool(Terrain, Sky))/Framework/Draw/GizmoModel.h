#pragma once

class GizmoModel
{
public:
	GizmoModel(vector<D3DXVECTOR3>&positions, vector<D3DXVECTOR3>&normals, UINT*indices)
	{
		Indices = indices;
		vector<D3DXVECTOR2*> texData;
		texData.push_back(new D3DXVECTOR2(0, 0));
		texData.push_back(new D3DXVECTOR2(1, 0));
		texData.push_back(new D3DXVECTOR2(1, 1));
		
		Vertices = new VertexTextureNormal[positions.size()];
		for (size_t i = 0; i < positions.size(); i++)
		{
			Vertices[i].Position = positions[i];
			Vertices[i].Normal = normals[i];
			Vertices[i].Uv = *texData[i % 3];
		}
	}
	~GizmoModel()
	{
		SAFE_DELETE_ARRAY(Vertices);
		SAFE_DELETE_ARRAY(Indices);
	}
	VertexTextureNormal*GetVertices() { return Vertices; }
	UINT*GetIndices() { return Indices; }
private:
	VertexTextureNormal*Vertices;
	UINT*Indices;
};

class Geometry
{
public:
	enum class GeometryType
	{
		AXIS_TYPE, // ������Ÿ��(SRT+line)
		GEO_TYPE // ����Ÿ��
	};
	enum class XYZAxis
	{
		XLine,
		YLine,
		ZLine,
		XAxis,
		YAxis,
		ZAxis,
		XZPlane,
		XYPlane,
		YZPlane,
		XBox,
		YBox,
		ZBox
	};
	enum class GizmoType
	{
		Component, // ������Ʈ�� ���ӵǴ� ���
		Own		   // ���ӵ��� �ʴ°��
	};
protected:
	GizmoModel*model;
	XYZAxis axisType;
	GizmoType gizmoType;
	GeometryType geoType;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	Shader*shader;

	D3DXVECTOR3 position;
	D3DXVECTOR3 rotate;
	D3DXVECTOR3 scale;

	D3DXVECTOR3 direction;
	D3DXVECTOR3 up;
	D3DXVECTOR3 right;

	D3DXMATRIX world;
	D3DXMATRIX rootAxis;
	D3DXMATRIX*targetWorld;

	UINT vertexCount, indicesCount;
	class LineMaker*line;
	D3DXVECTOR3 lineDir;// => �̰� �ٲ𶧸��� �����ϴ� �Լ��ʿ�
	float lineLength;

	int transModel;	//S,R,T���� �̵��ѻ��·� ó���ؾ� �ϹǷ� 1
					//�ڽ��� �÷��� ���� �׷��ʿ�����Ƿ� 0

	D3DXCOLOR gizmoColor;
	bool pickState; // ��ŷ�����

public:
	D3DXVECTOR3 Position();
	void Position(D3DXVECTOR3 pos);
	void Position(float x, float y, float z);

	D3DXVECTOR3 Rotation();
	void Rotation(D3DXVECTOR3 rotate);
	void Rotation(float x, float y, float z);
	void RotationDegree(D3DXVECTOR3 rotate);
	void RotationDegree(float x, float y, float z);
	
	D3DXVECTOR3 Scale();
	void Scale(D3DXVECTOR3 scale);
	void Scale(float x, float y, float z);

	void SetColor(float x, float y, float z);
	void SetColor(D3DXCOLOR color);
	void SetColor(D3DXVECTOR3 color);

	void SetLineLength(float length);
	void SetTargetWorld(D3DXMATRIX*target);

	bool IsPicked();
private:

	void UpdateOnColor();
	void UpdateOffColor();
	void UpdateWorld();
	void LineInit();
	void AxisInit();
	void PlaneInit();
	void BoxInit();
public:
	Geometry(XYZAxis axis,int TransModel=1);
	virtual ~Geometry();

	virtual void Update();
	virtual void Render();
};

//���߿� �ش���� ���带 �����ϰ��ϴ� �����ͺ����߰�
//��ŷ��� �߰��ؾ���
//�𵨿� �ش��ϴ� ���α׸��°͵� �߰�
class Geometry_Scale:public Geometry
{
public:
	Geometry_Scale(XYZAxis axis, GizmoType gizmoType);
	~Geometry_Scale();

	void Update() override;
	void Render() override;
};

class Geometry_Rotate :public Geometry
{
public:
	Geometry_Rotate(XYZAxis axis, GizmoType gizmoType);
	~Geometry_Rotate();

	void Update() override;
	void Render() override;
};

class Geometry_Trans :public Geometry
{
public:
	Geometry_Trans(XYZAxis axis, GizmoType gizmoType);
	~Geometry_Trans();

	void Update() override;
	void Render() override;
};

class Geometry_Plane :public Geometry
{
public:
	Geometry_Plane(XYZAxis axis, GizmoType gizmoType);
	~Geometry_Plane();

	void Update() override;
	void Render() override;
};

class Geometry_Line :public Geometry
{
public:
	Geometry_Line(XYZAxis axis, GizmoType gizmoType);
	~Geometry_Line();

	void Update() override;
	void Render() override;
};

class Geometry_Box :public Geometry
{
public:
	Geometry_Box(XYZAxis axis, GizmoType gizmoType);
	~Geometry_Box();

	void Update() override;
	void Render() override;
};