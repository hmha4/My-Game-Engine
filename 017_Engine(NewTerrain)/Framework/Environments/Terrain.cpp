#include "Framework.h"
#include "Terrain.h"
#include "HeightMap.h"
#include "TerrainRenderer.h"
#include "Systems\Window.h"
#include "Utilities/Xml.h"

const UINT Terrain::CellsPerPatch = 64;

Terrain::Terrain(InitDesc & desc)
	: desc(desc)
	, isPick(false)
	, terrainName(L"")
	, render(NULL)
	, heightMap(NULL)
	, loadDataFile(L"")
	, isLoad(false)
{
	for (int i = 0; i < 4; i++)
		materialValueFile[i] = L"";
}

Terrain::~Terrain()
{
	SAFE_DELETE(render);
	SAFE_DELETE(heightMap);
}

void Terrain::Initialize(Material* material)
{
	desc.material = material;

	if (render == NULL)
		render = new TerrainRenderer(this);

	if (heightMap == NULL)
		heightMap = new HeightMap((UINT)desc.HeightMapWidth, (UINT)desc.HeightMapHeight, desc.HeightScale);

	if (isLoad)
		Load(loadDataFile);
	heightMap->Load(desc.HeightMap);

	render->Initialize();
}

void Terrain::Ready(Material* material)
{
	desc.material = material;

	if (isLoad)
	{
		if (materialValueFile[0] != L"")
			desc.material->SetDiffuseMap(materialValueFile[0]);

		if (materialValueFile[1] != L"")
			desc.material->SetSpecularMap(materialValueFile[1]);

		if (materialValueFile[2] != L"")
			desc.material->SetNormalMap(materialValueFile[2]);

		if (materialValueFile[3] != L"")
			desc.material->SetDetailMap(materialValueFile[3]);
	}

	render->Ready(desc.material);
}

