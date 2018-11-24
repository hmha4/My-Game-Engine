#include "stdafx.h"
#include "TestGeometry.h"
#include "./Environment/Terrain.h"

TestGeometry::TestGeometry()
{
	terrain = new Terrain();

	shader = new Shader(Effects + L"006_GeometryBillboard.fx");

	VertexSize vertices[1000];
	for (int i = 0; i < 1000; i++)
	{
		D3DXVECTOR2 size;
		size.x = Math::Random(1.0f, 3.0f);
		size.y = Math::Random(1.0f, 3.0f);

		D3DXVECTOR3 position;
		position.x = Math::Random(0.0f, 255.0f);
		position.z = Math::Random(0.0f, 255.0f);
		position.y = terrain->GetHeight(position.x, position.z) + size.y * 0.5f;

		vertices[i].Position = position;
		vertices[i].Size = size;
	}

	//	CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexSize) * 1000;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	vector<wstring> textures;
	textures.push_back(Textures + L"White.png");
	textures.push_back(Textures + L"Red.png");
	textures.push_back(Textures + L"Green.png");
	textures.push_back(Textures + L"Blue.png");
	textures.push_back(Textures + L"Magenta.png");
	textures.push_back(Textures + L"Cyan.png");

	textureArray = new TextureArray(textures);
	shader->AsShaderResource("Map")->SetResource(textureArray->GetSRV());
}

TestGeometry::~TestGeometry()
{
	SAFE_DELETE(textureArray);
	SAFE_RELEASE(vertexBuffer);
	SAFE_DELETE(terrain);
	SAFE_DELETE(shader);
}

void TestGeometry::Update()
{
	terrain->Update();
}

void TestGeometry::PreRender()
{
}

void TestGeometry::Render()
{
	terrain->Render();

	UINT stride = sizeof(VertexSize);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	shader->Draw(0, 0, 1000);
}

void TestGeometry::PostRender()
{

}

void TestGeometry::ResizeScreen()
{
}
