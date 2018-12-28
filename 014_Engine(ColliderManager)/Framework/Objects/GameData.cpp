#include "Framework.h"
#include "GameData.h"
#include "Utilities/Xml.h"

GameDataSpecManager * GameDataSpecManager::instance = NULL;

GameDataSpecManager::GameDataSpecManager()
{
}

GameDataSpecManager::~GameDataSpecManager()
{
	specListIter = specList.begin();
	for (specListIter; specListIter != specList.end(); specListIter++)
		SAFE_DELETE((*specListIter).second);

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

GameDataSpec * GameDataSpecManager::Load(wstring file)
{
	GameCharacterSpec * spec = new GameCharacterSpec();

	Xml::XMLDocument * document = new Xml::XMLDocument();

	wstring tempFile = file;
	//	파일을 연다.
	Xml::XMLError error = document->LoadFile(String::ToString(tempFile).c_str());
	//	SML_SUCCESS가 반환되면 파일 열기 성공
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement * root = document->FirstChildElement();

	Xml::XMLElement * node = NULL;
	
	node = root->FirstChildElement();
	spec->UnitType = (UnitType)node->IntText();

	node = node->NextSiblingElement();
	spec->Life = node->IntText();

	node = node->NextSiblingElement();
	spec->Radius = node->FloatText();

	node = node->NextSiblingElement();
	spec->RunSpeed = node->FloatText();

	node = node->NextSiblingElement();
	spec->WalkSpeed = node->FloatText();

	node = node->NextSiblingElement();
	spec->WalkBackwardSpeed = node->FloatText();

	node = node->NextSiblingElement();
	spec->ModelFilePath = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	spec->AnimationFolderPath = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	spec->DefaultWeaponFilePath = String::ToWString(node->GetText());

	node = node->NextSiblingElement();
	
	Xml::XMLElement * element = node->FirstChildElement();
	spec->CameraTargetOffset.x = element->FloatText();

	element = element->NextSiblingElement();
	spec->CameraTargetOffset.y = element->FloatText();

	element = element->NextSiblingElement();
	spec->CameraTargetOffset.z = element->FloatText();

	node = node->NextSiblingElement();

	element = node->FirstChildElement();
	spec->CameraPositionOffset.x = element->FloatText();

	element = element->NextSiblingElement();
	spec->CameraPositionOffset.y = element->FloatText();

	element = element->NextSiblingElement();
	spec->CameraPositionOffset.z = element->FloatText();

	spec->SourceFilePath = file;

	wstring fileName = Path::GetFileName(file);

	specList.insert(make_pair(fileName, spec));

	SAFE_DELETE(document);

	return spec;
}

void GameDataSpecManager::Write(wstring saveFolder, wstring fileName, GameCharacterSpec * spec)
{
	Path::CreateFolder(saveFolder);

	Xml::XMLDocument * document = new Xml::XMLDocument();
	Xml::XMLDeclaration * decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement * root = document->NewElement("GameCharacterSpec");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	Xml::XMLElement * node = document->NewElement("UnitType");
		node->SetText(spec->UnitType);
	root->LinkEndChild(node);

	node = document->NewElement("Life");
	node->SetText(spec->Life);
	root->LinkEndChild(node);

	node = document->NewElement("Radius");
	node->SetText(spec->Radius);
	root->LinkEndChild(node);

	node = document->NewElement("RunSpeed");
	node->SetText(spec->RunSpeed);
	root->LinkEndChild(node);

	node = document->NewElement("WalkSpeed");
	node->SetText(spec->WalkSpeed);
	root->LinkEndChild(node);

	node = document->NewElement("WalkBackwardSpeed");
	node->SetText(spec->WalkBackwardSpeed);
	root->LinkEndChild(node);

	node = document->NewElement("ModelFilePath");
	node->SetText(spec->ModelFilePath.c_str());
	root->LinkEndChild(node);

	node = document->NewElement("AnimationFolderPath");
	node->SetText(spec->AnimationFolderPath.c_str());
	root->LinkEndChild(node);

	node = document->NewElement("DefaultWeaponFilePath");
	node->SetText(spec->DefaultWeaponFilePath.c_str());
	root->LinkEndChild(node);

	node = document->NewElement("CameraTargetOffset");
	root->LinkEndChild(node);

	Xml::XMLElement * element = document->NewElement("X");
	element->SetText(spec->CameraTargetOffset.x);
	node->LinkEndChild(element);

	element = document->NewElement("Y");
	element->SetText(spec->CameraTargetOffset.y);
	node->LinkEndChild(element);

	element = document->NewElement("Z");
	element->SetText(spec->CameraTargetOffset.z);
	node->LinkEndChild(element);

	node = document->NewElement("CameraPositionOffset");
	root->LinkEndChild(node);

	element = document->NewElement("X");
	element->SetText(spec->CameraPositionOffset.x);
	node->LinkEndChild(element);

	element = document->NewElement("Y");
	element->SetText(spec->CameraPositionOffset.y);
	node->LinkEndChild(element);

	element = document->NewElement("Z");
	element->SetText(spec->CameraPositionOffset.z);
	node->LinkEndChild(element);

	string file = String::ToString(saveFolder + fileName);
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
