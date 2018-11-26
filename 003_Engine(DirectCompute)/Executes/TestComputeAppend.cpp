#include "stdafx.h"
#include "TestComputeAppend.h"

TestComputeAppend::TestComputeAppend()
	: dataSize(16), timeStep(1.0f)
{
	shader = new Shader(Effects + L"009_Compute.fx", true);

	vector<Particle> data(dataSize);

	for (int i = 0; i < dataSize; i++)
	{
		data[i].Acceleration = D3DXVECTOR3
		(
			Math::Random(-0.5f, 0.5f),
			Math::Random(-0.5f, 0.5f),
			Math::Random(-0.5f, 0.5f)
		);
	}

	HRESULT hr;
	//	Create Input Buffer
	{
		D3D11_BUFFER_DESC inputDesc = { 0 };
		inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		inputDesc.ByteWidth = sizeof(Particle) * dataSize;
		inputDesc.CPUAccessFlags = 0;
		//	D3D11_RESOURCE_MISC_BUFFER_STRUCTURED
		//	����ȭ ���� : ����ü�� �迭�� ���� ����� �� �ִ� ����
		inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		inputDesc.StructureByteStride = sizeof(Particle);
		inputDesc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA data1 = { 0 };
		data1.pSysMem = &data[0];

		ID3D11Buffer *buffer1 = NULL;
		hr = D3D::GetDevice()->CreateBuffer(&inputDesc, &data1, &buffer1);
		assert(SUCCEEDED(hr));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.FirstElement = 0;
		srvDesc.BufferEx.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
		srvDesc.BufferEx.NumElements = dataSize;

		hr = D3D::GetDevice()->CreateShaderResourceView(buffer1, &srvDesc, &consume);
		assert(SUCCEEDED(hr));

		SAFE_RELEASE(buffer1);
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
		outputDesc.ByteWidth = sizeof(Particle) * dataSize;
		outputDesc.CPUAccessFlags = 0;
		outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		outputDesc.StructureByteStride = sizeof(Particle);
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

		hr = D3D::GetDevice()->CreateUnorderedAccessView(outputBuffer, &uavDesc, &append);
		assert(SUCCEEDED(hr));
	}
	ExecuteCS();
}

TestComputeAppend::~TestComputeAppend()
{
	SAFE_RELEASE(consume);
	SAFE_RELEASE(append);

	SAFE_RELEASE(outputBuffer);
	SAFE_RELEASE(outputDebugBuffer);

	SAFE_DELETE(shader);
}

void TestComputeAppend::Update()
{
	timeStep += Time::Delta();

	shader->AsScalar("TimeStep")->SetFloat(timeStep);
}

void TestComputeAppend::PreRender()
{
}

void TestComputeAppend::Render()
{
	
}

void TestComputeAppend::PostRender()
{

}

void TestComputeAppend::ResizeScreen()
{
}

void TestComputeAppend::ExecuteCS()
{
	shader->AsShaderResource("cInput")->SetResource(consume);
	shader->AsUAV("aOutput")->SetUnorderedAccessView(append);

	shader->Dispatch(0, 1, 16, 16, 1);

	FILE * fp = NULL;
	fopen_s(&fp, "Result2.txt", "wt");

	D3D::GetDC()->CopyResource(outputDebugBuffer, outputBuffer);

	D3D11_MAPPED_SUBRESOURCE mapped;
	D3D::GetDC()->Map(outputDebugBuffer, 0, D3D11_MAP_READ, 0, &mapped);
	{
		Particle * dataView = reinterpret_cast<Particle *>(mapped.pData);
		for (int i = 0; i < dataSize; i++)
		{
			fprintf(fp, "Position : (%f, %f, %f)\n", dataView[i].Position.x, dataView[i].Position.y, dataView[i].Position.z);
		}
	}
	D3D::GetDC()->Unmap(outputDebugBuffer, 0);

	fclose(fp);
}
