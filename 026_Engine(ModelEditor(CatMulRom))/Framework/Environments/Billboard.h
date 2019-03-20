#pragma once

#include "IShadow.h"
#include "Collider/ICollider.h"

class Billboard : public IShadow
{
public:
	enum class BillboardType
	{
		ONE,	//	한개사용
		TWO = 1,	//	두개사용
		FOUR = 3
	} bType;

	enum TextureType
	{
		Tree1 = 0,
		Tree2 = 1,
		Tree3 = 2,
		Tree4 = 3
	};

private:
	struct LoadDesc;

public:
	Billboard(class Terrain*terrain);
	~Billboard();

	void Initalize();
	void Ready();
	void Render();
	void Update();
	void ImGuiRender();

	void BuildBillboard(D3DXVECTOR3 pos);
	void RemoveBillboard();
	void ResourceUpdate(D3DXVECTOR2 setScale); // 스케일이나 텍스쳐바꿀때사용

	UINT Count() { return BillboardCount; }
	void CreateVector(UINT count);
	vector<LoadDesc>&GetDesc() { return loadDatas; }

	//=======================인터페이스=================//
	wstring&Name() { return billboardName; }
	bool IsPicked() { return isPicked; }
	void SetPickState(bool val) { isPicked = val; }
	class GData* Save() { return NULL; }
	void Load(wstring fileName) {}

	void SetEffect(D3DXMATRIX v, D3DXMATRIX p) override;
	void SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv) override;
	void ShadowUpdate() override;
	void NormalRender() override;
	void ShadowRender(UINT tech, UINT pass) override;
	//=================================================//

private:
	void LoadBillboardMap(wstring fileName);
	void LoadData();

private:
	struct InstanceBillboard
	{
		D3DXVECTOR3 TransPosition;
		D3DXMATRIX DeTransWorld;
		D3DXMATRIX RotateWorld;
		D3DXVECTOR2 Size;
		UINT TextureNumber;
		UINT Type;
	};

	struct LoadDesc
	{
		D3DXVECTOR3 TransPosition;
		D3DXVECTOR2 Size;
		UINT TextureNumber;
		UINT Type;
	};
	vector<LoadDesc> loadDatas;
	bool isLoad;
private:
	//임구이 대표설정값들
	UINT setTextureNumber;
	D3DXVECTOR2 setSize = { 10,10 };

	vector<Texture*> textures;

	TextureType treeType = (TextureType)0;

private:
	vector<InstanceBillboard> vertices;

	class Frustum* frustum;

	Vertex vertex;
	ID3D11Buffer*vertexBuffer[2];

	UINT BillboardCount;

	Material* material;
	TextureArray*billboardTexture;
	vector<wstring> textureNames;

	class Terrain*terrain;
	wstring billboardName;
	bool isPicked;

	Effect * lineEffect;
	bool showLine;
	struct BCollider
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 scale;
		class ColliderBox*Box;
	};
	vector<BCollider> colliders;

private:
	ID3DX11EffectVectorVariable * frustumVar;
	ID3DX11EffectMatrixVariable * lightViewVar;
	ID3DX11EffectMatrixVariable * lightProjVar;
	ID3DX11EffectMatrixVariable * shadowTransformVar;
	ID3DX11EffectShaderResourceVariable * shadowMapVar;

private:
	D3DXPLANE plane[6];
};
