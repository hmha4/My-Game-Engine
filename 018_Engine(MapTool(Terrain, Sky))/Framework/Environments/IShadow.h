#pragma once

class IShadow
{
public:
	virtual void SetShader(D3DXMATRIX v, D3DXMATRIX p) = 0;
	virtual void SetShader(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv = NULL) = 0;
	virtual void ShadowUpdate() = 0;
	virtual void NormalRender() = 0;
	virtual void ShadowRender(UINT tech, UINT pass) = 0;
};