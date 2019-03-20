#include "Framework.h"
#include "ModelMesh.h"
#include "ModelMeshPart.h"
#include "ModelBone.h"

ModelMesh::ModelMesh()
	: vertexBuffer(NULL)
{
}

ModelMesh::~ModelMesh()
{
	for (ModelMeshPart * part : meshParts)
		SAFE_DELETE(part);

	SAFE_DELETE_ARRAY(vertices);
	SAFE_RELEASE(vertexBuffer);
}

void ModelMesh::Render()
{
	UINT stride = sizeof(VertexTextureNormalTangentBlend);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (ModelMeshPart* part : meshParts)
		part->Render();
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

	mesh->vertexCount = this->vertexCount;

	mesh->vertices = new VertexTextureNormalTangentBlend[this->vertexCount];
	memcpy(mesh->vertices, this->vertices, sizeof(VertexTextureNormalTangentBlend) * vertexCount);

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
	CsResource::CreateRawBuffer(sizeof(VertexTextureNormalTangentBlend) * vertexCount, vertices, &vertexBuffer, false, true);
}