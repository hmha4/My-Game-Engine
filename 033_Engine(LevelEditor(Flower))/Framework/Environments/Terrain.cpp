#include "Framework.h"
#include "Terrain.h"
#include "HeightMap.h"
#include "TerrainRenderer.h"
#include "Billboard.h"
#include "Systems/Window.h"
#include "Grass.h"
#include "Flower.h"
#include "Utilities/Xml.h"
#include "Shadow.h"

const UINT Terrain::CellsPerPatch = 64;

Terrain::Terrain(InitDesc & desc)
	: desc(desc)
	, isPick(false)
	, terrainName(L"")
	, render(NULL)
	, heightMap(NULL)
	, loadDataFile(L"")
{
	for (int i = 0; i < 4; i++)
		materialValueFile[i] = L"";
}

Terrain::~Terrain()
{
	SAFE_DELETE(billboard);
	SAFE_DELETE(grass);
	SAFE_DELETE(flower);

	SAFE_DELETE(render);
	SAFE_DELETE(heightMap);
	SAFE_DELETE(desc.material);
}

void Terrain::Initialize(ScatterSky * sky)
{
	desc.material = new Material(Effects + L"022_Terrain.fx");
	brushTypeVar = desc.material->GetEffect()->AsScalar("Type");
	brushLocationVar = desc.material->GetEffect()->AsVector("Location");
	brushColorVar = desc.material->GetEffect()->AsVector("Color");
	brushRangeVar = desc.material->GetEffect()->AsScalar("Range");
	lineVar = desc.material->GetEffect()->AsScalar("LineType");
	lineColorVar = desc.material->GetEffect()->AsVector("LineColor");
	spacingVar = desc.material->GetEffect()->AsScalar("Spacing");
	thicknessVar = desc.material->GetEffect()->AsScalar("Thickness");
	lightViewVar = desc.material->GetEffect()->AsMatrix("LightView");
	lightProjVar = desc.material->GetEffect()->AsMatrix("LightProjection");
	shadowTransformVar = desc.material->GetEffect()->AsMatrix("ShadowTransform");
	shadowMapVar = desc.material->GetEffect()->AsSRV("ShadowMap");

	if (render == NULL)
		render = new TerrainRenderer(this);

	if (desc.HeightMap != L"")
	{
		if (heightMap == NULL)
			heightMap = new HeightMap((UINT)desc.HeightMapWidth, (UINT)desc.HeightMapHeight, desc.HeightScale);

		heightMap->Load(desc.HeightMap);
		render->Initialize();
	}

	render->GetSky(sky);
	grass = new Grass(this);
	flower = new Flower(this);
	billboard = new Billboard(this);
	billboard->Initalize();
}

void Terrain::Ready()
{
	if (materialValueFile[0] != L"")
		desc.material->SetDiffuseMap(materialValueFile[0]);

	if (materialValueFile[1] != L"")
		desc.material->SetSpecularMap(materialValueFile[1]);

	if (materialValueFile[2] != L"")
		desc.material->SetNormalMap(materialValueFile[2]);

	if (materialValueFile[3] != L"")
		desc.material->SetDetailMap(materialValueFile[3]);

	render->Ready(desc.material);

	grass->Ready();
	flower->Ready();
	billboard->Ready();
}

void Terrain::Update()
{
}

void Terrain::Render()
{
	render->Render();
}

void Terrain::LineUpdate()
{
	lineVar->SetInt(tline.LineType);
	lineColorVar->SetFloatVector(tline.LineColor);
	spacingVar->SetInt(tline.Spacing);
	thicknessVar->SetFloat(tline.Thickness);
}

void Terrain::BrushUpdate(D3DXVECTOR3 position)
{
	brushTypeVar->SetInt(brush.Type);
	brushLocationVar->SetFloatVector(position);
	brushColorVar->SetFloatVector(brush.Color);
	brushRangeVar->SetInt(brush.Range);
}

