#include "Framework.h"
#include "LightningRenderer.h"

LightningRenderer::LightningRenderer()
	: m_subdivide_buffer0(NULL), m_subdivide_buffer1(NULL)
	, gradientTexture(NULL), gradientSRV(NULL)
	, maxVertexCount(0)
	, m_num_scissor_rects(0), m_num_viewports(0)
	, m_down_sample_buffer_sizes((int)Devels::DecimationLevels)
	, m_down_sample_buffers((int)Devels::DecimationLevels - 1)
{
}

LightningRenderer::~LightningRenderer()
{
	SAFE_DELETE(effect);

	while (!m_down_sample_buffers.empty())
	{
		SAFE_DELETE(m_down_sample_buffers.back());
		m_down_sample_buffers.pop_back();
	}

	while (lightningSeeds.empty() == false)
		DestroyLightning(*lightningSeeds.begin());

	SAFE_RELEASE(gradientSRV);
	SAFE_RELEASE(gradientTexture);

	SAFE_DELETE(m_original_lightning_buffer);
	SAFE_DELETE(m_lightning_buffer0);
	SAFE_DELETE(m_lightning_buffer1);
	SAFE_DELETE(m_small_lightning_buffer0);
	SAFE_DELETE(m_small_lightning_buffer1);

	SAFE_DELETE(m_subdivide_buffer0);
	SAFE_DELETE(m_subdivide_buffer1);
}

void LightningRenderer::Initialize()
{
	
}

void LightningRenderer::Ready()
{
	effect = new Effect(Effects + L"038_Lightning.fx");
	if(DEFERRED == true)
		effect->AsSRV("ScreenDepth")->SetResource(DeferredRendering::Get()->DepthSRV());

	lightningAppVar = effect->AsConstantBuffer("LightningAppearance");
	lightningStrVar = effect->AsConstantBuffer("LightningStructure");

	worldVar = effect->AsMatrix("World");

	forkVar = effect->AsScalar("Fork");
	subDivisionLvVar = effect->AsScalar("SubdivisionLevel");

	animationSpeedVar = effect->AsScalar("AnimationSpeed");
	chargeVar = effect->AsScalar("Charge");

	bufferVar = effect->AsSRV("buffer");
	gradientVar = effect->AsSRV("gradient");
	bufferTexelSizeVar = effect->AsVector("BufferTexelSize");

	blurSigmaVar = effect->AsVector("BlurSigma");


	D3DDesc desc;
	D3D::GetDesc(&desc);
	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;
	m_lightning_buffer0 = new ColorRenderBuffer(DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc);
	m_lightning_buffer1 = new ColorRenderBuffer(DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc);
	m_small_lightning_buffer0 = new ColorRenderBuffer(DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc);
	m_small_lightning_buffer1 = new ColorRenderBuffer(DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc);
	m_original_lightning_buffer = new ColorRenderBuffer(DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc);
	BuildDownSampleBuffers(desc.Width, desc.Height, sampleDesc);
	D3DXVECTOR2 texelSize = D3DXVECTOR2(1.0f / desc.Width, 1.0f / desc.Height);
	bufferTexelSizeVar->SetFloatVector(texelSize);

	BuildGradientTexture();
	gradientVar->SetResource(gradientSRV);
}

PathLightning * LightningRenderer::CreatePathLightning(const vector<LightningPathSegment>& segments, int partternMask, UINT subDivisions)
{
	PathLightning * result = new PathLightning(effect, segments, partternMask, subDivisions);
	AddLightningSeed(result);

	return result;
}

ChainLightning * LightningRenderer::CreateChainLightning(int pattern_mask, UINT subDivisions)
{
	ChainLightning * result = new ChainLightning(effect, pattern_mask, subDivisions);
	AddLightningSeed(result);

	return result;
}

void LightningRenderer::DestroyLightning(LightningSeed * seed)
{
	RemoveLightningSeed(seed);
	SAFE_DELETE(seed);
}

