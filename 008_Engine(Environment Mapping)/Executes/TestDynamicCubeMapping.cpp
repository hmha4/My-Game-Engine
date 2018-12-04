#include "stdafx.h"
#include "TestDynamicCubeMapping.h"
#include "../Environment/CubeSky.h"
#include "../Viewer/Perspective.h"

TestDynamicCubeMapping::TestDynamicCubeMapping()
{
	BuildCubeTexture(300, 300);
	BuildGeometry();
	BuildViewMatrix(0.0f, 3.48f, 0.0f);

	rotation = D3DXVECTOR3(0, 0, 0);
}

TestDynamicCubeMapping::~TestDynamicCubeMapping()
{
	SAFE_DELETE(plane);
	SAFE_DELETE(planeMaterial);

	SAFE_DELETE(box);
	SAFE_DELETE(boxMaterial);

	for (int i = 0; i < 10; i++)
		SAFE_DELETE(cylinder[i]);
	SAFE_DELETE(cylinderMaterial);

	for (int i = 0; i < 10; i++)
		SAFE_DELETE(sphere[i]);
	SAFE_DELETE(sphereMaterial);

	SAFE_DELETE(envMappedSphere);
	SAFE_DELETE(bunny);

	SAFE_DELETE(sky);

	SAFE_DELETE(dsv);
	SAFE_DELETE(viewport);

	SAFE_RELEASE(cubeTexture);
	SAFE_RELEASE(srv);
	for(int i = 0; i < 6; i++)
		SAFE_RELEASE(rtv[i]);

	SAFE_DELETE(perspective);
}

void TestDynamicCubeMapping::Update()
{
	sky->Update();

	D3DXVECTOR3 position = bunny->Position();

	rotation.y += 1.0f * Time::Delta();

	D3DXMATRIX R, T;
	D3DXMatrixRotationY(&R, rotation.y);
	D3DXMatrixTranslation(&T, 5.0f, position.y, 0.0f);

	D3DXMATRIX final = T * R;

	bunny->Position(final._41, final._42, final._43);
	bunny->Rotation(0, rotation.y, 0);
}

void TestDynamicCubeMapping::PreRender()
{
	for (int i = 0; i < 6; i++)
	{
		D3D::Get()->SetRenderTarget(rtv[i], dsv->DSV());
		D3D::Get()->Clear(D3DXCOLOR(1, 1, 1, 1), rtv[i], dsv->DSV());
		viewport->RSSetViewport();

		D3DXMATRIX p;
		perspective->GetMatrix(&p);
		Context::Get()->UpdateVP(view[i], p);

		sky->Render();

		plane->Render();
		box->Render();

		for (int i = 0; i < 10; i++)
			cylinder[i]->Render();

		for (int i = 0; i < 10; i++)
			sphere[i]->Render();

		bunny->Render();
	}
	
	D3D::GetDC()->GenerateMips(srv);
	Context::Get()->InitVP();
}

void TestDynamicCubeMapping::Render()
{
	sky->Render();

	plane->Render();
	box->Render();

	for (int i = 0; i < 10; i++)
		cylinder[i]->Render();

	for (int i = 0; i < 10; i++)
		sphere[i]->Render();

	envMappedSphere->Render();
	bunny->Render();
}

void TestDynamicCubeMapping::PostRender()
{
	
}

void TestDynamicCubeMapping::ResizeScreen()
{
}

