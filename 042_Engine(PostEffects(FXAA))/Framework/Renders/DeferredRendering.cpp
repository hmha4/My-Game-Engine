#include "Framework.h"
#include "DeferredRendering.h"
#include "PostEffects/FXAA.h"

DeferredRendering * DeferredRendering::instance = NULL;
void DeferredRendering::Create(float width, float height)
{
	if (instance != NULL)
		return;

	instance = new DeferredRendering(width, height);
}

void DeferredRendering::Delete()
{
	if (instance == NULL)
		return;

	SAFE_DELETE(instance);
}

DeferredRendering * DeferredRendering::Get()
{
	return instance;
}

DeferredRendering::DeferredRendering(float width, float height)
	: width(width), height(height)
{
	effect = new Material(Effects + L"034_Deferred.fx");

	CreateTextures();
	CreateRTV();
	CreateDSV(&shadowBuffer, &shadowDSV);
	CreateDSV(&depthBuffer, &depthDSV);
	CreateSRV();

	viewport = new Viewport(width, height);
	for (int i = 0; i < Count; i++)
	{
		render[i] = new Render2D();
		render[i]->Position(0, i * (height / 7));
		render[i]->Scale(width / 7, height / 7);
		render[i]->SRV(srv[i]);
	}

	effect->GetEffect()->AsSRV("PositionGB")->SetResource(srv[Position]);
	effect->GetEffect()->AsSRV("NormalGB")->SetResource(srv[Normal]);
	effect->GetEffect()->AsSRV("AlbedoGB")->SetResource(srv[Albedo]);
	effect->GetEffect()->AsSRV("SpecularGB")->SetResource(srv[Specular]);
	effect->GetEffect()->AsSRV("ShadowMap")->SetResource(srv[Shadow]);


	//	ÈÄÃ³¸®
	fxaa = new FXAA(Effects + L"042_FXAA.fx");
	fxaa->SetSRV(deferredSRV);
}

DeferredRendering::~DeferredRendering()
{
	SAFE_DELETE(effect);

	for (int i = 0; i < 4; i++)
		SAFE_RELEASE(rtv[i]);

	for (int i = 0; i < Count; i++)
	{
		SAFE_RELEASE(srv[i]);
		SAFE_DELETE(render[i]);
	}

	SAFE_RELEASE(shadowDSV);
	SAFE_RELEASE(shadowBuffer);

	SAFE_DELETE(viewport);
}

void DeferredRendering::SetPreRender()
{
	D3D::GetDC()->OMSetRenderTargets(4, rtv, depthDSV);
	viewport->RSSetViewport();

	D3D::GetDC()->ClearRenderTargetView(rtv[Position], D3DXCOLOR(0, 0, 0, 1));
	D3D::GetDC()->ClearRenderTargetView(rtv[Normal], D3DXCOLOR(0, 0, 0, 1));
	D3D::GetDC()->ClearRenderTargetView(rtv[Albedo], D3DXCOLOR(0, 0, 0, 1));
	D3D::GetDC()->ClearRenderTargetView(rtv[Specular], D3DXCOLOR(0, 0, 0, 1));

	D3D::GetDC()->ClearDepthStencilView(depthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DeferredRendering::SetRender()
{
	ID3D11DepthStencilView * dsv = NULL;
	D3D::GetDC()->OMSetRenderTargets(1, &deferredRTV, dsv);
	viewport->RSSetViewport();

	D3D::GetDC()->ClearRenderTargetView(deferredRTV, D3DXCOLOR(0, 0, 0, 1));
	//D3D::GetDC()->ClearDepthStencilView(depthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DeferredRendering::Render()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ID3D11Buffer* nothing = 0;
	D3D::GetDC()->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	effect->GetEffect()->Draw(0, 0, 4);

	if (Keyboard::Get()->Press(VK_F1))
	{
		for (int i = 0; i < Count; i++)
			render[i]->Render();
	}
}

void DeferredRendering::PostRender()
{
	if (Keyboard::Get()->Press(VK_F2))
		fxaa->Render(false);
	else
		fxaa->Render(true);
}

void DeferredRendering::ResizeScreen(float width, float height)
{
	for (int i = 0; i < 4; i++)
		SAFE_RELEASE(rtv[i]);

	for (int i = 0; i < Count; i++)
	{
		SAFE_RELEASE(srv[i]);
		SAFE_DELETE(render[i]);
	}

	SAFE_RELEASE(shadowDSV);
	SAFE_RELEASE(shadowBuffer);

	SAFE_DELETE(viewport);

	this->width = width;
	this->height = height;

	CreateTextures();
	CreateRTV();
	CreateDSV(&shadowBuffer, &shadowDSV);
	CreateDSV(&depthBuffer, &depthDSV);
	CreateSRV();

	viewport = new Viewport(width, height);
	for (int i = 0; i < Count; i++)
	{
		render[i] = new Render2D();
		render[i]->Position(0, i * (height / 7));
		render[i]->Scale(width / 7, height / 7);
		render[i]->SRV(srv[i]);
	}

	effect->GetEffect()->AsSRV("PositionGB")->SetResource(srv[Position]);
	effect->GetEffect()->AsSRV("NormalGB")->SetResource(srv[Normal]);
	effect->GetEffect()->AsSRV("AlbedoGB")->SetResource(srv[Albedo]);
	effect->GetEffect()->AsSRV("SpecularGB")->SetResource(srv[Specular]);
	effect->GetEffect()->AsSRV("ShadowMap")->SetResource(srv[Shadow]);
}

void DeferredRendering::CreateTextures()
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC desc = { 0 };
	//	Create 32bit Texture(Position, Normal)
	{
		desc.Width = (UINT)width;
		desc.Height = (UINT)height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture[Position]);
		assert(SUCCEEDED(hr));
		hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture[Normal]);
		assert(SUCCEEDED(hr));
		hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture[Specular]);
		assert(SUCCEEDED(hr));
	}

	//	Create 8bit Texture(Diffuse)
	{
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture[Albedo]);
		assert(SUCCEEDED(hr));
		
		hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &deferredTex);
		assert(SUCCEEDED(hr));
	}
}

