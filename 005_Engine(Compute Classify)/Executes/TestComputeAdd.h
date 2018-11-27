#pragma once
#include "Execute.h"

class TestComputeAdd : public Execute
{
public:
	TestComputeAdd();
	~TestComputeAdd();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	void ExecuteCS();

private:
	CsResource * input[2];
	CsResource * output;
private:
	UINT dataSize;

	Shader * shader;

private:
	struct Data
	{
		D3DXVECTOR3 V1;
		D3DXVECTOR2 V2;
	};
};


