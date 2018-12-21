#include "stdafx.h"
#include "Model.h"
#include "ModelMesh.h"
#include "ModelBone.h"
#include "ModelMeshPart.h"
#include "../Utilities/Xml.h"
#include "../Utilities/BinaryFile.h"

void Model::ReadMaterial(wstring folder, wstring file)
{
	Models::LoadMaterial(folder + file, &materials);
}

void Model::ReadMesh(wstring folder, wstring file)
{
	Models::LoadMesh(folder + file, &bones, &meshes);

	BindingBone();
	BindingMesh();
}

void Model::BindingBone()
{
	this->root = bones[0];
	for (ModelBone * bone : bones)
	{
		if (bone->parentIndex > -1)
		{
			bone->parent = bones[bone->parentIndex];
			bone->parent->childs.push_back(bone);
		}
		else
		{
			bone->parent = NULL;
		}
	}
}

void Model::BindingMesh()
{
	startMinXZ = D3DXVECTOR3(0, 0, 0);
	startMaxXZ = D3DXVECTOR3(0, 0, 0);

	for (ModelMesh * mesh : meshes)
	{
		for (ModelBone * bone : bones)
		{
			if (mesh->parentBoneIndex == bone->index)
			{
				mesh->parentBone = bone;

				break;
			}
		}

		for (ModelMeshPart * part : mesh->meshParts)
		{
			for (ModelVertexType vertex : part->vertices)
			{
				D3DXVECTOR3 ver = vertex.Position;
				D3DXVec3TransformCoord(&ver, &vertex.Position, &mesh->ParentBone()->global);

				if (startMinXZ.x > ver.x) startMinXZ.x = ver.x;
				if (startMinXZ.y > ver.y) startMinXZ.y = ver.y;
				if (startMinXZ.z > ver.z) startMinXZ.z = ver.z;

				if (startMaxXZ.x < ver.x) startMaxXZ.x = ver.x;
				if (startMaxXZ.y < ver.y) startMaxXZ.y = ver.y;
				if (startMaxXZ.z < ver.z) startMaxXZ.z = ver.z;
			}

			for (Material * material : materials)
			{
				if (material->Name() == part->materialName)
				{
					part->material = material;

					break;
				}
			}
		}


		//	메쉬에 속한 모델 파트의 정점과 인덱스를
		//	버퍼에 담는다.
		mesh->Binding();
	}
}


// --------------------------------------------------------------------- //
//	Model Manager
// --------------------------------------------------------------------- //
map<wstring, vector<Material*>>Models::materialMap;
void Models::LoadMaterial(wstring file, vector<Material*>* materials)
{
	if (materialMap.count(file) < 1)
		ReadMaterialData(file);

	for (Material * material : materialMap[file])
	{
		Material * temp = NULL;
		material->Clone((void**)&temp);

		materials->push_back(temp);
	}
}

void Models::ReadMaterialData(wstring file)
{
	vector<Material*> materials;

	Xml::XMLDocument * document = new Xml::XMLDocument();

	wstring tempFile = file;
	//	파일을 연다.
	Xml::XMLError error = document->LoadFile(String::ToString(tempFile).c_str());
	//	SML_SUCCESS가 반환되면 파일 열기 성공
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement * root = document->FirstChildElement();
	Xml::XMLElement * matNode = root->FirstChildElement();

	do
	{
		Xml::XMLElement * node = NULL;
		Material * material = new Material();

		node = matNode->FirstChildElement();
		material->Name(String::ToWString(node->GetText()));

		node = node->NextSiblingElement();

		D3DXCOLOR dxColor;
		Xml::XMLElement * color = node->FirstChildElement();
		dxColor.r = color->FloatText();

		color = color->NextSiblingElement();
		dxColor.g = color->FloatText();

		color = color->NextSiblingElement();
		dxColor.b = color->FloatText();

		color = color->NextSiblingElement();
		dxColor.a = color->FloatText();
		material->SetDiffuse(dxColor);

		node = node->NextSiblingElement();
		wstring diffuseTexture = String::ToWString(node->GetText());

		node = node->NextSiblingElement();
		wstring normalTexture = String::ToWString(node->GetText());

		node = node->NextSiblingElement();
		wstring specularTexture = String::ToWString(node->GetText());

		wstring directory = Path::GetDirectoryName(file);
		if (diffuseTexture.length() > 0)
			material->SetDiffuseMap(directory + diffuseTexture);
		if (normalTexture.length() > 0)
			material->SetNormalMap(directory + normalTexture);
		if (specularTexture.length() > 0)
			material->SetSpecularMap(directory + specularTexture);

		materials.push_back(material);
		matNode = matNode->NextSiblingElement();
	} while (matNode != NULL);

	materialMap[file] = materials;

	SAFE_DELETE(document);
}

map<wstring, Models::MeshData> Models::meshDataMap;
void Models::LoadMesh(wstring file, vector<class ModelBone*>* bones, vector<class ModelMesh*>* meshes)
{
	if (meshDataMap.count(file) < 1)
		ReadMeshData(file);

	MeshData data = meshDataMap[file];
	for (size_t i = 0; i < data.Bones.size(); i++)
	{
		ModelBone * bone = NULL;
		data.Bones[i]->Clone((void**)&bone);

		bones->push_back(bone);
	}

	for (size_t i = 0; i < data.Meshes.size(); i++)
	{
		ModelMesh * mesh = NULL;
		data.Meshes[i]->Clone((void**)&mesh);

		meshes->push_back(mesh);
	}
}

void Models::ReadMeshData(wstring file)
{
	//	저장한 순서 그대로 읽어야 한다.
	BinaryReader *r = new BinaryReader();
	r->Open(file);

	vector<ModelBone *>bones;
	vector<ModelMesh *>meshes;

	UINT count = 0;
	count = r->UInt();

	for (UINT i = 0; i < count; i++)
	{
		ModelBone *bone = new ModelBone();

		bone->index = r->Int();
		bone->name = String::ToWString(r->String());
		bone->parentIndex = r->Int();

		bone->local = r->Matrix();
		bone->global = r->Matrix();

		bones.push_back(bone);
	}

	count = r->UInt();
	for (UINT i = 0; i < count; i++)
	{
		ModelMesh *mesh = new ModelMesh();
		mesh->name = String::ToWString(r->String());

		mesh->parentBoneIndex = r->Int();

		UINT partCount = r->UInt();
		for (UINT k = 0; k < partCount; k++)
		{
			ModelMeshPart *meshPart = new ModelMeshPart();
			meshPart->parent = mesh;

			meshPart->materialName = String::ToWString(r->String());

			//VertexData
			{
				UINT count = r->UInt();
				//	vector.assign(n, x) : vector에 x의 값으로 n개의 원소를 할당
				//	vector.reserve(n) : n개의 원소를 저장할 공간을 예약
				meshPart->vertices.assign(count, ModelVertexType());

				void *ptr = (void *)&(meshPart->vertices[0]);
				r->Byte(&ptr, sizeof(ModelVertexType) * count);
			}

			//IndexData
			{
				UINT count = r->UInt();
				meshPart->indices.assign(count, UINT());

				void *ptr = (void *)&(meshPart->indices[0]);
				r->Byte(&ptr, sizeof(UINT) * count);
			}

			mesh->meshParts.push_back(meshPart);
		}//	for(k)

		meshes.push_back(mesh);

	}//	for(i)


	r->Close();
	SAFE_DELETE(r);

	MeshData data;
	data.Bones.assign(bones.begin(), bones.end());
	data.Meshes.assign(meshes.begin(), meshes.end());

	meshDataMap[file] = data;
}
