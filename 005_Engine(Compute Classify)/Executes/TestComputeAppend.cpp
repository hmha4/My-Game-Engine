#include "stdafx.h"
#include "TestComputeAppend.h"
#include "./Environment/Terrain.h"

TestComputeAppend::TestComputeAppend()
	: dataSize(64)
{
	shader = new Shader(Effects + L"009_ComputeAppend.fx", true);

	vector<int> data;
	for (int i = 0; i < dataSize; i++)
		data.push_back(i);

	input = new CsAppend(sizeof(int), dataSize, &data[0]);
	output = new CsAppend(sizeof(Data), dataSize, NULL);

	ExecuteCS();
}

TestComputeAppend::~TestComputeAppend()
{
	SAFE_DELETE(shader);

	SAFE_DELETE(input);
	SAFE_DELETE(output);
}

void TestComputeAppend::Update()
{
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
	shader->AsUAV("Input")->SetUnorderedAccessView(input->UAV());
	shader->AsUAV("Output")->SetUnorderedAccessView(output->UAV());

	shader->Dispatch(0, 0, 1, 1, 1);

	vector<Data> dataView(dataSize);
	output->Read(&dataView[0]);

	FILE * file = fopen("Result2.txt", "wt");

	for (int i = 0; i < dataSize; i++)
	{
		fprintf
		(
			file, "%d, %d, %d\n",
			dataView[i].Id, dataView[i].Val, dataView[i].Sum
		);
	}


	fclose(file);
}
