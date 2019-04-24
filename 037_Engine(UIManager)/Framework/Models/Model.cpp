#include "Framework.h"
#include "Model.h"

Model::Model()
	: root(NULL)
	, minXZ(0, 0, 0), maxXZ(0, 0, 0)
{
}

Model::~Model()
{
	for (Material *material : materials)
		SAFE_DELETE(material);

	for (ModelBone *bone : bones)
		SAFE_DELETE(bone);

	for (ModelMesh *mesh : meshes)
		mesh->Release();

	for (ModelMesh *mesh : meshes)
		SAFE_DELETE(mesh);
}

Material * Model::MaterialByName(wstring name)
{
	for (Material * material : materials)
	{
		if (material->Name() == name)
			return material;
	}

	return NULL;
}

Material * Model::MaterialByName(string name)
{
	wstring materialName = String::ToWString(name);

	return MaterialByName(materialName);
}

ModelBone * Model::BoneByName(wstring name)
{
	for (ModelBone * bone : bones)
	{
		if (bone->Name() == name)
			return bone;
	}

	return NULL;
}

ModelBone * Model::BoneByName(string name)
{
	wstring boneName = String::ToWString(name);

	return BoneByName(boneName);
}

ModelMesh * Model::MeshByName(wstring name)
{
	for (ModelMesh * mesh : meshes)
	{
		if (mesh->Name() == name)
			return mesh;
	}

	return NULL;
}

ModelMesh * Model::MeshByName(string name)
{
	wstring meshName = String::ToWString(name);

	return MeshByName(meshName);
}

void Model::CopyGlobalBoneTo(vector<D3DXMATRIX>& transforms)
{
	D3DXMATRIX w;
	D3DXMatrixIdentity(&w);

	CopyGlobalBoneTo(transforms, w);
}

void Model::CopyGlobalBoneTo(vector<D3DXMATRIX>& transforms, D3DXMATRIX& w)
{
	transforms.clear();
	transforms.assign(bones.size(), D3DXMATRIX());

	for (size_t i = 0; i < bones.size(); i++)
	{
		ModelBone* bone = bones[i];

		if (bone->Parent() != NULL)
		{
			//	transform[i] : 본의 출력 위치
			//	local * 부모 transform
			int index = bone->parent->index;
			transforms[i] = bone->root * bone->local * transforms[index];
		}
		else
		{
			transforms[i] = bone->root * bone->local * w;
		}
		bone->world = transforms[i];
	}
}

void Model::CreateMinMax(D3DXMATRIX t)
{
	startMinXZ = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
	startMaxXZ = D3DXVECTOR3(FLT_MIN, FLT_MIN, FLT_MIN);

	for (ModelMesh * mesh : meshes)
	{
		for (size_t i = 0; i < mesh->VertexCount(); i++)
		{
			D3DXVECTOR3 ver = mesh->Vertices()[i].Position;
			D3DXVec3TransformCoord(&ver, &mesh->Vertices()[i].Position, &mesh->ParentBone()->Global());

			if (startMinXZ.x > ver.x) startMinXZ.x = ver.x;
			if (startMinXZ.y > ver.y) startMinXZ.y = ver.y;
			if (startMinXZ.z > ver.z) startMinXZ.z = ver.z;

			if (startMaxXZ.x < ver.x) startMaxXZ.x = ver.x;
			if (startMaxXZ.y < ver.y) startMaxXZ.y = ver.y;
			if (startMaxXZ.z < ver.z) startMaxXZ.z = ver.z;
		}
	}
}

void Model::UpdateMinMax(D3DXMATRIX pos)
{
	D3DXVec3TransformCoord(&minXZ, &startMinXZ, &pos);
	D3DXVec3TransformCoord(&maxXZ, &startMaxXZ, &pos);
}

void Models::Create()
{
}

void Models::Delete()
{
	for (pair<wstring, vector<Material *>>temp : materialMap)
	{
		for (Material * material : temp.second)
			SAFE_DELETE(material);
	}

	for (pair<wstring, MeshData> temp : meshDataMap)
	{
		MeshData data = temp.second;

		for (ModelBone* bone : data.Bones)
			SAFE_DELETE(bone);

		for (ModelMesh* mesh : data.Meshes)
			SAFE_DELETE(mesh);
	}
}