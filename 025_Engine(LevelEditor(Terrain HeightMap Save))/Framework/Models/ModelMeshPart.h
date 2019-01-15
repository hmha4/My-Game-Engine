#pragma once

class ModelMeshPart
{
public:
	friend class Model;
	friend class Models;
	friend class ModelMesh;

	// --------------------------------------------------------------------- //
	//  Vertices and Indices
	// --------------------------------------------------------------------- //
	void Vertices(vector<VertexTextureNormalTangentBlend>& vertices) { vertices = this->vertices; }	//	Get the vector of vertices
	void Indices(vector<UINT>& indices) { indices = this->indices; }					//	Get the vector of indices
	ID3D11Buffer * VertexBuffer() { return vertexBuffer; }

private:
	ModelMeshPart();
	~ModelMeshPart();

	void Render();
	void Release();

	// --------------------------------------------------------------------- //
	//  Bind Vertices and Indices
	// --------------------------------------------------------------------- //
	void Binding();

	// --------------------------------------------------------------------- //
	//  Effect technique pass number
	// --------------------------------------------------------------------- //
	void Pass(UINT val) { pass = val; }
	UINT Pass() { return pass; }
public:
	void Clone(void ** clone);

	// --------------------------------------------------------------------- //
	//  Materials
	// --------------------------------------------------------------------- //
	Material * GetMaterial() { return material; }	//	Get the corresponding material

private:
	UINT					pass;			//	Effect pass number

	Material				*material;		//	Pointer to the corresponding material
	wstring					materialName;	//	Name of the corresponding material

	class ModelMesh			*parent;		//	Pointer to the parent mesh

	vector<VertexTextureNormalTangentBlend> vertices;		//	Vertices of the model
	vector<UINT>			indices;		//	Indices of the model

	ID3D11Buffer			*vertexBuffer;	//	Vertex buffer for the Vertices
	ID3D11Buffer			*indexBuffer;	//	Index buffer for the indices
};