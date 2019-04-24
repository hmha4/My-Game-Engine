#include "Framework.h"
#include "Flower.h"
#include "Terrain.h"
#include "../Viewer/Freedom.h"
#include "Viewer/Frustum.h"

Flower::Flower(Terrain * terrain)
	: terrain(terrain)
	, vertexBuffer(NULL)
	, flowerCount(0)
	, flowerRootsNum(0), flowerRootsNumBefore(0)
{
	InitWind();
}

Flower::~Flower()
{
	SAFE_DELETE(frustum);

	SAFE_DELETE(windField);
	SAFE_DELETE(material);

	SAFE_RELEASE(vertexBuffer);

	for (FlowerDesc * desc : flowers)
	{
		desc->Delete();
		SAFE_DELETE(desc);
	}

	flowerDesc->Delete();
	SAFE_DELETE(flowerDesc);
}

void Flower::Initialize()
{
}

void Flower::Ready()
{
	frustum = new Frustum(300);

	material = new Material(Effects + L"010_Flower.fx");
	//material->SetDiffuseMap(Textures + L"free grass.png");
	vector<wstring> textureNames;
	textureNames.push_back(Textures + L"free grass.png");
	textureNames.push_back(Textures + L"Grass 2.png");
	textureNames.push_back(Textures + L"Grass 3.png");
	textures = new TextureArray(textureNames);
	material->GetEffect()->AsSRV("Map")->SetResource(textures->GetSRV());
	material->GetEffect()->AsScalar("MaxTextureCount")->SetInt(textureNames.size());

	for (size_t i = 0; i < textureNames.size(); i++)
		textureArray.push_back(new Texture(textureNames[i]));


	textureNumVar = material->GetEffect()->AsScalar("TextureNumber");

	flowerDesc = new FlowerDesc(material);

	if (flowerCount > 0)
	{
		LoadData();
		BindData();
	}

	material->GetEffect()->AsVector("WindVector")->SetFloatVector(windField->Velocity());
	material->GetEffect()->AsVector("WindAccel")->SetFloatVector(windField->Accel());

	frustumPlaneVar = material->GetEffect()->AsVector("FrustumPlanes");

	locationVar = terrain->Desc().material->GetEffect()->AsVector("Location");
	colorVar = terrain->Desc().material->GetEffect()->AsVector("Color");
	rangeVar = terrain->Desc().material->GetEffect()->AsScalar("Range");

	lightViewVar = material->GetEffect()->AsMatrix("LightView");
	lightProjVar = material->GetEffect()->AsMatrix("LightProjection");
	shadowTransformVar = material->GetEffect()->AsMatrix("ShadowTransform");
	shadowMapVar = material->GetEffect()->AsSRV("ShadowMap");
}

void Flower::Update()
{
	frustum->GetPlanes(frustumPlane);
	frustumPlaneVar->SetFloatVectorArray((float*)frustumPlane, 0, 6);
}

void Flower::Render()
{
	DrawFlower();
}

void Flower::RenderShader(UINT tech)
{
	DrawFlower(tech);
}

