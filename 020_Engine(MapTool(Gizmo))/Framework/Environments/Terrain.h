#pragma once
#include "Objects/IGameObject.h"

struct TerrainCP
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;
	D3DXVECTOR2 BoundsY;

	TerrainCP() {}
	TerrainCP(D3DXVECTOR3 position, D3DXVECTOR2 uv, D3DXVECTOR2 boundsY)
		: Position(position), Uv(uv), BoundsY(boundsY) {}
};

class Terrain : public IShadow, IGameObject
{
public:
	struct InitDesc;
	static const UINT CellsPerPatch;

public:
	Terrain(InitDesc& desc);
	~Terrain();

	void Initialize(class ScatterSky * sky = NULL);
	void Ready();
	void Update();
	void Render();

	void LineUpdate();

public:
	float Width();
	float Depth();

	void Data(UINT row, UINT col, float data);
	float Data(UINT row, UINT col);
	float GetHeight(float x, float z);

	bool Picking(OUT D3DXVECTOR3 *pickPixel, OUT D3DXVECTOR3 * pickWorldPos);
	

	class HeightMap* GetHeightMap() { return heightMap; }
	class TerrainRenderer*GetTerrainRender() { return render; }

	void SetMaterial(UINT index, wstring fileName) {
		materialValueFile[index] = fileName;
	}

	// ===================================================================== //
	//	Grass 관련
	// ===================================================================== //
	class Grass * GetGrass() { return grass; }

private:
	wstring terrainName;
	wstring tag;

	bool isPick;

	class TerrainRenderer * render;
	class HeightMap * heightMap;
	class Grass * grass;

	wstring loadDataFile;
	wstring materialValueFile[4]; //머터리얼속성

	MaterialProperty::ColorDesc colorDesc;
	MaterialProperty::MaterialMap materialMap;

public:
	struct InitDesc
	{
		wstring HeightMap;
		float HeightScale = 60.0f; // 높이에 대한 비율값
		float HeightMapWidth = 2049;
		float HeightMapHeight = 2049;

		wstring layerMapFile[3];
		float CellSpacing = 0.5f; // 한 셀의 크기

		D3DXVECTOR4 blendHeight = { 0.06f, 0.40f, 0.0f, 0.75f };
		D3DXVECTOR4 activeBlend = { 1,1,1,1 };

		Material* material;

		void HeightMapInit(wstring heightMap, float heightScale)
		{
			this->HeightMap = heightMap;
			this->HeightScale = heightScale;
		}

		void BlendMapInit(vector<wstring>&layerMap)
		{
			for (size_t i = 0; i < layerMap.size(); i++)
				layerMapFile[i] = layerMap[i];
		}

		void BlendMapInit(wstring layerMap, int index)
		{
			layerMapFile[index] = layerMap;
		}
	} desc;

	InitDesc & Desc() { return desc; }

public:
	struct TerrainDetail
	{
		float DetailValue = 0.999f;
		float DetailIntensity = 1;
	} detail;

	struct TerrainBrush
	{
		int Type = 0;
		int Range = 10;
		D3DXVECTOR3 Color = { 1,0,0 };
		float BrushPower = 1;
	} brush;

	struct TerrainLine
	{
		int LineType = 0;
		D3DXVECTOR3 LineColor = { 1,1,1 };
		int Spacing = 5; // 한칸의 넓이
		float Thickness = 0.1f;
	} tline;

public:
	// IShadow을(를) 통해 상속됨
	virtual void SetShader(D3DXMATRIX v, D3DXMATRIX p) override;
	virtual void SetShader(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv) override;
	virtual void ShadowUpdate() override;
	virtual void NormalRender() override;
	virtual void ShadowRender(UINT tech, UINT pass) override;

	// IGameObject을(를) 통해 상속됨
	virtual wstring & Name() override;
	virtual wstring & Tag() override;
	virtual void Load(wstring fileName) override;
	virtual void Save() override;
	virtual void Delete() override;

	// IGameObject을(를) 통해 상속됨
	bool IsPicked();
	void SetPickState(bool val);
	void Position(float x, float y, float z) override {}
	void Position(D3DXVECTOR3 & vec) override {}
	void Position(D3DXVECTOR3* vec) override {}
	void Rotation(float x, float y, float z) override {}
	void Rotation(D3DXVECTOR3 & vec) override {}
	void Rotation(D3DXVECTOR3* vec) override {}
	void RotationDegree(float x, float y, float z) override {}
	void RotationDegree(D3DXVECTOR3 & vec) override {}
	void RotationDegree(D3DXVECTOR3* vec) override {}
	void Scale(float x, float y, float z) override {}
	void Scale(D3DXVECTOR3 & vec) override {}
	void Scale(D3DXVECTOR3* vec) override {}
	void Matrix(D3DXMATRIX* mat) override {}
	void Forward(D3DXVECTOR3* vec) override {}
	void Up(D3DXVECTOR3* vec) override {}
	void Right(D3DXVECTOR3* vec) override {}
};