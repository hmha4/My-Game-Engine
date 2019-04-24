#include "Framework.h"
#include "CubeSky.h"
#include "../Viewer/Camera.h"

CubeSky::CubeSky(wstring cubeFile)
{
	material = new Material(Effects + L"014_CubeSky.fx");
	sphere = new MeshSphere(material, 500.0f, 30, 30);

	HRESULT hr = D3DX11CreateShaderResourceViewFromFile
	(
		D3D::GetDevice(), cubeFile.c_str(), NULL, NULL, &srv, NULL
	);
	assert(SUCCEEDED(hr));

	material->GetEffect()->AsSRV("CubeMap")->SetResource(srv);
}

CubeSky::~CubeSky()
{
	SAFE_DELETE(material);
	SAFE_DELETE(sphere);

	SAFE_RELEASE(srv);
}

void CubeSky::Update()
{
	D3DXVECTOR3 position;
	Context::Get()->GetMainCamera()->Position(&position);

	sphere->Position(position);
}

void CubeSky::Render()
{
	sphere->Render();
}