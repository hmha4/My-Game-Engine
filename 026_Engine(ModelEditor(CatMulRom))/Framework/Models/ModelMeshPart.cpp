#include "Framework.h"
#include "ModelMeshPart.h"
#include "ModelMesh.h"

ModelMeshPart::ModelMeshPart()
	: pass(0)
{
}

ModelMeshPart::~ModelMeshPart()
{
}

void ModelMeshPart::Render()
{
	material->GetEffect()->Draw(0, pass, vertexCount, startVertex);
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