#pragma once

struct TerrainCP
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;
	D3DXVECTOR2 BoundsY;

	TerrainCP() {}
	TerrainCP(D3DXVECTOR3 position, D3DXVECTOR2 uv, D3DXVECTOR2 boundsY)
		: Position(position), Uv(uv), BoundsY(boundsY) {}
};

class Terrain : public IShadow
{
public:
	struct InitDesc;

public:
	Terrain(InitDesc& desc);
	~Terrain();

	void Initialize(Material* material = NULL);

	void Ready(Material*material);

	void Load(wstring terrainData);

	void Update();
	void Render();
	void LineUpdate();

	void Data(UINT row, UINT col, float data);
	float Data(UINT row, UINT col);

	float Width();
	float Depth();

	float GetHeight(float x, float z);
	bool Picking(OUT D3DXVECTOR3 *pickPixel, OUT D3DXVECTOR3 * pickWorldPos);

	class HeightMap* GetHeightMap() { return heightMap; }
	class TerrainRenderer*GetTerrainRender() { return render; }

	wstring&Name() { return terrainName; }

	bool IsPicked();
	void SetPickState(bool val);

	void SaveTerrain();
	void UseLoad(wstring loadFile)
	{
		loadDataFile = loadFile;
		isLoad = true;
	}
public:
	static const UINT CellsPerPatch;

private:
	class TerrainRenderer* render;
	class HeightMap* heightMap;

	wstring loadDataFile;
	bool isLoad;

	wstring materialValueFile[4]; //머터리얼속성

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
	};
private:
	InitDesc desc;
	wstring terrainName;
	bool isPick;

public:
	struct TerrainDetail
	{
		float DetailValue = 0.999f;
		float DetailIntensity = 1;
	}detail;

	struct TerrainBrush
	{
		int Type = 0;
		int Range = 10;
		D3DXVECTOR3 Color = { 1,0,0 };
		float BrushPower = 1;
	}brush;

	struct TerrainLine
	{
		int LineType = 0;
		D3DXVECTOR3 LineColor = { 1,1,1 };
		int Spacing = 5; // 한칸의 넓이
		float Thickness = 0.1f;
	}tline;

public:
	InitDesc & Desc() { return desc; }

	// IShadow을(를) 통해 상속됨
	virtual void SetShader(D3DXMATRIX v, D3DXMATRIX p) override;
	virtual void SetShader(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv) override;
	virtual void ShadowUpdate() override;
	virtual void NormalRender() override;
	virtual void ShadowRender(UINT tech, UINT pass) override;
};