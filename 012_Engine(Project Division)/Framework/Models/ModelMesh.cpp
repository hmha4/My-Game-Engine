#include "Framework.h"
#include "ModelMesh.h"
#include "ModelMeshPart.h"
#include "ModelBone.h"

ModelMesh::ModelMesh()
{
}

ModelMesh::~ModelMesh()
{
	for (ModelMeshPart * part : meshParts)
		SAFE_DELETE(part);
}

void ModelMesh::Render()
{
	for (ModelMeshPart* part : meshParts)
	{
		part->Render();
	}
}

void ModelMesh::Release()
{
	for (ModelMeshPart * part : meshParts)
		part->Release();
}

void ModelMesh::Pass(UINT val)
{
	for (ModelMeshPart * part : meshParts)
		part->Pass(val);
}

void ModelMesh::Clone(void ** clone)
{
	ModelMesh *mesh = new ModelMesh();
	mesh->name = name;
	mesh->parentBoneIndex = parentBoneIndex;

	for (ModelMeshPart * part : meshParts)
	{
		ModelMeshPart *temp = NULL;
		part->Clone((void **)&temp);

		temp->parent = mesh;
		mesh->meshParts.push_back(temp);
	}

	*clone = mesh;
}

void ModelMesh::Binding()
{
	for (ModelMeshPart* part : meshParts)
		part->Binding();
}