#include "stdafx.h"
#include "TestComputeAdd.h"

TestComputeAdd::TestComputeAdd()
	: dataSize(64)
{
	shader = new Shader(Effects + L"009_Compute.fx", true);

	vector<Data> dataA(dataSize);
	vector<Data> dataB(dataSize);

	for (int i = 0; i < dataSize; i++)
	{
		dataA[i].V1 = D3DXVECTOR3(i, i, i);
		dataA[i].V2 = D3DXVECTOR2(i, 0.0f);

		dataB[i].V1 = D3DXVECTOR3(-i, i, 0.0f);
		dataB[i].V2 = D3DXVECTOR2(i, -i);
	}

	HRESULT hr;
	//	Create Input Buffer
	{
		D3D11_BUFFER_DESC inputDesc = { 0 };
		inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		inputDesc.ByteWidth = sizeof(Data) * dataSize;
		inputDesc.CPUAccessFlags = 0;
		//	D3D11_RESOURCE_MISC_BUFFER_STRUCTURED
		//	����ȭ ���� : ����ü�� �迭�� ���� ����� �� �ִ� ����
		inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		inputDesc.StructureByteStride = sizeof(Data);
		inputDesc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA data1 = { 0 };
		data1.pSysMem = &dataA[0];

		ID3D11Buffer *buffer1 = NULL;
		hr = D3D::GetDevice()->CreateBuffer(&inputDesc, &data1, &buffer1);
		assert(SUCCEEDED(hr));

		D3D11_SUBRESOURCE_DATA data2 = { 0 };
		data2.pSysMem = &dataB[0];

		ID3D11Buffer *buffer2 = NULL;
		hr = D3D::GetDevice()->CreateBuffer(&inputDesc, &data2, &buffer2);
		assert(SUCCEEDED(hr));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.FirstElement = 0;
		srvDesc.BufferEx.Flags = 0;
		srvDesc.BufferEx.NumElements = dataSize;

		hr = D3D::GetDevice()->CreateShaderResourceView(buffer1, &srvDesc, &srv1);
		assert(SUCCEEDED(hr));

		hr = D3D::GetDevice()->CreateShaderResourceView(buffer2, &srvDesc, &srv2);
		assert(SUCCEEDED(hr));

		SAFE_RELEASE(buffer1);
		SAFE_RELEASE(buffer2);
	}

	//	Create Output Buffer
	{
		//	��ǻƮ ���̴��� ��� ����� ���� ���ҽ��� ��������.
		//	�� ������ ��� ����� CPU������ ��������
		//	���۰� CPU�κ��� �о���� �� �־���Ѵ�.
		//	�׷��� CPU�κ��� �о���� �� �ִ� ���۴� D3D11_USAGE_STAGING
		//	�ʵ�� ������ ���� �ۿ� ����.
		//	�� ��� ������������ ����¿��� ���ε� �� �� ����.

		//	�� ������ ��ǻƮ ���̴��� ��꿡 ���Ǵ� �Ͱ���
		//	������ ������� ���� ���۸� D3D11_USAGE_STAGING �ʵ��
		//	�����Ͽ� ����� �д�.

		//	GPU���� ����ϴ� ���۷κ��� CPU���� �о���� �� �ְ� �����͸� ī�� �Ϸ���
		//	1. ���� ��ü�� ī���ϴ� GetDC()->CopyResource() �Լ���
		//	2. ���� �Ϻκ��� ī���ϴ� GetDC()->CopySubresourceRegion() �Լ��� �ִ�.

		D3D11_BUFFER_DESC outputDesc = { 0 };
		outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		outputDesc.ByteWidth = sizeof(Data) * dataSize;
		outputDesc.CPUAccessFlags = 0;
		outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		outputDesc.StructureByteStride = sizeof(Data);
		outputDesc.Usage = D3D11_USAGE_DEFAULT;

		hr = D3D::GetDevice()->CreateBuffer(&outputDesc, NULL, &outputBuffer);
		assert(SUCCEEDED(hr));

		outputDesc.Usage = D3D11_USAGE_STAGING;
		outputDesc.BindFlags = 0;
		outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		hr = D3D::GetDevice()->CreateBuffer(&outputDesc, NULL, &outputDebugBuffer);
		assert(SUCCEEDED(hr));

		//	��ǻƮ ���̴��� ��¿��� UAV��� ������ �䰡 �ʿ��ϴ�.
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = dataSize;

		hr = D3D::GetDevice()->CreateUnorderedAccessView(outputBuffer, &uavDesc, &uav);
		assert(SUCCEEDED(hr));
	}
	ExecuteCS();
}

TestComputeAdd::~TestComputeAdd()
{
	SAFE_RELEASE(srv1);
	SAFE_RELEASE(srv2);
	SAFE_RELEASE(uav);

	SAFE_RELEASE(outputBuffer);
	SAFE_RELEASE(outputDebugBuffer);

	SAFE_DELETE(shader);
}

void TestComputeAdd::Update()
{
	
}

void TestComputeAdd::PreRender()
{
}

void TestComputeAdd::Render()
{
	
}

void TestComputeAdd::PostRender()
{

}

void TestComputeAdd::ResizeScreen()
{
}

void TestComputeAdd::ExecuteCS()
{
	shader->AsShaderResource("Input1")->SetResource(srv1);
	shader->AsShaderResource("Input2")->SetResource(srv2);
	shader->AsUAV("Output")->SetUnorderedAccessView(uav);

	shader->Dispatch(0, 0, 64, 1, 1);

	FILE * fp = NULL;
	fopen_s(&fp, "Result1.txt", "wt");

	D3D::GetDC()->CopyResource(outputDebugBuffer, outputBuffer);

	D3D11_MAPPED_SUBRESOURCE mapped;
	D3D::GetDC()->Map(outputDebugBuffer, 0, D3D11_MAP_READ, 0, &mapped);
	{
		Data * dataView = reinterpret_cast<Data *>(mapped.pData);
		for (int i = 0; i < dataSize; i++)
		{
			fprintf(fp, "V1 : (%f, %f, %f) - ", dataView[i].V1.x, dataView[i].V1.y, dataView[i].V1.z);
			fprintf(fp, "V2 : (%f, %f)\n", dataView[i].V2.x, dataView[i].V2.y);
		}
	}
	D3D::GetDC()->Unmap(outputDebugBuffer, 0);

	fclose(fp);
}
