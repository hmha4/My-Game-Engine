#include "Framework.h"
#include "Grass.h"
#include "Terrain.h"
#include "../Viewer/Freedom.h"

Grass::Grass(Terrain * terrain)
	: terrain(terrain)
	, vertexBufferG(NULL), vertexBufferF(NULL)
	, grassCount(0)
	, numOfRootsBefore(0), numOfRoots(0), numOfPatches(0)
	, flowerRootsNum(0), flowerRootsNumBefore(0)
{
	InitWind();
}

Grass::~Grass()
{
	SAFE_DELETE(windField);
	SAFE_DELETE(material);

	SAFE_RELEASE(vertexBufferG);
	SAFE_RELEASE(vertexBufferF);

	for (GrassDesc * desc : grasses)
	{
		desc->Delete();
		SAFE_DELETE(desc);
	}

	grassDesc->Delete();
	SAFE_DELETE(grassDesc);
}

void Grass::Initialize()
{
}

void Grass::Ready()
{
	material = new Material(Effects + L"010_Grass.fx");
	grassDesc = new GrassDesc(material);

	if (grassCount > 0)
	{
		LoadData();
		BindData();
	}
}

void Grass::Update()
{
	// Calculate new velocity
	windField->Velocity() = windField->Velocity() + (windField->Accel() * Time::Get()->Running());
}

void Grass::Render()
{
	//D3DXVECTOR2 rotation;
	//camera->RotationDegree(&rotation);
	//ImGui::DragFloat2("Rotation", (float*)&rotation, 1, 0, 360);
	//camera->RotationDegree(rotation.x, rotation.y);

	//material->GetShader()->AsVector("TimeVector")->SetFloatVector(D3DXVECTOR2(Time::Get()->Running() / 1000.0f, Time::Get()->Running()));
	material->GetShader()->AsVector("WindVector")->SetFloatVector(windField->Velocity());

	DrawGrass();
	DrawFlower();
}

void Grass::RenderShadow(UINT tech, UINT pass)
{
	//shader->AsVector("TimeVector")->SetFloatVector(D3DXVECTOR2(Time::Get()->Running() / 1000.0f, Time::Get()->Running()));
	material->GetShader()->AsVector("WindVector")->SetFloatVector(windField->Velocity());

	DrawGrass(0, 1);
	DrawFlower(1, 1);
}


void Grass::CreateVector()
{
	for (UINT i = 0; i < grassCount; i++)
		grasses.push_back(new GrassDesc());
}

void Grass::GenerateGrass(D3DXVECTOR3 position, UINT terrainSize, UINT numberOfPatchRows, UINT numberOfRootsInPatch)
{
	GrassDesc * grass = new GrassDesc(material);
	material->SetDiffuseMap(grassDesc->materialMap.diffuseMap->GetFile());
	material->SetNormalMap(grassDesc->materialMap.normalMap->GetFile());
	material->SetSpecularMap(grassDesc->materialMap.specularMap->GetFile());
	material->SetDetailMap(grassDesc->materialMap.detailMap->GetFile());

	grass->isFlower = grassDesc->isFlower;
	grass->flowerNum = grassDesc->flowerNum;

	grass->position = position;
	grass->size = terrainSize;

	grass->numOfPatchRows = numberOfPatchRows;
	grass->numOfRootsInPatch = numberOfRootsInPatch;

	/*if (grass.size > 1000)
		grass.numOfPatchRows = 160;
	else if (grass.size > 500)
		grass.numOfPatchRows = 190;
	else
		grass.numOfPatchRows = 75;*/

	numOfPatches = grass->numOfPatchRows * grass->numOfPatchRows;
	grass->numOfRoots = numOfPatches * grass->numOfRootsInPatch;
	numOfRoots += numOfPatches * grass->numOfRootsInPatch;

	verticesG.resize(numOfRoots);
	//vertices = new VertexTextureNormal[numOfRoots];

	int currentVertex = 0;

	D3DXVECTOR3 startPosition{ 0, 0, 0 };
	D3DXVECTOR3 patchSize;

	patchSize.x = (float)grass->size / grass->numOfPatchRows;
	patchSize.y = 0;
	patchSize.z = (float)grass->size / grass->numOfPatchRows;

	// Generate grid of patches
	for (UINT x = 0; x < grass->numOfPatchRows; x++)
	{
		for (UINT y = 0; y < grass->numOfPatchRows; y++)
		{
			currentVertex = GeneratePatch(grass, startPosition, patchSize, currentVertex);
			startPosition.x += patchSize.x;
		}

		startPosition.x = 0;
		startPosition.z += (patchSize.z);
	}

	// Create Vertex Buffer
	if (vertexBufferG != NULL)
	{
		SAFE_RELEASE(vertexBufferG);

		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * numOfRoots;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &verticesG[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBufferG);
		assert(SUCCEEDED(hr));
		/*D3D::GetDC()->UpdateSubresource(
			vertexBufferV, 0, NULL, &vertices[0], 
			sizeof(VertexTextureNormal) * numOfRoots, 0
		);*/
	}
	else
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * numOfRoots;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &verticesG[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBufferG);
		assert(SUCCEEDED(hr));

	}

	if (grass->isFlower == true)
		GenerateFlower(grass);

	grasses.push_back(grass);
	grassCount++;
	numOfRootsBefore = numOfRoots;
}

