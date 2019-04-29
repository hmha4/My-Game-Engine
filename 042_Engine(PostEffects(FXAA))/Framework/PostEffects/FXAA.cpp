#include "Framework.h"
#include "FXAA.h"

FXAA::FXAA(wstring effectFile)
{
	effect = new Effect(effectFile, true);

	D3DDesc desc;
	D3D::GetDesc(&desc);
	D3DXVECTOR4 rcp = D3DXVECTOR4(1.0f / desc.Width, 1.0f / desc.Height, 0.0f, 0.0f);
	effect->AsVector("RCPFrame")->SetFloatVector(rcp);
}

FXAA::~FXAA()
{
}

void FXAA::Render(bool use)
{
	effect->AsScalar("Use")->SetBool(use);
	
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ID3D11Buffer* nothing = 0;
	D3D::GetDC()->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	effect->Draw(0, 0, 4);
}

void FXAA::SetSRV(ID3D11ShaderResourceView * srv)
{
	effect->AsSRV("InputTexture")->SetResource(srv);
}
