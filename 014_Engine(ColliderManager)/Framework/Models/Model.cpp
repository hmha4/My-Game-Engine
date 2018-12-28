#include "Framework.h"
#include "Model.h"
#include "ModelBone.h"
#include "ModelMesh.h"
#include "ModelClip.h"


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

void Model::RetargetBone(ModelClip *clip, ModelKeyframe * keyframe, ModelBone * bone, int index, int parent)
{
	ModelKeyframe * temp = new ModelKeyframe();
	{
		temp->boneName = bone->name;
		temp->index = index;
		temp->parentIndex = parent;
		temp->duration = keyframe->duration;
		temp->frameRate = keyframe->frameRate;
		temp->frameCount = keyframe->frameCount;
		temp->transforms.assign(keyframe->transforms.begin(), keyframe->transforms.end());
	}
	clip->retargetFrame.insert(ModelClip::Pair(temp->boneName, temp));

	for (size_t i = 0; i < keyframe->childs.size(); i++)
	{
		if(bone->childs.size() > i)
			RetargetBone(clip, keyframe->childs[i], bone->childs[i], clip->retargetFrame.size(), index);
	}
}

void Model::RetargetBone(ModelClip * clip)
{
	clip->DeleteRetargetData();

	if(bones[1]->parentIndex == -1)
		RetargetBone(clip, clip->GetKeyframeByIndex(0), bones[1], -1, -1);
	else
		RetargetBone(clip, clip->GetKeyframeByIndex(0), bones[0], -1, -1);

	
	for (ModelClip::Pair it : clip->keyframeMap)
	{
		SAFE_DELETE(it.second);
	}
	clip->keyframeMap.clear();

	clip->keyframeMap = clip->retargetFrame;
	clip->BindingKeyframe();
}

void Model::CreateMinMax(D3DXMATRIX t)
{

}

void Model::UpdateMinMax(D3DXMATRIX pos)
{
	D3DXVec3TransformCoord(&minXZ, &startMinXZ, &pos);
	D3DXVec3TransformCoord(&maxXZ, &startMaxXZ, &pos);
	//minXZ = startMinXZ + D3DXVECTOR3(pos._41, pos._42, pos._43);
	//maxXZ = startMaxXZ + D3DXVECTOR3(pos._41, pos._42, pos._43);
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