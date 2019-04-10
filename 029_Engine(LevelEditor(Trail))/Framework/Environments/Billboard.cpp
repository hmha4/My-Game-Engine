#include "Framework.h"
#include "Billboard.h"
#include "Terrain.h"
#include "TerrainRenderer.h"
#include "Viewer/Frustum.h"
#include "Collider/ColliderBox.h"
#include "Utilities/Xml.h"

Billboard::Billboard(Terrain * terrain)
	: billboardTexture(NULL)
	, terrain(terrain)
	, BillboardCount(0)
	, showLine(false), isLoad(false)
{
	vertexBuffer[0] = NULL;
	vertexBuffer[1] = NULL;

	bType = BillboardType::TWO;
}

Billboard::~Billboard()
{
	for (Texture * tex : textures)
		SAFE_DELETE(tex);

	SAFE_RELEASE(vertexBuffer[0]);
	SAFE_RELEASE(vertexBuffer[1]);

	SAFE_DELETE(material);
	SAFE_DELETE(frustum);

	SAFE_DELETE(billboardTexture);

	for (BCollider col : colliders)
		SAFE_DELETE(col.Box);
	SAFE_DELETE(lineEffect);
}

void Billboard::Initalize()
{
	frustum = new Frustum(500);
	vertex.Position = { 0,0,0 };
	vertices.clear();
	vertices.shrink_to_fit();
}

void Billboard::Ready()
{
	material = new Material(Effects + L"025_Billboard.fx");
	frustumVar = material->GetEffect()->AsVector("WorldFrustumPlanes");
	lightViewVar = material->GetEffect()->AsMatrix("LightView");
	lightProjVar = material->GetEffect()->AsMatrix("LightProjection");
	shadowTransformVar = material->GetEffect()->AsMatrix("ShadowTransform");
	shadowMapVar = material->GetEffect()->AsSRV("ShadowMap");

	lineEffect = new Effect(Effects + L"002_Line.fx");

	material->SetDiffuse(1, 1, 1, 1);
	material->SetAmbient(1, 1, 1, 1);
	material->SetSpecular(1, 1, 1, 16.0f);

	textureNames.push_back(Textures + L"Tree.png");
	textureNames.push_back(Textures + L"Tree2.png");
	textureNames.push_back(Textures + L"Tree3.png");
	textureNames.push_back(Textures + L"Tree4.png");
	textureNames.push_back(Textures + L"dead tree model03-texture.png");
	textureNames.push_back(Textures + L"dead tree model04-texture.png");
	textureNames.push_back(Textures + L"dead tree model05-texture.png");

	textures.push_back(new Texture(Textures + L"Tree.png"));
	textures.push_back(new Texture(Textures + L"Tree2.png"));
	textures.push_back(new Texture(Textures + L"Tree3.png"));
	textures.push_back(new Texture(Textures + L"Tree4.png"));
	textures.push_back(new Texture(Textures + L"dead tree model03-texture.png"));
	textures.push_back(new Texture(Textures + L"dead tree model04-texture.png"));
	textures.push_back(new Texture(Textures + L"dead tree model05-texture.png"));

	billboardTexture = new TextureArray(textureNames);
	material->GetEffect()->AsSRV("Map")->SetResource(billboardTexture->GetSRV());
	material->GetEffect()->AsScalar("FogStart")->SetFloat(terrain->GetTerrainRender()->GetBuffer().FogStart);
	material->GetEffect()->AsScalar("FogRange")->SetFloat(terrain->GetTerrainRender()->GetBuffer().FogRange);

	

	//	CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(Vertex);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &vertex;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer[0]);
		assert(SUCCEEDED(hr));
	}

	if (loadDatas.size() > 0)
		LoadData();
}

