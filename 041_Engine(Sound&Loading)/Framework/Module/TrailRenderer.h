#pragma once
#include "Interfaces/IAlpha.h"

class TrailRenderer : public IAlpha
{
public:
	TrailRenderer(class GameModel * model, Effect * effect);
	~TrailRenderer();

	void Ready();
	void Update();
	void PreRender();
	void Render() override;
	void ImGuiRender(UINT index);
	void PostRender();
	void ResizeScreen();

	void UseDissolve(bool val) { useDissolve = val; }
	void BoneIndex(int val) 
	{ 
		boneIndex = val; 
		bone = model->BoneByIndex(boneIndex);
	}
	int BoneIndex() { return boneIndex; }
	void LifeTime(float val) { lifeTime = val; }
	float LifeTime() { return lifeTime; }

	void Run(bool val) { 
		run = val; 
		if (run == false)
			ResetTrail();
	}
	void ResetTrail();

	void DiffuseMap(wstring diffuseMap) { SAFE_DELETE(diffuse); diffuse = new Texture(Textures + diffuseMap); }
	wstring DiffuseMap() { return Path::GetFileName(diffuse->GetFile()); }
	void AlphaMap(wstring alphaMap) { SAFE_DELETE(alpha); alpha = new Texture(Textures + alphaMap); }
	wstring AlphaMap() { return Path::GetFileName(alpha->GetFile()); }

	void StartMat(D3DXMATRIX& val) { startPos = val; }
	D3DXMATRIX StartMat() { return startPos; }
	void EndMat(D3DXMATRIX& val) { endPos = val; }
	D3DXMATRIX EndMat() { return endPos; }

private:
	void MakeRomLines();
	void CreateTexture();
	void CreateBuffer();
	void LoadTexture(wstring file, UINT type);

private:
	struct MyVertex
	{
		D3DXVECTOR3	Position = D3DXVECTOR3(0, 0, 0);
		float Time = 0;
	};

	bool		run;
	bool		useDissolve;
	float		count;
	float		lifeTime;
	float		sliceAmount;

	Effect		*effect;
	Effect		*lineEffect;

	Texture		*diffuse;
	Texture		*alpha;
	Texture		*dissolveMap;

	//	모델이랑 본 관련
	int					boneIndex;
	D3DXMATRIX			boneMatrix;
	GameModel			*gameModel;
	class Model* model;
	class ModelBone * bone;

	//	startPos : 무기 끝 지점
	//	endPos : 손잡이 쪽
	D3DXMATRIX			startPos;
	D3DXMATRIX			endPos;

	//	CatMullRom 해서 보간한 좌표들
	vector<D3DXVECTOR3>	start;
	vector<D3DXVECTOR3>	end;

	//	보간 하기전 좌표들
	vector<MyVertex>	linePoints[2];

	VertexTextureNormal	*vertices;
	UINT				*indices;
	ID3D11Buffer		*vertexBuffer, *indexBuffer;
	UINT				indexCount, vertexCount;

	DebugLine * debugLine[2];
private:
	ID3DX11EffectShaderResourceVariable * trailMapVar;
	ID3DX11EffectShaderResourceVariable * alphaMapVar;
	ID3DX11EffectScalarVariable *sliceAmountVar;
};