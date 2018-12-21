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
}

void TestTerrain::Ready()
{
	terrainMaterial = new Material(Effects + L"022_Terrain.fx");
	terrainMaterial->SetDiffuse(1, 1, 1);
	terrainMaterial->SetAmbient(1, 1, 1);
	terrainMaterial->SetSpecular(1, 1, 1);
	terrain->Ready(terrainMaterial);
}

void TestTerrain::Destroy()
{
	SAFE_DELETE(sky);

	SAFE_DELETE(terrainMaterial);
	SAFE_DELETE(terrain);
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
}

void TestTerrain::PostRender()
{
}

void TestTerrain::ResizeScreen()
{
}
