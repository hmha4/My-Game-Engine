#include "Framework.h"
#include "MeshCube.h"

MeshCube::MeshCube(Material * material, float width, float height, float depth)
	: Mesh(material), width(width), height(height), depth(depth)
{

}

MeshCube::~MeshCube()
{

}

void MeshCube::CreateData()
{
	vector<VertexTextureNormalTangent> vertices;

	float w = width * 0.5f;
	float h = height * 0.5f;
	float d = depth * 0.5f;

	//Front
	vertices.push_back(VertexTextureNormalTangent(-w, -h, -d, 0, 1, 0, 0, -1, 1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(-w, +h, -d, 0, 0, 0, 0, -1, 1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(+w, +h, -d, 1, 0, 0, 0, -1, 1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(+w, -h, -d, 1, 1, 0, 0, -1, 1, 0, 0));

	//Back
	vertices.push_back(VertexTextureNormalTangent(-w, -h, +d, 1, 1, 0, 0, 1, -1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(+w, -h, +d, 0, 1, 0, 0, 1, -1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(+w, +h, +d, 0, 0, 0, 0, 1, -1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(-w, +h, +d, 1, 0, 0, 0, 1, -1, 0, 0));

	//Top
	vertices.push_back(VertexTextureNormalTangent(-w, +h, -d, 0, 1, 0, 1, 0, 1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(-w, +h, +d, 0, 0, 0, 1, 0, 1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(+w, +h, +d, 1, 0, 0, 1, 0, 1, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(+w, +h, -d, 1, 1, 0, 1, 0, 1, 0, 0));

	//Bottom
	vertices.push_back(VertexTextureNormalTangent(-w, -h, -d, 0, -1, 0, -1, 0, 0, 1, 1));
	vertices.push_back(VertexTextureNormalTangent(+w, -h, -d, 0, -1, 0, -1, 0, 0, 0, 1));
	vertices.push_back(VertexTextureNormalTangent(+w, -h, +d, 0, -1, 0, -1, 0, 0, 0, 0));
	vertices.push_back(VertexTextureNormalTangent(-w, -h, +d, 0, -1, 0, -1, 0, 0, 1, 0));

	//Left
	vertices.push_back(VertexTextureNormalTangent(-w, -h, +d, 0, 1, -1, 0, 0, 0, 0, -1));
	vertices.push_back(VertexTextureNormalTangent(-w, +h, +d, 0, 0, -1, 0, 0, 0, 0, -1));
	vertices.push_back(VertexTextureNormalTangent(-w, +h, -d, 1, 0, -1, 0, 0, 0, 0, -1));
	vertices.push_back(VertexTextureNormalTangent(-w, -h, -d, 1, 1, -1, 0, 0, 0, 0, -1));

	//Right
	vertices.push_back(VertexTextureNormalTangent(+w, -h, -d, 0, 1, 1, 0, 0, 0, 0, 1));
	vertices.push_back(VertexTextureNormalTangent(+w, +h, -d, 0, 0, 1, 0, 0, 0, 0, 1));
	vertices.push_back(VertexTextureNormalTangent(+w, +h, +d, 1, 0, 1, 0, 0, 0, 0, 1));
	vertices.push_back(VertexTextureNormalTangent(+w, -h, +d, 1, 1, 1, 0, 0, 0, 0, 1));

	this->vertices = new VertexTextureNormalTangent[vertices.size()];
	vertexCount = vertices.size();
	copy
	(
		vertices.begin(), vertices.end(),
		stdext::checked_array_iterator<VertexTextureNormalTangent *>(this->vertices, vertexCount)
	);


	indexCount = 36;
	this->indices = new UINT[indexCount]
	{
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};
}
