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
	void RenderInstance(UINT count, UINT tech = 0);
	void Release();

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
	wstring MaterialName() { return materialName; }

	UINT StartVertex() { return startVertex; }
	UINT VertexCount() { return vertexCount; }

private:
	UINT					pass;			//	Effect pass number

	Material				*material;		//	Pointer to the corresponding material
	wstring					materialName;	//	Name of the corresponding material

	class ModelMesh			*parent;		//	Pointer to the parent mesh

	UINT startVertex;
	UINT vertexCount;

	ID3DX11EffectScalarVariable * boneIndexVar;
};