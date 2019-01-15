#pragma once
#include "Systems/IExecute.h"

class TestByteAddress : public IExecute
{
public:
	void Initialize() override;
	void Ready() override;
	void Destroy() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	void ResizeScreen() override;

private:
	ModelVertexType data[128];

	Effect * effect;

	ID3D11Buffer * input;
	ID3D11ShaderResourceView * inputSRV;

	ID3D11Buffer * vertexBuffer;
	ID3D11UnorderedAccessView * outputUAV;
};