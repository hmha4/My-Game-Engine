#pragma once

#include "Particles/ChainLightning.h"
#include "Particles/PathLightning.h"


class SimpleVertexBuffer
{
public:
	SimpleVertexBuffer(vector<SubdivideVertex>& vertices, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT bindflags = D3D11_BIND_VERTEX_BUFFER, UINT cpu_flags = 0)
		: vertexBuffer(NULL), vertexCount(vertices.size())
	{
		D3D11_BUFFER_DESC bd;
		bd.Usage = usage;
		bd.ByteWidth = sizeof(SubdivideVertex) * vertexCount;
		bd.BindFlags = bindflags;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &vertices[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&bd, &initData, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
	~SimpleVertexBuffer()
	{
		SAFE_RELEASE(vertexBuffer);
	}

	UINT VertexCount() { return vertexCount; }
	ID3D11Buffer * VertexBuffer() { return vertexBuffer; }

	void BindToIA()
	{
		UINT stride = sizeof(SubdivideVertex);
		UINT offset = 0;
		D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	}

	void BindToSO()
	{
		UINT offset = 0;
		D3D::GetDC()->SOSetTargets(1, &vertexBuffer, &offset);
	}
private:
	UINT vertexCount;
	ID3D11Buffer * vertexBuffer;
};

class LightningRenderer
{
public:

	enum class Devels
	{
		DecimationLevels = 2,
	};

public:
	LightningRenderer();
	~LightningRenderer();

	void Initialize();
	void Ready();
	void PreRender();
	void Render(LightningSeed* seed);
	void Render(LightningSeed* seed, const LightningAppearance& appearance, float charge, float animation_speed);
	void PostRender(bool glow, D3DXVECTOR3 blurSigma);

public:
	PathLightning * CreatePathLightning(const vector<LightningPathSegment>& segments, int partternMask, UINT subDivisions);
	ChainLightning * CreateChainLightning(int pattern_mask, UINT subDivisions);

	void DestroyLightning(LightningSeed * seed);


private:
	void BuildSubdivisionBuffers();
	SimpleVertexBuffer * SubDivide(LightningSeed * seed);

	//void DownSample(ColorRenderBuffer * buffer);

	void BuildGradientTexture();
	//void SaveViewports();
	//void ResizeViewport(UINT width, UINT height);
	//void RestoreViewports();

	void AddLightningSeed(LightningSeed * seed);
	void RemoveLightningSeed(LightningSeed * seed);

	void DrawQuad();

private:
	Effect * effect;

	ID3D11Buffer * lightningAppearanceBuffer;
	ID3D11Buffer * lightningStructureBuffer;

	set<LightningSeed *> lightningSeeds;
	UINT maxVertexCount;

	ID3D11Texture2D * gradientTexture;
	ID3D11ShaderResourceView * gradientSRV;

	ColorRenderBuffer*		m_original_lightning_buffer;

	ColorRenderBuffer*		m_lightning_buffer0;
	ColorRenderBuffer*		m_lightning_buffer1;

	ColorRenderBuffer*		m_small_lightning_buffer0;
	ColorRenderBuffer*		m_small_lightning_buffer1;

	SimpleVertexBuffer *	m_subdivide_buffer0;
	SimpleVertexBuffer *	m_subdivide_buffer1;

private:
	ID3DX11EffectMatrixVariable * worldVar;

	ID3DX11EffectScalarVariable * forkVar;
	ID3DX11EffectScalarVariable * subDivisionLvVar;
	ID3DX11EffectScalarVariable * animationSpeedVar;
	ID3DX11EffectScalarVariable * chargeVar;

	ID3DX11EffectShaderResourceVariable * bufferVar;
	ID3DX11EffectShaderResourceVariable * gradientVar;
	ID3DX11EffectVectorVariable * bufferTexelSizeVar;

	ID3DX11EffectVectorVariable * blurSigmaVar;

	ID3DX11EffectConstantBuffer * lightningAppVar;
	ID3DX11EffectConstantBuffer * lightningStrVar;
};