void LightningRenderer::PreRender()
{
	m_lightning_buffer0->GetRTV()->Set();
	BuildSubdivisionBuffers();
}

void LightningRenderer::Render(LightningSeed * seed)
{
	if (seed->Settings.InUse == false)
	{
		seed->Settings.Beam.BoltWidth.x -= 0.2f * Time::Delta();

		if (seed->Settings.Beam.BoltWidth.x <= 0)
		{
			seed->Settings.Beam.BoltWidth.x = 0;
			return;
		}
	}
	else
	{
		if (seed->Settings.Beam.BoltWidth.x <= seed->localWidth)
			seed->Settings.Beam.BoltWidth.x += 0.5f * Time::Delta();
		else
			seed->Settings.Beam.BoltWidth.x = seed->localWidth;
	}

	{
		UINT offset[1] = { 0 };
		ID3D11Buffer* zero[1] = { 0 };
		D3D::GetDC()->SOSetTargets(1, zero, offset);
	}

	chargeVar->SetFloat(1.0f);
	animationSpeedVar->SetFloat(seed->Settings.AnimationSpeed);


	HRESULT hr = lightningAppVar->SetRawValue(const_cast<LightningAppearance*>(&seed->Settings.Beam), 0, sizeof(LightningAppearance));
	assert(SUCCEEDED(hr));

	seed->SetConstants();
	
	SimpleVertexBuffer * subdivided = SubDivide(seed);

	subdivided->BindToIA();
	subDivisionLvVar->SetInt(0);

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	worldVar->SetMatrix(seed->world);
	effect->Draw(3, 0, seed->GetNumVertices(seed->subDivisions), 0);
}

void LightningRenderer::Render(LightningSeed * seed, const LightningAppearance & appearance, float charge, float animation_speed)
{
	{
		UINT offset[1] = { 0 };
		ID3D11Buffer* zero[1] = { 0 };
		D3D::GetDC()->SOSetTargets(1, zero, offset);
	}

	chargeVar->SetFloat(charge);
	animationSpeedVar->SetFloat(animation_speed);

	HRESULT hr = lightningAppVar->SetRawValue(const_cast<LightningAppearance*>(&appearance), 0, sizeof(LightningAppearance));
	assert(SUCCEEDED(hr));

	seed->SetConstants();

	SimpleVertexBuffer * subdivided = SubDivide(seed);

	subdivided->BindToIA();
	subDivisionLvVar->SetInt(0);

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	effect->Draw(3, 0, seed->GetNumVertices(seed->subDivisions), 0);
}

