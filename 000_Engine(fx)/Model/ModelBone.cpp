#include "stdafx.h"
#include "ModelBone.h"

ModelBone::ModelBone()
	: index(-1), name(L"")
	, parent(NULL), parentIndex(-1)
{
	D3DXMatrixIdentity(&local);
	D3DXMatrixIdentity(&global);
	D3DXMatrixIdentity(&root);
	D3DXMatrixIdentity(&world);
}

ModelBone::~ModelBone()
{

}

void ModelBone::Clone(void ** clone)
{
	ModelBone * bone = new ModelBone();

	bone->index = this->index;
	bone->name = this->name;

	bone->parentIndex = this->parentIndex;

	bone->local = this->local;
	bone->global = this->global;
	bone->root = this->root;
	bone->world = this->world;

	*clone = bone;
}
