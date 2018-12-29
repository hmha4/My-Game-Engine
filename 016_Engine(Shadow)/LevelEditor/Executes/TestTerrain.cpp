#include "stdafx.h"
#include "TestTerrain.h"
#include "Environments/CubeSky.h"
#include "Environments/Terrain.h"
#include "Environments/HeightMap.h"
#include "Environments/Shadow.h"

void TestTerrain::Initialize()
{
	sky = new CubeSky(Textures + L"sunsetcube1024.dds");

	////	Create Terrain
	//{
	//	Terrain::InitDesc desc =
	//	{
	//		Textures + L"terrainTextures/terrain.raw", 50.0f, 2049.0f, 2049.0f,

	//		//	LayerMapFile
	//	{
	//		Textures + L"terrainTextures/grass.dds",
	//		Textures + L"terrainTextures/darkdirt.dds",
	//		Textures + L"terrainTextures/stone.dds",
	//		Textures + L"terrainTextures/lightdirt.dds",
	//		Textures + L"terrainTextures/snow.dds"
	//	},

	//		Textures + L"terrainTextures/blend.dds",

	//		0.5f,
	//	};

	//	terrain = new Terrain(desc);
	//	terrain->Initialize();
	//}

	CollisionContext::Get()->AddLayer(L"Collision Weapon");
	CollisionContext::Get()->AddLayer(L"Collision Item");
	CollisionContext::Get()->AddLayer(L"Collision Player");
	CollisionContext::Get()->AddLayer(L"Collision Enemy");
	CollisionContext::Get()->AddLayer(L"Collision Character");
	CollisionContext::Get()->AddLayer(L"Collision World");

	shadow = new Shadow(2048, 2048);
	shadow->Initialize();
}

void TestTerrain::Ready()
{
	/*terrainShader = new Shader(Effects + L"022_Terrain.fx");
	terrainMaterial = new Material(terrainShader);
	terrainMaterial->SetDiffuse(1, 1, 1);
	terrainMaterial->SetAmbient(1, 1, 1);
	terrainMaterial->SetSpecular(1, 1, 1);
	terrain->Ready(terrainMaterial);*/

	shader = new Shader(Effects + L"013_Mesh&Model.fx");
	material = new Material(shader);
	material->SetAmbient(1, 1, 1);
	material->SetDiffuse(1, 1, 1);
	material->SetSpecular(0.9f, 0.9f, 0.9f, 16.0f);
	material->SetDiffuseMap(Textures + L"Box_06.png");
	material->SetNormalMap(Textures + L"Box_06_normal.png");
	material->SetSpecularMap(Textures + L"Box_06_specular.png");

	grid = new MeshGrid(material, 300, 300, 50, 50, 6, 6);
	grid->Position(50, 0, 50);

	//material = new Material(shader);
	material->SetAmbient(1, 1, 1);
	material->SetDiffuse(1, 1, 1);
	material->SetSpecular(0.9f, 0.9f, 0.9f, 16.0f);
	material->SetDiffuseMap(Textures + L"Stone.jpg");
	material->SetNormalMap(Textures + L"Stone_normal.jpg");
	material->SetSpecularMap(Textures + L"Stone_specular.jpg");

	obj = new MeshSphere(material, 5, 20, 20);
	obj->Position(55, 5, 55);

	obj1 = new MeshCube(material, 10, 10, 10);
	obj1->Position(45, 5, 45);

	shadow->Ready();
	shadow->Add(obj);
	shadow->Add(grid);
	shadow->Add(obj1);
	//shadow->Add(terrain);
}

void TestTerrain::Destroy()
{
	SAFE_DELETE(sky);

	/*SAFE_DELETE(terrainMaterial);
	SAFE_DELETE(terrain);*/

	SAFE_DELETE(shader);
	SAFE_DELETE(material);

	SAFE_DELETE(obj1)
	SAFE_DELETE(obj);
	SAFE_DELETE(grid);

	SAFE_DELETE(shadow);
}

void TestTerrain::Update()
{
	sky->Update();

	shadow->Update();
}

void TestTerrain::PreRender()
{
	shadow->PreRender();
}

void TestTerrain::Render()
{
	sky->Render();
	//terrain->Render();

	//obj->Render();

	shadow->Render();
}

void TestTerrain::PostRender()
{
}

void TestTerrain::ResizeScreen()
{
}
