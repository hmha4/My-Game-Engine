#pragma once

class GizmoModel
{
public:
	GizmoModel(vector<D3DXVECTOR3>&positions, vector<D3DXVECTOR3>&normals, UINT*indices)
	{
		Indices = indices;
		
		texData.push_back(new D3DXVECTOR2(0, 0));
		texData.push_back(new D3DXVECTOR2(1, 0));
		texData.push_back(new D3DXVECTOR2(1, 1));

		Vertices = new VertexTextureNormal[positions.size()];
		for (int i = 0; i < positions.size(); i++)
		{
			Vertices[i].Position = positions[i];
			Vertices[i].Normal = normals[i];
			Vertices[i].Uv = *texData[i % 3];
		}
	}
	~GizmoModel()
	{
		for (D3DXVECTOR2 * data : texData)
			SAFE_DELETE(data);
		texData.clear();
		texData.shrink_to_fit();

		SAFE_DELETE_ARRAY(Vertices);
		SAFE_DELETE_ARRAY(Indices);
	}
	VertexTextureNormal*GetVertices() { return Vertices; }
	UINT*GetIndices() { return Indices; }
private:
	VertexTextureNormal*Vertices;
	UINT*Indices;

	vector<D3DXVECTOR2*> texData;
};

class Geometry
{
public:
	enum class GeometryType
	{
		AXIS_TYPE, // 고정축타입(SRT+line)
		GEO_TYPE // 자유타입
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
		Component, // 컴포넌트로 종속되는 경우
		Own		   // 종속되지 않는경우
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
	Shader * lineShader;
	class LineMaker*line;
	D3DXVECTOR3 lineDir;// => 이거 바뀔때마다 조정하는 함수필요
	float lineLength;

	int transModel;	//S,R,T모델은 이동한상태로 처리해야 하므로 1
					//박스나 플레인 모델은 그럴필요없으므로 0

	D3DXCOLOR gizmoColor;

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
	Geometry(XYZAxis axis, Shader*shader, class Shader*line, int TransModel = 1);
	virtual ~Geometry();

	virtual void Initialize();
	virtual void Ready();
	virtual void Update();
	virtual void Render();
};

//나중에 해당모델의 월드를 조작하게하는 포인터변수추가
//피킹기능 추가해야함
//모델에 해당하는 라인그리는것도 추가
class Geometry_Scale:public Geometry
{
public:
	Geometry_Scale(XYZAxis axis, Shader*shader, class Shader*line, GizmoType gizmoType);
	~Geometry_Scale();


	void Initialize() override;
	void Ready() override;
	void Update() override;
	void Render() override;
};

class Geometry_Rotate :public Geometry
{
public:
	Geometry_Rotate(XYZAxis axis, Shader*shader, class Shader*line, GizmoType gizmoType);
	~Geometry_Rotate();

	void Initialize() override;
	void Ready() override;
	void Update() override;
	void Render() override;
};

class Geometry_Trans :public Geometry
{
public:
	Geometry_Trans(XYZAxis axis, Shader*shader, class Shader*line, GizmoType gizmoType);
	~Geometry_Trans();

	void Initialize() override;
	void Ready() override;
	void Update() override;
	void Render() override;
};

class Geometry_Plane :public Geometry
{
public:
	Geometry_Plane(XYZAxis axis, Shader*shader, class Shader*line, GizmoType gizmoType);
	~Geometry_Plane();

	void Initialize() override;
	void Ready() override;
	void Update() override;
	void Render() override;
};

class Geometry_Line :public Geometry
{
public:
	Geometry_Line(XYZAxis axis, Shader*shader, class Shader*line, GizmoType gizmoType);
	~Geometry_Line();

	void Initialize() override;
	void Ready() override;
	void Update() override;
	void Render() override;
};

class Geometry_Box :public Geometry
{
public:
	Geometry_Box(XYZAxis axis, Shader*shader, class Shader*line, GizmoType gizmoType);
	~Geometry_Box();

	void Initialize() override;
	void Ready() override;
	void Update() override;
	void Render() override;
};