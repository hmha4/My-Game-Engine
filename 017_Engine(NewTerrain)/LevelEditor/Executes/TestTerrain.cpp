#include "stdafx.h"
#include "TestTerrain.h"
#include "Environments/CubeSky.h"
#include "Environments/Terrain.h"
#include "Environments/HeightMap.h"
#include "Environments/Shadow.h"
#include "Environments\Grass.h"

void TestTerrain::Initialize()
{
	CollisionContext::Get()->AddLayer(L"Collision Weapon");
	CollisionContext::Get()->AddLayer(L"Collision Item");
	CollisionContext::Get()->AddLayer(L"Collision Player");
	CollisionContext::Get()->AddLayer(L"Collision Enemy");
	CollisionContext::Get()->AddLayer(L"Collision Character");
	CollisionContext::Get()->AddLayer(L"Collision World");

	sky = new CubeSky(Textures + L"sunsetcube1024.dds");

	shadow = new Shadow(2048, 2048);
	shadow->Initialize();

	//터레인데이터 로드할때 방식
	//원래 이니셜라이져에 머터리얼 안넣는데 로드할때만 넣어주게해야함
	terrainMaterial = new Material(Effects + L"022_Terrain.fx");
	terrainMaterial->SetDiffuse(1, 1, 1);
	terrainMaterial->SetAmbient(1, 1, 1);
	terrainMaterial->SetSpecular(0.9f, 0.9f, 0.9f, 16.0f);
	Terrain::InitDesc desc;
	terrain = new Terrain(desc);
	terrain->UseLoad(Datas + L"Terrain0.terrain");
	terrain->Initialize(terrainMaterial);
}

void TestTerrain::Ready()
{
	terrain->Ready(terrainMaterial);

	shader = new Shader(Effects + L"013_Mesh&Model.fx");
	material = new Material(shader);
	material->SetAmbient(1, 1, 1);
	material->SetDiffuse(1, 1, 1);
	material->SetSpecular(0.9f, 0.9f, 0.9f, 16.0f);
	material->SetDiffuseMap(Textures + L"Stone.jpg");
	material->SetNormalMap(Textures + L"Stone_normal.jpg");
	material->SetSpecularMap(Textures + L"Stone_specular.jpg");

	obj = new MeshSphere(material, 5, 20, 20);
	obj->Position(55, 10, 55);

	obj1 = new MeshCube(material, 10, 10, 10);
	obj1->Position(45, 10, 45);

	shadow->Ready();
	shadow->Add(terrain);
	shadow->Add(obj);
	shadow->Add(obj1);
}

void TestTerrain::Destroy()
{
	SAFE_DELETE(sky);

	SAFE_DELETE(terrainMaterial);
	SAFE_DELETE(terrain);

	SAFE_DELETE(shader);
	SAFE_DELETE(material);
	SAFE_DELETE(obj1)
	SAFE_DELETE(obj);

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

	shadow->Render();
}

void TestTerrain::PostRender()
{
}

void TestTerrain::ResizeScreen()
{
}
