#pragma once

class TerrainRenderer
{
private:
	struct TerrainSRT
	{
		D3DXVECTOR3 scale;
		D3DXVECTOR3 rotate;
		D3DXVECTOR3 trans;
	};
	TerrainSRT terrainSrt;

public:
	static const float MaxDistance;
	static const float MinDistance;
	static const float MaxTessellation;
	static const float MinTessellation;

public:
	TerrainRenderer(class Terrain* terrain);
	~TerrainRenderer();

	void Initialize();
	void Ready(Material*material);
	void ChangeImageMap(Material*material, wstring fileName, int type);

	void AdjustY(Effect*effect, ID3D11ShaderResourceView*srv, ID3D11UnorderedAccessView*uav);
	void BlendSet();

	void Render();
	void RenderShadow(UINT tech, UINT pass);
	void RenderReflection(D3DXMATRIX reflectionView);

	TerrainSRT&GetSRT() { return terrainSrt; }
	ID3D11ShaderResourceView*GetLayerView() { return layerMapArray->GetView(); }
	void GetSky(class ScatterSky * sky) { this->sky = sky; }

private:
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT row, UINT col);
	void BuildQuadPatchVB();
	void BuildQuadPatchIB();

	void CreateBlendMap();
	void SmoothBlendMap(vector<D3DXCOLOR>& colors);

private:
	D3DXMATRIX world;
	ID3D11Buffer* cBuffer;

	class ScatterSky * sky;
	class Terrain* terrain;

	ID3D11Buffer* quadPatchVB;
	ID3D11Buffer* quadPatchIB;
	struct TerrainCP* patchVertices;

	TextureArray* layerMapArray;

	ID3D11ShaderResourceView* layerMapArraySRV;
	ID3D11ShaderResourceView* blendMapSRV;
	ID3D11ShaderResourceView* heightMapSRV;

	UINT patchVerticesCount;
	UINT patchQuadFacesCount;
	UINT patchVertexRows;
	UINT patchVertexCols;

	Material* material;
	vector<D3DXVECTOR2> patchBoundsY;

	class Frustum* frustum;
	int bWireFrame = 0;

private:
	ID3DX11EffectShaderResourceVariable * heightMapVar;
	ID3DX11EffectShaderResourceVariable * layerMapVar;
	ID3DX11EffectShaderResourceVariable * blendMapVar;
	ID3DX11EffectVectorVariable * blendPosVar;
	ID3DX11EffectVectorVariable * activeBlendVar;
	ID3DX11EffectScalarVariable * maxHeightVar;
	ID3DX11EffectScalarVariable * maxDetailIntensityVar;
	ID3DX11EffectScalarVariable * maxDetailValurVar;
	ID3DX11EffectConstantBuffer * cbTerrainVar;

private:
	RenderTargetView * renderTargetView;

	ID3D11RenderTargetView* rtvs[2];
	ID3D11DepthStencilView* dsv;
public:
	ID3D11Texture2D * GetRenderTargetTexture() { return renderTargetView->Texture(); }
	ID3D11ShaderResourceView*GetHeightMapSRV() { return heightMapSRV; }
	int&WireFrameMode() { return bWireFrame; }
	ID3D11Buffer*GetVertexBuffer() { return quadPatchVB; }
	struct TerrainCP*GetVertices() { return patchVertices; }
	int GetVertexCount() { return patchVerticesCount; }

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

		int TexScale = 32;
		float Padding[2];

		D3DXPLANE WorldFrustumPlanes[6];
	} buffer;

	
public:
	Buffer & GetBuffer() { return buffer; }
};
