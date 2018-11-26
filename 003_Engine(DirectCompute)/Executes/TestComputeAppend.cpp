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
		//	구조화 버퍼 : 구조체를 배열과 같이 사용할 수 있는 버퍼
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
		//	컴퓨트 셰이더의 계산 결과는 버퍼 리소스에 쓰여진다.
		//	그 때문에 계산 결과를 CPU측에서 받으려면
		//	버퍼가 CPU로부터 읽어들일 수 있어야한다.
		//	그러나 CPU로부터 읽어들일 수 있는 버퍼는 D3D11_USAGE_STAGING
		//	필드로 지정한 버퍼 밖에 없다.
		//	이 경우 스테이지로의 입출력에는 바인드 할 수 없다.

		//	이 때문에 컴퓨트 셰이더의 계산에 사용되는 것과는
		//	별도로 계산결과를 받을 버퍼를 D3D11_USAGE_STAGING 필드로
		//	지정하여 만들어 둔다.

		//	GPU에서 사용하는 버퍼로부터 CPU에서 읽어들일 수 있게 데이터를 카피 하려면
		//	1. 버퍼 전체를 카피하는 GetDC()->CopyResource() 함수와
		//	2. 버퍼 일부분을 카피하는 GetDC()->CopySubresourceRegion() 함수가 있다.

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

		//	컴퓨트 셰이더의 출력에는 UAV라는 형식의 뷰가 필요하다.
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
