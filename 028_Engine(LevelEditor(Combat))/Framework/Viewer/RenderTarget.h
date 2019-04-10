#pragma once

class RenderTarget
{
public:
	//DXGI_FORMAT_R8G8B8A8_UNORM
	RenderTarget(UINT width = 0, UINT height = 0, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	~RenderTarget();

	void Set(D3DXCOLOR clear = 0xFF000000);

	ID3D11ShaderResourceView* SRV() { return srv; }
	void SaveTexture(wstring saveFile);

	ID3D11RenderTargetView* RTV() { return rtv; }
	ID3D11Texture2D*GetTexture() { return rtvTexture; }
	ID3D11DepthStencilView* DSV() { return dsv; }

private:
	UINT width, height;
	DXGI_FORMAT format;

	ID3D11ShaderResourceView* srv;
	ID3D11Texture2D* rtvTexture;
	ID3D11RenderTargetView* rtv;

	ID3D11Texture2D* dsvTexture;
	ID3D11DepthStencilView* dsv;

	class Viewport* viewport;
};
