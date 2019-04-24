#include "Framework.h"
#include "ColorBuffers.h"

ColorRenderBuffer::ColorRenderBuffer(DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc)
	: format(format), sampleDesc(sample_desc)
{
	Create(1, 1, format, sample_desc, 0, 0);
}

ColorRenderBuffer::ColorRenderBuffer(UINT width, UINT height, DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc, UINT cpu_access, UINT misc_flags)
	: format(format), sampleDesc(sample_desc)
{
	Create(width, height, format, sample_desc, cpu_access, misc_flags);
}

ColorRenderBuffer::~ColorRenderBuffer()
{
	SAFE_DELETE(rtv);
}

void ColorRenderBuffer::Resize(UINT width, UINT height)
{
	Create(width, height, format, sampleDesc, TextureDesc().CPUAccessFlags, TextureDesc().MiscFlags);
}

D3D11_TEXTURE2D_DESC ColorRenderBuffer::TextureDesc()
{
	D3D11_TEXTURE2D_DESC result;
	Texture()->GetDesc(&result);
	return result;
}

ID3D11Texture2D * ColorRenderBuffer::Texture()
{
	return rtv->GetTexture();
}

ID3D11ShaderResourceView * ColorRenderBuffer::SRV()
{
	return rtv->SRV();
}

RenderTarget * ColorRenderBuffer::RTV()
{
	return rtv;
}

void ColorRenderBuffer::Create(UINT w, UINT h, DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc, UINT cpu_access, UINT misc_flags)
{
	unsigned int num_quality_levels = 0;
	D3D::GetDevice()->CheckMultisampleQualityLevels(format, sample_desc.Count, &num_quality_levels);

	if (0 == num_quality_levels)
	{
		throw 0;
	}

	rtv = new RenderTarget(w, h, format);
}
