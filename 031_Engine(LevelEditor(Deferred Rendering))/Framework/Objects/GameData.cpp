#include "Framework.h"
#include "GameData.h"
#include "Utilities/Xml.h"
#include "Module/MyBehaviourTree.h"

GameDataSpecManager * GameDataSpecManager::instance = NULL;

GameDataSpecManager::GameDataSpecManager()
{
}

GameDataSpecManager::~GameDataSpecManager()
{
	for (GameDataSpec * spec : specList)
		SAFE_DELETE(spec);

	Clear();
}

void GameDataSpecManager::Create()
{
	if (instance != NULL)
		assert("instance already exists");

	instance = new GameDataSpecManager();
}

void GameDataSpecManager::Delete()
{
	if (instance == NULL)
		assert("instance does not exist");

	SAFE_DELETE(instance);
}

GameDataSpecManager * GameDataSpecManager::Get()
{
	if (instance == NULL)
		assert("instance does not exist");

	return instance;
}

GameDataSpec * GameDataSpecManager::LoadCharacter(wstring file)
{
	GameCharacterSpec * spec = new GameCharacterSpec();

	Xml::XMLDocument*document = new Xml::XMLDocument();

	wstring tempFile = Datas + L"Character/" + file;
	Xml::XMLError error = document->LoadFile(String::ToString(tempFile).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement*root = document->FirstChildElement();
	Xml::XMLElement*matNode = root->FirstChildElement();

	Xml::XMLElement*node = NULL;

	node = matNode;
	spec->ModelType = (ModelType)((int)node->FloatText());

	node = node->NextSiblingElement();
	spec->Name = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	spec->Tag = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	Xml::XMLElement * collider = node->FirstChildElement();
	spec->ColliderCount = (UINT)collider->FloatText();

	for (UINT i = 0; i < spec->ColliderCount; i++)
	{
		GameCharacterSpec::Collider col;

		collider = collider->NextSiblingElement();
		col.Name = String::ToWString(collider->GetText());

		collider = collider->NextSiblingElement();
		col.Type = (UINT)collider->FloatText();

		collider = collider->NextSiblingElement();
		col.BoneIndex = (int)collider->FloatText();

		D3DXMATRIX W, s, r, t;
		D3DXVECTOR3 S, R, T;

		collider = collider->NextSiblingElement();
		Xml::XMLElement * temp = collider->FirstChildElement();

		GetTransforms(&temp, &col.RootMatrix);

		spec->Colliders.push_back(col);
	}

	node = node->NextSiblingElement();
	Xml::XMLElement * trail = node->FirstChildElement();
	spec->TrailCount = (UINT)trail->FloatText();

	for (UINT i = 0; i < spec->TrailCount; i++)
	{
		GameCharacterSpec::Trail t;

		trail = trail->NextSiblingElement();
		t.BoneIndex = (int)trail->FloatText();

		trail = trail->NextSiblingElement();
		t.LifeTime = trail->FloatText();

		trail = trail->NextSiblingElement();
		t.DiffuseMap = String::ToWString(trail->GetText());

		trail = trail->NextSiblingElement();
		t.AlphaMap = String::ToWString(trail->GetText());

		trail = trail->NextSiblingElement();
		GetTransforms(&trail, &t.StartMat);

		trail = trail->NextSiblingElement();
		GetTransforms(&trail, &t.EndMat);

		spec->Trails.push_back(t);
	}

	node = node->NextSiblingElement();
	spec->IsChild = (int)node->FloatText();

	node = node->NextSiblingElement();
	spec->ParentBoneIndex = (int)node->FloatText();

	node = node->NextSiblingElement();
	spec->ParentName = node->GetText();

	node = node->NextSiblingElement();
	GetTransforms(&node, &spec->RootMatrix);
	
	node = node->NextSiblingElement();
	Xml::XMLElement * clip = node->FirstChildElement();
	spec->ClipCount = (UINT)clip->FloatText();

	for (UINT i = 0; i < spec->ClipCount; i++)
	{
		clip = clip->NextSiblingElement();
		spec->ClipNames.push_back(clip->GetText());
	}

	node = node->NextSiblingElement();
	spec->SourceFilePath = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	spec->BT = new MyBehaviourTree();
	spec->BT->Name() = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	Xml::XMLElement * childSib = node->FirstChildElement();
	if (spec->BT->Name() != L"")
	{
		spec->BT->ReadBehaviorTree(childSib, spec->BT->GetRoot());
		spec->BT->BindNodes();
	}
	
	specList.push_back(spec);

	SAFE_DELETE(document);

	return spec;
}

void GameDataSpecManager::WriteCharacter(wstring fileName, GameCharacterSpec * spec)
{
	Path::CreateFolder(Datas + L"Character/");

	Xml::XMLDocument * document = new Xml::XMLDocument();
	Xml::XMLDeclaration * decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement * root = document->NewElement("GameCharacterSpec");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	Xml::XMLElement * node = document->NewElement("ModelType");
	node->SetText((int)spec->ModelType);
	root->LinkEndChild(node);

	node = document->NewElement("Name");
	node->SetText(String::ToString(spec->Name).c_str());
	root->LinkEndChild(node);

	node = document->NewElement("Tag");
	node->SetText(String::ToString(spec->Tag).c_str());
	root->LinkEndChild(node);

	node = document->NewElement("Colliders");
	root->LinkEndChild(node);

	Xml::XMLElement * element = NULL;

	element = document->NewElement("ColliderCount");
	element->SetText(spec->ColliderCount);
	node->LinkEndChild(element);

	for (size_t i = 0; i < spec->Colliders.size(); i++)
	{
		element = document->NewElement("Name");
		element->SetText(String::ToString(spec->Colliders[i].Name).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("Type");
		element->SetText(spec->Colliders[i].Type);
		node->LinkEndChild(element);

		element = document->NewElement("BoneIndex");
		element->SetText(spec->Colliders[i].BoneIndex);
		node->LinkEndChild(element);

		element = document->NewElement("RootMatrix");
		node->LinkEndChild(element);

		Xml::XMLElement * vector = NULL;
		SetTransforms(vector, element, document, spec->Colliders[i].RootMatrix);
	}

	node = document->NewElement("Trails");
	root->LinkEndChild(node);

	Xml::XMLElement * trail = NULL;

	trail = document->NewElement("TrailCount");
	trail->SetText(spec->TrailCount);
	node->LinkEndChild(trail);

	for (size_t i = 0; i < spec->TrailCount; i++)
	{
		trail = document->NewElement("BoneIndex");
		trail->SetText(spec->Trails[i].BoneIndex);
		node->LinkEndChild(trail);

		trail = document->NewElement("LifeTime");
		trail->SetText(spec->Trails[i].LifeTime);
		node->LinkEndChild(trail);

		trail = document->NewElement("DiffuseMap");
		trail->SetText(String::ToString(spec->Trails[i].DiffuseMap).c_str());
		node->LinkEndChild(trail);

		trail = document->NewElement("AlphaMap");
		trail->SetText(String::ToString(spec->Trails[i].AlphaMap).c_str());
		node->LinkEndChild(trail);

		SetTransforms(trail, node, document, spec->Trails[i].StartMat);
		SetTransforms(trail, node, document, spec->Trails[i].EndMat);
	}
	

	node = document->NewElement("IsChild");
	node->SetText(spec->IsChild);
	root->LinkEndChild(node);

	node = document->NewElement("ParentBoneIndex");
	node->SetText(spec->ParentBoneIndex);
	root->LinkEndChild(node);

	node = document->NewElement("ParentName");
	node->SetText(spec->ParentName.c_str());
	root->LinkEndChild(node);

	SetTransforms(node, root, document, spec->RootMatrix);

	node = document->NewElement("ClipNames");
	root->LinkEndChild(node);

	Xml::XMLElement * clip = NULL;

	clip = document->NewElement("ClipCount");
	clip->SetText(spec->ClipCount);
	node->LinkEndChild(clip);
	for (size_t i = 0; i < spec->ClipNames.size(); i++)
	{
		clip = document->NewElement("ClipName");
		clip->SetText(spec->ClipNames[i].c_str());
		node->LinkEndChild(clip);
	}

	node = document->NewElement("FilePath");
	node->SetText(String::ToString(spec->SourceFilePath).c_str());
	root->LinkEndChild(node);

	node = document->NewElement("BTName");
	if (spec->BT == NULL)
		node->SetText("");
	else
		node->SetText(String::ToString(spec->BT->Name()).c_str());
	root->LinkEndChild(node);

	Xml::XMLElement * tempEle = NULL;
	node = document->NewElement("BehaviourTree");

	if (spec->BT != NULL)
		spec->BT->SaveTree(node, tempEle, document, spec->BT->GetRoot());
		
	root->LinkEndChild(node);

	string file = String::ToString(Datas + L"Character/" + fileName);
	document->SaveFile(file.c_str());

	SAFE_DELETE(document);
}

GameDataSpec * GameDataSpecManager::LoadTerrain(wstring file)
{
	GameTerrainSpec * spec = new GameTerrainSpec();

	Xml::XMLDocument*document = new Xml::XMLDocument();

	wstring tempFile = Datas + L"Terrain/" + file;
	Xml::XMLError error = document->LoadFile(String::ToString(tempFile).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement*root = document->FirstChildElement();
	Xml::XMLElement*matNode = root->FirstChildElement();

	Xml::XMLElement*node = NULL;

	node = matNode->FirstChildElement(); // HeightMap
	spec->HeightMap = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); //HeightScale
	spec->HeightScale = node->FloatText();

	node = node->NextSiblingElement(); // HeightMapWidth
	spec->HeightMapWidth = node->FloatText();

	node = node->NextSiblingElement(); // HeightMapHeight
	spec->HeightMapHeight = node->FloatText();

	node = node->NextSiblingElement(); // LayerMaps
	Xml::XMLElement*layerMap = node->FirstChildElement();
	spec->LayerMapFile[0] = String::ToWString(layerMap->GetText());

	layerMap = layerMap->NextSiblingElement();
	spec->LayerMapFile[1] = String::ToWString(layerMap->GetText());

	layerMap = layerMap->NextSiblingElement();
	spec->LayerMapFile[2] = String::ToWString(layerMap->GetText());

	node = node->NextSiblingElement(); // BlendHeight
	Xml::XMLElement*blendHeight = node->FirstChildElement();
	spec->BlendHeight.x = blendHeight->FloatText();

	blendHeight = blendHeight->NextSiblingElement();
	spec->BlendHeight.y = blendHeight->FloatText();

	blendHeight = blendHeight->NextSiblingElement();
	spec->BlendHeight.z = blendHeight->FloatText();

	blendHeight = blendHeight->NextSiblingElement();
	spec->BlendHeight.w = blendHeight->FloatText();

	node = node->NextSiblingElement(); // BlendUse
	Xml::XMLElement*blendUse = node->FirstChildElement();
	spec->ActiveBlend.x = blendUse->FloatText();

	blendUse = blendUse->NextSiblingElement();
	spec->ActiveBlend.y = blendUse->FloatText();

	blendUse = blendUse->NextSiblingElement();
	spec->ActiveBlend.z = blendUse->FloatText();

	node = node->NextSiblingElement(); // DetailValue
	spec->DetailValue = node->FloatText();

	node = node->NextSiblingElement(); // DetailIntensity
	spec->DetailIntensity = node->FloatText();

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

	spec->AmbientColor = aColor;

	node = node->NextSiblingElement(); // DiffuseMap
	spec->MaterialValueFile[0] = String::ToWString(node->GetText());

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

	spec->DiffuseColor = dColor;

	node = node->NextSiblingElement(); // SpecularMap
	spec->MaterialValueFile[1] = String::ToWString(node->GetText());

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

	spec->SpecularColor = sColor;

	node = node->NextSiblingElement(); // SpecularIntensity
	spec->Shininess = node->FloatText();

	node = node->NextSiblingElement(); // NormalMap
	spec->MaterialValueFile[2] = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // DetailMap
	spec->MaterialValueFile[3] = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // TexScale
	spec->TexScale = (int)node->FloatText();

	node = node->NextSiblingElement(); // FogStart
	spec->FogStart = node->FloatText();

	node = node->NextSiblingElement(); // FogRange
	spec->FogRange = node->FloatText();

	matNode = matNode->NextSiblingElement();
	node = matNode->FirstChildElement();
	Xml::XMLElement * wind = NULL;
	wind = node->FirstChildElement();

	spec->WindVelocity.x = wind->FloatText();
	wind = wind->NextSiblingElement();
	spec->WindVelocity.y = wind->FloatText();

	node = node->NextSiblingElement();

	spec->GrassCount = (UINT)node->FloatText();

	Xml::XMLElement * grass = NULL;
	spec->grassDesc.resize(spec->GrassCount);
	for (UINT i = 0; i < spec->GrassCount; i++)
	{
		node = node->NextSiblingElement();
		grass = node->FirstChildElement();

		spec->grassDesc[i].GrassDiffuseMap = String::ToWString(grass->GetText());
		grass = grass->NextSiblingElement();

		spec->grassDesc[i].GrassNormalMap = String::ToWString(grass->GetText());
		grass = grass->NextSiblingElement();

		spec->grassDesc[i].GrassSpecularMap = String::ToWString(grass->GetText());
		grass = grass->NextSiblingElement();

		spec->grassDesc[i].GrassDetailMap = String::ToWString(grass->GetText());
		grass = grass->NextSiblingElement();

		spec->grassDesc[i].GrassSize = (UINT)grass->FloatText();
		grass = grass->NextSiblingElement();

		Xml::XMLElement * position = grass->FirstChildElement();
		spec->grassDesc[i].GrassPosition.x = position->FloatText();
		position = position->NextSiblingElement();
		spec->grassDesc[i].GrassPosition.y = position->FloatText();
		position = position->NextSiblingElement();
		spec->grassDesc[i].GrassPosition.z = position->FloatText();
		position = position->NextSiblingElement();

		grass = grass->NextSiblingElement();

		spec->grassDesc[i].NumOfPatchRows = (UINT)grass->FloatText();
		grass = grass->NextSiblingElement();

		spec->grassDesc[i].NumOfRootsInPatch = (UINT)grass->FloatText();
		grass = grass->NextSiblingElement();

		spec->grassDesc[i].NumOfRoots = (UINT)grass->FloatText();
		grass = grass->NextSiblingElement();

		spec->grassDesc[i].IsFlower = (UINT)grass->FloatText();
		grass = grass->NextSiblingElement();

		spec->grassDesc[i].FlowerNum = (UINT)grass->FloatText();
	}

	matNode = matNode->NextSiblingElement();
	node = matNode->FirstChildElement();
	spec->BillboardCount = (UINT)node->FloatText();

	Xml::XMLElement * bill = NULL;
	spec->billDesc.resize(spec->BillboardCount);
	for (UINT i = 0; i < spec->BillboardCount; i++)
	{
		node = node->NextSiblingElement();
		bill = node->FirstChildElement();

		spec->billDesc[i].BillType = (UINT)bill->FloatText();
		bill = bill->NextSiblingElement();

		spec->billDesc[i].TextureNumber = (UINT)bill->FloatText();
		bill = bill->NextSiblingElement();

		Xml::XMLElement * size = bill->FirstChildElement();
		spec->billDesc[i].Size.x = size->FloatText();
		size = size->NextSiblingElement();
		spec->billDesc[i].Size.y = size->FloatText();
		size = size->NextSiblingElement();

		bill = bill->NextSiblingElement();

		Xml::XMLElement * position = bill->FirstChildElement();
		spec->billDesc[i].Position.x = position->FloatText();
		position = position->NextSiblingElement();
		spec->billDesc[i].Position.y = position->FloatText();
		position = position->NextSiblingElement();
		spec->billDesc[i].Position.z = position->FloatText();
		position = position->NextSiblingElement();
	}

	spec->SourceFilePath = file;

	specList.push_back(spec);

	SAFE_DELETE(document);

	return spec;
}

void GameDataSpecManager::WriteTerrain(wstring fileName, GameTerrainSpec * spec)
{
	Path::CreateFolder(Datas + L"Terrain/");

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
	element->SetText(String::ToString(spec->HeightMap).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("HeightScale");
	element->SetText(spec->HeightScale);
	node->LinkEndChild(element);

	element = document->NewElement("HeightMapWidth");
	element->SetText(spec->HeightMapWidth);
	node->LinkEndChild(element);

	element = document->NewElement("HeightMapHeight");
	element->SetText(spec->HeightMapHeight);
	node->LinkEndChild(element);

	element = document->NewElement("LayerMaps");
	node->LinkEndChild(element);

	Xml::XMLElement * layerMap = NULL;
	layerMap = document->NewElement("layer1");
	layerMap->SetText(String::ToString(spec->LayerMapFile[0]).c_str());
	element->LinkEndChild(layerMap);

	layerMap = document->NewElement("layer2");
	layerMap->SetText(String::ToString(spec->LayerMapFile[1]).c_str());
	element->LinkEndChild(layerMap);

	layerMap = document->NewElement("layer3");
	layerMap->SetText(String::ToString(spec->LayerMapFile[2]).c_str());
	element->LinkEndChild(layerMap);

	element = document->NewElement("BlendHeight");
	node->LinkEndChild(element);

	Xml::XMLElement * blendHeight = NULL;
	blendHeight = document->NewElement("blendHeight1");
	blendHeight->SetText(spec->BlendHeight.x);
	element->LinkEndChild(blendHeight);

	blendHeight = document->NewElement("blendHeight2");
	blendHeight->SetText(spec->BlendHeight.y);
	element->LinkEndChild(blendHeight);

	blendHeight = document->NewElement("blendHeight3");
	blendHeight->SetText(spec->BlendHeight.z);
	element->LinkEndChild(blendHeight);

	blendHeight = document->NewElement("blendHeight4");
	blendHeight->SetText(spec->BlendHeight.w);
	element->LinkEndChild(blendHeight);

	element = document->NewElement("BlendUse");
	node->LinkEndChild(element);

	Xml::XMLElement * useBlend = NULL;
	useBlend = document->NewElement("UseBlend1");
	useBlend->SetText(spec->ActiveBlend.x);
	element->LinkEndChild(useBlend);

	useBlend = document->NewElement("UseBlend2");
	useBlend->SetText(spec->ActiveBlend.y);
	element->LinkEndChild(useBlend);

	useBlend = document->NewElement("UseBlend3");
	useBlend->SetText(spec->ActiveBlend.z);
	element->LinkEndChild(useBlend);

	//TerrainDetail정보 저장
	element = document->NewElement("DetailValue");
	element->SetText(spec->DetailValue);
	node->LinkEndChild(element);

	element = document->NewElement("DetailIntensity");
	element->SetText(spec->DetailIntensity);
	node->LinkEndChild(element);

	element = document->NewElement("AmbientColor");
	node->LinkEndChild(element);

	Xml::XMLElement * ambientColor = NULL;
	ambientColor = document->NewElement("AmbientR");
	ambientColor->SetText(spec->AmbientColor.r);
	element->LinkEndChild(ambientColor);

	ambientColor = document->NewElement("AmbientG");
	ambientColor->SetText(spec->AmbientColor.g);
	element->LinkEndChild(ambientColor);

	ambientColor = document->NewElement("AmbientB");
	ambientColor->SetText(spec->AmbientColor.b);
	element->LinkEndChild(ambientColor);

	ambientColor = document->NewElement("AmbientA");
	ambientColor->SetText(spec->AmbientColor.a);
	element->LinkEndChild(ambientColor);

	//터레인 디퓨즈,스펙큘러,노말,디테일 복사
	element = document->NewElement("TerrainDiffuse");
	element->SetText(String::ToString(spec->MaterialValueFile[0]).c_str());

	node->LinkEndChild(element);

	element = document->NewElement("DiffuseColor");
	Xml::XMLElement * diffuseColor = NULL;
	diffuseColor = document->NewElement("DiffuseR");
	diffuseColor->SetText(spec->DiffuseColor.r);
	element->LinkEndChild(diffuseColor);

	diffuseColor = document->NewElement("DiffuseG");
	diffuseColor->SetText(spec->DiffuseColor.g);
	element->LinkEndChild(diffuseColor);

	diffuseColor = document->NewElement("DiffuseB");
	diffuseColor->SetText(spec->DiffuseColor.b);
	element->LinkEndChild(diffuseColor);

	diffuseColor = document->NewElement("DiffuseA");
	diffuseColor->SetText(spec->DiffuseColor.a);
	element->LinkEndChild(diffuseColor);
	node->LinkEndChild(element);

	element = document->NewElement("TerrainSpecular");
	element->SetText(String::ToString(spec->MaterialValueFile[1]).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("SpecularColor");

	Xml::XMLElement * specularColor = NULL;
	specularColor = document->NewElement("SpecularR");
	specularColor->SetText(spec->SpecularColor.r);
	element->LinkEndChild(specularColor);

	specularColor = document->NewElement("SpecularG");
	specularColor->SetText(spec->SpecularColor.g);
	element->LinkEndChild(specularColor);

	specularColor = document->NewElement("SpecularB");
	specularColor->SetText(spec->SpecularColor.b);
	element->LinkEndChild(specularColor);

	specularColor = document->NewElement("SpecularA");
	specularColor->SetText(spec->SpecularColor.a);
	element->LinkEndChild(specularColor);
	node->LinkEndChild(element);

	element = document->NewElement("SpecularIntensity");
	element->SetText(spec->SpecularColor.a);
	node->LinkEndChild(element);

	element = document->NewElement("TerrainNormal");
	element->SetText(String::ToString(spec->MaterialValueFile[2]).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("TerrainDetail");
	element->SetText(String::ToString(spec->MaterialValueFile[3]).c_str());
	node->LinkEndChild(element);

	//터레인렌더 버퍼저장
	element = document->NewElement("TexScale");
	element->SetText(spec->TexScale);
	node->LinkEndChild(element);

	element = document->NewElement("FogStart");
	element->SetText(spec->FogStart);
	node->LinkEndChild(element);

	element = document->NewElement("FogRange");
	element->SetText(spec->FogRange);
	node->LinkEndChild(element);

	node = document->NewElement("GrassDesc");
	root->LinkEndChild(node);

	element = document->NewElement("Wind");
	node->LinkEndChild(element);

	Xml::XMLElement * wind = NULL;
	wind = document->NewElement("WindX");
	wind->SetText(spec->WindVelocity.x);
	element->LinkEndChild(wind);

	wind = document->NewElement("WindY");
	wind->SetText(spec->WindVelocity.y);
	element->LinkEndChild(wind);

	element = document->NewElement("GrassCount");
	element->SetText(spec->GrassCount);
	node->LinkEndChild(element);

	Xml::XMLElement * grass = NULL;
	for (UINT i = 0; i < spec->GrassCount; i++)
	{
		element = document->NewElement(string("Grass").append(to_string(i)).c_str());
		node->LinkEndChild(element);

		grass = document->NewElement("GrassDiffuseMap");
		grass->SetText(String::ToString(spec->grassDesc[i].GrassDiffuseMap).c_str());
		element->LinkEndChild(grass);

		grass = document->NewElement("GrassNormalMap");
		grass->SetText(String::ToString(spec->grassDesc[i].GrassNormalMap).c_str());
		element->LinkEndChild(grass);

		grass = document->NewElement("GrassSpecularMap");
		grass->SetText(String::ToString(spec->grassDesc[i].GrassSpecularMap).c_str());
		element->LinkEndChild(grass);

		grass = document->NewElement("GrassDetailMap");
		grass->SetText(String::ToString(spec->grassDesc[i].GrassDetailMap).c_str());
		element->LinkEndChild(grass);

		grass = document->NewElement("GrassSize");
		grass->SetText(spec->grassDesc[i].GrassSize);
		element->LinkEndChild(grass);

		grass = document->NewElement("GrassPosition");
		element->LinkEndChild(grass);

		Xml::XMLElement * position = NULL;
		position = document->NewElement("GrassX");
		position->SetText(spec->grassDesc[i].GrassPosition.x);
		grass->LinkEndChild(position);
		position = document->NewElement("GrassY");
		position->SetText(spec->grassDesc[i].GrassPosition.y);
		grass->LinkEndChild(position);
		position = document->NewElement("GrassZ");
		position->SetText(spec->grassDesc[i].GrassPosition.z);
		grass->LinkEndChild(position);

		grass = document->NewElement("NumOfPatchRows");
		grass->SetText(spec->grassDesc[i].NumOfPatchRows);
		element->LinkEndChild(grass);

		grass = document->NewElement("NumOfRootsInPatch");
		grass->SetText(spec->grassDesc[i].NumOfRootsInPatch);
		element->LinkEndChild(grass);

		grass = document->NewElement("NumOfRoots");
		grass->SetText(spec->grassDesc[i].NumOfRoots);
		element->LinkEndChild(grass);

		grass = document->NewElement("IsFlower");
		grass->SetText(spec->grassDesc[i].IsFlower);
		element->LinkEndChild(grass);

		grass = document->NewElement("FlowerNum");
		grass->SetText(spec->grassDesc[i].FlowerNum);
		element->LinkEndChild(grass);
	}

	node = document->NewElement("BillboardDesc");
	root->LinkEndChild(node);

	element = document->NewElement("BillboardCount");
	element->SetText(spec->BillboardCount);
	node->LinkEndChild(element);

	Xml::XMLElement * bill = NULL;
	for (UINT i = 0; i < spec->BillboardCount; i++)
	{
		element = document->NewElement(string("Billboard").append(to_string(i)).c_str());
		node->LinkEndChild(element);

		bill = document->NewElement("BillboardType");
		bill->SetText(spec->billDesc[i].BillType);
		element->LinkEndChild(bill);

		bill = document->NewElement("TextureNum");
		bill->SetText(spec->billDesc[i].TextureNumber);
		element->LinkEndChild(bill);

		bill = document->NewElement("BillSize");
		element->LinkEndChild(bill);

		Xml::XMLElement * size = NULL;
		size = document->NewElement("X");
		size->SetText(spec->billDesc[i].Size.x);
		bill->LinkEndChild(size);
		size = document->NewElement("Y");
		size->SetText(spec->billDesc[i].Size.y);
		bill->LinkEndChild(size);

		bill = document->NewElement("BillPosition");
		element->LinkEndChild(bill);

		Xml::XMLElement * position = NULL;
		position = document->NewElement("X");
		position->SetText(spec->billDesc[i].Position.x);
		bill->LinkEndChild(position);
		position = document->NewElement("Y");
		position->SetText(spec->billDesc[i].Position.y);
		bill->LinkEndChild(position);
		position = document->NewElement("Z");
		position->SetText(spec->billDesc[i].Position.z);
		bill->LinkEndChild(position);
	}

	string file = String::ToString(Datas + L"Terrain/" + fileName + L".spec");
	document->SaveFile(file.c_str());

	SAFE_DELETE(document);
}

GameDataSpec * GameDataSpecManager::LoadSky(wstring file)
{
	GameSkySpec * spec = new GameSkySpec();

	Xml::XMLDocument*document = new Xml::XMLDocument();

	wstring tempFile = Datas + L"Sky/" + file;
	Xml::XMLError error = document->LoadFile(String::ToString(tempFile).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement*root = document->FirstChildElement();
	Xml::XMLElement*matNode = root->FirstChildElement();

	Xml::XMLElement*node = NULL;

	node = matNode->FirstChildElement(); // loadStarMap
	spec->LoadStarMap = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // loadMoonMap
	spec->LoadMoonMap = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // loadGlowMap
	spec->LoadGlowMap = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // loadcloudMap
	spec->LoadCloudMap = String::ToWString(node->GetText());

	node = node->NextSiblingElement(); // Leans
	spec->SetAngle = node->FloatText();

	node = node->NextSiblingElement(); // TimeFactor
	spec->TimeFactor = node->FloatText();

	node = node->NextSiblingElement(); // SampleCount
	spec->SampleCount = (int)node->FloatText();

	node = node->NextSiblingElement(); // WaveLength
	Xml::XMLElement*waveLength = node->FirstChildElement();
	spec->WaveLength.x = waveLength->FloatText();

	waveLength = waveLength->NextSiblingElement();
	spec->WaveLength.y = waveLength->FloatText();

	waveLength = waveLength->NextSiblingElement();
	spec->WaveLength.z = waveLength->FloatText();

	node = node->NextSiblingElement(); // SampleCount
	spec->StartTheta = node->FloatText();

	spec->SourceFilePath = file;

	specList.push_back(spec);

	SAFE_DELETE(document);

	return spec;
}

void GameDataSpecManager::WriteSky(wstring fileName, GameSkySpec * spec)
{
	Path::CreateFolder(Datas + L"Sky/");

	//Desc의 정보들 전부 저장
	Xml::XMLDocument*document = new Xml::XMLDocument();
	Xml::XMLDeclaration * decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement * root = document->NewElement("SkyDesc");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	Xml::XMLElement*node = document->NewElement("Sky");
	root->LinkEndChild(node);

	Xml::XMLElement * element = NULL;

	element = document->NewElement("StarMap");
	element->SetText(String::ToString(spec->LoadStarMap).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("MoonMap");
	element->SetText(String::ToString(spec->LoadMoonMap).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("GlowMap");
	element->SetText(String::ToString(spec->LoadGlowMap).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("CloudMap");
	element->SetText(String::ToString(spec->LoadCloudMap).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("Leans");
	element->SetText(spec->SetAngle);
	node->LinkEndChild(element);

	element = document->NewElement("TimeFactor");
	element->SetText(spec->TimeFactor);
	node->LinkEndChild(element);

	element = document->NewElement("SampleCount");
	element->SetText(spec->SampleCount);
	node->LinkEndChild(element);

	element = document->NewElement("WaveLength");
	node->LinkEndChild(element);

	Xml::XMLElement * wave = NULL;
	wave = document->NewElement("WaveLengthX");
	wave->SetText(spec->WaveLength.x);
	element->LinkEndChild(wave);

	wave = document->NewElement("WaveLengthY");
	wave->SetText(spec->WaveLength.y);
	element->LinkEndChild(wave);

	wave = document->NewElement("WaveLengthZ");
	wave->SetText(spec->WaveLength.z);
	element->LinkEndChild(wave);

	node->LinkEndChild(element);
	element = document->NewElement("StartTheta");
	element->SetText(spec->StartTheta);
	node->LinkEndChild(element);

	string file = String::ToString(Datas + L"Sky/" + fileName + L".spec");
	document->SaveFile(file.c_str());

	SAFE_DELETE(document);
}

GameDataSpec * GameDataSpecManager::LoadScene(wstring file)
{
	GameSceneSpec * spec = new GameSceneSpec();

	Xml::XMLDocument*document = new Xml::XMLDocument();

	wstring tempFile = Datas + L"Scene/" + file;
	Xml::XMLError error = document->LoadFile(String::ToString(tempFile).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement*root = document->FirstChildElement();
	Xml::XMLElement*matNode = root->FirstChildElement();

	Xml::XMLElement*node = NULL;

	node = matNode->FirstChildElement();
	spec->SkySpec = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	spec->TerrainSpec = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	Xml::XMLElement * player = NULL;
	player = node->FirstChildElement();
	spec->PlayerSpec.Name = String::ToWString(player->GetText());

	player = player->NextSiblingElement();

	D3DXMATRIX world;
	GetTransforms(&player, &world);
	GetVectors(&spec->PlayerSpec.Scale, &spec->PlayerSpec.Rotation, &spec->PlayerSpec.Position, world);


	node = node->NextSiblingElement();
	Xml::XMLElement * enemy = NULL;
	enemy = node->FirstChildElement();
	spec->EnemyCount = (UINT)enemy->FloatText();

	for (UINT i = 0; i < spec->EnemyCount; i++)
	{
		enemy = enemy->NextSiblingElement();

		GameSceneSpec::Spec enemySpec;
		enemySpec.Name = String::ToWString(enemy->GetText());

		enemy = enemy->NextSiblingElement();
		GetTransforms(&enemy, &world);
		GetVectors(&enemySpec.Scale, &enemySpec.Rotation, &enemySpec.Position, world);

		spec->EnemySpecs.push_back(enemySpec);
	}

	node = node->NextSiblingElement();
	Xml::XMLElement * obj = NULL;
	obj = node->FirstChildElement();
	spec->ObjectCount = (UINT)obj->FloatText();

	for (UINT i = 0; i < spec->ObjectCount; i++)
	{
		obj = obj->NextSiblingElement();

		GameSceneSpec::Spec objSpec;
		objSpec.Name = String::ToWString(obj->GetText());

		obj = obj->NextSiblingElement();
		GetTransforms(&obj, &world);
		GetVectors(&objSpec.Scale, &objSpec.Rotation, &objSpec.Position, world);

		spec->ObjectSpecs.push_back(objSpec);
	}

	spec->SourceFilePath = file;
	specList.push_back(spec);

	SAFE_DELETE(document);

	return spec;
}

void GameDataSpecManager::WriteScene(wstring fileName, GameSceneSpec * spec)
{
	Path::CreateFolder(Datas + L"Scene/");

	//Desc의 정보들 전부 저장
	Xml::XMLDocument*document = new Xml::XMLDocument();
	Xml::XMLDeclaration * decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement * root = document->NewElement("SceneSpec");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	Xml::XMLElement*node = document->NewElement("Scene");
	root->LinkEndChild(node);

	Xml::XMLElement * element = NULL;

	element = document->NewElement("SkySpec");
	element->SetText(String::ToString(spec->SkySpec).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("TerrainSpec");
	element->SetText(String::ToString(spec->TerrainSpec).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("PlayerSpec");
	node->LinkEndChild(element);

	Xml::XMLElement * player = NULL;
	player = document->NewElement("Spec");
	player->SetText(String::ToString(spec->PlayerSpec.Name).c_str());
	element->LinkEndChild(player);

	D3DXMATRIX world = GetMatrix(spec->PlayerSpec.Scale, spec->PlayerSpec.Rotation, spec->PlayerSpec.Position);
	SetTransforms(player, element, document, world);

	element = document->NewElement("EnemySpecs");
	node->LinkEndChild(element);

	Xml::XMLElement * enemy = NULL;
	enemy = document->NewElement("EnemyCount");
	enemy->SetText(spec->EnemyCount);
	element->LinkEndChild(enemy);

	for (UINT i = 0; i < spec->EnemyCount; i++)
	{
		enemy = document->NewElement(string("Enemy").append(to_string(i)).c_str());
		enemy->SetText(String::ToString(spec->EnemySpecs[i].Name).c_str());
		element->LinkEndChild(enemy);

		D3DXMATRIX world = GetMatrix(spec->EnemySpecs[i].Scale, spec->EnemySpecs[i].Rotation, spec->EnemySpecs[i].Position);
		SetTransforms(enemy, element, document, world);
	}

	element = document->NewElement("ObjectSpecs");
	node->LinkEndChild(element);

	Xml::XMLElement * obj = NULL;
	obj = document->NewElement("ObjectCount");
	obj->SetText(spec->ObjectCount);
	element->LinkEndChild(obj);

	for (UINT i = 0; i < spec->ObjectCount; i++)
	{
		obj = document->NewElement(string("Object").append(to_string(i)).c_str());
		obj->SetText(String::ToString(spec->ObjectSpecs[i].Name).c_str());
		element->LinkEndChild(obj);

		D3DXMATRIX world = GetMatrix(spec->ObjectSpecs[i].Scale, spec->ObjectSpecs[i].Rotation, spec->ObjectSpecs[i].Position);
		SetTransforms(obj, element, document, world);
	}

	string file = String::ToString(Datas + L"Scene/" + fileName + L".spec");
	document->SaveFile(file.c_str());

	SAFE_DELETE(document);
}

GameDataSpec * GameDataSpecManager::Find(wstring fileName)
{
	for (GameDataSpec * spec : specList)
	{
		GameCharacterSpec * charSpec = dynamic_cast<GameCharacterSpec*>(spec);
		if (charSpec != NULL)
		{
			if (charSpec->Name == fileName)
				return spec;
		}
		else
		{
			if (spec->SourceFilePath == fileName)
				return spec;
		}
	}

	return nullptr;
}

void GameDataSpecManager::SetTransforms(Xml::XMLElement * parent, Xml::XMLElement * element, Xml::XMLDocument * document, D3DXMATRIX  matrix)
{
	parent = document->NewElement("Position");
	element->LinkEndChild(parent);

	D3DXVECTOR3 trans, scale, rot;
	D3DXQUATERNION quat;
	D3DXMatrixDecompose(&scale, &quat, &trans, &matrix);
	Math::QuatToYawPithRoll(quat, rot.x, rot.y, rot.z);

	Xml::XMLElement * transforms = NULL;
	{
		transforms = document->NewElement("X");
		transforms->SetText(trans.x);
		parent->LinkEndChild(transforms);
		transforms = document->NewElement("Y");
		transforms->SetText(trans.y);
		parent->LinkEndChild(transforms);
		transforms = document->NewElement("Z");
		transforms->SetText(trans.z);
		parent->LinkEndChild(transforms);
	}
	parent = document->NewElement("Rotation");
	element->LinkEndChild(parent);
	{
		transforms = document->NewElement("X");
		transforms->SetText(rot.x);
		parent->LinkEndChild(transforms);
		transforms = document->NewElement("Y");
		transforms->SetText(rot.y);
		parent->LinkEndChild(transforms);
		transforms = document->NewElement("Z");
		transforms->SetText(rot.z);
		parent->LinkEndChild(transforms);
	}
	parent = document->NewElement("Scale");
	element->LinkEndChild(parent);
	{
		transforms = document->NewElement("X");
		transforms->SetText(scale.x);
		parent->LinkEndChild(transforms);
		transforms = document->NewElement("Y");
		transforms->SetText(scale.y);
		parent->LinkEndChild(transforms);
		transforms = document->NewElement("Z");
		transforms->SetText(scale.z);
		parent->LinkEndChild(transforms);
	}
}

void GameDataSpecManager::GetTransforms(Xml::XMLElement ** parent, D3DXMATRIX * matrix)
{
	Xml::XMLElement * transform = NULL;

	D3DXVECTOR3 trans, scale, rot;

	transform = (*parent)->FirstChildElement();
	trans.x = transform->FloatText();
	transform = transform->NextSiblingElement();
	trans.y = transform->FloatText();
	transform = transform->NextSiblingElement();
	trans.z = transform->FloatText();

	*parent = (*parent)->NextSiblingElement();
	transform = (*parent)->FirstChildElement();
	rot.x = transform->FloatText();
	transform = transform->NextSiblingElement();
	rot.y = transform->FloatText();
	transform = transform->NextSiblingElement();
	rot.z = transform->FloatText();

	*parent = (*parent)->NextSiblingElement();
	transform = (*parent)->FirstChildElement();
	scale.x = transform->FloatText();
	transform = transform->NextSiblingElement();
	scale.y = transform->FloatText();
	transform = transform->NextSiblingElement();
	scale.z = transform->FloatText();

	D3DXMATRIX S, R, T;
	D3DXQUATERNION Q;
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXQuaternionRotationYawPitchRoll(&Q, rot.y, rot.x, rot.z);
	D3DXMatrixRotationQuaternion(&R, &Q);
	D3DXMatrixTranslation(&T, trans.x, trans.y, trans.z);

	*matrix = S * R * T;
}

D3DXMATRIX GameDataSpecManager::GetMatrix(D3DXVECTOR3 s, D3DXVECTOR3 r, D3DXVECTOR3 p)
{
	D3DXMATRIX S, R, T;
	D3DXQUATERNION Q;
	D3DXMatrixScaling(&S, s.x, s.y, s.z);
	D3DXQuaternionRotationYawPitchRoll(&Q, r.y, r.x, r.z);
	D3DXMatrixRotationQuaternion(&R, &Q);
	D3DXMatrixTranslation(&T, p.x, p.y, p.z);

	return S * R * T;
}

void GameDataSpecManager::GetVectors(D3DXVECTOR3 * s, D3DXVECTOR3 * r, D3DXVECTOR3 * p, D3DXMATRIX w)
{
	D3DXVECTOR3 trans, rot, scale;
	D3DXQUATERNION quat;
	D3DXMatrixDecompose(&scale, &quat, &trans, &w);
	Math::QuatToYawPithRoll(quat, rot.x, rot.y, rot.z);

	*s = scale;
	*r = rot;
	*p = trans;
}

GameCharacterSpec::~GameCharacterSpec() 
{
	if(bDelete == true)
		SAFE_DELETE(BT);
}