void Terrain::Load(wstring terrainData)
{
	Xml::XMLDocument*document = new Xml::XMLDocument();

	wstring tempFile = terrainData;
	Xml::XMLError error = document->LoadFile(String::ToString(tempFile).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement*root = document->FirstChildElement();
	Xml::XMLElement*matNode = root->FirstChildElement();

	Xml::XMLElement*node = NULL;

	node = matNode->FirstChildElement(); // HeightMap
	desc.HeightMap = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); //HeightScale
	desc.HeightScale = node->FloatText();

	node = node->NextSiblingElement(); // HeightMapWidth
	desc.HeightMapWidth = node->FloatText();

	node = node->NextSiblingElement(); // HeightMapHeight
	desc.HeightMapHeight = node->FloatText();

	node = node->NextSiblingElement(); // LayerMaps
	Xml::XMLElement*layerMap = node->FirstChildElement();
	desc.layerMapFile[0] = String::ToWString(layerMap->GetText());

	layerMap = layerMap->NextSiblingElement();
	desc.layerMapFile[1] = String::ToWString(layerMap->GetText());

	layerMap = layerMap->NextSiblingElement();
	desc.layerMapFile[2] = String::ToWString(layerMap->GetText());

	node = node->NextSiblingElement(); // BlendHeight
	Xml::XMLElement*blendHeight = node->FirstChildElement();
	desc.blendHeight.x = blendHeight->FloatText();

	blendHeight = blendHeight->NextSiblingElement();
	desc.blendHeight.y = blendHeight->FloatText();

	blendHeight = blendHeight->NextSiblingElement();
	desc.blendHeight.z = blendHeight->FloatText();

	blendHeight = blendHeight->NextSiblingElement();
	desc.blendHeight.w = blendHeight->FloatText();

	node = node->NextSiblingElement(); // BlendUse
	Xml::XMLElement*blendUse = node->FirstChildElement();
	desc.activeBlend.x = blendUse->FloatText();

	blendUse = blendUse->NextSiblingElement();
	desc.activeBlend.y = blendUse->FloatText();

	blendUse = blendUse->NextSiblingElement();
	desc.activeBlend.z = blendUse->FloatText();

	node = node->NextSiblingElement(); // DetailValue
	detail.DetailValue = node->FloatText();

	node = node->NextSiblingElement(); // DetailIntensity
	detail.DetailIntensity = node->FloatText();

	node = node->NextSiblingElement(); // AmbientColor
	Xml::XMLElement*ambientColor = node->FirstChildElement();
	D3DXCOLOR aColor;
	aColor.r = ambientColor->FloatText();

	ambientColor = ambientColor->NextSiblingElement();
	aColor.g = ambientColor->FloatText();

	ambientColor = ambientColor->NextSiblingElement();
	aColor.b = ambientColor->FloatText();

	ambientColor = ambientColor->NextSiblingElement();
	aColor.a = ambientColor->FloatText();

	desc.material->SetAmbient(aColor);

	node = node->NextSiblingElement(); // DiffuseMap
	materialValueFile[0] = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // DiffuseColor
	Xml::XMLElement*diffuseColor = node->FirstChildElement();

	D3DXCOLOR dColor;
	dColor.r = diffuseColor->FloatText();

	diffuseColor = diffuseColor->NextSiblingElement();
	dColor.g = diffuseColor->FloatText();

	diffuseColor = diffuseColor->NextSiblingElement();
	dColor.b = diffuseColor->FloatText();

	diffuseColor = diffuseColor->NextSiblingElement();
	dColor.a = diffuseColor->FloatText();

	desc.material->SetDiffuse(dColor);

	node = node->NextSiblingElement(); // SpecularMap
	materialValueFile[1] = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // SpecularColor
	Xml::XMLElement*specularColor = node->FirstChildElement();

	D3DXCOLOR sColor;
	sColor.r = specularColor->FloatText();

	specularColor = specularColor->NextSiblingElement();
	sColor.g = specularColor->FloatText();

	specularColor = specularColor->NextSiblingElement();
	sColor.b = specularColor->FloatText();

	specularColor = specularColor->NextSiblingElement();
	sColor.a = specularColor->FloatText();

	desc.material->SetSpecular(sColor);

	node = node->NextSiblingElement(); // SpecularIntensity
	desc.material->SetShininess(node->FloatText());

	node = node->NextSiblingElement(); // NormalMap
	materialValueFile[2] = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // DetailMap
	materialValueFile[3] = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // TexScale
	GetTerrainRender()->GetBuffer().TexScale = (int)node->FloatText();

	node = node->NextSiblingElement(); // FogStart
	GetTerrainRender()->GetBuffer().FogStart = node->FloatText();

	node = node->NextSiblingElement(); // FogRange
	GetTerrainRender()->GetBuffer().FogRange = node->FloatText();

	SAFE_DELETE(document);
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
	desc.material->GetShader()->AsScalar("LineType")->SetInt(tline.LineType);
	desc.material->GetShader()->AsVector("LineColor")->SetFloatVector(tline.LineColor);
	desc.material->GetShader()->AsScalar("Spacing")->SetInt(tline.Spacing);
	desc.material->GetShader()->AsScalar("Thickness")->SetFloat(tline.Thickness);
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
	float d = (z - 0.5f * Depth()) / desc.CellSpacing;
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	float h00 = heightMap->Data(row, col);
	float h01 = heightMap->Data(row, col + 1);
	float h10 = heightMap->Data(row + 1, col);
	float h11 = heightMap->Data(row + 1, col + 1);

	float s = c - col;
	float t = d - row;

	if (s + t <= 1.0f)
	{
		float uy = h01 - h00;
		float vy = h01 - h11;
		return h00 + (1.0f - s) * uy + (1.0f - t) * vy;
	}
	else
	{
		float uy = h10 - h11;
		float vy = h01 - h11;
		return h11 + (1.0f - s) * uy + (1.0f - t) * vy;
	}
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

bool Terrain::IsPicked()
{
	return isPick;
}

void Terrain::SetPickState(bool val)
{
	isPick = val;
}

void Terrain::SaveTerrain()
{
	//Desc의 정보들 전부 저장
	Xml::XMLDocument*document = new Xml::XMLDocument();
	Xml::XMLDeclaration * decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement * root = document->NewElement("TerrainDesc");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	Xml::XMLElement*node = document->NewElement("Desc");
	root->LinkEndChild(node);

	Xml::XMLElement * element = NULL;

	element = document->NewElement("HeightMap");
	element->SetText(String::ToString(desc.HeightMap).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("HeightScale");
	element->SetText(desc.HeightScale);
	node->LinkEndChild(element);

	element = document->NewElement("HeightMapWidth");
	element->SetText(desc.HeightMapWidth);
	node->LinkEndChild(element);

	element = document->NewElement("HeightMapHeight");
	element->SetText(desc.HeightMapHeight);
	node->LinkEndChild(element);

	element = document->NewElement("LayerMaps");
	node->LinkEndChild(element);

	Xml::XMLElement * layerMap = NULL;
	layerMap = document->NewElement("layer1");
	layerMap->SetText(String::ToString(desc.layerMapFile[0]).c_str());
	element->LinkEndChild(layerMap);

	layerMap = document->NewElement("layer2");
	layerMap->SetText(String::ToString(desc.layerMapFile[1]).c_str());
	element->LinkEndChild(layerMap);

	layerMap = document->NewElement("layer3");
	layerMap->SetText(String::ToString(desc.layerMapFile[2]).c_str());
	element->LinkEndChild(layerMap);

	element = document->NewElement("BlendHeight");
	node->LinkEndChild(element);

	Xml::XMLElement * blendHeight = NULL;
	blendHeight = document->NewElement("blendHeight1");
	blendHeight->SetText(desc.blendHeight.x);
	element->LinkEndChild(blendHeight);

	blendHeight = document->NewElement("blendHeight2");
	blendHeight->SetText(desc.blendHeight.y);
	element->LinkEndChild(blendHeight);

	blendHeight = document->NewElement("blendHeight3");
	blendHeight->SetText(desc.blendHeight.z);
	element->LinkEndChild(blendHeight);

	blendHeight = document->NewElement("blendHeight4");
	blendHeight->SetText(desc.blendHeight.w);
	element->LinkEndChild(blendHeight);

	element = document->NewElement("BlendUse");
	node->LinkEndChild(element);

	Xml::XMLElement * useBlend = NULL;
	useBlend = document->NewElement("UseBlend1");
	useBlend->SetText(desc.activeBlend.x);
	element->LinkEndChild(useBlend);

	useBlend = document->NewElement("UseBlend2");
	useBlend->SetText(desc.activeBlend.y);
	element->LinkEndChild(useBlend);

	useBlend = document->NewElement("UseBlend3");
	useBlend->SetText(desc.activeBlend.z);
	element->LinkEndChild(useBlend);

	//TerrainDetail정보 저장
	element = document->NewElement("DetailValue");
	element->SetText(detail.DetailValue);
	node->LinkEndChild(element);

	element = document->NewElement("DetailIntensity");
	element->SetText(detail.DetailIntensity);
	node->LinkEndChild(element);

	element = document->NewElement("AmbientColor");
	node->LinkEndChild(element);

	Xml::XMLElement * ambientColor = NULL;
	ambientColor = document->NewElement("AmbientR");
	ambientColor->SetText(desc.material->GetAmbient().r);
	element->LinkEndChild(ambientColor);

	ambientColor = document->NewElement("AmbientG");
	ambientColor->SetText(desc.material->GetAmbient().g);
	element->LinkEndChild(ambientColor);

	ambientColor = document->NewElement("AmbientB");
	ambientColor->SetText(desc.material->GetAmbient().b);
	element->LinkEndChild(ambientColor);

	ambientColor = document->NewElement("AmbientA");
	ambientColor->SetText(desc.material->GetAmbient().a);
	element->LinkEndChild(ambientColor);

	//터레인 디퓨즈,스펙큘러,노말,디테일 복사
	element = document->NewElement("TerrainDiffuse");
	if (desc.material->GetDiffuseMap() != NULL)
		element->SetText(String::ToString(desc.material->GetDiffuseMap()->GetFile()).c_str());
	else
		element->SetText("");
	node->LinkEndChild(element);

	element = document->NewElement("DiffuseColor");
	Xml::XMLElement * diffuseColor = NULL;
	diffuseColor = document->NewElement("DiffuseR");
	diffuseColor->SetText(desc.material->GetDiffuse().r);
	element->LinkEndChild(diffuseColor);

	diffuseColor = document->NewElement("DiffuseG");
	diffuseColor->SetText(desc.material->GetDiffuse().g);
	element->LinkEndChild(diffuseColor);

	diffuseColor = document->NewElement("DiffuseB");
	diffuseColor->SetText(desc.material->GetDiffuse().b);
	element->LinkEndChild(diffuseColor);

	diffuseColor = document->NewElement("DiffuseA");
	diffuseColor->SetText(desc.material->GetDiffuse().a);
	element->LinkEndChild(diffuseColor);
	node->LinkEndChild(element);

	element = document->NewElement("TerrainSpecular");

	if (desc.material->GetSpecularMap() != NULL)
		element->SetText(String::ToString(desc.material->GetSpecularMap()->GetFile()).c_str());
	else
		element->SetText("");
	node->LinkEndChild(element);

	element = document->NewElement("SpecularColor");

	Xml::XMLElement * specularColor = NULL;
	specularColor = document->NewElement("SpecularR");
	specularColor->SetText(desc.material->GetDiffuse().r);
	element->LinkEndChild(specularColor);

	specularColor = document->NewElement("SpecularG");
	specularColor->SetText(desc.material->GetDiffuse().g);
	element->LinkEndChild(specularColor);

	specularColor = document->NewElement("SpecularB");
	specularColor->SetText(desc.material->GetDiffuse().b);
	element->LinkEndChild(specularColor);

	specularColor = document->NewElement("SpecularA");
	specularColor->SetText(desc.material->GetDiffuse().a);
	element->LinkEndChild(specularColor);
	node->LinkEndChild(element);

	element = document->NewElement("SpecularIntensity");
	element->SetText(desc.material->GetShininess());
	node->LinkEndChild(element);

	element = document->NewElement("TerrainNormal");
	if (desc.material->GetNormalMap() != NULL)
		element->SetText(String::ToString(desc.material->GetNormalMap()->GetFile()).c_str());
	else
		element->SetText("");
	node->LinkEndChild(element);

	element = document->NewElement("TerrainDetail");
	if (desc.material->GetDetailMap() != NULL)
		element->SetText(String::ToString(desc.material->GetDetailMap()->GetFile()).c_str());
	else
		element->SetText("");
	node->LinkEndChild(element);

	//터레인렌더 버퍼저장
	element = document->NewElement("TexScale");
	element->SetText(GetTerrainRender()->GetBuffer().TexScale);
	node->LinkEndChild(element);

	element = document->NewElement("FogStart");
	element->SetText(GetTerrainRender()->GetBuffer().FogStart);
	node->LinkEndChild(element);

	element = document->NewElement("FogRange");
	element->SetText(GetTerrainRender()->GetBuffer().FogRange);
	node->LinkEndChild(element);

	string file = String::ToString(Datas + terrainName + L".terrain");
	document->SaveFile(file.c_str());
}



void Terrain::SetShader(D3DXMATRIX v, D3DXMATRIX p)
{
	desc.material->GetShader()->AsMatrix("LightView")->SetMatrix(v);
	desc.material->GetShader()->AsMatrix("LightProjection")->SetMatrix(p);
}

void Terrain::SetShader(D3DXMATRIX shadowTransform, ID3D11ShaderResourceView * srv)
{
	desc.material->GetShader()->AsMatrix("ShadowTransform")->SetMatrix(shadowTransform);
	desc.material->GetShader()->AsShaderResource("ShadowMap")->SetResource(srv);
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
	render->RenderShadow(tech, pass);
}
