#pragma once
#include "Framework.h"

class LineMaker
{
private:
	Effect * effect;

	D3DXVECTOR3 lineDir;
	D3DXMATRIX lineWorld;

	UINT vertexCount, indexCount;

	ID3D11Buffer*vertexBuffer;
	ID3D11Buffer*indexBuffer;

	Vertex*vertices;
	UINT*indices;

	D3DXCOLOR lineColor;
	bool outEffect = false;
private:
	ID3DX11EffectMatrixVariable * worldVar;
	ID3DX11EffectVectorVariable * lineColorVar;

private:
	void SetVertexBuffer()
	{
		for (UINT i = 0; i < indexCount; i++)
			indices[i] = i;

		{
			D3D11_BUFFER_DESC desc = { 0 };
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ByteWidth = sizeof(Vertex) * vertexCount;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			D3D11_SUBRESOURCE_DATA data = { 0 };
			data.pSysMem = vertices;

			HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc,
				&data, &vertexBuffer);
			assert(SUCCEEDED(hr));
		}

		{
			D3D11_BUFFER_DESC desc = { 0 };
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ByteWidth = sizeof(UINT) * indexCount;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

			D3D11_SUBRESOURCE_DATA data = { 0 };
			data.pSysMem = indices;

			HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data,
				&indexBuffer);
			assert(SUCCEEDED(hr));
		}
	}
public:
	LineMaker()
	{
		effect = new Effect(Effects + L"052_Line.hlsl");
		worldVar = effect->AsMatrix("World");
		lineColorVar = effect->AsVector("Color");

		vertices = new Vertex[2];
		indices = new UINT[2];

		D3DXMatrixIdentity(&lineWorld);
		lineColor = { 1,0,0,1 };

		vertexCount = indexCount = 2;

		SetVertexBuffer();
	}
	LineMaker(Effect*effect)
	{
		this->effect = effect;
		outEffect = true;
		vertices = new Vertex[2];
		indices = new UINT[2];

		D3DXMatrixIdentity(&lineWorld);
		lineColor = { 1,0,0,1 };

		vertexCount = indexCount = 2;

		SetVertexBuffer();
	}
	~LineMaker()
	{
		SAFE_RELEASE(vertexBuffer);
		SAFE_RELEASE(indexBuffer);

		SAFE_DELETE_ARRAY(vertices);
		SAFE_DELETE_ARRAY(indices);

		if (!outEffect)
			SAFE_DELETE(effect);
	}

	////////////////////////////포지션이 직접적으로 변할때 업데이트에서 사용/////////////////////////
	D3DXVECTOR3 UpdateLine(D3DXVECTOR3 startPos, D3DXVECTOR3 lineDir, float line)
	{
		vertices[0].Position = startPos;
		vertices[1].Position =
			D3DXVECTOR3(
				lineDir.x*line + startPos.x, //버텍스 위치에서 일정방향 위치
				lineDir.y*line + startPos.y,
				lineDir.z*line + startPos.z
			);

		D3D::GetDC()->UpdateSubresource(vertexBuffer, 0, NULL, &vertices[0], sizeof(VertexTextureNormal), vertexCount);

		return vertices[1].Position - vertices[0].Position;
	}
	void UpdateLine(D3DXVECTOR3 startPos, D3DXVECTOR3 endPos)
	{
		vertices[0].Position = startPos;
		vertices[1].Position = endPos;

		D3D::GetDC()->UpdateSubresource(vertexBuffer, 0, NULL, &vertices[0], sizeof(VertexTextureNormal), vertexCount);
	}
	////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////랜더////////////////////////////////////////////
	void DrawLine(int path)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		worldVar->SetMatrix(lineWorld);
		lineColorVar->SetFloatVector(lineColor);

		effect->DrawIndexed(0, path, indexCount);
	}
	////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////색상셋팅///////////////////////////////////////
	void SetColor(D3DXCOLOR color)
	{
		lineColor = color;
	}
	////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////초기셋팅값에 메트릭스변환시킬때/////////////////////////////
	void World(D3DXMATRIX&matrix)
	{
		lineWorld = matrix;
	}
	////////////////////////////////////////////////////////////////////////////////////////
};
