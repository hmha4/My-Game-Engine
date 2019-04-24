#pragma once

class ColorRenderBuffer
{
public:
	ColorRenderBuffer() {}
	ColorRenderBuffer(DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc);
	ColorRenderBuffer(UINT width, UINT height, DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc, UINT  cpu_access = 0, UINT  misc_flags = 0);
	~ColorRenderBuffer();

	void	Resize(UINT width, UINT height);

	D3D11_TEXTURE2D_DESC TextureDesc();
	ID3D11Texture2D*			Texture();
	ID3D11ShaderResourceView*	SRV();
	RenderTarget*		RTV();

private:
	void						Create(UINT w, UINT h, DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc, UINT cpu_access, UINT misc_flags);

	RenderTarget*				rtv;

	DXGI_FORMAT					format;
	DXGI_SAMPLE_DESC			sampleDesc;
};