#pragma once
#include "Framework.h"
#include "LightningSeed.h"

class ChainLightning : public LightningSeed
{
	friend class LightningRenderer;

public:
	struct ChainLightningProperties
	{
		enum
		{
			MaxTargets = 8
		};
		D3DXVECTOR3	ChainSource;
		float Dummy0;

		D3DXVECTOR4 ChainTargetPositions[MaxTargets];

		int			NumTargets;
	};

	ChainLightningProperties Properties;

private:
	ChainLightning(Effect * effect, int pattern_mask, UINT subdivisions)
		: LightningSeed(effect, pattern_mask, subdivisions)
	{
		chainCB = effect->AsConstantBuffer("LightningChain");

	}

	virtual void SetChildConstants()
	{
		HRESULT hr = chainCB->SetRawValue(const_cast<ChainLightningProperties*>(&Properties), 0, sizeof(ChainLightningProperties));
		assert(SUCCEEDED(hr));
	}

	virtual void Render(UINT pass = 0, UINT tech = 0) override
	{
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		ID3D11Buffer* zero = 0;
		UINT nought = 0;
		D3D::GetDC()->IASetVertexBuffers(0, 1, &zero, &nought, &nought);
		D3D::GetDC()->IASetInputLayout(0);

		effect->Draw(1, 0, GetNumVertices(0), 0);
	}

	virtual unsigned int GetMaxNumVertices()
	{
		return  ChainLightningProperties::MaxTargets * GetNumBoltVertices(subDivisions);
	}

	virtual unsigned int GetNumVertices(unsigned int level)
	{

		return Properties.NumTargets * GetNumBoltVertices(level);
	}

private:
	ID3DX11EffectConstantBuffer * chainCB;
	ID3D11Buffer * chainBuffer;
};