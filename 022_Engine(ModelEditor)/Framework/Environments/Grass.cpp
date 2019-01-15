#include "Framework.h"
#include "Grass.h"
#include "Terrain.h"
#include "../Viewer/Freedom.h"
#include "Viewer/Frustum.h"

Grass::Grass(Terrain * terrain)
	: terrain(terrain)
	, vertexBufferG(NULL), vertexBufferF(NULL)
	, grassCount(0)
	, numOfRootsBefore(0), numOfRoots(0), numOfPatches(0)
	, flowerRootsNum(0), flowerRootsNumBefore(0)
	, patchRows(1), rootsInPatchRows(1)
{
	InitWind();
}

Grass::~Grass()
{
	SAFE_DELETE(frustum);

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
	frustum = new Frustum(300);

	material = new Material(Effects + L"010_Grass.fx");
	material->SetDiffuseMap(Textures + L"grassBlade.png");
	material->SetNormalMap(Textures + L"grassBladeAlpha.png");
	material->SetSpecularMap(Textures + L"grassBladeAlpha2.png");
	material->SetDetailMap(Textures + L"flower.png");

	grassDesc = new GrassDesc(material);

	if (grassCount > 0)
	{
		LoadData();
		BindData();
	}

	material->GetShader()->AsVector("WindVector")->SetFloatVector(windField->Velocity());
	material->GetShader()->AsVector("WindAccel")->SetFloatVector(windField->Accel());
}

void Grass::Update()
{
	frustum->GetPlanes(frustumPlane);
	material->GetShader()->AsVector("FrustumPlanes")->SetFloatVectorArray((float*)frustumPlane, 0, 6);
}

void Grass::Render()
{
	DrawGrass();
	DrawFlower();
}

void Grass::ImGuiRender()
{
	ImGui::SliderInt("PatchRows##Grass", (int*)&patchRows, 1, 100);
	ImGui::SliderInt("RootsInPatch##Grass", (int*)&rootsInPatchRows, 1, 100);
	if (ImGui::DragFloat2("Velocity", (float*)&windField->Velocity(), 0.01f, -3, 3))
		SetVelocity();
	ImGui::Spacing();

	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Delete"))
		Delete();

	ImGui::Text("Material");
	ImGui::Separator();
	ImGui::Columns(2);

	ImGui::Text("Grass"); ImGui::NextColumn();
	ImGui::Checkbox("", &grassDesc->isFlower);
	ImGui::SameLine();
	ImGui::Text("Flower"); ImGui::NextColumn();

	if (ImGui::ImageButton(grassDesc->material->GetDiffuseMap()->SRV(), ImVec2(50, 50)))
		Path::OpenFileDialog(bind(&Grass::LoadGrassMapFile, this, placeholders::_1, 0));
	ImGui::NextColumn();

	if (grassDesc->isFlower == true)
	{
		if (ImGui::ImageButton(grassDesc->material->GetDetailMap()->SRV(), ImVec2(50, 50)))
			Path::OpenFileDialog(bind(&Grass::LoadGrassMapFile, this, placeholders::_1, 1));

		ImGui::Text("Count");
		ImGui::SliderInt("", (int*)&grassDesc->flowerNum, 1, patchRows * patchRows * rootsInPatchRows);
	}
	ImGui::Columns(1);

	D3DXVECTOR3 position;
	bool bPicked = terrain->Picking(NULL, &position);
	if (bPicked)
	{
		terrain->Desc().material->GetShader()->AsVector("Location")->SetFloatVector(position);
		terrain->Desc().material->GetShader()->AsVector("Color")->SetFloatVector(terrain->brush.Color);
		terrain->Desc().material->GetShader()->AsScalar("Range")->SetInt(terrain->brush.Range);

		if (Mouse::Get()->Down(2))
		{
			terrain->GetGrass()->GenerateGrass(position, terrain->brush.Range * 2, patchRows, rootsInPatchRows);
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();
}

void Grass::LoadGrassMapFile(wstring filename, int index)
{
	grassDesc->ChangeTexture(filename, index);
}

void Grass::Delete()
{
	if (grassCount == 0) return;

	
	if (grassCount == 1)
	{
		verticesG.clear();
		verticesG.shrink_to_fit();
		SAFE_RELEASE(vertexBufferG);

		grasses[0]->Delete();
		SAFE_DELETE(grasses[0]);
		grasses.clear();
		grasses.shrink_to_fit();

		grassCount = 0;
		numOfRoots = 0;
		numOfRootsBefore = 0;

		return;
	}

	UINT index = grassCount - 1;

	vector<VertexTextureNormal> vertices;

	UINT start = numOfRoots - grasses[index]->numOfRoots;
	UINT end = numOfRoots;
	vertices.assign(verticesG.begin(), verticesG.begin() + start);

	verticesG.clear();
	verticesG.shrink_to_fit();

	verticesG.assign(vertices.begin(), vertices.end());

	SAFE_RELEASE(vertexBufferG);

	D3D11_BUFFER_DESC desc = { 0 };
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(VertexTextureNormal) * vertices.size();
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = &verticesG[0];

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBufferG);
	assert(SUCCEEDED(hr));

	numOfRoots = vertices.size();
	numOfRootsBefore = start;

	grasses[index]->Delete();
	SAFE_DELETE(grasses[index]);
	grasses.erase(grasses.begin() + index);

	grassCount--;
}

void Grass::SetVelocity()
{
	material->GetShader()->AsVector("WindVector")->SetFloatVector(windField->Velocity());
}

void Grass::CreateVector()
{
	for (UINT i = 0; i < grassCount; i++)
		grasses.push_back(new GrassDesc());
}

void Grass::GenerateGrass(D3DXVECTOR3 position, UINT terrainSize, UINT numberOfPatchRows, UINT numberOfRootsInPatch)
{
	GrassDesc * grass = new GrassDesc(material);
	/*material->SetDiffuseMap(grassDesc->material->GetDiffuseMap()->GetFile());
	material->SetNormalMap(grassDesc->material->GetNormalMap()->GetFile());
	material->SetSpecularMap(grassDesc->material->GetSpecularMap()->GetFile());
	material->SetDetailMap(grassDesc->material->GetDetailMap()->GetFile());*/

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

	UINT stride[2] = { sizeof(Vertex), sizeof(VertexTextureNormal) };
	UINT offset[2] = { 0 };

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBufferG, &stride[1], &offset[0]);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT temp = 0;
	for (UINT i = 0; i < grassCount; i++)
	{
		material->GetShader()->Draw(tech, pass, grasses[i]->numOfRoots, temp);
		temp += grasses[i]->numOfRoots;
	}
}

void Grass::LoadData()
{
	for (UINT i = 0; i < grassCount; i++)
	{
		grasses[i]->material = this->material;
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
	
}