void Terrain::SaveHeightMap(Effect * effect, ID3D11Buffer ** buffer, ID3D11UnorderedAccessView ** uav)
{
	float * dest = new float[(UINT)(desc.HeightMapWidth * desc.HeightMapHeight)];
	CsResource::CreateStructuredBuffer
	(
		sizeof(float), (UINT)(desc.HeightMapWidth * desc.HeightMapHeight),
		dest, buffer, false, true
	);

	CsResource::CreateUAV(*buffer, uav);

	effect->AsSRV("Input")->SetResource(render->GetHeightMapSRV());
	effect->AsUAV("Output")->SetUnorderedAccessView(*uav);
	effect->Dispatch(0, 0, 70, 70, 1);

	ID3D11Buffer * temp = CsResource::CreateAndCopyBuffer(*buffer);
	D3D11_MAPPED_SUBRESOURCE resource;
	D3D::GetDC()->Map(temp, 0, D3D11_MAP_READ, 0, &resource);
	{
		memcpy(heightMap->Datas(), resource.pData, sizeof(float) * (size_t)(desc.HeightMapWidth * desc.HeightMapHeight));
	}
	D3D::GetDC()->Unmap(temp, 0);

	heightMap->Save(Textures + L"HeightMap/MyTerrain.raw");

	SAFE_DELETE_ARRAY(dest);
	SAFE_RELEASE(temp);
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

float Terrain::GetHeight(float x, float z)
{
	float c = (x + 0.5f * Width()) / desc.CellSpacing;
	float d = (z - 0.5f * Depth()) / -desc.CellSpacing;
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	float h00 = heightMap->Data(row, col);
	float h01 = heightMap->Data(row, col + 1);
	float h10 = heightMap->Data(row + 1, col);
	float h11 = heightMap->Data(row + 1, col + 1);

	float s = c - col;
	float t = d - row;

	float uy, vy;
	if (s + t <= 1.0f)
	{
		uy = h01 - h00;
		vy = h01 - h11;

		return h00 + (1.0f - s) * uy + (1.0f - t) * vy;
	}

	uy = h10 - h11;
	vy = h01 - h11;

	return h11 + (1.0f - s) * uy + (1.0f - t) * vy;
}

bool Terrain::Picking(OUT D3DXVECTOR3 * pickPixel, OUT D3DXVECTOR3 * pickWorldPos)
{
	float x, z;
	D3DXVECTOR3 position = Mouse::Get()->GetPosition();
	D3DXVECTOR4 result = Texture::ReadPixel128(render->GetRenderTargetTexture(), (UINT)position.x, (UINT)position.y);
	x = result.x;
	z = result.y;

	if (pickPixel != NULL)
	{
		pickPixel->x = x * (desc.HeightMapWidth - 1.0f);
		pickPixel->z = z * (desc.HeightMapHeight - 1.0f);
	}

	if (pickWorldPos != NULL)
	{
		pickWorldPos->x = x * Width() - (Width() * 0.5f);
		pickWorldPos->z = z * Depth() - (Depth() * 0.5f);
	}

	return x + z > 0;
}

void Terrain::SetShadows(Shadow * shadow)
{
	shadow->Add(this);
	shadow->Add(grass);
	shadow->Add(flower);
	shadow->Add(billboard);
}

bool Terrain::IsPicked()
{
	return isPick;
}

void Terrain::SetPickState(bool val)
{
	isPick = val;
}

void Terrain::SetEffect(D3DXMATRIX v, D3DXMATRIX p)
{
	lightViewVar->SetMatrix(v);
	lightProjVar->SetMatrix(p);
}

void Terrain::SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv)
{
	shadowTransformVar->SetMatrix(shadowTransform);

	if (srv != NULL)
		shadowMapVar->SetResource(srv);
}

void Terrain::ShadowUpdate()
{
}

void Terrain::NormalRender()
{
	render->Render();
}

void Terrain::ShadowRender(UINT tech, UINT pass)
{
	if (tech == 0)
		NormalRender();
	else
		render->RenderShadow(tech, pass);
}

wstring & Terrain::Name()
{
	return terrainName;
}

wstring & Terrain::Tag()
{
	return tag;
}

