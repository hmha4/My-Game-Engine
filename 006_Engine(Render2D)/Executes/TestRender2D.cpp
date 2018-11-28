#include "stdafx.h"
#include "TestRender2D.h"
#include "../Environment/SimpleSky.h"

TestRender2D::TestRender2D()
{
	sky = new SimpleSky();

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


	quadMaterial = new Material(Effects + L"002_Mesh.fx");
	quadMaterial->SetDiffuseMap(floor);
	quadMaterial->SetAmbient(0.8f, 0.8f, 0.8f);
	quadMaterial->SetDiffuse(0.8f, 0.8f, 0.8f);
	quadMaterial->SetSpecular(0.0f, 0.0f, 0.0f, 1.0f);

	quad = new MeshQuad(quadMaterial);
	quad->Scale(5, 3, 1);
	quad->Position(0, 1.5f, -5.0f);


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

	bunny = new GameModel
	(
		Models + L"Rabbit/", L"stanford-bunny.material",
		Models + L"Rabbit/", L"stanford-bunny.mesh"
	);
	bunny->SetShader(shaderFile);
	bunny->Position(0, 3.48f, 0);
	bunny->Scale(0.01f, 0.01f, 0.01f);

	bunny->SetDiffuseMap(Textures + L"White.png");
	bunny->SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
	bunny->SetSpecular(0.8f, 0.8f, 0.8f, 16.0f);

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

	//Create RenderTarget
	{
		D3DDesc desc;
		D3D::GetDesc(&desc);

		float width = (float)desc.Width, height = (float)desc.Height;

		//	ÇØ»óµµ
		//width = 100;
		//height = 100;

		viewport = new Viewport(width, height);
		rtv = new RenderTargetView(width, height);
		dsv = new DepthStencilView(width, height);
	}

	render2D = new Render2D();
	render2D->Position(0, 0);
	render2D->Scale(500, 500);
	render2D->SRV(rtv->SRV());
}

TestRender2D::~TestRender2D()
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

	SAFE_DELETE(bunny);

	SAFE_DELETE(sky);

	SAFE_DELETE(quad);
	SAFE_DELETE(quadMaterial);

	SAFE_DELETE(rtv);
	SAFE_DELETE(dsv);
	SAFE_DELETE(viewport);

	SAFE_DELETE(render2D);
}

void TestRender2D::Update()
{
	sky->Update();
	bunny->Update();
}

void TestRender2D::PreRender()
{
	D3D::Get()->SetRenderTarget(rtv->RTV(), dsv->DSV());
	D3D::Get()->Clear(D3DXCOLOR(1, 1, 1, 1), rtv->RTV(), dsv->DSV());
	viewport->RSSetViewport();

	sky->Render();

	plane->Render();
	box->Render();

	for (int i = 0; i < 10; i++)
		cylinder[i]->Render();

	for (int i = 0; i < 10; i++)
		sphere[i]->Render();

	bunny->Render();
}

void TestRender2D::Render()
{
	sky->Render();

	plane->Render();
	box->Render();

	for (int i = 0; i < 10; i++)
		cylinder[i]->Render();

	for (int i = 0; i < 10; i++)
		sphere[i]->Render();


	bunny->Render();

	quadMaterial->GetShader()->AsShaderResource("DiffuseMap")->SetResource(rtv->SRV());
	quad->Render();

	render2D->Render();
}

void TestRender2D::PostRender()
{
	
}

void TestRender2D::ResizeScreen()
{
}

