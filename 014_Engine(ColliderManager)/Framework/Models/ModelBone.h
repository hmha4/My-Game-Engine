#pragma once

class ModelBone
{
public:
	friend class Model;
	friend class Models;

private:
	ModelBone();
	~ModelBone();

public:
	// --------------------------------------------------------------------- //
	//  Current bone data
	// --------------------------------------------------------------------- //
	int Index() { return index; }				//	Index of the current bone
	wstring Name() { return name; }				//	Name of the current bone

	// --------------------------------------------------------------------- //
	//  Parent bone data
	// --------------------------------------------------------------------- //
	int ParentIndex() { return parentIndex; }	//	Index of the parent bone
	ModelBone* Parent() { return parent; }		//	Pointer to the parent bone

	// --------------------------------------------------------------------- //
	//  Child bone data
	// --------------------------------------------------------------------- //
	UINT ChildCount() { return childs.size(); }				//	Size of children's vector
	ModelBone *Child(UINT index) { return childs[index]; }	//	Get a child bone with index
	vector<ModelBone *>& GetChilds() { return childs; }		//	Get child bone's vector

	// --------------------------------------------------------------------- //
	//  Matrixes of the current bone
	// --------------------------------------------------------------------- //
	D3DXMATRIX Local() { return local; }				//	Get local matrix of the bone
	void Local(D3DXMATRIX& matrix) { local = matrix; }	//	Set local matrix of the bone

	D3DXMATRIX Global() { return global; }				//	Get global matrix of the bone
	void Global(D3DXMATRIX& matrix) { global = matrix; }//	Set global matrix of the bone

	D3DXMATRIX Root() { return root; }					//	Get root matrix of the bone
	void Root(D3DXMATRIX& matrix) { root = matrix; }	//	Set root matrix of the bone

	D3DXMATRIX World() { return world; }				//	Get world matrix of the bone
	void World(D3DXMATRIX& matrix) { world = matrix; }	//	Set world matrix of the bone

public:
	void Clone(void ** clone);

private:
	int						index;			//	Index of the bone
	wstring					name;			//	Name of the bone

	int						parentIndex;	//	Index of the parent bone
	ModelBone				*parent;		//	Point to the parent bone

	vector<ModelBone *>		childs;			//	Child bone vector

	D3DXMATRIX				local;			//	Local matrix
	D3DXMATRIX				global;			//	Global matrix
	D3DXMATRIX				root;			//	Root matrix
	D3DXMATRIX				world;			//	World matrix
};