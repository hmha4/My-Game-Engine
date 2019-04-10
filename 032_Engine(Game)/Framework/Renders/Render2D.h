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

private:
	void UpdateWorld();

private:
	Effect * effect;

	ID3D11Buffer*vertexBuffer;

	D3DXMATRIX view;
	D3DXMATRIX orthographic;

	D3DXVECTOR2 position;
	D3DXVECTOR2 scale;

private:
	ID3DX11EffectMatrixVariable * worldVar;
	ID3DX11EffectShaderResourceVariable * mapVariable;
};