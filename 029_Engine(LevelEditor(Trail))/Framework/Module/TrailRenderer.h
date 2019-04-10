#pragma once


class TrailRenderer
{
public:
	TrailRenderer(class GameModel * model, Effect * effect);
	~TrailRenderer();

	void Ready();
	void Update();
	void PreRender();
	void Render();
	void ImGuiRender(UINT index);
	void PostRender();
	void ResizeScreen();

	void SetSlice(bool val) { doSlice = val; }
	void SetBone(int val) { boneIndex = val; }
	int GetBone() { return boneIndex; }
	void SetLifeTime(float val) { lifeTime = val; }
	float GetLifeTime() { return lifeTime; }

	void SetRun(bool val) { 
		run = val; 
		if (run == false)
			ResetTrail();
	}
	void ResetTrail();

	void SetDiffuseMap(wstring diffuseMap) { SAFE_DELETE(diffuse); diffuse = new Texture(Textures + diffuseMap); }
	wstring GetDiffuseMap() { return Path::GetFileName(diffuse->GetFile()); }
	void SetAlphaMap(wstring alphaMap) { SAFE_DELETE(alpha); alpha = new Texture(Textures + alphaMap); }
	wstring GetAlphaMap() { return Path::GetFileName(alpha->GetFile()); }

	void SetStartMat(D3DXMATRIX& val) { startPos = val; }
	D3DXMATRIX GetStartMat() { return startPos; }
	void SetEndMat(D3DXMATRIX& val) { endPos = val; }
	D3DXMATRIX GetEndMat() { return endPos; }

private:
	void MakeRomLines();
	void CreateTexture();

	void CreateBuffer();

private:
	void LoadTexture(wstring file, UINT type);

private:
	struct MyVertex
	{
		D3DXVECTOR3	Position = D3DXVECTOR3(0, 0, 0);
		float Time = 0;
	};

	bool		run;
	bool		doSlice;
	float		count;
	float		lifeTime;
	float		sliceAmount;

	Effect		*effect;
	Effect		*lineEffect;

	Texture		*diffuse;
	Texture		*alpha;
	Texture		*sliceMap;

	//	모델이랑 본 관련
	int					boneIndex;
	D3DXMATRIX			boneMatrix;
	GameModel			*gameModel;
	class Model* model;

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