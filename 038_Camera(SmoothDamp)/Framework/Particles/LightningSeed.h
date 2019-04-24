#pragma once
#include "Framework.h"

struct SubdivideVertex
{

	SubdivideVertex() :
		Start(D3DXVECTOR3(0, 0, 0)),
		End(D3DXVECTOR3(0, 0, 0)),
		Up(D3DXVECTOR3(0, 0, 0)),
		Level(0)
	{
	}

	SubdivideVertex(const D3DXVECTOR3& start, const D3DXVECTOR3& end, const D3DXVECTOR3& up) :
		Start(start),
		End(end),
		Up(up),
		Level(0)
	{
	}

	D3DXVECTOR3	Start;
	D3DXVECTOR3	End;
	D3DXVECTOR3	Up;


	UINT		Level;
};

// Geometric properties of a single bolt
// this one matches the constant buffer layout in the Lightning.fx file
struct LightningStructure
{
	// for ZigZag pattern
	D3DXVECTOR2 ZigZagFraction;
	D3DXVECTOR2 ZigZagDeviationRight;

	D3DXVECTOR2 ZigZagDeviationUp;
	float		ZigZagDeviationDecay;
	float Dummy0;						// dummy to match HLSL padding

										// for Fork pattern
	D3DXVECTOR2 ForkFraction;
	D3DXVECTOR2 ForkZigZagDeviationRight;

	D3DXVECTOR2 ForkZigZagDeviationUp;
	float		ForkZigZagDeviationDecay;
	float Dummy1;						// dummy to match HLSL padding

	D3DXVECTOR2 ForkDeviationRight;
	D3DXVECTOR2 ForkDeviationUp;

	D3DXVECTOR2 ForkDeviationForward;
	float		ForkDeviationDecay;
	float Dummy2;						// dummy to match HLSL padding

	D3DXVECTOR2	ForkLength;
	float		ForkLengthDecay;

};


struct LightningAppearance
{
	D3DXVECTOR3 ColorInside;
	float		ColorFallOffExponent;

	D3DXVECTOR3 ColorOutside;
	float		Dummy1;

	D3DXVECTOR2 BoltWidth;
};

struct LightSettings
{
	LightSettings() :
		AnimationSpeed(15)
		, InUse(false)
	{
	}

	bool    InUse;
	float   AnimationSpeed;

	LightningAppearance Beam;
};

class LightningRenderer;
class LightningSeed
{
	friend class LightningRenderer;

public:
	LightningStructure Structure;
	LightSettings Settings;

public:
	LightningSeed() {}
	LightningSeed(Effect* effect, int pattern_mask, unsigned int subdivisions)
		: effect(effect), patternMask(pattern_mask), subDivisions(subdivisions)
	{
		lightningBufferVar = effect->AsConstantBuffer("LightningStructure");

		Settings.Beam.BoltWidth = D3DXVECTOR2(0.25f, 0.5f);
		Settings.Beam.ColorInside = D3DXVECTOR3(0, 1, 1);
		Settings.Beam.ColorOutside = D3DXVECTOR3(0, 0, 1);
		Settings.Beam.ColorFallOffExponent = 5.0f;


		D3DXMatrixIdentity(&world);
	}
	virtual ~LightningSeed() 
	{

	}
	
	void World(D3DXMATRIX& mat) { world = mat; }
	void LocalWidth(float val) { localWidth = val; }
protected:
	virtual void Render(UINT pass = 0, UINT tech = 0)
	{
		ID3D11Buffer * zero = NULL;
		UINT nought = 0;

		D3D::GetDC()->IASetVertexBuffers(0, 1, &zero, &nought, &nought);
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		D3D::GetDC()->IASetInputLayout(0);

		effect->Draw(pass, tech, GetNumVertices(0), 0);
	}
	virtual unsigned int GetMaxNumVertices() { return GetNumVertices(subDivisions); }
	virtual unsigned int GetNumVertices(unsigned int level) { return 0; }

	unsigned int GetNumBoltVertices(unsigned int level)
	{
		unsigned int result = 1;
		for (unsigned int i = 0; i < level; ++i)
		{

			if (patternMask & (1 << i))
				result *= 3;
			else
				result *= 2;
		}
		return result;
	}

	virtual void SetChildConstants()
	{
	}

	void SetConstants()
	{
		HRESULT hr = lightningBufferVar->SetRawValue(const_cast<LightningStructure*>(&Structure), 0, sizeof(LightningStructure));
		assert(SUCCEEDED(hr));

		SetChildConstants();
	}
	
	void CreateBuffer(vector<SubdivideVertex>& vertices)
	{
		vertexCount = vertices.size();

		D3D11_BUFFER_DESC bd;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SubdivideVertex) * vertexCount;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &vertices[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&bd, &initData, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
protected:
	Effect * effect;

	int patternMask;
	UINT subDivisions;

	UINT vertexCount;
	ID3D11Buffer * vertexBuffer;

	ID3DX11EffectConstantBuffer * lightningBufferVar;
	ID3D11Buffer * lightningBuffer;

	D3DXMATRIX world;
	float localWidth;
};