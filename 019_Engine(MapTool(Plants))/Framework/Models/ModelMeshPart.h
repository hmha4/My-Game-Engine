#pragma once

class ModelMeshPart
{
public:
	friend class Model;
	friend class Models;
	friend class ModelMesh;

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
	//  Vertices and Indices
	// --------------------------------------------------------------------- //
	vector<VertexTextureNormalTangentBlend>& Vertices() { return vertices; }	//	Get the vector of vertices
	vector<UINT>& Indices() { return indices; }					//	Get the vector of indices

	// --------------------------------------------------------------------- //
	//  Materials
	// --------------------------------------------------------------------- //
	Material * GetMaterial() { return material; }	//	Get the corresponding material

	// --------------------------------------------------------------------- //
	//  Effect technique pass number
	// --------------------------------------------------------------------- //
	void Pass(UINT val) { pass = val; }
	UINT Pass() { return pass; }
public:
	void Clone(void ** clone);

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