#include "stdafx.h"
#include "TessTriangleLOD.h"

TessTriangleLOD::TessTriangleLOD()
{
	shader = new Shader(Effects + L"019_TessTriangleLOD.fx");

	shader->AsScalar("Ratio")->SetInt(1);

	D3DXMATRIX W, T;
	D3DXMatrixScaling(&W, 10, 10, 1);
	D3DXMatrixTranslation(&T, 20, 0, 0);

	shader->AsMatrix("World")->SetMatrix(W * T);

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

TessTriangleLOD::~TessTriangleLOD()
{
	SAFE_DELETE(shader);

	SAFE_RELEASE(vertexBuffer);
}

void TessTriangleLOD::Update()
{
	
}

void TessTriangleLOD::PreRender()
{

}

void TessTriangleLOD::Render()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//	D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : HS, DS 를 사용하기 위함
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	shader->Draw(0, 0, 3);
}

void TessTriangleLOD::PostRender()
{

}

void TessTriangleLOD::ResizeScreen()
{
}