void Terrain::Load(wstring fileName)
{
	GameTerrainSpec * spec = NULL;

	if (loadDataFile == fileName)
		spec = (GameTerrainSpec*)GameDataSpecManager::Get()->Find(loadDataFile);
	else
		spec = (GameTerrainSpec*)GameDataSpecManager::Get()->LoadTerrain(fileName);

	desc.HeightMap = spec->HeightMap;
	desc.HeightScale = spec->HeightScale;
	desc.HeightMapWidth = spec->HeightMapWidth;
	desc.HeightMapHeight = spec->HeightMapHeight;
	for (int i = 0; i < 3; i++)
		desc.layerMapFile[i] = spec->LayerMapFile[i];
	desc.blendHeight = spec->BlendHeight;
	desc.activeBlend = spec->ActiveBlend;
	detail.DetailValue = spec->DetailValue;
	detail.DetailIntensity = spec->DetailIntensity;
	desc.material->SetAmbient(spec->AmbientColor);
	for (int i = 0; i < 4; i++)
		materialValueFile[i] = spec->MaterialValueFile[i];
	desc.material->SetDiffuse(spec->DiffuseColor);
	D3DXCOLOR specular = spec->SpecularColor;
	specular.a = spec->Shininess;
	desc.material->SetSpecular(specular);
	render->GetBuffer().TexScale = spec->TexScale;
	render->GetBuffer().FogStart = spec->FogStart;
	render->GetBuffer().FogRange = spec->FogRange;

	if (heightMap == NULL)
		heightMap = new HeightMap((UINT)desc.HeightMapWidth, (UINT)desc.HeightMapHeight, desc.HeightScale);

	heightMap->Load(desc.HeightMap);
	render->Initialize();

	grass->GetWind()->Velocity() = spec->WindVelocity;
	grass->GrassCount() = spec->GrassCount;
	grass->CreateVector();

	for (UINT i = 0; i < spec->GrassCount; i++)
	{
		grass->Grasses()[i]->DiffuseMap = spec->grassDesc[i].GrassDiffuseMap;
		grass->Grasses()[i]->NormalMap = spec->grassDesc[i].GrassNormalMap;
		grass->Grasses()[i]->SpecularMap = spec->grassDesc[i].GrassSpecularMap;
		grass->Grasses()[i]->DetailMap = spec->grassDesc[i].GrassDetailMap;
		grass->Grasses()[i]->size = spec->grassDesc[i].GrassSize;
		grass->Grasses()[i]->position = spec->grassDesc[i].GrassPosition;
		grass->Grasses()[i]->numOfPatchRows = spec->grassDesc[i].NumOfPatchRows;
		grass->Grasses()[i]->numOfRootsInPatch = spec->grassDesc[i].NumOfRootsInPatch;
		grass->Grasses()[i]->numOfRoots = spec->grassDesc[i].NumOfRoots;
		grass->Grasses()[i]->isFlower = (bool)spec->grassDesc[i].IsFlower;
		grass->Grasses()[i]->flowerNum = spec->grassDesc[i].FlowerNum;
	}

	//TODO: add flower
	{
		flower->GetWind()->Velocity() = spec->WindVelocity;
		flower->FlowerCount() = spec->FlowerCount;
		flower->CreateVector();

		for (UINT i = 0; i < spec->FlowerCount; i++)
		{
			flower->Flowers()[i]->DiffuseMap = spec->flowerDesc[i].FlowerDiffuseMap;
			flower->Flowers()[i]->FlowerNum = spec->flowerDesc[i].FlowerNum;
			flower->Flowers()[i]->Size = spec->flowerDesc[i].FlowerSize;
			flower->Flowers()[i]->Position = spec->flowerDesc[i].FlowerPosition;
			flower->Flowers()[i]->RootsNum = spec->flowerDesc[i].NumOfRoots;
		}
	}

	billboard->CreateVector(spec->BillboardCount);
	for (UINT i = 0; i < spec->BillboardCount; i++)
	{
		billboard->GetDesc()[i].TransPosition = spec->billDesc[i].Position;
		billboard->GetDesc()[i].Size = spec->billDesc[i].Size;
		billboard->GetDesc()[i].TextureNumber = spec->billDesc[i].TextureNumber;
		billboard->GetDesc()[i].Type = spec->billDesc[i].BillType;
	}

	loadDataFile = fileName;
}

