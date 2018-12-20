#include "Framework.h"
#include "Terrain.h"
#include "HeightMap.h"
#include "TerrainRenderer.h"

const UINT Terrain::CellsPerPatch = 64;

Terrain::Terrain(InitDesc& desc)
	: desc(desc)
{
	renderer = new TerrainRenderer(desc.material, this);

	heightMap = new HeightMap((UINT)desc.HeightMapWidth, (UINT)desc.HeightMapHeight, desc.HeightScale);
	heightMap->Load(desc.HeightMap);

	renderer->Initialize();
}

Terrain::~Terrain()
{
	SAFE_DELETE(heightMap);
	SAFE_DELETE(renderer);
}

void Terrain::Update()
{

}

void Terrain::Render()
{
	renderer->Render();
}

void Terrain::Data(UINT row, UINT col, float data)
{
	heightMap->Data(row, col, data);
}

float Terrain::Data(UINT row, UINT col)
{
	return heightMap->Data(row, col);
}

float Terrain::Width()
{
	return (desc.HeightMapWidth - 1) * desc.CellSpacing;
}

float Terrain::Depth()
{
	return (desc.HeightMapHeight - 1) * desc.CellSpacing;
}
