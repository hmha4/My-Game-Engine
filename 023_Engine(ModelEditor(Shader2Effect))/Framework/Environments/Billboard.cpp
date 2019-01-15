#include "Framework.h"
#include "Billboard.h"
#include "Terrain.h"
#include "Viewer\Frustum.h"

Billboard::Billboard(Terrain * terrain)
	: billboardTexture(NULL)
	, terrain(terrain)
	, BillboardCount(0)
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

	SAFE_RELEASE(cBuffer);
}

void Billboard::Initalize()
{
	frustum = new Frustum(500);
	vertex.Position = { 0,0,0 };
	vertices.clear();
}

void Billboard::Ready()
{
	material = new Material(Effects + L"025_Billboard.fx");

	material->SetDiffuse(1, 1, 1, 1);
	material->SetAmbient(1, 1, 1, 1);
	material->SetSpecular(1, 1, 1, 16.0f);

	textureNames.push_back(Textures + L"Tree.png");
	textureNames.push_back(Textures + L"Tree2.png");
	textureNames.push_back(Textures + L"Tree3.png");
	textureNames.push_back(Textures + L"Tree4.png");

	textures.push_back(new Texture(Textures + L"Tree.png"));
	textures.push_back(new Texture(Textures + L"Tree2.png"));
	textures.push_back(new Texture(Textures + L"Tree3.png"));
	textures.push_back(new Texture(Textures + L"Tree4.png"));

	billboardTexture = new TextureArray(textureNames);
	material->GetEffect()->AsShaderResource("Map")->SetResource(billboardTexture->GetSRV());

	//CreateVertexBuffer
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

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(Buffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &cBuffer);
	assert(SUCCEEDED(hr));
}

void Billboard::Render()
{
	UINT stride[2] = { sizeof(Vertex),sizeof(InstanceBillboard) };
	UINT offset[2] = { 0,0 };

	D3D::GetDC()->IASetVertexBuffers(0, 2, vertexBuffer, stride, offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	material->GetEffect()->DrawInstanced(0, 0, 1, vertices.size());
}

void Billboard::Update()
{
	frustum->GetPlanes(buffer.plane);
	material->GetEffect()->AsVector("WorldFrustumPlanes")->SetFloatVectorArray((float*)buffer.plane, 0, 6);
}

void Billboard::ImGuiRender()
{
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::BulletText("Billboard Count : %d", vertices.size());

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

	switch (bType)
	{
	case Billboard::BillboardType::ONE:
	{
		D3DXVECTOR3 position = pos;
		position.y = terrain->GetHeight(pos.x, pos.z);
		position.y += this->setSize.y / 2.0f;

		D3DXMATRIX rotate;
		D3DXMatrixIdentity(&rotate);

		D3DXMATRIX T;
		D3DXMatrixTranslation(&T, position.x, position.y, position.z);

		D3DXMATRIX DT;
		D3DXMatrixIdentity(&DT);

		InstanceBillboard billboard;

		billboard.TransWorld = T;
		billboard.DeTransWorld = DT;
		billboard.RotateWorld = rotate;
		billboard.Size = setSize;
		billboard.TextureNumber = setTextureNumber;

		vertices.push_back(billboard);
	}
	break;
	case Billboard::BillboardType::TWO:
	{
		for (int i = 0; i < 2; i++)
		{
			if (i == 0)
				theta = 35;
			else if (i == 1)
				theta = -35;

			D3DXVECTOR3 position = pos;
			position.y = terrain->GetHeight(pos.x, pos.z);
			position.y += this->setSize.y / 2.0f;

			D3DXMATRIX rotate;
			D3DXMatrixRotationYawPitchRoll(&rotate, Math::ToRadian(theta), 0, 0);

			D3DXMATRIX T;
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

			billboard.TransWorld = T;
			billboard.DeTransWorld = DT;
			billboard.RotateWorld = rotate;
			billboard.Size = setSize;
			billboard.TextureNumber = setTextureNumber;

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

			D3DXVECTOR3 position = pos;
			position.y = terrain->GetHeight(pos.x, pos.z);
			position.y += this->setSize.y / 2.0f;

			D3DXMATRIX rotate;
			D3DXMatrixRotationYawPitchRoll(&rotate, Math::ToRadian(theta), 0, 0);

			D3DXMATRIX T;
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

			billboard.TransWorld = T;
			billboard.DeTransWorld = DT;
			billboard.RotateWorld = rotate;
			billboard.Size = setSize;
			billboard.TextureNumber = setTextureNumber;

			vertices.push_back(billboard);
		}
	}
	break;
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

		D3DXVECTOR3 pos;
		D3DXVec3TransformCoord(&pos, &D3DXVECTOR3(0, 0, 0), &vertices[vertices.size() - 1].TransWorld);

		D3DXVECTOR3 position = pos;
		position.y = terrain->GetHeight(pos.x, pos.z);
		position.y += setScale.y / 2.0f;

		D3DXMATRIX T;
		D3DXMatrixTranslation(&T, position.x, position.y, position.z);

		vertices[vertices.size() - 1 - i].TransWorld = T;
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

void Billboard::SetEffect(D3DXMATRIX v, D3DXMATRIX p)
{
	material->GetEffect()->AsMatrix("LightView")->SetMatrix(v);
	material->GetEffect()->AsMatrix("LightProjection")->SetMatrix(p);
}

void Billboard::SetEffect(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv)
{
	material->GetEffect()->AsMatrix("ShadowTransform")->SetMatrix(shadowTransform);

	if (srv != NULL)
		material->GetEffect()->AsShaderResource("ShadowMap")->SetResource(srv);
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