void Billboard::Render()
{
	if (vertices.size() < 1) return;

	for (BCollider col : colliders)
		col.Box->Render();

	UINT stride[2] = { sizeof(Vertex),sizeof(InstanceBillboard) };
	UINT offset[2] = { 0,0 };

	D3D::GetDC()->IASetVertexBuffers(0, 2, vertexBuffer, stride, offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	material->GetEffect()->DrawInstanced(0, 0, 1, vertices.size());
}

void Billboard::Update()
{
	frustum->GetPlanes(plane);
	frustumVar->SetFloatVectorArray((float*)plane, 0, 6);
}

void Billboard::ImGuiRender()
{
	isLoad = false;

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::BulletText("Billboard Count : %d", vertices.size());
	ImGui::SameLine();
	if (ImGui::Checkbox("Show Line", &showLine))
	{
		if (colliders.size() > 0)
		{
			D3DXVECTOR2 setScale = vertices[vertices.size() - 1].Size;
			D3DXVECTOR3 position = vertices[vertices.size() - 1].TransPosition;

			position.y = terrain->GetHeight(position.x, position.z);
			position.y += setScale.y / 2.0f;

			D3DXMATRIX S, T;
			D3DXMatrixTranslation(&T, position.x, position.y, position.z);
			D3DXMatrixScaling(&S, setScale.x, setScale.y, setScale.x);
			colliders[colliders.size() - 1].Box->IsDraw(showLine);
			colliders[colliders.size() - 1].Box->Transform(S * T);
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Sample Count");
	ImGui::RadioButton("One", (int*)&bType, 0); ImGui::SameLine();
	ImGui::RadioButton("Two", (int*)&bType, 1); ImGui::SameLine();
	ImGui::RadioButton("Four", (int*)&bType, 3);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();

	D3DXVECTOR2 preSize = setSize;
	ImGui::Text("Settings");
	ImGui::Spacing();
	ImGui::SliderFloat2("Scale", setSize, 1, 50);

	if (preSize != setSize)
		ResourceUpdate(setSize);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Text("Select Image");

	float width = ImGui::GetWindowContentRegionWidth();
	ImGui::BeginChild("", ImVec2(width, width), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
	ImGui::Columns(3);

	for (size_t i = 0; i < textures.size(); i++)
	{
		ImGui::Image(textures[i]->SRV(), ImVec2(80, 100));
		string temp = string("##").append(to_string(i));
		ImGui::RadioButton(temp.c_str(), (int*)&treeType, i);
		ImGui::NextColumn();
	}
	ImGui::Columns(1);
	ImGui::EndChild();

	setTextureNumber = treeType;

	if (ImGui::Button("Delete", ImVec2(60, 30)))
		RemoveBillboard();
	//사이즈 설정하면 가장마지막애가 변함 그래야 보이니까
}

void Billboard::BuildBillboard(D3DXVECTOR3 pos)
{
	float theta;

	D3DXVECTOR3 position;
	D3DXMATRIX S, T;

	switch (bType)
	{
	case Billboard::BillboardType::ONE:
	{
		position = pos;
		position.y = terrain->GetHeight(pos.x, pos.z);
		position.y += this->setSize.y / 2.0f;

		D3DXMATRIX rotate;
		D3DXMatrixIdentity(&rotate);

		D3DXMatrixTranslation(&T, position.x, position.y, position.z);
		D3DXMatrixScaling(&S, setSize.x, setSize.y, setSize.x);

		D3DXMATRIX DT;
		D3DXMatrixIdentity(&DT);

		InstanceBillboard billboard;

		billboard.TransPosition = position;
		billboard.DeTransWorld = DT;
		billboard.RotateWorld = rotate;
		billboard.Size = setSize;
		billboard.TextureNumber = setTextureNumber;
		billboard.Type = (int)bType;

		vertices.push_back(billboard);
	}
	break;
	case Billboard::BillboardType::TWO:
	{
		position;
		for (int i = 0; i < 2; i++)
		{
			if (i == 0)
				theta = 35;
			else if (i == 1)
				theta = -35;

			position = pos;
			position.y = terrain->GetHeight(pos.x, pos.z);
			position.y += this->setSize.y / 2.0f;

			D3DXMATRIX rotate;
			D3DXMatrixRotationYawPitchRoll(&rotate, Math::ToRadian(theta), 0, 0);

			D3DXMatrixTranslation(&T, position.x, position.y, position.z);

			D3DXVECTOR3 xzPosition = position;
			xzPosition.y = 0;

			float radius = Math::Distance(D3DXVECTOR3(0, 0, 0), xzPosition);
			D3DXVECTOR3 resultPos;
			D3DXVec3TransformCoord(&resultPos, &xzPosition, &rotate);

			D3DXVECTOR3 targetVector = xzPosition - resultPos;
			D3DXVec3Normalize(&targetVector, &targetVector);

			float zValue = fabs(radius*sinf(Math::ToRadian(theta)));
			float xValue = fabs(radius - fabs(radius*cosf(Math::ToRadian(-theta))));

			float targetValue = sqrtf(zValue*zValue + xValue * xValue);

			D3DXVECTOR3 setPos =
			{
				targetVector.x*targetValue,
				0,
				targetVector.z*targetValue
			};

			D3DXMATRIX DT;
			D3DXMatrixTranslation(&DT, setPos.x, setPos.y, setPos.z);

			InstanceBillboard billboard;

			billboard.TransPosition = position;
			billboard.DeTransWorld = DT;
			billboard.RotateWorld = rotate;
			billboard.Size = setSize;
			billboard.TextureNumber = setTextureNumber;
			billboard.Type = (int)bType;

			vertices.push_back(billboard);
		}
	}
	break;
	case Billboard::BillboardType::FOUR:
	{
		for (int i = 0; i < 4; i++)
		{
			if (i == 0)
				theta = 25;
			else if (i == 1)
				theta = -25;
			else if (i == 2)
				theta = 60;
			else if (i == 3)
				theta = -60;

			position = pos;
			position.y = terrain->GetHeight(pos.x, pos.z);
			position.y += this->setSize.y / 2.0f;

			D3DXMATRIX rotate;
			D3DXMatrixRotationYawPitchRoll(&rotate, Math::ToRadian(theta), 0, 0);

			D3DXMatrixTranslation(&T, position.x, position.y, position.z);

			D3DXVECTOR3 xzPosition = position;
			xzPosition.y = 0;

			float radius = Math::Distance(D3DXVECTOR3(0, 0, 0), xzPosition);
			D3DXVECTOR3 resultPos;
			D3DXVec3TransformCoord(&resultPos, &xzPosition, &rotate);

			D3DXVECTOR3 targetVector = xzPosition - resultPos;
			D3DXVec3Normalize(&targetVector, &targetVector);

			float zValue = fabs(radius*sinf(Math::ToRadian(theta)));
			float xValue = fabs(radius - fabs(radius*cosf(Math::ToRadian(-theta))));

			float targetValue = sqrtf(zValue*zValue + xValue * xValue);

			D3DXVECTOR3 setPos =
			{
				targetVector.x*targetValue,
				0,
				targetVector.z*targetValue
			};

			D3DXMATRIX DT;
			D3DXMatrixTranslation(&DT, setPos.x, setPos.y, setPos.z);

			InstanceBillboard billboard;

			billboard.TransPosition = position;
			billboard.DeTransWorld = DT;
			billboard.RotateWorld = rotate;
			billboard.Size = setSize;
			billboard.TextureNumber = setTextureNumber;
			billboard.Type = (int)bType;

			vertices.push_back(billboard);
		}

		
	}
	break;
	}

	D3DXMatrixScaling(&S, setSize.x, setSize.y, setSize.x);
	BCollider col;
	ColliderBox * box = new ColliderBox(
		lineEffect,
		L"Tree Collider (" + to_wstring(colliders.size()) + L")",
		D3DXVECTOR3(-0.04f, -0.5f, -0.04f), D3DXVECTOR3(0.04f, 0.5f, 0.04f)
	);

	BillboardCount++;

	box->IsDraw(showLine);
	box->Transform(S * T);
	col.position = position;
	col.scale = setSize;
	col.Box = box;
	colliders.push_back(col);

	if (isLoad == false)
	{
		LoadDesc loadDesc;
		loadDesc.Size = setSize;
		loadDesc.TransPosition = position;
		loadDesc.Type = (int)bType;
		loadDesc.TextureNumber = setTextureNumber;
		loadDatas.push_back(loadDesc);
	}

	SAFE_RELEASE(vertexBuffer[1]);
	D3D11_BUFFER_DESC desc = { 0 };
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(InstanceBillboard) * vertices.size();
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = &vertices[0];

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer[1]);
	assert(SUCCEEDED(hr));
}

void Billboard::RemoveBillboard()
{
	switch (bType)
	{
	case Billboard::BillboardType::ONE:
		if (vertices.size() < 1)
			break;
		vertices.pop_back();
		break;
	case Billboard::BillboardType::TWO:
		if (vertices.size() < 1)
			break;
		vertices.pop_back();
		vertices.pop_back();
		break;
	case Billboard::BillboardType::FOUR:
		if (vertices.size() < 1)
			break;
		vertices.pop_back();
		vertices.pop_back();
		vertices.pop_back();
		vertices.pop_back();
		break;
	}

	loadDatas.pop_back();
	SAFE_DELETE(colliders[colliders.size() - 1].Box);
	colliders.pop_back();

	SAFE_RELEASE(vertexBuffer[1]);

	if (vertices.size() > 0)
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(InstanceBillboard) * vertices.size();
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &vertices[0];

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer[1]);
		assert(SUCCEEDED(hr));
	}
}

void Billboard::ResourceUpdate(D3DXVECTOR2 setScale)
{
	if (vertices.size() < 1)
		return;

	for (int i = 0; i < (int)bType + 1; i++)
	{
		vertices[vertices.size() - 1 - i].Size = setScale;

		D3DXVECTOR3 position = vertices[vertices.size() - 1].TransPosition;

		position.y = terrain->GetHeight(position.x, position.z);
		position.y += setScale.y / 2.0f;

		D3DXMATRIX S, T;
		D3DXMatrixTranslation(&T, position.x, position.y, position.z);
		D3DXMatrixScaling(&S, setScale.x, setScale.y, setScale.x);
		colliders[colliders.size() - 1].position = position;
		colliders[colliders.size() - 1].scale = setScale;
		colliders[colliders.size() - 1].Box->Transform(S * T);

		vertices[vertices.size() - 1 - i].TransPosition = position;
		vertices[vertices.size() - 1 - i].Size = setScale;
	}

	SAFE_RELEASE(vertexBuffer[1]);
	D3D11_BUFFER_DESC desc = { 0 };
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(InstanceBillboard) * vertices.size();
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = &vertices[0];

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer[1]);
	assert(SUCCEEDED(hr));
}

