#include "stdafx.h"
#include "TessRectLOD.h"

TessRectLOD::TessRectLOD()
{
	shader = new Shader(Effects + L"021_TessRectLOD.fx");

	shader->AsScalar("Ratio")->SetInt(1);

	D3DXMATRIX W, T;
	D3DXMatrixScaling(&W, 10, 10, 1);
	D3DXMatrixTranslation(&T, 20, -20, 0);

	shader->AsMatrix("World")->SetMatrix(W * T);

	Vertex vertices[4];
	vertices[0].Position = D3DXVECTOR3(-0.5f, -0.5f, 0);
	vertices[1].Position = D3DXVECTOR3(-0.5f, 0.5f, 0);
	vertices[2].Position = D3DXVECTOR3(0.5f, -0.5f, 0);
	vertices[3].Position = D3DXVECTOR3(0.5f, 0.5f, 0);

	// Create Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(Vertex) * 4;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
}

TessRectLOD::~TessRectLOD()
{
	SAFE_DELETE(shader);

	SAFE_RELEASE(vertexBuffer);
}

void TessRectLOD::Update()
{
	
}

void TessRectLOD::PreRender()
{

}

void TessRectLOD::Render()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//	D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : HS, DS 를 사용하기 위함
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	shader->Draw(0, 0, 4);
}

void TessRectLOD::PostRender()
{

}

void TessRectLOD::ResizeScreen()
{
}
