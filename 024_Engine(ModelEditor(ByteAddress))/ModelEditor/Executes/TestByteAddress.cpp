#include "stdafx.h"
#include "TestByteAddress.h"

void TestByteAddress::Initialize()
{
	for (int i = 0; i < 128; i++)
	{
		data[i].Position.x = i * 10;
		data[i].Position.y = i * 100;
		data[i].Position.z = i * 1000;

		data[i].Normal.x = i + 10;
		data[i].Normal.y = i + 100;
		data[i].Normal.z = i + 1000;

		data[i].Tangent.x = i - 10;
		data[i].Tangent.y = i - 100;
		data[i].Tangent.z = i - 1000;
	}
}

void TestByteAddress::Ready()
{
	CsResource::CreateRawBuffer(sizeof(ModelVertexType) * 128, data, &input);
	CsResource::CreateRawBuffer(sizeof(ModelVertexType) * 128, NULL, &vertexBuffer);

	CsResource::CreateSRV(input, &inputSRV);
	CsResource::CreateUAV(vertexBuffer, &outputUAV);

	effect = new Effect(Effects + L"009_ComputeByteAddress.fx", true);
	effect->AsShaderResource("data")->SetResource(inputSRV);
	effect->AsUAV("result")->SetUnorderedAccessView(outputUAV);

	effect->Dispatch(0, 0, 1, 1, 1);

	ModelVertexType result[128];
	ID3D11Buffer * dest = CsResource::CreateAndCopyBuffer(vertexBuffer);
	D3D11_MAPPED_SUBRESOURCE resource;
	D3D::GetDC()->Map(dest, 0, D3D11_MAP_READ, 0, &resource);
	{
		memcpy(result, resource.pData, sizeof(ModelVertexType) * 128);
	}
	D3D::GetDC()->Unmap(dest, 0);

	for (int i = 0; i < 128; i++)
	{
		wstring temp = L"";
		temp = L"Position ==>" + to_wstring(i) + L" : " + to_wstring(result[i].Position.x) + L", " + to_wstring(result[i].Position.y) + L", " + to_wstring(result[i].Position.z) + L"\n";
		temp += L"Normal ==>" + to_wstring(i) + L" : " + to_wstring(result[i].Normal.x) + L", " + to_wstring(result[i].Normal.y) + L", " + to_wstring(result[i].Normal.z) + L"\n";
		temp += L"Tangent ==>" + to_wstring(i) + L" : " + to_wstring(result[i].Tangent.x) + L", " + to_wstring(result[i].Tangent.y) + L", " + to_wstring(result[i].Tangent.z) + L"\n";

		OutputDebugString(temp.c_str());
	}
}

void TestByteAddress::Destroy()
{
	SAFE_DELETE(effect);

	SAFE_RELEASE(input);
	SAFE_RELEASE(inputSRV);

	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(outputUAV);
}

void TestByteAddress::Update()
{
}

void TestByteAddress::PreRender()
{
}

void TestByteAddress::Render()
{
}

void TestByteAddress::PostRender()
{
}

void TestByteAddress::ResizeScreen()
{
}