#include "stdafx.h"
#include "TestTerrain.h"
#include "Environments/CubeSky.h"
#include "Environments/Terrain.h"
#include "Environments/HeightMap.h"

void TestTerrain::Initialize()
{
	sky = new CubeSky(Textures + L"sunsetcube1024.dds");

	//	Create Terrain
	{
		Terrain::InitDesc desc =
		{
			Textures + L"terrainTextures/terrain.raw", 50.0f, 2049.0f, 2049.0f,

			//	LayerMapFile
		{
			Textures + L"terrainTextures/grass.dds",
			Textures + L"terrainTextures/darkdirt.dds",
			Textures + L"terrainTextures/stone.dds",
			Textures + L"terrainTextures/lightdirt.dds",
			Textures + L"terrainTextures/snow.dds"
		},

			Textures + L"terrainTextures/blend.dds",

			0.5f,
		};

		terrain = new Terrain(desc);
		terrain->Initialize();
	}




	CollisionContext::Get()->AddLayer(L"Collision Weapon");
	CollisionContext::Get()->AddLayer(L"Collision Item");
	CollisionContext::Get()->AddLayer(L"Collision Player");
	CollisionContext::Get()->AddLayer(L"Collision Enemy");
	CollisionContext::Get()->AddLayer(L"Collision Character");
	CollisionContext::Get()->AddLayer(L"Collision World");
}

void TestTerrain::Ready()
{
	terrainMaterial = new Material(Effects + L"022_Terrain.fx");
	terrainMaterial->SetDiffuse(1, 1, 1);
	terrainMaterial->SetAmbient(1, 1, 1);
	terrainMaterial->SetSpecular(1, 1, 1);
	terrain->Ready(terrainMaterial);

	material = new Material(Effects + L"013_Mesh&Model.fx");
	material->SetAmbient(1, 1, 1);
	material->SetDiffuse(1, 1, 1);
	material->SetSpecular(0.9f, 0.9f, 0.9f, 16.0f);
	material->SetDiffuseMap(Textures + L"Stone.jpg");
	material->SetNormalMap(Textures + L"Stone_normal.jpg");
	material->SetSpecularMap(Textures + L"Stone_specular.jpg");

	obj = new MeshSphere(material, 5, 20, 20);
	obj->Position(50, 50, 50);
}

void TestTerrain::Destroy()
{
	SAFE_DELETE(sky);

	SAFE_DELETE(terrainMaterial);
	SAFE_DELETE(terrain);

	SAFE_DELETE(material);
	SAFE_DELETE(obj);
}

void TestTerrain::Update()
{
	sky->Update();

}

void TestTerrain::PreRender()
{
}

void TestTerrain::Render()
{
	sky->Render();
	terrain->Render();

	obj->Render();
}

void TestTerrain::PostRender()
{
}

void TestTerrain::ResizeScreen()
{
}
