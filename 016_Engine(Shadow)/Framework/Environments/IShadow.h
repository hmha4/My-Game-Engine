#pragma once

class IShadow
{
public:
	virtual void SetShader(Shader * shader, D3DXMATRIX v, D3DXMATRIX p, D3DXMATRIX s, ID3D11ShaderResourceView * srv) = 0;
	virtual void ShadowUpdate() = 0;
	virtual void ShadowRender() = 0;
};