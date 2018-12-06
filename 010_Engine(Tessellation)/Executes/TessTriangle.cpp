#include "stdafx.h"
#include "TessTriangle.h"

TessTriangle::TessTriangle()
{
	shader = new Shader(Effects + L"018_TessTriangle.fx");

	 int amount = 10;
	 int amountInside = 10;

	shader->AsScalar("TsAmount")->SetInt(amount);
	shader->AsScalar("TsAmountInside")->SetInt(amountInside);

	Vertex vertices[3];
	vertices[0].Position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	vertices[1].Position = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	vertices[2].Position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);

	// Create Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(Vertex) * 3;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
}

TessTriangle::~TessTriangle()
{
	SAFE_DELETE(shader);

	SAFE_RELEASE(vertexBuffer);
}

void TessTriangle::Update()
{
	D3DXMATRIX W;
	D3DXMatrixScaling(&W, 10, 10, 1);

	shader->AsMatrix("World")->SetMatrix(W);
}

void TessTriangle::PreRender()
{

}

void TessTriangle::Render()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//	D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : HS, DS 를 사용하기 위함
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	shader->Draw(0, 0, 3);
}

void TessTriangle::PostRender()
{

}

void TessTriangle::ResizeScreen()
{
}
