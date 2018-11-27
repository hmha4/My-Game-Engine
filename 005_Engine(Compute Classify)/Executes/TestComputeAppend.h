#pragma once
#include "Execute.h"

class TestComputeAppend : public Execute
{
public:
	TestComputeAppend();
	~TestComputeAppend();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	void ExecuteCS();

private:
	CsAppend * input;
	CsAppend * output;

private:
	UINT dataSize;

	Shader * shader;

private:
	struct Data
	{
		int Id = 0;
		int Val = 0;
		int Sum = 0;
	};
};


