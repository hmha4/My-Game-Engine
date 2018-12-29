#include "Framework.h"
#include "Shadow.h"
#include "IShadow.h"

Shadow::Shadow(UINT width, UINT height)
	: width(width), height(height)
{
	sceneBounds = new BSphere(D3DXVECTOR3(50, 0, 50), 100);

	
}

Shadow::~Shadow()
{
	SAFE_DELETE(sceneBounds);

	SAFE_DELETE(depthShadow);

	SAFE_DELETE(viewport);
	SAFE_DELETE(depthMapDSV);
	SAFE_DELETE(depthMapRTV);

	SAFE_RELEASE(depthMapSRV);

	SAFE_DELETE(normalShader);
	SAFE_DELETE(shadowShader);
}

void Shadow::Initialize()
{
	
}

void Shadow::Ready()
{
	normalShader = new Shader(Effects + L"013_Mesh&Model.fx");
	shadowShader = new Shader(Effects + L"023_BuildShadowMap.fx");

	viewport = new Viewport(width, height, 0.0f, 0.0f, 0.0f, 1.0f);

	depthShadow = new Render2D();
	depthShadow->Position(0, 0);
	depthShadow->Scale(200, 200);

	depthMapDSV = new DepthStencilView(width, height);
	depthMapRTV = new RenderTargetView(width, height);

	//	Create SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.MostDetailedMip = 0;

		D3D::GetDevice()->CreateShaderResourceView(depthMapDSV->Texture(), &desc, &depthMapSRV);
	}

	BuildShadowTransform();
}

void Shadow::Update()
{
	
}

void Shadow::PreRender()
{
	//D3D::Get()->SetRenderTarget(depthMapRTV->RTV(), depthMapDSV->DSV());
	//D3D::Get()->Clear(D3DXCOLOR(1, 1, 1, 1), depthMapRTV->RTV(), depthMapDSV->DSV());
	ID3D11RenderTargetView * rtv = NULL;
	D3D::GetDC()->OMSetRenderTargets(1, &rtv, depthMapDSV->DSV());
	D3D::GetDC()->ClearDepthStencilView(depthMapDSV->DSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	viewport->RSSetViewport();
	
	for (IShadow * obj : objs)
	{
		obj->SetShader(shadowShader, lightView, lightProjection, shadowTransform, depthMapSRV);
		obj->ShadowUpdate();
		obj->ShadowRender();
	}
}

void Shadow::Render()
{
	for (IShadow * obj : objs)
	{
		obj->SetShader(normalShader, lightView, lightProjection, shadowTransform, depthMapSRV);
		obj->ShadowUpdate();
		obj->ShadowRender();
	}

	depthShadow->SRV(depthMapSRV);
	depthShadow->Update();
	depthShadow->Render();

	if(ImGui::DragFloat3("direction", (float*)&Context::Get()->GetGlobalLight()->Direction, 0.01f, -1, 1))
		BuildShadowTransform();

	Context::Get()->ChangeGlobalLight();
}

void Shadow::Delete()
{
}

void Shadow::Add(IShadow * shadow)
{
	objs.push_back(shadow);
	D3DXMATRIX temp;
	D3DXMatrixIdentity(&temp);
	shadow->SetShader(normalShader, temp, temp, shadowTransform, depthMapSRV);
}

void Shadow::BuildShadowTransform()
{
	D3DXVECTOR3 lightDir = Context::Get()->GetGlobalLight()->Direction;
	D3DXVECTOR3 lightPos = -2.0f * sceneBounds->Radius * lightDir;
	D3DXVECTOR3 targetPos = sceneBounds->Center;
	D3DXVECTOR3 up = D3DXVECTOR3(0, 1, 0);

	//	View
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &lightPos, &targetPos, &up);

	//	중심정을 LS(Light Space)로 변환
	D3DXVECTOR3 sphereCenterLS;
	D3DXVec3TransformCoord(&sphereCenterLS, &targetPos, &V);

	float l = sphereCenterLS.x - sceneBounds->Radius;
	float r = sphereCenterLS.x + sceneBounds->Radius;
	float b = sphereCenterLS.y - sceneBounds->Radius;
	float t = sphereCenterLS.y + sceneBounds->Radius;
	float zn = sphereCenterLS.z - sceneBounds->Radius;
	float zf = sphereCenterLS.z + sceneBounds->Radius;

	//	Perspective
	D3DXMATRIX P;
	D3DXMatrixOrthoOffCenterLH(&P, l, r, b, t, zn, zf);

	//	Texture Space;
	//	NDC -> UV
	//	[-1,+1], [-1,+1], [-1, +1] ==>  [0,1], [0,1]
	D3DXMATRIX T;
	D3DXMatrixIdentity(&T);
	T._11 = 0.5f; T._22 = -0.5f; T._33 = 1.0f;
	T._41 = 0.5f; T._42 = 0.5f; T._44 = 1.0f;

	D3DXMATRIX S = V * P * T;

	lightView = V;
	lightProjection = P;
	shadowTransform = S;
}
