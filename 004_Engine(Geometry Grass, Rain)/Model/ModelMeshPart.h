#pragma once

#include "../Interfaces/ICloneable.h"

class ModelMeshPart : public ICloneable
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
	vector<ModelVertexType>& Vertices() { return vertices; }	//	Get the vector of vertices
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
	// ICloneable을(를) 통해 상속됨
	virtual void Clone(void ** clone) override;

private:
	UINT					pass;			//	Effect pass number

	Material				*material;		//	Pointer to the corresponding material
	wstring					materialName;	//	Name of the corresponding material

	class ModelMesh			*parent;		//	Pointer to the parent mesh

	vector<ModelVertexType> vertices;		//	Vertices of the model
	vector<UINT>			indices;		//	Indices of the model

	ID3D11Buffer			*vertexBuffer;	//	Vertex buffer for the Vertices
	ID3D11Buffer			*indexBuffer;	//	Index buffer for the indices
};