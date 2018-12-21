#include "stdafx.h"
#include "TessTerrain.h"
#include "Environment/CubeSky.h"
#include "Environment/Terrain.h"
#include "Environment\HeightMap.h"

TessTerrain::TessTerrain()
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

		desc.material = new Material(Effects + L"022_Terrain.fx");

		terrain = new Terrain(desc);
	}
}

TessTerrain::~TessTerrain()
{
	SAFE_DELETE(sky);
	SAFE_DELETE(terrain);
}

void TessTerrain::Update()
{
	sky->Update();
}

void TessTerrain::PreRender()
{

}

void TessTerrain::Render()
{
	sky->Render();
	terrain->Render();
}

void TessTerrain::PostRender()
{

}

void TessTerrain::ResizeScreen()
{

}
