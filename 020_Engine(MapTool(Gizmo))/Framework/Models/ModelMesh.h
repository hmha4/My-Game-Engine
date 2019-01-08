#pragma once


class ModelMesh
{
public:
	friend class Model;
	friend class Models;

public:

	void Render();
	void Release();

	// --------------------------------------------------------------------- //
	//  Current mesh data
	// --------------------------------------------------------------------- //
	wstring Name() { return name; }		//	Get the name of the current mesh

	// --------------------------------------------------------------------- //
	//  Parent bone data
	// --------------------------------------------------------------------- //
	int ParentBoneIndex() { return parentBoneIndex; }		//	Get the index of the parent bone
	class ModelBone *ParentBone() { return parentBone; }	//	Get the pointer to the parent bone

	// --------------------------------------------------------------------- //
	//  Mesh parts data
	// --------------------------------------------------------------------- //
	vector<class ModelMeshPart *>& MeshParts() { return meshParts; }	//	Get the vector of mesh parts

	// --------------------------------------------------------------------- //
	//  Effect technique pass number
	// --------------------------------------------------------------------- //
	void Pass(UINT val);	//	Sets the pass number of the effect
public:
	void Clone(void ** clone);

private:
	ModelMesh();
	~ModelMesh();

	void Binding();		//	Bind mesh parts' vertices and indices

private:
	typedef vector<class ModelMeshPart *> vMeshParts;

	wstring				name;				//	Name of the current mesh

	int					parentBoneIndex;	//	Index of the parent bone
	class ModelBone		*parentBone;		//	Pointer to the parent bone

	vMeshParts			meshParts;			//	Vector of mesh parts
};