void DeferredRendering::CreateRTV()
{
	HRESULT hr;

	D3D11_RENDER_TARGET_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

	//	Create 32bit RTV
	{
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		hr = D3D::GetDevice()->CreateRenderTargetView(texture[Position], &desc, &rtv[Position]);
		assert(SUCCEEDED(hr));
		hr = D3D::GetDevice()->CreateRenderTargetView(texture[Normal], &desc, &rtv[Normal]);
		assert(SUCCEEDED(hr));
		hr = D3D::GetDevice()->CreateRenderTargetView(texture[Specular], &desc, &rtv[Specular]);
		assert(SUCCEEDED(hr));
	}

	//	Create 8bit RTV
	{
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		hr = D3D::GetDevice()->CreateRenderTargetView(texture[Albedo], &desc, &rtv[Albedo]);
		assert(SUCCEEDED(hr));

		hr = D3D::GetDevice()->CreateRenderTargetView(deferredTex, &desc, &deferredRTV);
		assert(SUCCEEDED(hr));
	}
}

void DeferredRendering::CreateDSV(ID3D11Texture2D ** buffer, ID3D11DepthStencilView ** dsv)
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
	//	Create Depth Buffer
	{
		bufferDesc.Width = (UINT)width;
		bufferDesc.Height = (UINT)height;
		bufferDesc.MipLevels = 1;
		bufferDesc.ArraySize = 1;
		bufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		hr = D3D::GetDevice()->CreateTexture2D(&bufferDesc, NULL, buffer);
		assert(SUCCEEDED(hr));
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	//	Create DSV
	{
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		hr = D3D::GetDevice()->CreateDepthStencilView((*buffer), &desc, dsv);
	}
}

void DeferredRendering::CreateSRV()
{
	HRESULT hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;

	//	Create 32bit SRV
	{
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		
		hr = D3D::GetDevice()->CreateShaderResourceView(texture[Position], &desc, &srv[Position]);
		assert(SUCCEEDED(hr));
		hr = D3D::GetDevice()->CreateShaderResourceView(texture[Normal], &desc, &srv[Normal]);
		assert(SUCCEEDED(hr));
		hr = D3D::GetDevice()->CreateShaderResourceView(texture[Specular], &desc, &srv[Specular]);
		assert(SUCCEEDED(hr));
	}

	//	Create 8bit SRV
	{
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		hr = D3D::GetDevice()->CreateShaderResourceView(texture[Albedo], &desc, &srv[Albedo]);
		assert(SUCCEEDED(hr));
		hr = D3D::GetDevice()->CreateShaderResourceView(deferredTex, &desc, &deferredSRV);
		assert(SUCCEEDED(hr));
	}

	//	Create Depth SRV
	{
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		hr = D3D::GetDevice()->CreateShaderResourceView(shadowBuffer, &desc, &srv[Shadow]);
		assert(SUCCEEDED(hr));
		hr = D3D::GetDevice()->CreateShaderResourceView(depthBuffer, &desc, &srv[Depth]);
		assert(SUCCEEDED(hr));
	}

	
	for (int i = 0; i < 4; i++)
		SAFE_RELEASE(texture[i]);
}