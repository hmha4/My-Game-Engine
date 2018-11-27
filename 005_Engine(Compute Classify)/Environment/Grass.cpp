#include "stdafx.h"
#include "Grass.h"
#include "Terrain.h"
#include "../Viewer/Freedom.h"

Grass::Grass(UINT terrainSize, class Terrain * terrain)
	: terrainSize(terrainSize), terrain(terrain)
{
	grassBlade = new Texture(Textures + L"grassBlade.png");
	grassBladeAlpha1 = new Texture(Textures + L"grassBladeAlpha.png");
	grassBladeAlpha2 = new Texture(Textures + L"grassBladeAlpha2.png");
	flower = new Texture(Textures + L"flower.png");

	shader = new Shader(Effects + L"010_Grass.fx");
	
	shader->AsShaderResource("GrassMap")->SetResource(grassBlade->SRV());
	shader->AsShaderResource("GrassAlphaMap1")->SetResource(grassBladeAlpha1->SRV());
	shader->AsShaderResource("GrassAlphaMap2")->SetResource(grassBladeAlpha2->SRV());
	shader->AsShaderResource("FlowerMap")->SetResource(flower->SRV());

	GenerateGrass();
	GenerateFlower();
}

Grass::~Grass()
{
	SAFE_DELETE(grassBlade);
	SAFE_DELETE(grassBladeAlpha1);
	SAFE_DELETE(grassBladeAlpha2);
	SAFE_DELETE(flower);

	SAFE_DELETE(shader);

	SAFE_DELETE_ARRAY(vertices);
	SAFE_RELEASE(vertexBuffer);

	SAFE_RELEASE(vertexBufferF);

	SAFE_DELETE(windField);
}

void Grass::Update()
{
	// Calculate new velocity
	windField->Velocity() = windField->Velocity() + (windField->Accel() * Time::Get()->Running());

	// TODO: Dampen the acceleration vector
	//wind.Acceleration = new Vector2(wind.Acceleration.X - (float) gameTime.ElapsedGameTime.TotalSeconds / 1000.0f, wind.Acceleration.Y - (float) gameTime.ElapsedGameTime.TotalSeconds / 1000.0f);

	// TODO: Diffuse the velocity by using surrounding vectors
}

void Grass::Render()
{
	ImGui::DragFloat2("Velocity", (float*)&windField->Velocity(), 0.01f, -3, 3);

	shader->AsVector("TimeVector")->SetFloatVector(D3DXVECTOR2(Time::Get()->Running() / 1000.0f, Time::Get()->Running()));
	shader->AsVector("WindVector")->SetFloatVector(windField->Velocity());

	DrawGrass();
	DrawFlower();
}

void Grass::GenerateGrass(UINT numberOfPatchRows, UINT numberOfRootsInPatch)
{
	numOfPatchRows = numberOfPatchRows;
	numOfRootsInPatch = numberOfRootsInPatch;

	if (terrainSize > 1000)
		numOfPatchRows = 160;
	else if (terrainSize > 500)
		numOfPatchRows = 190;
	else
		numOfPatchRows = 75;

	numOfPatches = numOfPatchRows * numOfPatchRows;
	numOfRoots = numOfPatches * numOfRootsInPatch;

	vertices = new VertexTextureNormal[numOfRoots];
	
	int currentVertex = 0;

	D3DXVECTOR3 startPosition{ 0, 0, 0 };
	D3DXVECTOR3 patchSize;
	patchSize.x = (float)terrainSize / numOfPatchRows;
	patchSize.y = 0;
	patchSize.z = (float)terrainSize / numOfPatchRows;

	// Generate grid of patches
	for (int x = 0; x < numOfPatchRows; x++)
	{
		for (int y = 0; y < numOfPatchRows; y++)
		{
			currentVertex = GeneratePatch(startPosition, patchSize, currentVertex);
			startPosition.x += patchSize.x;
		}

		startPosition.x = 0;
		startPosition.z += patchSize.z;
	}

	// Create Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * numOfRoots;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	InitWind();
}

int Grass::GeneratePatch(D3DXVECTOR3 startPos, D3DXVECTOR3 patchSize, int currentVertex)
{
	for (UINT i = 0; i < numOfRootsInPatch; i++)
	{
		// Generate random numbers within the patch size
		float randomizedZDistance = Math::Random(0.0f, patchSize.z);
		float randomizedXDistance = Math::Random(0.0f, patchSize.x);

		D3DXVECTOR3 currentPos;
		currentPos.x = startPos.x + randomizedXDistance;
		currentPos.z = startPos.z + randomizedZDistance;
		currentPos.y = terrain->GetHeight(currentPos.x, currentPos.z);
	
		vertices[currentVertex].Position = currentPos;
		currentVertex++;
	}

	return currentVertex;
}

void Grass::DrawGrass()
{
	UINT stride = sizeof(VertexTextureNormal);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);


	shader->Draw(0, 0, numOfRoots);
}

void Grass::InitWind()
{
	windField = new Wind();

	//windField->Accel() = D3DXVECTOR2(0, 0);
	//windField->Velocity() = D3DXVECTOR2((float)x / numOfPatchRows + 0.1f, (float)y / numOfPatchRows);

	windField->Accel() = D3DXVECTOR2(0, 0);
	windField->Velocity() = D3DXVECTOR2(1, 0);
}

void Grass::GenerateFlower()
{
	VertexTextureNormal vertices[1000];
	for (int i = 0; i < 1000; i++)
	{
		D3DXVECTOR3 position;
		position.x = Math::Random(0.0f, 255.0f);
		position.z = Math::Random(0.0f, 255.0f);
		position.y = terrain->GetHeight(position.x, position.z);

		vertices[i].Position = position;
	}

	//	CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * 1000;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBufferF);
		assert(SUCCEEDED(hr));
	}
}

void Grass::DrawFlower()
{
	UINT stride = sizeof(VertexTextureNormal);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBufferF, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	shader->Draw(1, 0, 1000);
}