void Terrain::Save()
{
	GameTerrainSpec * spec = new GameTerrainSpec();
	spec->HeightMap = desc.HeightMap;
	spec->HeightScale = desc.HeightScale;
	spec->HeightMapWidth = desc.HeightMapWidth;
	spec->HeightMapHeight = desc.HeightMapHeight;
	for (int i = 0; i < 3; i++)
		spec->LayerMapFile[i] = desc.layerMapFile[i];
	spec->BlendHeight = desc.blendHeight;
	spec->ActiveBlend = desc.activeBlend;
	spec->DetailValue = detail.DetailValue;
	spec->DetailIntensity = detail.DetailIntensity;
	spec->AmbientColor = desc.material->GetAmbient();
	for (int i = 0; i < 4; i++)
		spec->MaterialValueFile[i] = materialValueFile[i];
	spec->DiffuseColor = desc.material->GetDiffuse();
	spec->SpecularColor = desc.material->GetSpecular();
	spec->Shininess = desc.material->GetSpecular().a;
	spec->TexScale = render->GetBuffer().TexScale;
	spec->FogStart = render->GetBuffer().FogStart;
	spec->FogRange = render->GetBuffer().FogRange;

	//	Grass
	spec->WindVelocity = grass->GetWind()->Velocity();
	spec->GrassCount = grass->GrassCount();
	for (UINT i = 0; i < spec->GrassCount; i++)
	{
		GameTerrainSpec::GrassDesc desc;
		desc.GrassDiffuseMap = grass->Grasses()[i]->material->GetDiffuseMap()->GetFile();
		desc.GrassNormalMap = grass->Grasses()[i]->material->GetNormalMap()->GetFile();
		desc.GrassSpecularMap = grass->Grasses()[i]->material->GetSpecularMap()->GetFile();
		desc.GrassDetailMap = grass->Grasses()[i]->material->GetDetailMap()->GetFile();
		desc.GrassSize = grass->Grasses()[i]->size;
		desc.GrassPosition = grass->Grasses()[i]->position;
		desc.NumOfPatchRows = grass->Grasses()[i]->numOfPatchRows;
		desc.NumOfRootsInPatch = grass->Grasses()[i]->numOfRootsInPatch;
		desc.NumOfRoots = grass->Grasses()[i]->numOfRoots;
		desc.IsFlower = grass->Grasses()[i]->isFlower;
		desc.FlowerNum = grass->Grasses()[i]->flowerNum;

		spec->grassDesc.push_back(desc);
	}

	//TODO: add flower
	{
		spec->WindVelocity = flower->GetWind()->Velocity();
		spec->FlowerCount = flower->FlowerCount();

		for (UINT i = 0; i < spec->FlowerCount; i++)
		{
			GameTerrainSpec::FlowerDesc desc;
			desc.FlowerDiffuseMap = flower->Flowers()[i]->DiffuseMap;
			desc.FlowerNum = flower->Flowers()[i]->FlowerNum;
			desc.FlowerSize = flower->Flowers()[i]->Size;
			desc.FlowerPosition = flower->Flowers()[i]->Position;
			desc.NumOfRoots = flower->Flowers()[i]->RootsNum;

			spec->flowerDesc.push_back(desc);
		}
	}

	spec->BillboardCount = billboard->Count();
	for (UINT i = 0; i < spec->BillboardCount; i++)
	{
		GameTerrainSpec::BillboardDesc desc;
		desc.BillType = billboard->GetDesc()[i].Type;
		desc.TextureNumber = billboard->GetDesc()[i].TextureNumber;
		desc.Position = billboard->GetDesc()[i].TransPosition;
		desc.Size = billboard->GetDesc()[i].Size;

		spec->billDesc.push_back(desc);
	}

	GameDataSpecManager::Get()->WriteTerrain(terrainName, spec);
	SAFE_DELETE(spec);
}

void Terrain::Delete()
{
	if (this != NULL)
	{
		delete this;
	}
	//SAFE_DELETE(render);
	//SAFE_DELETE(heightMap);
}