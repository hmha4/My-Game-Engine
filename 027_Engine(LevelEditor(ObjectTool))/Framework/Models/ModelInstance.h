#pragma once

class ModelInstance
{
public:
	ModelInstance(class Model*model,wstring shaderFile);
	ModelInstance(class Model*model, Effect*effect);
	~ModelInstance();

	void Ready();

	void AddWorld(D3DXMATRIX&world);
	void DeleteWorld(UINT instanceID);

	void Update();
	void Render(UINT tech = 0);

	void UpdateWorld(UINT instanceNumber, D3DXMATRIX&world);
	class Model*GetModel();
private:


private:
	class Model*model;
	
	UINT maxCount;
	D3DXMATRIX transforms[64][128];

	ID3D11Texture2D*transTexture;
	ID3D11ShaderResourceView*transSrv;
	
	D3DXMATRIX worlds[64];

private:
	vector<ID3DX11EffectVectorVariable*> frustumVars;
	class Frustum*frustum;
	D3DXPLANE plane[6];
};
