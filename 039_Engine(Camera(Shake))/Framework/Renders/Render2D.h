#pragma once

class Render2D
{
public:
	Render2D(wstring effectFile = L"");
	virtual ~Render2D();

	void Position(float x, float y);
	void Position(D3DXVECTOR2 vec);

	void Scale(float x, float y);
	void Scale(D3DXVECTOR2 vec);

	D3DXVECTOR2 Scale() { return scale; }

	void Update();
	virtual void Render();

	void SRV(ID3D11ShaderResourceView*srv);
	void SRVSnd(ID3D11ShaderResourceView * srv);

	Effect * GetEffect() { return effect; }
	void SetUV(wstring uv, float val, float max);
	void SetColor(float a, float g, float b);
private:
	void UpdateWorld();

private:
	Effect * effect;

	ID3D11Buffer*vertexBuffer;

	D3DXMATRIX view;
	D3DXMATRIX orthographic;

	D3DXVECTOR2 position;
	D3DXVECTOR2 scale;

	bool sndTexture;
private:
	ID3DX11EffectMatrixVariable * worldVar;
	ID3DX11EffectShaderResourceVariable * mapVariable;
	ID3DX11EffectScalarVariable * timeVar;
};