void Flower::ImGuiRender()
{
	ImGui::SliderInt("Num of Flowers##Flower", (int*)&flowerDesc->FlowerNum, 1, 5000);
	if (ImGui::DragFloat2("Velocity", (float*)&windField->Velocity(), 0.01f, -3, 3))
		SetVelocity();
	ImGui::Spacing();

	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Delete"))
		Delete();

	ImGui::Text("Material");
	ImGui::Separator();

	float width = ImGui::GetWindowContentRegionWidth();
	ImGui::BeginChild("", ImVec2(width, width), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
	ImGui::Columns(3);

	for (size_t i = 0; i < textureArray.size(); i++)
	{
		ImGui::Image(textureArray[i]->SRV(), ImVec2(80, 100));
		string temp = string("##").append(to_string(i));
		ImGui::RadioButton(temp.c_str(), (int*)&flowerDesc->TextureNumber, i);
		ImGui::NextColumn();
	}
	ImGui::Columns(1);
	ImGui::EndChild();

	D3DXVECTOR3 position;
	bool bPicked = terrain->Picking(NULL, &position);
	if (bPicked)
	{
		locationVar->SetFloatVector(position);
		colorVar->SetFloatVector(terrain->brush.Color);
		rangeVar->SetInt(terrain->brush.Range);

		if (Mouse::Get()->Down(2))
		{
			GenerateFlower(position, (float)terrain->brush.Range * 2.0f, flowerDesc->FlowerNum);
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();
}

void Flower::LoadFlowerMapFile(wstring filename)
{
	flowerDesc->ChangeTexture(filename);
}

void Flower::Delete()
{
	if (flowerCount == 0) return;

	if (flowerCount == 1)
	{
		vertices.clear();
		vertices.shrink_to_fit();
		SAFE_RELEASE(vertexBuffer);

		flowers[0]->Delete();
		SAFE_DELETE(flowers[0]);
		flowers.clear();
		flowers.shrink_to_fit();

		flowerCount = 0;
		flowerRootsNum = 0;
		flowerRootsNumBefore = 0;

		return;
	}

	UINT index = flowerCount - 1;

	vector<VertexTextureNormal> vertices;

	UINT start = flowerRootsNum - flowers[index]->RootsNum;
	UINT end = flowerRootsNum;
	vertices.assign(vertices.begin(), vertices.begin() + start);

	vertices.clear();
	vertices.shrink_to_fit();

	vertices.assign(vertices.begin(), vertices.end());

	SAFE_RELEASE(vertexBuffer);

	D3D11_BUFFER_DESC desc = { 0 };
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(VertexTextureNormal) * vertices.size();
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = &vertices[0];

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
	assert(SUCCEEDED(hr));

	flowerRootsNum = vertices.size();
	flowerRootsNumBefore = start;

	flowers[index]->Delete();
	SAFE_DELETE(flowers[index]);
	flowers.erase(flowers.begin() + index);

	flowerCount--;
}

void Flower::SetVelocity()
{
	material->GetEffect()->AsVector("WindVector")->SetFloatVector(windField->Velocity());
}

void Flower::CreateVector()
{
	for (UINT i = 0; i < flowerCount; i++)
		flowers.push_back(new FlowerDesc());
}

void Flower::GenerateFlower(D3DXVECTOR3 position, float size, UINT numOfRoot)
{
	if (numOfRoot == 0)
		return;

	FlowerDesc * flower = new FlowerDesc(material);

	flower->TextureNumber = flowerDesc->TextureNumber;
	flower->FlowerNum = flowerDesc->FlowerNum;

	flower->Position = position;
	flower->Size = size;

	flower->RootsNum = numOfRoot;
	flowerRootsNum += flower->RootsNum;

	vertices.resize(flowerRootsNum);

	for (UINT i = 0; i < flower->FlowerNum; i++)
	{
		D3DXVECTOR3 currPos;
		currPos.x = position.x + Math::Random(-size * 0.5f, size * 0.5f);
		currPos.z = position.z + Math::Random(-size * 0.5f, size * 0.5f);
		currPos.y = 0.0f;

		currPos.y = terrain->GetHeight(currPos.x, currPos.z);

		vertices[flowerRootsNumBefore + i].Position = currPos;
	}

	// Create Vertex Buffer
	if (vertexBuffer != NULL)
	{
		SAFE_RELEASE(vertexBuffer);
	}

	// Create Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * flowerRootsNum;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &vertices[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	flowers.push_back(flower);
	flowerCount++;
	flowerRootsNumBefore = flowerRootsNum;
}

void Flower::GenerateFlower(FlowerDesc * desc)
{
	for (UINT i = 0; i < desc->FlowerNum; i++)
	{
		D3DXVECTOR3 currPos;
		currPos.x = desc->Position.x + Math::Random(-desc->Size * 0.5f, desc->Size * 0.5f);
		currPos.z = desc->Position.z + Math::Random(-desc->Size * 0.5f, desc->Size * 0.5f);
		currPos.y = 0.0f;

		currPos.y = terrain->GetHeight(currPos.x, currPos.z);

		vertices[flowerRootsNumBefore + i].Position = currPos;
	}
}

void Flower::DrawFlower(UINT tech, UINT pass)
{
	if (flowerCount < 1) return;

	UINT stride = { sizeof(VertexTextureNormal) };
	UINT offset = { 0 };

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT temp = 0;
	for (UINT i = 0; i < flowerCount; i++)
	{
		textureNumVar->SetInt(flowers[i]->TextureNumber);
		material->GetEffect()->Draw(tech, pass, flowers[i]->RootsNum, temp);
		temp += flowers[i]->RootsNum;
	}
}

void Flower::LoadData()
{
	for (UINT i = 0; i < flowerCount; i++)
	{
		flowers[i]->material = this->material;
		
		flowerRootsNum += flowers[i]->RootsNum;

		vertices.resize(flowerRootsNum);

		GenerateFlower(flowers[i]);

		flowerRootsNumBefore = flowerRootsNum;
	}
}

void Flower::BindData()
{
	if (vertices.size() == 0) return;

	//	Flower
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTextureNormal) * flowerRootsNum;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &vertices[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
}

void Flower::InitWind()
{
	windField = new Wind();

	windField->Accel() = D3DXVECTOR2(0, 0);
	windField->Velocity() = D3DXVECTOR2(0, 0);
}


void Flower::SetEffect(D3DXMATRIX v, D3DXMATRIX p)
{
	lightViewVar->SetMatrix(v);
	lightProjVar->SetMatrix(p);
}

void Flower::SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv)
{
	shadowTransformVar->SetMatrix(shadowTransform);

	if (srv != NULL)
		shadowMapVar->SetResource(srv);
}

void Flower::ShadowUpdate()
{
	Update();
}

void Flower::NormalRender()
{
	Render();
}

void Flower::ShadowRender(UINT tech, UINT pass)
{
	if (tech == 1)
		return;

	if(tech == 2)
		RenderShader(1);
	else
		RenderShader(tech);
}