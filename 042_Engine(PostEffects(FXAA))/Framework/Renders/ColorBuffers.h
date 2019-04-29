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
	ID3D11RenderTargetView*		RTV();
	RenderTarget*				GetRTV();

private:
	void						Create(UINT w, UINT h, DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc, UINT cpu_access, UINT misc_flags);

	RenderTarget*				rtv;

	DXGI_FORMAT					format;
	DXGI_SAMPLE_DESC			sampleDesc;
};

class RenderTargetPingPong
{
public:
	RenderTargetPingPong(ColorRenderBuffer*	source, ColorRenderBuffer*	target, ID3D11DepthStencilView* depth_stencil_view, ID3DX11EffectShaderResourceVariable* shader_resource_variable);

	void Apply(UINT tech, UINT pass, Effect * effect);
	ColorRenderBuffer*	LastTarget();

private:
	ColorRenderBuffer * m_source;
	ColorRenderBuffer*	m_target;
	ColorRenderBuffer*	m_last_target;

	ID3DX11EffectShaderResourceVariable* m_shader_resource_variable;
	ID3D11DepthStencilView* m_depth_stencil_view;

};
