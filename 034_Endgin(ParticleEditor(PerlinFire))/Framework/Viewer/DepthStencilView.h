#pragma once

class DepthStencilView
{
public:
	DepthStencilView(UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT);
	~DepthStencilView();

	ID3D11Texture2D* Texture() { return texture; }
	ID3D11DepthStencilView* DSV() { return dsv; }

private:
	UINT width, height;
	DXGI_FORMAT format;

	ID3D11Texture2D* texture;
	ID3D11DepthStencilView* dsv;
};