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
	specListIter = specList.begin();
	for (specListIter; specListIter != specList.end(); specListIter++)
	{
		SAFE_DELETE((*specListIter).second);
	}

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

		Xml::XMLElement * position = temp->FirstChildElement();
		T.x = position->FloatText();
		position = position->NextSiblingElement();
		T.y = position->FloatText();
		position = position->NextSiblingElement();
		T.z = position->FloatText();

		temp = temp->NextSiblingElement();
		Xml::XMLElement * rotation = temp->FirstChildElement();
		R.x = rotation->FloatText();
		rotation = rotation->NextSiblingElement();
		R.y = rotation->FloatText();
		rotation = rotation->NextSiblingElement();
		R.z = rotation->FloatText();

		temp = temp->NextSiblingElement();
		Xml::XMLElement * scale = temp->FirstChildElement();
		S.x = scale->FloatText();
		scale = scale->NextSiblingElement();
		S.y = scale->FloatText();
		scale = scale->NextSiblingElement();
		S.z = scale->FloatText();

		D3DXMatrixScaling(&s, S.x, S.y, S.z);
		D3DXMatrixRotationYawPitchRoll(&r, R.y, R.x, R.z);
		D3DXMatrixTranslation(&t, T.x, T.y, T.z);

		W = s * r * t;

		col.RootMatrix = W;

		spec->Colliders.push_back(col);
	}

	node = node->NextSiblingElement();
	spec->IsChild = node->FloatText();

	node = node->NextSiblingElement();
	spec->ParentBoneIndex = (int)node->FloatText();

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
	spec->BT->ReadBehaviorTree(childSib, spec->BT->GetRoot());
	spec->BT->BindNodes();

	specListIter = specList.find(file);

	if (specListIter == specList.end())
		specList.insert(make_pair(file, spec));

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

		D3DXVECTOR3 scale, trans, rot;
		D3DXQUATERNION quat;
		D3DXMatrixDecompose(&scale, &quat, &trans, &spec->Colliders[i].RootMatrix);
		Math::QuatToYawPithRoll(quat, rot.x, rot.y, rot.z);

		Xml::XMLElement * vector = NULL;
		vector = document->NewElement("Position");
		element->LinkEndChild(vector);

		Xml::XMLElement * position = NULL;
		position = document->NewElement("X");
		position->SetText(trans.x);
		vector->LinkEndChild(position);
		position = document->NewElement("Y");
		position->SetText(trans.y);
		vector->LinkEndChild(position);
		position = document->NewElement("Z");
		position->SetText(trans.z);
		vector->LinkEndChild(position);

		vector = document->NewElement("Rotation");
		element->LinkEndChild(vector);

		Xml::XMLElement * rotation = NULL;
		rotation = document->NewElement("X");
		rotation->SetText(rot.x);
		vector->LinkEndChild(rotation);
		rotation = document->NewElement("Y");
		rotation->SetText(rot.y);
		vector->LinkEndChild(rotation);
		rotation = document->NewElement("Z");
		rotation->SetText(rot.z);
		vector->LinkEndChild(rotation);

		vector = document->NewElement("Scale");
		element->LinkEndChild(vector);

		Xml::XMLElement * scaling = NULL;
		scaling = document->NewElement("X");
		scaling->SetText(scale.x);
		vector->LinkEndChild(scaling);
		scaling = document->NewElement("Y");
		scaling->SetText(scale.y);
		vector->LinkEndChild(scaling);
		scaling = document->NewElement("Z");
		scaling->SetText(scale.z);
		vector->LinkEndChild(scaling);
	}

	node = document->NewElement("IsChild");
	node->SetText(spec->IsChild);
	root->LinkEndChild(node);

	node = document->NewElement("ParentBoneIndex");
	node->SetText(spec->ParentBoneIndex);
	root->LinkEndChild(node);

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
	node->SetText(String::ToString(spec->BT->Name()).c_str());
	root->LinkEndChild(node);

	Xml::XMLElement * tempEle = NULL;
	node = document->NewElement("BehaviourTree");
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

	specListIter = specList.find(file);

	if(specListIter == specList.end())
		specList.insert(make_pair(file, spec));

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

	specListIter = specList.find(file);

	if (specListIter == specList.end())
	specList.insert(make_pair(file, spec));

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
	spec->PlayerSpec = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	spec->EnemyCount = (UINT)node->FloatText();

	node = node->NextSiblingElement();

	Xml::XMLElement * enemySpecs = node->FirstChildElement();
	for (UINT i = 0; i < spec->EnemyCount; i++)
	{
		spec->EnemySpecs.push_back(String::ToWString(enemySpecs->GetText()));
		enemySpecs->NextSiblingElement();
	}

	node = node->NextSiblingElement();
	spec->ObjectCount = (UINT)node->FloatText();

	node = node->NextSiblingElement();

	Xml::XMLElement * objSpecs = node->FirstChildElement();
	for (UINT i = 0; i < spec->ObjectCount; i++)
	{
		spec->ObjectSpecs.push_back(String::ToWString(objSpecs->GetText()));
		objSpecs->NextSiblingElement();
	}

	spec->SourceFilePath = file;

	specListIter = specList.find(file);

	if (specListIter == specList.end())
	specList.insert(make_pair(file, spec));

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
	element->SetText(String::ToString(spec->PlayerSpec).c_str());
	node->LinkEndChild(element);

	element = document->NewElement("EnemyCount");
	element->SetText(spec->EnemyCount);
	node->LinkEndChild(element);

	element = document->NewElement("EnemySpecs");
	node->LinkEndChild(element);

	Xml::XMLElement * child = NULL;
	for (UINT i = 0; i < spec->EnemyCount; i++)
	{
		child = document->NewElement(string("Enemy").append(to_string(i)).c_str());
		child->SetText(String::ToString(spec->EnemySpecs[i]).c_str());
		element->LinkEndChild(child);
	}

	element = document->NewElement("ObjectCount");
	element->SetText(spec->ObjectCount);
	node->LinkEndChild(element);

	element = document->NewElement("ObjectSpecs");
	node->LinkEndChild(element);

	for (UINT i = 0; i < spec->ObjectCount; i++)
	{
		child = document->NewElement(string("Object").append(to_string(i)).c_str());
		child->SetText(String::ToString(spec->ObjectSpecs[i]).c_str());
		element->LinkEndChild(child);
	}

	string file = String::ToString(Datas + L"Scene/" + fileName + L".spec");
	document->SaveFile(file.c_str());

	SAFE_DELETE(document);
}

GameDataSpec * GameDataSpecManager::Find(wstring fileName)
{
	specListIter = specList.find(fileName);

	if (specListIter != specList.end())
		return (*specListIter).second;

	return nullptr;
}