int Grass::GeneratePatch(GrassDesc * grass, D3DXVECTOR3 startPos, D3DXVECTOR3 patchSize, int currentVertex)
{
	D3DXVECTOR3 grassStartPos = D3DXVECTOR3(-(int)grass->size * 0.5f, 0, -(int)grass->size * 0.5f);

	for (UINT i = 0; i < grass->numOfRootsInPatch; i++)
	{
		float randomizedXDistance = Math::Random(0.0f, patchSize.x);
		float randomizedZDistance = Math::Random(0.0f, patchSize.z);

		D3DXVECTOR3 currentPos;
		currentPos.x = startPos.x + randomizedXDistance;
		currentPos.z = startPos.z + randomizedZDistance;
		currentPos += grass->position + grassStartPos;
		currentPos.y = terrain->GetHeight(currentPos.x, currentPos.z);

		verticesG[numOfRootsBefore + currentVertex].Position = currentPos;
		currentVertex++;
	}

	return currentVertex;
}

void Grass::DrawGrass(UINT tech, UINT pass)
{
	if (grassCount < 1) return;

	UINT stride = sizeof(VertexTextureNormal);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBufferG, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	for (UINT i = 0; i < grassCount; i++)
	{
		material->ChangeMaterialDesc(&grasses[i]->colorDesc);
		material->ChangeMaterialMap(&grasses[i]->materialMap);
		material->GetShader()->Draw(tech, pass, grasses[i]->numOfRoots, offset);
		offset += grasses[i]->numOfRoots;
	}
}

void Grass::LoadData()
{
	for (UINT i = 0; i < grassCount; i++)
	{
		grasses[i]->material = this->material;
		grasses[i]->material->ChangeMaterialDesc(&grasses[i]->colorDesc);
		grasses[i]->material->ChangeMaterialMap(&grasses[i]->materialMap);
		grasses[i]->material->SetDiffuseMap(grasses[i]->DiffuseMap);
		grasses[i]->material->SetNormalMap(grasses[i]->NormalMap);
		grasses[i]->material->SetSpecularMap(grasses[i]->SpecularMap);
		grasses[i]->material->SetDetailMap(grasses[i]->DetailMap);

		numOfRoots += grasses[i]->numOfRoots;

		verticesG.resize(numOfRoots);

		int currentVertex = 0;
		D3DXVECTOR3 startPosition{ 0, 0, 0 };
		D3DXVECTOR3 patchSize;

		patchSize.x = (float)grasses[i]->size / grasses[i]->numOfPatchRows;
		patchSize.y = 0;
		patchSize.z = (float)grasses[i]->size / grasses[i]->numOfPatchRows;

		// Generate grid of patches
		for (UINT x = 0; x < grasses[i]->numOfPatchRows; x++)
		{
			for (UINT y = 0; y < grasses[i]->numOfPatchRows; y++)
			{
				currentVertex = GeneratePatch(grasses[i], startPosition, patchSize, currentVertex);
				startPosition.x += patchSize.x;
			}

			startPosition.x = 0;
			startPosition.z += (patchSize.z);
		}

		if (grasses[i]->isFlower == true)
			GenerateFlower(grasses[i]);

		numOfRootsBefore = numOfRoots;
	}
}

