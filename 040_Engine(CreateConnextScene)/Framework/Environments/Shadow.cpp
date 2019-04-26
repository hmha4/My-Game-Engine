#include "Framework.h"
#include "Shadow.h"
#include "IShadow.h"

Shadow::Shadow(UINT width, UINT height)
	: width(width), height(height)
	, depthMapDSV(NULL), depthMapSRV(NULL)
{
}

Shadow::~Shadow()
{
	SAFE_DELETE(sceneBounds);
	SAFE_DELETE(viewport);

	SAFE_DELETE(depthMapDSV);
	SAFE_RELEASE(depthMapSRV);
}

void Shadow::Initialize(float x, float y, float z, float radius)
{
	sceneBounds = new BSphere(D3DXVECTOR3(x, y, z), radius);
	BuildShadowTransform();
}

void Shadow::Ready()
{
	viewport = new Viewport((float)width, (float)height, 0.0f, 0.0f, 0.0f, 1.0f);

	if (DEFERRED == false)
	{
		depthMapDSV = new DepthStencilView(width, height, DXGI_FORMAT_R24G8_TYPELESS);

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
	}
}

void Shadow::Update()
{
	BuildShadowTransform();
}

void Shadow::PreRender()
{
	ID3D11RenderTargetView * rtv = NULL;
	if (DEFERRED == true)
	{
		D3D::GetDC()->OMSetRenderTargets(1, &rtv, DeferredRendering::Get()->ShadowDSV());
		D3D::GetDC()->ClearDepthStencilView(DeferredRendering::Get()->ShadowDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
	else
	{
		D3D::GetDC()->OMSetRenderTargets(1, &rtv, depthMapDSV->DSV());
		D3D::GetDC()->ClearDepthStencilView(depthMapDSV->DSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	viewport->RSSetViewport();

	for (IShadow * obj : objs)
	{
		obj->SetEffect(lightView, lightProjection);
		obj->ShadowRender(1, 0);
	}

	InstanceManager::Get()->Render(1);
}

void Shadow::Render()
{
	for (IShadow * obj : objs)
	{
		obj->SetEffect(shadowTransform);
		obj->ShadowUpdate();
		if (DEFERRED == true)
			obj->ShadowRender(2, 0);
		else
			obj->ShadowRender(0, 0);
	}

	if (DEFERRED == true)
	{
		InstanceManager::Get()->Render(2);
		DeferredRendering::Get()->GetEffect()->AsMatrix("ShadowTransform")->SetMatrix(shadowTransform);
	}
	else
		InstanceManager::Get()->Render(0);
}

void Shadow::ResizeScreen()
{
	SAFE_DELETE(viewport);
	
	
	D3DDesc desc;
	D3D::GetDesc(&desc);
	
	viewport = new Viewport((float)desc.Width, (float)desc.Height, 0.0f, 0.0f, 0.0f, 1.0f);
	
	if (DEFERRED == false)
	{
		SAFE_DELETE(depthMapDSV);
		SAFE_RELEASE(depthMapSRV);
	
		depthMapDSV = new DepthStencilView((UINT)desc.Width, (UINT)desc.Height, DXGI_FORMAT_R24G8_TYPELESS);
	
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
	}
}

void Shadow::Add(IShadow * shadow)
{
	objs.push_back(shadow);

	if(DEFERRED == false)
		shadow->SetEffect(shadowTransform, depthMapSRV);
}

void Shadow::Remove(UINT index)
{
	for (size_t i = 0; i < objs.size(); i++)
	{
		if (i == index)
			objs.erase(objs.begin() + i);
	}
}

void Shadow::RemoveAll()
{
	objs.clear();
	objs.shrink_to_fit();
}

void Shadow::BuildShadowTransform()
{
	D3DXVECTOR3 lightDir = Context::Get()->GetGlobalLight()->Direction;
	if (lightDir.y >= 0.0f)
		lightDir = lightDir * -1.0f;

	D3DXVECTOR3 lightPos = -2.0f * sceneBounds->Radius * lightDir;

	//Context::Get()->GetMainCamera()->Position(&sceneBounds->Center);
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