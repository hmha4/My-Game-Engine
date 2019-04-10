#include "Framework.h"
#include "DebugLine.h"
#include "ILine.h"

const UINT DebugLine::MaxCount = 5000;

DebugLine::DebugLine(Effect * effect)
	: effect(effect), color(0, 0, 0, 1)
	, vertexBuffer(NULL)
{
	worldVar = effect->AsMatrix("World");
	lineColorVar = effect->AsVector("Color");
}

DebugLine::~DebugLine()
{
	//SAFE_DELETE(effect);
	SAFE_RELEASE(vertexBuffer);
}

void DebugLine::Initialize()
{
}

void DebugLine::Ready()
{
	//effect = new Effect(Effects + L"002_Line.fx");

	//Create Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(Vertex) * MaxCount * 2;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
}

void DebugLine::Draw(D3DXMATRIX& world, ILine * line)
{
	vector<D3DXVECTOR3> temp;
	line->GetLine(world, temp);

	lines.clear();
	for (size_t i = 0; i < temp.size(); i += 2)
	{
		Line line;
		line.start = temp[i + 0];
		line.end = temp[i + 1];
		lines.push_back(line);
	}

	D3D11_MAPPED_SUBRESOURCE subResource;
	HRESULT hr = D3D::GetDC()->Map
	(
		vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource
	);

	memcpy(subResource.pData, &lines[0], sizeof(Line) * lines.size());

	D3D::GetDC()->Unmap(vertexBuffer, 0);
}

void DebugLine::Color(float r, float g, float b)
{
	D3DXCOLOR val(r, g, b, 1.0f);

	Color(val);
}

void DebugLine::Color(D3DXCOLOR & vec)
{
	color = vec;
}

void DebugLine::Render(UINT pass, D3DXMATRIX *world)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	lineColorVar->SetFloatVector(color);

	D3DXMATRIX W;
	if (world == NULL)
		D3DXMatrixIdentity(&W);
	else
		W = *world;

	worldVar->SetMatrix(W);

	effect->Draw(0, pass, lines.size() * 2);
}