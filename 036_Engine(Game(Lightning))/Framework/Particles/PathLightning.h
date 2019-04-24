#pragma once
#include "LightningSeed.h"

struct LightningPathSegment
{
	LightningPathSegment(const D3DXVECTOR3&	start, const D3DXVECTOR3& end, const D3DXVECTOR3& up) :
		Start(start),
		End(end),
		Up(up)
	{
	}
	LightningPathSegment(float sx, float sy, float sz, float ex, float ey, float ez, float ux = 0, float uy = 0, float uz = 1) :
		Start(sx, sy, sz),
		End(ex, ey, ez),
		Up(ux, uy, uz)
	{
	}

	operator SubdivideVertex() const
	{
		SubdivideVertex result;

		result.Start = this->Start;
		result.End = this->End;
		result.Up = this->Up;

		result.Level = 0;

		return result;
	}

	D3DXVECTOR3	Start;
	D3DXVECTOR3	End;
	D3DXVECTOR3	Up;
};

class LightningRenderer;
class PathLightning : public LightningSeed
{
	friend class LightningRenderer;
public:
	PathLightning(Effect * effect, const vector<LightningPathSegment>& segments, int pattern_mask, UINT subdivisions)
		: LightningSeed(effect, pattern_mask, subdivisions)
	{
		vector<SubdivideVertex> vertices;
		copy(segments.begin(), segments.end(), back_inserter(vertices));
		CreateBuffer(vertices);
	}
	~PathLightning()
	{

	}

public:
	virtual void Render(UINT pass = 0, UINT tech = 0) override
	{
		UINT stride = { sizeof(SubdivideVertex) };
		UINT offset = 0;
		D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		D3D::GetDC()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

		effect->Draw(pass, tech, GetNumVertices(0), 0);
	}

	virtual unsigned int GetNumVertices(unsigned int level) override
	{
		return vertexCount * GetNumBoltVertices(level);
	}

private:
	
};