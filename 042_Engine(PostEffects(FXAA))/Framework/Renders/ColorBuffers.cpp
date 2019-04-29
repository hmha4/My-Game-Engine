#include "Framework.h"
#include "ColorBuffers.h"

ColorRenderBuffer::ColorRenderBuffer(DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc)
	: format(format), sampleDesc(sample_desc)
	, rtv(NULL)
{
	Create(1, 1, format, sample_desc, 0, 0);
}

ColorRenderBuffer::ColorRenderBuffer(UINT width, UINT height, DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc, UINT cpu_access, UINT misc_flags)
	: format(format), sampleDesc(sample_desc)
	, rtv(NULL)
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

ID3D11RenderTargetView * ColorRenderBuffer::RTV()
{
	if (rtv == NULL)
		return NULL;

	return rtv->RTV();
}

RenderTarget * ColorRenderBuffer::GetRTV()
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


RenderTargetPingPong::RenderTargetPingPong(ColorRenderBuffer*	source, ColorRenderBuffer*	target, ID3D11DepthStencilView* depth_stencil_view, ID3DX11EffectShaderResourceVariable* shader_resource_variable) :
	m_shader_resource_variable(shader_resource_variable),
	m_depth_stencil_view(depth_stencil_view),
	m_source(source),
	m_target(target),
	m_last_target(source)
{
}

void RenderTargetPingPong::Apply(UINT tech, UINT pass, Effect * effect)
{
	ID3D11Buffer* zero = 0;
	UINT nought = 0;
	D3D::GetDC()->IASetVertexBuffers(0, 1, &zero, &nought, &nought);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	D3D::GetDC()->IASetInputLayout(0);

	{
		ID3D11RenderTargetView* view[] = { m_target->RTV() };
		D3D::GetDC()->OMSetRenderTargets(1, const_cast<ID3D11RenderTargetView**> (view), m_depth_stencil_view);
	}

	m_shader_resource_variable->SetResource(m_source->SRV());

	effect->Draw(tech, pass, 4, 0);

	m_shader_resource_variable->SetResource(0);
	m_last_target = m_target;
	std::swap(m_source, m_target);
}

ColorRenderBuffer*	RenderTargetPingPong::LastTarget()
{
	return m_last_target;
}