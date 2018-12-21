#pragma once

class TerrainRenderer
{
public:
	TerrainRenderer(class Terrain * terrain);
	~TerrainRenderer();

	void Initialize();
	void Ready(Material * material);
	void Render();

public:
	static const float MaxDistance;
	static const float MinDistance;
	static const float MaxTessellation;
	static const float MinTessellation;

private:
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT row, UINT col);
	void BuildQuadPatchVB();
	void BuildQuadPatchIB();

	void CreateBlendMap();
	void SmoothBlendMap(vector<D3DXCOLOR>& colors);

private:
	D3DXMATRIX world;
	ID3D11Buffer * cBuffer;

	Terrain * terrain;

	ID3D11Buffer * quadPatchVB;
	ID3D11Buffer * quadPatchIB;


	TextureArray * layMapArray;
	ID3D11ShaderResourceView * layMapArraySRV;
	ID3D11ShaderResourceView * blendMapSRV;
	ID3D11ShaderResourceView * heightMapSRV;

	UINT patchVerticesCount;
	UINT patchQuadFacesCount;
	UINT patchVertexRows;
	UINT patchVertexCols;

	Material * material;
	vector<D3DXVECTOR2> patchBoundsY;


	UINT aabbCount;

	class Frustum* frustum;
private:
	struct Buffer
	{
		D3DXCOLOR FogColor;
		float FogStart;
		float FogRange;

		float MinDistance;
		float MaxDistance;
		float MinTessellation;
		float MaxTessellation;

		float TexelCellSpaceU;
		float TexelCellSpaceV;
		float WorldCellSpace;

		D3DXVECTOR2 TexScale = D3DXVECTOR2(66, 66);
		float Padding;

		D3DXPLANE WorldFrustumPlanes[6];

	} buffer;
};