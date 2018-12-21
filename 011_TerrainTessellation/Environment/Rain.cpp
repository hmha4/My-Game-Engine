#include "stdafx.h"
#include "Rain.h"

#include "../Viewer/Freedom.h"

Rain::Rain( D3DXVECTOR3 & size, UINT count)
	: particleCount(count)
{
	shader = new Shader(Effects + L"011_Rain.fx");

	VertexRain * vertices = new VertexRain[particleCount];
	//	Create Data
	{
		for (UINT i = 0; i < particleCount; i++)
		{
			D3DXVECTOR2 S;
			S.x = Math::Random(0.1f, 0.4f);
			S.y = Math::Random(4.0f, 8.0f);

			D3DXVECTOR3 P;
			P.x = Math::Random(-size.x * 0.5f, size.x * 0.5f);
			P.y = Math::Random(-size.y * 0.5f, size.y * 0.5f);
			P.z = Math::Random(-size.z * 0.5f, size.z * 0.5f);

			UINT textureID = (UINT)Math::Random(0, 370);

			//this number is used to randomly increase the brightness of some rain particles
			float random = 1;
			float randomIncrease = float((double)rand() / ((double)(RAND_MAX)+(double)(1)));
			if (randomIncrease > 0.8)
				random += randomIncrease;

			vertices[i] = VertexRain(P, S, textureID, random);
		}
	}

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexRain) * particleCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
	SAFE_DELETE_ARRAY(vertices);

	vector<wstring> textureArray;
	for (int i = 0; i < 370; i++)
	{
		wstring temp;
		if(i >= 100)
			temp = Textures + L"rainTextures/cv0_vPositive_0";
		else if(i >= 10)
			temp = Textures + L"rainTextures/cv0_vPositive_00";
		else
			temp = Textures + L"rainTextures/cv0_vPositive_000";

		textureArray.push_back(temp.append(to_wstring(i) + L".dds"));
	}

	rainMaps = new TextureArray(textureArray);
	shader->AsShaderResource("Map")->SetResource(rainMaps->GetSRV());

	shader->AsVector("Size")->SetFloatVector(size);
	shader->AsVector("Velocity")->SetFloatVector(D3DXVECTOR3(0, -200, 0));
	shader->AsScalar("DrawDistance")->SetFloat(1000.0f);
	shader->AsVector("Color")->SetFloatVector(D3DXCOLOR(1, 1, 1, 1));
	shader->AsVector("Origin")->SetFloatVector(D3DXVECTOR3(0, 0, 0));
}

Rain::~Rain()
{
	SAFE_DELETE(rainMaps);

	SAFE_DELETE(shader);

	SAFE_RELEASE(vertexBuffer);
}

void Rain::Update()
{
	float rainTime = Time::Get()->Running() * 1000.0f / 750.0f;
	shader->AsScalar("RainTime")->SetFloat(rainTime);

	D3DXVECTOR3 camPos;
	Context::Get()->GetMainCamera()->Position(&camPos);

	shader->AsVector("Origin")->SetFloatVector(camPos);
}

void Rain::Render()
{
	UINT stride = sizeof(VertexRain);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	shader->Draw(0, 0, particleCount);
}
