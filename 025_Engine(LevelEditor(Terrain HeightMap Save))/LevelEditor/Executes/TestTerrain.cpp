#include "stdafx.h"
#include "TestTerrain.h"
#include "Environments/ScatterSky.h"
#include "Environments/Terrain.h"
#include "Environments/HeightMap.h"
#include "Environments/Shadow.h"
#include "Environments\Grass.h"

void TestTerrain::Initialize()
{
	GameSceneSpec * sceneSpec = (GameSceneSpec*)GameDataSpecManager::Get()->LoadScene(L"Scene.spec");

	sky = new ScatterSky();
	sky->Load(sceneSpec->SkySpec);
	sky->Initialize();

	shadow = new Shadow(2048, 2048);
	shadow->Initialize();

	Terrain::InitDesc desc;
	terrain = new Terrain(desc);
	terrain->Initialize(sky);
	terrain->Load(sceneSpec->TerrainSpec);
}

void TestTerrain::Ready()
{
	terrain->Ready();
	sky->Ready();

	//shader = new Shader(Effects + L"013_Mesh&Model.fx");
	//material = new Material(Effects + L"013_Mesh&Model.fx");
	//material->SetAmbient(1, 1, 1);
	//material->SetDiffuse(1, 1, 1);
	//material->SetSpecular(0.9f, 0.9f, 0.9f, 16.0f);

	//obj = new MeshSphere(material, 5, 20, 20);
	//obj->Position(55, 10, 55);
	//material->SetDiffuseMap(Textures + L"Stone.jpg");
	//material->SetNormalMap(Textures + L"Stone_normal.jpg");
	//material->SetSpecularMap(Textures + L"Stone_specular.jpg");
	//
	//
	//obj1 = new MeshCube(material, 10, 10, 10);
	//obj1->Position(45, 10, 45);
	//material->SetDiffuseMap(Textures + L"Box_06.png");
	//material->SetNormalMap(Textures + L"Box_06_normal.png");
	//material->SetSpecularMap(Textures + L"Box_06_specular.png");

	//grass = new Grass(terrain);
	//grass->GenerateGrass(D3DXVECTOR3(50, 0, 50), 100);
	//grass->GenerateGrass(D3DXVECTOR3(0, 0, 0), 50, 5, 10);
	//grass->GenerateFlower(5, D3DXVECTOR3(50, 0, 50), 5);
	//grass->GenerateFlower(50, D3DXVECTOR3(0, 0, 0), 50);

	shadow->Ready();
	shadow->Add(terrain);
	shadow->Add(terrain->GetGrass());
	//shadow->Add(obj);
	//shadow->Add(obj1);
}

void TestTerrain::Destroy()
{
	SAFE_DELETE(sky);

	//SAFE_DELETE(terrainMaterial);
	SAFE_DELETE(terrain);

	//SAFE_DELETE(shader);
	//SAFE_DELETE(material);
	//SAFE_DELETE(obj1)
	//SAFE_DELETE(obj);

	SAFE_DELETE(grass);

	SAFE_DELETE(shadow);
}

void TestTerrain::Update()
{
	sky->Update();

	shadow->Update();
}

void TestTerrain::PreRender()
{
	sky->PreRender();

	shadow->PreRender();
}

void TestTerrain::Render()
{
	sky->Render();

	shadow->Render();

	MyGui::ShowSkySetting(sky);
}

void TestTerrain::PostRender()
{
}

void TestTerrain::ResizeScreen()
{
}