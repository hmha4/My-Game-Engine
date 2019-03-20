#include "Framework.h"
#include "Model.h"
#include "../Utilities/Xml.h"
#include "../Utilities/BinaryFile.h"

void Model::ReadMaterial(wstring file)
{
	Models::LoadMaterial(file, &materials);
}

void Model::ReadMesh(wstring file)
{
	Models::LoadMesh(file, &bones, &meshes);
	name = Path::GetFileName(file);

	ReorderMesh();
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
			for (Material * material : materials)
			{
				if (material->Name() == part->materialName)
				{
					part->material = material;

					break;
				}
			}
		}

		//	Create vertex buffer
		mesh->Binding();
	}
}

void Model::ReorderMesh()
{
	//TODO: Have to Reorder Mesh
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
	//	Load file
	Xml::XMLError error = document->LoadFile(String::ToString(tempFile).c_str());
	//	it's a success if error returns SML_SUCCESS
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
		wstring diffuseTexture = String::ToWString(node->GetText());

		node = node->NextSiblingElement();
		wstring specularTexture = String::ToWString(node->GetText());

		node = node->NextSiblingElement();
		wstring normalTexture = String::ToWString(node->GetText());

		wstring directory = Path::GetDirectoryName(file);
		if (diffuseTexture.length() > 0)
			material->SetDiffuseMap(directory + diffuseTexture);
		if (specularTexture.length() > 0)
			material->SetSpecularMap(directory + specularTexture);
		if (normalTexture.length() > 0)
			material->SetNormalMap(directory + normalTexture);

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
		color = node->FirstChildElement();
		dxColor.r = color->FloatText();

		color = color->NextSiblingElement();
		dxColor.g = color->FloatText();

		color = color->NextSiblingElement();
		dxColor.b = color->FloatText();

		color = color->NextSiblingElement();
		dxColor.a = color->FloatText();

		color = node->NextSiblingElement();
		dxColor.a = color->FloatText();
		//material->SetShininess(dxColor.a);
		material->SetSpecular(dxColor);

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
	//	Read in-order you writed
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

		//	VertexData
		{
			UINT count = r->UInt();
			//	vector.assign(n, x) : allocate n elements of x value
			//	vector.reserve(n) : reserve n size space to allocate n elements
			vector<VertexTextureNormalTangentBlend> vertices;
			vertices.assign(count, VertexTextureNormalTangentBlend());

			void* ptr = (void *)&(vertices[0]);
			r->Byte(&ptr, sizeof(VertexTextureNormalTangentBlend) * count);

			mesh->vertices = new VertexTextureNormalTangentBlend[count];
			mesh->vertexCount = count;
			copy
			(
				vertices.begin(), vertices.end(),
				stdext::checked_array_iterator<VertexTextureNormalTangentBlend *>(mesh->vertices, count)
			);
		}

		UINT partCount = r->UInt();
		for (UINT k = 0; k < partCount; k++)
		{
			ModelMeshPart *meshPart = new ModelMeshPart();
			meshPart->parent = mesh;
			meshPart->materialName = String::ToWString(r->String());

			meshPart->startVertex = r->UInt();
			meshPart->vertexCount = r->UInt();

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