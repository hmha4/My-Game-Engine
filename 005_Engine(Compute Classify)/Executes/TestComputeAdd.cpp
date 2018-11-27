#include "stdafx.h"
#include "TestComputeAdd.h"

TestComputeAdd::TestComputeAdd()
	: dataSize(128)
{
	shader = new Shader(Effects + L"009_ComputeAdd.fx", true);

	vector<Data> dataA(dataSize);
	vector<Data> dataB(dataSize);

	for (int i = 0; i < dataSize; i++)
	{
		dataA[i].V1 = D3DXVECTOR3(i, i, i);
		dataA[i].V2 = D3DXVECTOR2(i, 0.0f);

		dataB[i].V1 = D3DXVECTOR3(-i, i, 0.0f);
		dataB[i].V2 = D3DXVECTOR2(i, -i);
	}

	input[0] = new CsResource(sizeof(Data), dataSize, &dataA[0]);
	input[1] = new CsResource(sizeof(Data), dataSize, &dataB[0]);
	output = new CsResource(sizeof(Data), dataSize, NULL);

	ExecuteCS();
}

TestComputeAdd::~TestComputeAdd()
{
	SAFE_DELETE(shader);

	SAFE_DELETE(input[0]);
	SAFE_DELETE(input[1]);
	SAFE_DELETE(output);
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
	shader->AsShaderResource("Input1")->SetResource(input[0]->SRV());
	shader->AsShaderResource("Input2")->SetResource(input[1]->SRV());
	shader->AsUAV("Output")->SetUnorderedAccessView(output->UAV());

	shader->Dispatch(0, 0, 2, 1, 1);

	vector<Data> dataView(dataSize);
	output->Read(&dataView[0]);

	FILE * file = fopen("Result.txt", "w");

	for (int i = 0; i < dataSize; i++)
	{
		fprintf(file, "V1 : (%f, %f, %f)", dataView[i].V1.x, dataView[i].V1.y, dataView[i].V1.z);
		fprintf(file, " - V2 : (%f, %f)\n", dataView[i].V2.x, dataView[i].V2.y);
	}

	fclose(file);
}