void Grass::BindData()
{
	if (verticesG.size() == 0) return;

	//	Grass
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * numOfRoots;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &verticesG[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBufferG);
		assert(SUCCEEDED(hr));
	}
	
	//if (verticesF.size() == 0) return;

	////	Flower
	//{
	//	D3D11_BUFFER_DESC desc = { 0 };
	//	desc.Usage = D3D11_USAGE_DEFAULT;
	//	desc.ByteWidth = sizeof(VertexTextureNormal) * flowerRootsNum;
	//	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	//	D3D11_SUBRESOURCE_DATA data = { 0 };
	//	data.pSysMem = &verticesF[0];

	//	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBufferF);
	//	assert(SUCCEEDED(hr));
	//}
}

void Grass::InitWind()
{
	windField = new Wind();

	windField->Accel() = D3DXVECTOR2(0, 0);
	windField->Velocity() = D3DXVECTOR2(1, 0);
}

void Grass::GenerateFlower(GrassDesc * grass)
{
	flowerRootsNum += grass->flowerNum;

	verticesF.resize(flowerRootsNum);
	for (UINT i = 0; i < grass->flowerNum; i++)
	{
		int index1 = (int)Math::Random((float)numOfRootsBefore, (float)numOfRootsBefore + (float)grass->numOfRoots - 1);

		D3DXVECTOR3 currPos;
		currPos.x = verticesG[index1].Position.x;
		currPos.z = verticesG[index1].Position.z;
		currPos.y = verticesG[index1].Position.y;

		verticesF[flowerRootsNumBefore + i].Position = currPos;
	}

	//	CreateVertexBuffer
	if (vertexBufferF != NULL)
	{
		SAFE_RELEASE(vertexBufferF);

		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * flowerRootsNum;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &verticesF[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBufferF);
		assert(SUCCEEDED(hr));
	}
	else
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * flowerRootsNum;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &verticesF[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBufferF);
		assert(SUCCEEDED(hr));
	}

	flowerRootsNumBefore = flowerRootsNum;
}

void Grass::DrawFlower(UINT tech, UINT pass)
{
	if (grassCount < 1) return;

	UINT stride = sizeof(VertexTextureNormal);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBufferF, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	for (UINT i = 0; i < grassCount; i++)
	{
		if (grasses[i]->flowerNum == 0) continue;

		material->ChangeMaterialDesc(&grasses[i]->colorDesc);
		material->ChangeMaterialMap(&grasses[i]->materialMap);
		material->GetShader()->Draw(tech, pass, grasses[i]->flowerNum, offset);
		offset += grasses[i]->flowerNum;
	}
}

void Grass::SetShader(D3DXMATRIX v, D3DXMATRIX p)
{
	material->GetShader()->AsMatrix("LightView")->SetMatrix(v);
	material->GetShader()->AsMatrix("LightProjection")->SetMatrix(p);
}

void Grass::SetShader(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv)
{
	material->GetShader()->AsMatrix("ShadowTransform")->SetMatrix(shadowTransform);

	if (srv != NULL)
		material->GetShader()->AsShaderResource("ShadowMap")->SetResource(srv);
}

void Grass::ShadowUpdate()
{
	Update();
}

void Grass::NormalRender()
{
	Render();
}

void Grass::ShadowRender(UINT tech, UINT pass)
{
	//Render();
	//RenderShadow(tech, pass);
}
