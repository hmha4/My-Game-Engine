#pragma once

//=======================================================================================

struct Vertex
{
	Vertex()
		: Position(0, 0, 0) {}

	D3DXVECTOR3 Position;
};

struct VertexSize
{
	VertexSize()
		: Position(0, 0, 0)
		, Size(0, 0) {}

	D3DXVECTOR3 Position;
	D3DXVECTOR2 Size;
};

//=======================================================================================

struct VertexColor
{
	VertexColor()
		: Position(0, 0, 0)
		, Color(0, 0, 0, 1) {}

	VertexColor(D3DXVECTOR3 vec, D3DXCOLOR color)
		: Position(vec), Color(color) {}

	D3DXVECTOR3	Position;
	D3DXCOLOR Color;
};

//=======================================================================================

struct VertexColorNormal
{
	VertexColorNormal()
		: Position(0, 0, 0)
		, Color(0, 0, 0, 1)
		, Normal(0, 0, 0) {}

	D3DXVECTOR3 Position;
	D3DXCOLOR Color;
	D3DXVECTOR3 Normal;
};

//=======================================================================================

struct VertexTexture
{
	VertexTexture()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Alpha(1.0f)
		, Time(0.0f) {}

	D3DXVECTOR3	Position;
	D3DXVECTOR2	Uv;
	float Alpha;
	float Time;
};

//=======================================================================================

struct VertexTextureNormal
{
	VertexTextureNormal()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Normal(0, 0, 0) {}

	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;
	D3DXVECTOR3 Normal;
};

//=======================================================================================

struct VertexColorTextureNormal
{
	VertexColorTextureNormal()
		: Position(0, 0, 0)
		, Color(0, 0, 0, 1)
		, Uv(0, 0)
		, Normal(0, 0, 0)
		, Time(0.0f) {}

	D3DXVECTOR3 Position;
	D3DXCOLOR Color;
	D3DXVECTOR2 Uv;
	D3DXVECTOR3 Normal;
	float Time;
};

//=======================================================================================

struct VertexTextureNormalBlend
{
	VertexTextureNormalBlend()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Normal(0, 0, 0)
		, BlendIndices(0, 0, 0, 0)
		, BlendWeights(0, 0, 0, 0) {}

	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;
	D3DXVECTOR3 Normal;
	D3DXVECTOR4 BlendIndices;
	D3DXVECTOR4 BlendWeights;
};

//=======================================================================================

struct VertexTextureNormalTangent
{
	VertexTextureNormalTangent()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Normal(0, 0, 0)
		, Tangent(0, 0, 0)
	{}

	VertexTextureNormalTangent(float px, float py, float pz, float u, float v, float nx, float ny, float nz, float tx, float ty, float tz)
	{
		Position = D3DXVECTOR3(px, py, pz);
		Uv = D3DXVECTOR2(u, v);
		Normal = D3DXVECTOR3(nx, ny, nz);
		Tangent = D3DXVECTOR3(tx, ty, tz);
	}

	D3DXVECTOR3	Position;
	D3DXVECTOR2	Uv;
	D3DXVECTOR3	Normal;
	D3DXVECTOR3 Tangent;
};

struct VertexTextureNormalTangentBlend
{
	VertexTextureNormalTangentBlend()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Normal(0, 0, 0)
		, Tangent(0, 0, 0)
		, BlendIndices(0, 0, 0, 0)
		, BlendWeights(0, 0, 0, 0)
	{}

	D3DXVECTOR3	Position;
	D3DXVECTOR2	Uv;
	D3DXVECTOR3	Normal;
	D3DXVECTOR3 Tangent;
	D3DXVECTOR4 BlendIndices;
	D3DXVECTOR4 BlendWeights;
};

struct VertexTextureColor2
{
	VertexTextureColor2()
		: Position(0, 0, 0)
		, Uv(0, 0)
		, Color(0, 0, 0, 1)
		, Color2(0, 0, 0, 1)
	{
	}

	D3DXVECTOR3 Position;
	D3DXVECTOR2 Uv;
	D3DXCOLOR Color;
	D3DXCOLOR Color2;
};