void TestDynamicCubeMapping::BuildGeometry()
{
	//sky = new CubeSky(Textures + L"sunsetcube1024.dds");
	sky = new CubeSky(Textures + L"snowcube1024.dds");
	//sky = new CubeSky(Textures + L"grasscube1024.dds");
	//sky = new CubeSky(Textures + L"desertcube1024.dds");

	floor = Textures + L"Floor.png";
	stone = Textures + L"Stones.png";
	brick = Textures + L"Bricks.png";

	wstring shaderFile = Effects + L"013_Mesh&Model.fx";

	boxMaterial = new Material(shaderFile);
	boxMaterial->SetDiffuseMap(stone);
	boxMaterial->SetAmbient(1.0f, 1.0f, 1.0f);
	boxMaterial->SetDiffuse(1.0f, 1.0f, 1.0f);
	boxMaterial->SetSpecular(0.8f, 0.8f, 0.8f, 16.0f);

	box = new MeshCube(boxMaterial, 1, 1, 1);
	box->Position(0, 0.5f, 0);
	box->Scale(5.0f, 1.0f, 5.0f);


	planeMaterial = new Material(shaderFile);
	planeMaterial->SetDiffuseMap(floor);
	planeMaterial->SetAmbient(0.8f, 0.8f, 0.8f);
	planeMaterial->SetDiffuse(0.8f, 0.8f, 0.8f);
	planeMaterial->SetSpecular(0.0f, 0.0f, 0.0f, 1.0f);

	plane = new MeshGrid(planeMaterial, 60, 40, 20, 30, 3, 3);

	cylinderMaterial = new Material(shaderFile);
	cylinderMaterial->SetDiffuseMap(brick);
	cylinderMaterial->SetAmbient(1.0f, 1.0f, 1.0f);
	cylinderMaterial->SetDiffuse(1.0f, 1.0f, 1.0f);
	cylinderMaterial->SetSpecular(0.8f, 0.8f, 0.5f, 16.0f);

	sphereMaterial = new Material(shaderFile);
	sphereMaterial->SetDiffuseMap(stone);
	sphereMaterial->SetAmbient(0.6f, 0.8f, 0.9f);
	sphereMaterial->SetDiffuse(0.6f, 0.8f, 0.9f);
	sphereMaterial->SetSpecular(0.9f, 0.9f, 0.9f, 16.0f);

	for (UINT i = 0; i < 5; i++)
	{
		cylinder[i * 2] = new MeshCylinder(cylinderMaterial, 0.5f, 0.3f, 3.0f, 20, 20);
		cylinder[i * 2]->Position(-5.0f, 1.5f, -10.0f + i * 5.0f);

		cylinder[i * 2 + 1] = new MeshCylinder(cylinderMaterial, 0.5f, 0.3f, 3.0f, 20, 20);
		cylinder[i * 2 + 1]->Position(5.0f, 1.5f, -10.0f + i * 5.0f);


		sphere[i * 2] = new MeshSphere(sphereMaterial, 0.5f, 20, 20);
		sphere[i * 2]->Position(-5.0f, 3.5f, -10.0f + i * 5.0f);

		sphere[i * 2 + 1] = new MeshSphere(sphereMaterial, 0.5f, 20, 20);
		sphere[i * 2 + 1]->Position(5.0f, 3.5f, -10.0f + i * 5.0f);
	}

	envMappedSphere = new GameModel
	(
		Models + L"Sphere/", L"Sphere.material",
		Models + L"Sphere/", L"Sphere.mesh"
	);
	envMappedSphere->SetShader(Effects + L"016_DynamicCubMapping.fx");
	envMappedSphere->Position(0, 3.48f, 0);
	//bunny->Scale(0.01f, 0.01f, 0.01f);
	envMappedSphere->Scale(3, 3, 3);

	envMappedSphere->SetDiffuseMap(Textures + L"White.png");
	envMappedSphere->SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
	envMappedSphere->SetSpecular(0.8f, 0.8f, 0.8f, 16.0f);

	for (Material * material : envMappedSphere->GetModel()->Materials())
		material->GetShader()->AsShaderResource("CubeMap")->SetResource(srv);

	for (ModelMesh* mesh : envMappedSphere->GetModel()->Meshes())
		mesh->Pass(1);

	bunny = new GameModel
	(
		Models + L"Rabbit/", L"stanford-bunny.material",
		Models + L"Rabbit/", L"stanford-bunny.mesh"
	);
	bunny->SetShader(shaderFile);
	bunny->Position(0, 3.48f, 0);
	bunny->Scale(0.005f, 0.005f, 0.005f);

	bunny->SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
	bunny->SetSpecular(0.8f, 0.8f, 0.8f, 50.0f);

	for (ModelMesh* mesh : bunny->GetModel()->Meshes())
		mesh->Pass(1);

	SpotLightDesc spotLight;
	spotLight.Ambient = D3DXCOLOR(0, 0, 0, 1);
	spotLight.Diffuse = D3DXCOLOR(1, 0, 0, 1);
	spotLight.Specular = D3DXCOLOR(1, 0, 0, 1);
	spotLight.Attenuation = D3DXVECTOR3(1, 0, 0);
	spotLight.Spot = 20.0f;
	spotLight.Position = D3DXVECTOR3(0, 10, 0);
	spotLight.Direction = D3DXVECTOR3(0, -1, 0);

	Context::Get()->AddSpotLight(spotLight);
}

void TestDynamicCubeMapping::BuildCubeTexture(UINT width, UINT height)
{
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//	Create Texture2D
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 0;
		desc.ArraySize = 6;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Format = format;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &cubeTexture);
		assert(SUCCEEDED(hr));
	}

	//	Create RTVs(Render Target Views)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.MipSlice = 0;

		for (int i = 0; i < 6; i++)
		{
			desc.Texture2DArray.FirstArraySlice = i;
			HRESULT hr = D3D::GetDevice()->CreateRenderTargetView(cubeTexture, &desc, &rtv[i]);
			assert(SUCCEEDED(hr));
		}
	}

	//	Create SRV(Shader Resource View)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MostDetailedMip = 0;
		desc.TextureCube.MipLevels = -1;

		HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(cubeTexture, &desc, &srv);
		assert(SUCCEEDED(hr));
	}

	dsv = new DepthStencilView(width, height, DXGI_FORMAT_D32_FLOAT);
	viewport = new Viewport(width, height);
}

void TestDynamicCubeMapping::BuildViewMatrix(float x, float y, float z)
{
	D3DXVECTOR3 target[] =
	{
		D3DXVECTOR3(x + 1, y, z),
		D3DXVECTOR3(x - 1, y, z),
		D3DXVECTOR3(x, y + 1, z),
		D3DXVECTOR3(x, y - 1, z),
		D3DXVECTOR3(x, y, z + 1),
		D3DXVECTOR3(x, y, z - 1)
	};

	D3DXVECTOR3 up[] =
	{
		D3DXVECTOR3(0, 1, 0),
		D3DXVECTOR3(0, 1, 0),
		D3DXVECTOR3(0, 0, -1),
		D3DXVECTOR3(0, 0, 1),
		D3DXVECTOR3(0, 1, 0),
		D3DXVECTOR3(0, 1, 0),
	};

	D3DXVECTOR3 position(x, y, z);

	for (int i = 0; i < 6; i++)
		D3DXMatrixLookAtLH(&view[i], &position, &target[i], &up[i]);

	perspective = new Perspective(1, 1, Math::PI * 0.5f, 0.1f, 1000.0f);
}