void LightningRenderer::PostRender(bool glow, D3DXVECTOR3 blurSigma)
{
	D3D::GetDC()->ResolveSubresource(m_original_lightning_buffer->Texture(), 0, m_lightning_buffer0->Texture(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	if (glow)
	{
		blurSigmaVar->SetFloatVector(blurSigma);
		SaveViewports();

		DownSample(m_original_lightning_buffer);
		ResizeViewport(m_down_sample_buffer_sizes.back().cx, m_down_sample_buffer_sizes.back().cy);
		
			RenderTargetPingPong ping_pong(m_small_lightning_buffer0, m_small_lightning_buffer1, 0, bufferVar);

			ping_pong.Apply(7, 0, effect);
			ping_pong.Apply(8, 0, effect);
		
		RestoreViewports();

		D3D::Get()->SetRenderTarget();
		bufferVar->SetResource(ping_pong.LastTarget()->SRV());
		DrawQuad(5, 0);
	}
	else
	{
		D3D::Get()->SetRenderTarget();
	}

	bufferVar->SetResource(m_original_lightning_buffer->SRV());
	DrawQuad(5, 0);
}

void LightningRenderer::BuildSubdivisionBuffers()
{
	UINT max_segments = 0;

	for (std::set<LightningSeed*>::iterator it = lightningSeeds.begin(); it != lightningSeeds.end(); ++it)
	{
		max_segments = max(max_segments, (*it)->GetMaxNumVertices());
	}

	if ((m_subdivide_buffer0 != NULL) && (max_segments == m_subdivide_buffer0->VertexCount()))
		return;

	delete m_subdivide_buffer0;
	delete m_subdivide_buffer1;


	vector<SubdivideVertex> init_data(max_segments, SubdivideVertex());

	D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
	UINT flags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	m_subdivide_buffer0 = new SimpleVertexBuffer(init_data, usage, flags);
	m_subdivide_buffer1 = new SimpleVertexBuffer(init_data, usage, flags);
}

void LightningRenderer::SaveViewports()
{
	D3D::GetDC()->RSGetViewports(&m_num_viewports, 0);
	D3D::GetDC()->RSGetViewports(&m_num_viewports, m_viewports);

	D3D::GetDC()->RSGetScissorRects(&m_num_scissor_rects, 0);

	if (0 != m_num_scissor_rects)
		D3D::GetDC()->RSGetScissorRects(&m_num_scissor_rects, m_scissor_rects);

}

void LightningRenderer::ResizeViewport(UINT width, UINT height)
{
	D3D11_VIEWPORT viewport = { 0, 0, width, height, 0.0f, 1.0f };
	D3D11_RECT	   scissor_rect = { 0, 0, width, height };

	D3D::GetDC()->RSSetViewports(1, &viewport);
	D3D::GetDC()->RSSetScissorRects(1, &scissor_rect);
}

void LightningRenderer::RestoreViewports()
{
	D3D::GetDC()->RSSetViewports(m_num_viewports, m_viewports);
	D3D::GetDC()->RSSetScissorRects(m_num_scissor_rects, m_scissor_rects);
}


SimpleVertexBuffer * LightningRenderer::SubDivide(LightningSeed * seed)
{
	SimpleVertexBuffer * source = m_subdivide_buffer0;
	SimpleVertexBuffer * target = m_subdivide_buffer1;
	SimpleVertexBuffer * last_target = target;

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	target->BindToSO();

	subDivisionLvVar->SetInt(0);
	forkVar->SetBool(seed->patternMask & (1 << 0));
	seed->Render();
	{
		UINT offset[1] = { 0 };
		ID3D11Buffer* zero[1] = { 0 };
		D3D::GetDC()->SOSetTargets(1, zero, offset);
	}

	last_target = target;
	swap(source, target);

	for (UINT i = 1; i < seed->subDivisions; ++i)
	{
		source->BindToIA();
		target->BindToSO();

		subDivisionLvVar->SetInt(i);
		forkVar->SetBool(seed->patternMask & (1 << i));


		effect->Draw(0, 0, seed->GetNumVertices(i), 0);
		{
			UINT offset[1] = { 0 };
			ID3D11Buffer* zero[1] = { 0 };
			D3D::GetDC()->SOSetTargets(1, zero, offset);
		}

		last_target = target;
		swap(source, target);
	}

	{
		UINT offset[1] = { 0 };
		ID3D11Buffer* zero[1] = { 0 };
		D3D::GetDC()->SOSetTargets(1, zero, offset);
		D3D::GetDC()->IASetVertexBuffers(0, 1, zero, offset, offset);
	}

	return last_target;
}

void LightningRenderer::BuildDownSampleBuffers(UINT width, UINT height, DXGI_SAMPLE_DESC sampleDesc)
{
	for (size_t i = 0; i < (size_t)Devels::DecimationLevels; ++i)
	{
		width >>= 1;
		height >>= 1;
		SIZE s = { width, height };
		m_down_sample_buffer_sizes[i] = s;
	}

	for (size_t i = 0; i < m_down_sample_buffers.size(); ++i)
	{
		SIZE s = m_down_sample_buffer_sizes[i];
		if (0 == m_down_sample_buffers[i])
			m_down_sample_buffers[i] = new ColorRenderBuffer(s.cx, s.cy, DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc);
		else
			m_down_sample_buffers[i]->Resize(s.cx, s.cy);
	}

	m_small_lightning_buffer0->Resize(m_down_sample_buffer_sizes.back().cx, m_down_sample_buffer_sizes.back().cy);
	m_small_lightning_buffer1->Resize(m_down_sample_buffer_sizes.back().cx, m_down_sample_buffer_sizes.back().cy);
}

void LightningRenderer::DownSample(ColorRenderBuffer* buffer)
{
	vector<ColorRenderBuffer*> sources;
	vector<ColorRenderBuffer*> targets;
	sources.push_back(buffer);

	copy(m_down_sample_buffers.begin(), m_down_sample_buffers.end(), back_inserter(sources));
	copy(m_down_sample_buffers.begin(), m_down_sample_buffers.end(), back_inserter(targets));

	targets.push_back(m_small_lightning_buffer0);

	for (size_t i = 0; i < sources.size(); ++i)
	{
		ID3D11RenderTargetView* view[] = { targets[i]->RTV() };
		D3D::GetDC()->OMSetRenderTargets(1, const_cast<ID3D11RenderTargetView**> (view), 0);
		ResizeViewport(m_down_sample_buffer_sizes[i].cx, m_down_sample_buffer_sizes[i].cy);

		bufferVar->SetResource(sources[i]->SRV());
		DrawQuad(9, 0);
	}
}

void LightningRenderer::BuildGradientTexture()
{
	UINT w = 512;
	UINT h = 512;
	const UINT mip_levels = 4;
	D3D11_TEXTURE2D_DESC desc = { 0 };
	{
		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = mip_levels;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D10_RESOURCE_MISC_GENERATE_MIPS;
	}

	std::vector<UCHAR> data(4 * w * h);

	for (UINT y = 0; y < h; ++y)
	{
		UINT rowStart = y * w * 4;

		for (UINT x = 0; x < w; ++x)
		{
			float nx = x / float(w - 1);
			float ny = y / float(h - 1);

			float u = 2 * nx - 1;
			float v = 2 * ny - 1;

			float vv = max(0, min(1, 1 - sqrtf(u*u + v * v)));
			UCHAR value = (UCHAR)(max(0, min(255, vv * 255)));

			data[rowStart + x * 4 + 0] = value;
			data[rowStart + x * 4 + 1] = value;
			data[rowStart + x * 4 + 2] = value;
			data[rowStart + x * 4 + 3] = 255;
		}

	}

	D3D11_SUBRESOURCE_DATA sr[mip_levels];
	for (UINT i = 0; i < mip_levels; ++i)
	{
		sr[i].pSysMem = &data[0];
		sr[i].SysMemPitch = w * 4;
		sr[i].SysMemSlicePitch = 4 * w * h;
	}

	HRESULT hr = S_OK;
	hr = D3D::GetDevice()->CreateTexture2D(&desc, sr, &gradientTexture);
	assert(SUCCEEDED(hr));
	hr = D3D::GetDevice()->CreateShaderResourceView(gradientTexture, 0, &gradientSRV);
	assert(SUCCEEDED(hr));

	D3D::GetDC()->GenerateMips(gradientSRV);
}

void LightningRenderer::AddLightningSeed(LightningSeed * seed)
{
	lightningSeeds.insert(seed);
}

void LightningRenderer::RemoveLightningSeed(LightningSeed * seed)
{
	lightningSeeds.erase(seed);
}

void LightningRenderer::DrawQuad(UINT tech, UINT pass)
{
	ID3D11Buffer* zero = 0;
	UINT nought = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &zero, &nought, &nought);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	D3D::GetDC()->IASetInputLayout(0);

	effect->Draw(tech, pass, 4, 0);
}

