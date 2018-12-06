#include "stdafx.h"
#include "Terrain.h"

Terrain::Terrain()
{
	heightMap = new Texture(Contents + L"HeightMaps/HeightMap256.png");

	vector<D3DXCOLOR> heights;
	heightMap->ReadPixels(DXGI_FORMAT_R8G8B8A8_UNORM, &heights);

	width = heightMap->GetWidth() - 1;
	height = heightMap->GetHeight() - 1;

	diffuseMap = new Texture(Textures + L"Dirt.png");

	shader = new Shader(Effects + L"007_Terrain.fx");
	shader->AsShaderResource("DiffuseMap")->SetResource(diffuseMap->SRV());

	D3DXMATRIX W; D3DXMatrixIdentity(&W);
	shader->AsMatrix("World")->SetMatrix(W);

	//Create VertexData
	{
		vertexCount = (width + 1) * (height + 1);
		vertices = new VertexTextureNormal[vertexCount];

		for (UINT z = 0; z <= height; z++)
		{
			for (UINT x = 0; x <= width; x++)
			{
				UINT index = (width + 1) * z + x;

				vertices[index].Position.x = (float)x;
				vertices[index].Position.y = (float)(heights[index].r * 255.0f) / 7.5f;
				vertices[index].Position.z = (float)z;

				vertices[index].Uv.x = (float)x / (float)width;
				vertices[index].Uv.y = (float)z / (float)height;
			}
		}//for(z)
	}

	//CreateIndexData
	{
		indexCount = width * height * 6;
		indices = new UINT[indexCount];

		UINT index = 0;
		for (UINT z = 0; z < height; z++)
		{
			for (UINT x = 0; x < width; x++)
			{
				indices[index + 0] = (width + 1) * z + x; //0
				indices[index + 1] = (width + 1) * (z + 1) + x; //1
				indices[index + 2] = (width + 1) * z + x + 1; //2

				indices[index + 3] = (width + 1) * z + x + 1; //2
				indices[index + 4] = (width + 1) * (z + 1) + x; //1
				indices[index + 5] = (width + 1) * (z + 1) + x + 1; //1

				index += 6;
			}
		}//for(z)
	}

	//CreateNormal
	{
		for (UINT i = 0; i < (indexCount / 3); i++)
		{
			UINT index0 = indices[i * 3 + 0];
			UINT index1 = indices[i * 3 + 1];
			UINT index2 = indices[i * 3 + 2];

			VertexTextureNormal v0 = vertices[index0];
			VertexTextureNormal v1 = vertices[index1];
			VertexTextureNormal v2 = vertices[index2];

			D3DXVECTOR3 d1 = v1.Position - v0.Position;
			D3DXVECTOR3 d2 = v2.Position - v0.Position;

			D3DXVECTOR3 normal;
			D3DXVec3Cross(&normal, &d1, &d2);

			vertices[index0].Normal += normal;
			vertices[index1].Normal += normal;
			vertices[index2].Normal += normal;
		}

		for (UINT i = 0; i < vertexCount; i++)
			D3DXVec3Normalize(&vertices[i].Normal, &vertices[i].Normal);
	}

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * vertexCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(UINT) * indexCount;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = indices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &indexBuffer);
		assert(SUCCEEDED(hr));
	}

	SAFE_DELETE(heightMap);
	
}

Terrain::~Terrain()
{
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);

	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);

	SAFE_DELETE(shader);
	SAFE_DELETE(diffuseMap);
}

void Terrain::Update()
{
	
}

void Terrain::Render()
{
	UINT stride = sizeof(VertexTextureNormal);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->DrawIndexed(0, 0, indexCount);
}

float Terrain::GetHeight(float positionX, float positionZ)
{
	UINT x = (UINT)positionX;
	UINT z = (UINT)positionZ;

	if (x < 0 || x >= width) return 0.0f;
	if (z < 0 || z >= height) return 0.0f;

	UINT index[4];
	index[0] = (width + 1) * z + x;
	index[1] = (width + 1) * (z + 1) + x;
	index[2] = (width + 1) * z + (x + 1);
	index[3] = (width + 1) * (z + 1) + x + 1;

	D3DXVECTOR3 v[4];
	for (int i = 0; i < 4; i++)
		v[i] = vertices[index[i]].Position;


	float ddx = (x - v[0].x) / 1.0f;
	float ddz = (z - v[0].z) / 1.0f;


	D3DXVECTOR3 temp;
	if (ddx + ddz <= 1)
	{
		temp = v[0] + (v[2] - v[0]) * ddx + (v[1] - v[0]) * ddz;
	}
	else
	{
		ddx = 1 - ddx;
		ddz = 1 - ddz;

		temp = v[3] + (v[1] - v[3]) * ddx + (v[2] - v[3]) * ddz;
	}

	return temp.y;
}