void Billboard::CreateVector(UINT count)
{
	loadDatas.assign(count, LoadDesc());
}

void Billboard::SetEffect(D3DXMATRIX v, D3DXMATRIX p)
{
	lightViewVar->SetMatrix(v);
	lightProjVar->SetMatrix(p);
}

void Billboard::SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv)
{
	shadowTransformVar->SetMatrix(shadowTransform);

	if (srv != NULL)
		shadowMapVar->SetResource(srv);
}

void Billboard::ShadowUpdate()
{
	Update();
}

void Billboard::NormalRender()
{
	Render();
}

void Billboard::ShadowRender(UINT tech, UINT pass)
{
	if (vertices.size() < 1) return;

	UINT stride[2] = { sizeof(Vertex),sizeof(InstanceBillboard) };
	UINT offset[2] = { 0,0 };

	D3D::GetDC()->IASetVertexBuffers(0, 2, vertexBuffer, stride, offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	material->GetEffect()->DrawInstanced(1, 1, 1, vertices.size());
}

void Billboard::LoadBillboardMap(wstring fileName)
{
	//SAFE_DELETE(billboardTexture);

	//billboardTexture = new Texture(fileName);
	//BillboardMaterial->GetShader()->AsShaderResource("Map")->SetResource(billboardTexture->SRV());
}

void Billboard::LoadData()
{
	isLoad = true;

	for (size_t i = 0; i < loadDatas.size(); i++)
	{
		bType = (BillboardType)loadDatas[i].Type;
		setTextureNumber = (TextureType)loadDatas[i].TextureNumber;
		setSize = loadDatas[i].Size;

		BuildBillboard(loadDatas[i].TransPosition);
	}
}