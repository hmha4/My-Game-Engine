#pragma once

class ModelBone;
class ModelMesh;

class Model
{
public:
	friend class Models;

public:
	Model();
	~Model();

	wstring GetModelName() { return name; }

	// --------------------------------------------------------------------- //
	//  Min & Max for bounding box
	// --------------------------------------------------------------------- //
	D3DXVECTOR3 Min() { return minXZ; }		//	Min vector3
	D3DXVECTOR3 Max() { return maxXZ; }		//	Max vector3

	// --------------------------------------------------------------------- //
	//  Material Datas
	// --------------------------------------------------------------------- //
	UINT MaterialCount() { return materials.size(); }					//	The number of materials
	vector<Material*>& Materials() { return materials; }				//	Get the vector of materials
	Material *MaterialByIndex(UINT index) { return materials[index]; }	//	Get a material by its index
	Material *MaterialByName(wstring name);								//	Get a material by its name
	Material *MaterialByName(string name);								//	Get a material by its name

	// --------------------------------------------------------------------- //
	//  Bone Datas
	// --------------------------------------------------------------------- //
	UINT BoneCount() { return bones.size(); }							//	The number of bones
	vector<ModelBone *>& Bones() { return bones; }						//	Get the vector of bones
	ModelBone *BoneByIndex(UINT index) { return bones[index]; }			//	Get a bone by its index
	ModelBone *BoneByName(wstring name);								//	Get a bone by its name
	ModelBone *BoneByName(string name);									//	Get a bone by its name

	// --------------------------------------------------------------------- //
	//  Mesh Datas
	// --------------------------------------------------------------------- //
	UINT MeshCount() { return meshes.size(); }							//	The number of meshes
	vector<ModelMesh*>& Meshes() { return meshes; }						//	Get the vector of meshes
	ModelMesh *MeshByIndex(UINT index) { return meshes[index]; }		//	Get a mesh by its index
	ModelMesh *MeshByName(wstring name);								//	Get a mesh by its name
	ModelMesh *MeshByName(string name);									//	Get a mesh by its name

	// --------------------------------------------------------------------- //
	//  Read Mesh & Material Datas
	// --------------------------------------------------------------------- //
	void ReadMaterial(wstring file);					//	Read material file
	void ReadMesh(wstring file);						//	Read mesh file

	// --------------------------------------------------------------------- //
	//  Copy Global Bone Datas
	// --------------------------------------------------------------------- //
	void CopyGlobalBoneTo(vector<D3DXMATRIX>& transforms);					//	Calculate bones' global with an identity matrix
	void CopyGlobalBoneTo(vector<D3DXMATRIX>& transforms, D3DXMATRIX& w);	//	Calculate bones' global with a matrix

	// --------------------------------------------------------------------- //
	//  Retargeting Bones
	// --------------------------------------------------------------------- //
	//void RetargetBone(class ModelClip *clip);
	//void RetargetBone(ModelClip *clip, class ModelKeyframe * keyframe, class ModelBone * bone, int index, int parent);

	// --------------------------------------------------------------------- //
	//  Min & Max
	// --------------------------------------------------------------------- //
	void CreateMinMax(D3DXMATRIX t);
	void UpdateMinMax(D3DXMATRIX pos);

	bool IsFirstName() { return isFirstName; }
private:
	// --------------------------------------------------------------------- //
	//  Bindings
	// --------------------------------------------------------------------- //
	void BindingBone();		//	Binding indices of bones
	void BindingMesh();		//	Binding bones and materials of meshes

private:
	wstring name;
	class ModelBone *root;				//	Root of bones

	vector<Material*> materials;		//	Vector of materials
	vector<class ModelMesh*> meshes;	//	Vector of meshes
	vector<class ModelBone*> bones;		//	Vector of bones

	D3DXVECTOR3 minXZ, startMinXZ;
	D3DXVECTOR3 maxXZ, startMaxXZ;

	bool isFirstName;
};

// --------------------------------------------------------------------- //
//  Model Manager for cloning models
// --------------------------------------------------------------------- //
class Models
{
public:
	friend class Model;

public:
	static void Create();
	static void Delete();
private:
	static void LoadMaterial(wstring file, vector<Material *> *materials);
	static void ReadMaterialData(wstring file);
	static bool LoadMesh(wstring file, vector<class ModelBone *> *bones, vector<class ModelMesh *> *meshes);
	static bool ReadMeshData(wstring file);
private:
	static map<wstring, vector<Material *>>materialMap;

	struct MeshData
	{
		vector<class ModelBone *> Bones;
		vector<class ModelMesh *> Meshes;
	};
	static map<wstring, MeshData> meshDataMap;
};
