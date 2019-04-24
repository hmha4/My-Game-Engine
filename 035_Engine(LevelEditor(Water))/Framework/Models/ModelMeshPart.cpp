#include "Framework.h"
#include "ModelMeshPart.h"

ModelMeshPart::ModelMeshPart()
	: pass(0), boneIndexVar(NULL)
{
}

ModelMeshPart::~ModelMeshPart()
{
}

void ModelMeshPart::Render()
{
	if (boneIndexVar == NULL)
		boneIndexVar = material->GetEffect()->AsScalar("BoneIndex");

	boneIndexVar->SetInt(parent->ParentBoneIndex());
	material->GetEffect()->Draw(0, pass, vertexCount, startVertex);
}

void ModelMeshPart::RenderInstance(UINT count, UINT tech)
{
	if (boneIndexVar == NULL)
		boneIndexVar = material->GetEffect()->AsScalar("BoneIndex");

	boneIndexVar->SetInt(parent->ParentBoneIndex());
	material->GetEffect()->DrawInstanced(tech, pass, vertexCount, count, startVertex);
}

void ModelMeshPart::Release()
{
}

void ModelMeshPart::Clone(void ** clone)
{
	ModelMeshPart *part = new ModelMeshPart();

	part->materialName = this->materialName;
	part->startVertex = this->startVertex;
	part->vertexCount = this->vertexCount;

	*clone = part;
}