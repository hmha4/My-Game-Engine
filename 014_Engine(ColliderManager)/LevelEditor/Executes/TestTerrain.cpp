#include "stdafx.h"
#include "TestTerrain.h"
#include "Environments/CubeSky.h"
#include "Environments/Terrain.h"
#include "Environments/HeightMap.h"
#include "Collider/ColliderBox.h"
#include "Collider/ColliderRay.h"
#include "Collider/ColliderSphere.h"

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

	player = new ColliderRay(L"Player Collider", D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 1, 0));
	//player = new ColliderSphere(L"Player Collider", D3DXVECTOR3(0, 0, 0), 0.5f);
	//player = new ColliderBox(L"Player Collider", D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(1, 1, 1));
	playerLine = new DebugLine();
	playerLine->Color(0, 1, 0);
	

	enemy = new ColliderRay(L"Enemy Collider", D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0.5f, 0.5f, 0));
	//enemy = new ColliderSphere(L"Enemy Collider", D3DXVECTOR3(0, 0, 0), 0.5f);
	//enemy = new ColliderBox(L"Enemy Collider", D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(1, 1, 1));
	enemyLine = new DebugLine();
	enemyLine->Color(0, 0, 1);
	

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

	D3DXMATRIX p, e;
	D3DXMatrixTranslation(&p, 50, 50, 50);
	D3DXMatrixTranslation(&e, 60, 50, 50);

	player->Transform(p);
	enemy->Transform(e);

	D3DXMatrixIdentity(&p);
	D3DXMatrixIdentity(&e);

	playerLine->Ready();
	playerLine->Draw(p, player->GetRay());
	enemyLine->Ready();
	enemyLine->Draw(e, enemy->GetRay());

	tempX = 60.0f;

	CollisionContext::Get()->GetLayer(L"Collision Player")->AddCollider(player);
	CollisionContext::Get()->GetLayer(L"Collision Enemy")->AddCollider(enemy);
}

void TestTerrain::Destroy()
{
	SAFE_DELETE(sky);

	SAFE_DELETE(terrainMaterial);
	SAFE_DELETE(terrain);

	SAFE_DELETE(player);
	SAFE_DELETE(playerLine);
	SAFE_DELETE(enemy);
	SAFE_DELETE(enemyLine);
}

void TestTerrain::Update()
{
	sky->Update();

	tempX -= Time::Delta();
	
	D3DXMatrixTranslation(&temp, tempX, 50, 50);
	enemy->Transform(temp);
	
	D3DXMatrixIdentity(&temp);
	enemyLine->Draw(temp, enemy->GetRay());

	CollisionResult * result = CollisionContext::Get()->HitTest(enemy, CollisionContext::Get()->GetLayer(L"Collision Player"), CollisionResult::ResultType::ENearestOne);
	if (result != NULL)
	{
		enemyLine->Color(1, 0, 0);
	}
	else
	{
		enemyLine->Color(0, 0, 1);
	}
}

void TestTerrain::PreRender()
{
}

void TestTerrain::Render()
{
	sky->Render();
	terrain->Render();

	playerLine->Render();
	enemyLine->Render();
}

void TestTerrain::PostRender()
{
}

void TestTerrain::ResizeScreen()
{
}
