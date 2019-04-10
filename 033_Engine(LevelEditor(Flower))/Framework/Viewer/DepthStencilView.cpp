#include "Framework.h"
#include "DepthStencilView.h"

DepthStencilView::DepthStencilView(UINT width, UINT height, DXGI_FORMAT format)
	: width(width), height(height), format(format)
{
	//Create Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;

		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		if (format == DXGI_FORMAT_R24G8_TYPELESS)
			desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture);
		assert(SUCCEEDED(hr));
	}

	//Create DSV
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		HRESULT hr = D3D::GetDevice()->CreateDepthStencilView(texture, &desc, &dsv);
		assert(SUCCEEDED(hr));
	}
}

DepthStencilView::~DepthStencilView()
{
	SAFE_RELEASE(dsv);
	SAFE_RELEASE(texture);
}