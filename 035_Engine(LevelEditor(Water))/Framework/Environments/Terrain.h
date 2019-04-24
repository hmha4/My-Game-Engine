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
	void RenderReflection(D3DXMATRIX reflectionView);

	void LineUpdate();
	void BrushUpdate(D3DXVECTOR3 position);
public:
	float Width();
	float Depth();

	void SaveHeightMap(Effect * effect, ID3D11Buffer ** buffer, ID3D11UnorderedAccessView ** uav);
	void Data(UINT row, UINT col, float data);
	float Data(UINT row, UINT col);
	float GetHeight(float x, float z);

	bool Picking(OUT D3DXVECTOR3 *pickPixel, OUT D3DXVECTOR3 * pickWorldPos);

	class HeightMap* GetHeightMap() { return heightMap; }
	class TerrainRenderer*GetTerrainRender() { return render; }

	void SetMaterial(UINT index, wstring fileName) {
		materialValueFile[index] = fileName;
	}

	void SetShadows(class Shadow * shadow);

	// ===================================================================== //
	//	Grass 관련
	// ===================================================================== //
	class Grass * GetGrass() { return grass; }
	class Flower * GetFlower() { return flower; }

	// ===================================================================== //
	//	Billboard 관련
	// ===================================================================== //
	class Billboard*GetBillboard() { return billboard; }

private:
	wstring fileName;
	wstring terrainName;
	wstring tag;

	bool isPick;

	class TerrainRenderer * render;
	class HeightMap * heightMap;
	class Grass * grass;
	class Flower * flower;
	class Billboard * billboard;

	wstring loadDataFile;
	wstring materialValueFile[4]; //머터리얼속성

private:
	ID3DX11EffectScalarVariable * brushTypeVar;
	ID3DX11EffectVectorVariable * brushLocationVar;
	ID3DX11EffectVectorVariable * brushColorVar;
	ID3DX11EffectScalarVariable * brushRangeVar;

	ID3DX11EffectScalarVariable * lineVar;
	ID3DX11EffectVectorVariable * lineColorVar;
	ID3DX11EffectScalarVariable * spacingVar;
	ID3DX11EffectScalarVariable * thicknessVar;

	ID3DX11EffectMatrixVariable * lightViewVar;
	ID3DX11EffectMatrixVariable * lightProjVar;
	ID3DX11EffectMatrixVariable * shadowTransformVar;
	ID3DX11EffectShaderResourceVariable * shadowMapVar;

public:
	struct InitDesc
	{
		wstring HeightMap;
		float HeightScale = 50.0f; // 높이에 대한 비율값
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
	virtual void SetEffect(D3DXMATRIX v, D3DXMATRIX p) override;
	virtual void SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv) override;
	virtual void ShadowUpdate() override;
	virtual void NormalRender() override;
	virtual void ShadowRender(UINT tech, UINT pass) override;

	// IGameObject을(를) 통해 상속됨
	virtual wstring & FileName() override { return fileName; }
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

	// IGameObject을(를) 통해 상속됨
	virtual Model * GetModel() override { return NULL; }
	virtual void Clone(void ** clone, wstring fileName) override {}
};