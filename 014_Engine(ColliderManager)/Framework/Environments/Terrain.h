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

class Terrain
{
public:
	struct InitDesc;

public:
	Terrain(InitDesc& desc);
	~Terrain();

	void Initialize();
	void Ready(Material * material);
	void Update();
	void Render();

	void Data(UINT row, UINT col, float data);
	float Data(UINT row, UINT col);

	float Width();
	float Depth();

	class HeightMap *  GetHeightMap() { return heightMap; }

private:

public:
	static const UINT CellsPerPatch;

private:
	class HeightMap * heightMap;
	class TerrainRenderer * renderer;

public:
	struct InitDesc
	{
		wstring HeightMap;
		float	HeightScale;
		float	HeightMapWidth;
		float	HeightMapHeight;

		wstring layerMapFile[5];
		wstring blendMapFile;

		float CellSpacing;

		Material* material;
	};

private:
	InitDesc desc;

public:
	InitDesc & Desc() { return desc; }
};