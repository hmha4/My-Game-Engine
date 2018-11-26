#include "stdafx.h"
#include "MeshQuad.h"

MeshQuad::MeshQuad(Material * material, float width, float height)
	: Mesh(material), width(width), height(height)
{

}

MeshQuad::~MeshQuad()
{

}

void MeshQuad::CreateData()
{
	vector<VertexTextureNormalTangent> vertices;

	float w = width * 0.5f;
	float h = height * 0.5f;

	vertices.push_back(VertexTextureNormalTangent(-w, -h, 0, 0, 1, 0, 0, -1, 1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(-w, +h, 0, 0, 0, 0, 0, -1, 1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(+w, -h, 0, 1, 1, 0, 0, -1, 1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(+w, +h, 0, 1, 0, 0, 0, -1, 1, 0, 0));


	this->vertices = new VertexTextureNormalTangent[vertices.size()];
	vertexCount = vertices.size();
	copy
	(
		vertices.begin(), vertices.end(),
		stdext::checked_array_iterator<VertexTextureNormalTangent *>(this->vertices, vertexCount)
	);

	indexCount = 6;
	this->indices = new UINT[indexCount]
	{
		0, 1, 2, 2, 1, 3
	};
}
