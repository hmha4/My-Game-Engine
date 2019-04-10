#pragma once
#include "stdafx.h"

struct FbxMaterial
{
	string Name;

	//	Diffuse
	D3DXCOLOR Diffuse;
	string DiffuseFile;

	//	Specular
	string SpecularFile;
	D3DXCOLOR Specular;
	float SpecularExp;

	//	Normal
	string NormalMapFile;
};

struct FbxBoneData
{
	int Index;
	string Name;

	int Parent;

	D3DXMATRIX LocalTransform;
	D3DXMATRIX GlobalTransform;
};

struct FbxBlendWeight
{
	D3DXVECTOR4 Indices = D3DXVECTOR4(0, 0, 0, 0);
	D3DXVECTOR4 Weights = D3DXVECTOR4(0, 0, 0, 0);

	void Set(UINT index, UINT boneIndex, float weight)
	{
		float i = (float)boneIndex;
		float w = weight;

		switch (index)
		{
		case 0: Indices.x = i; Weights.x = w; break;
		case 1: Indices.y = i; Weights.y = w; break;
		case 2: Indices.z = i; Weights.z = w; break;
		case 3: Indices.w = i; Weights.w = w; break;
		}
	}
};

struct FbxBoneWeights
{
private:
	typedef pair<int, float> Pair;
	vector<Pair> BoneWeights;

public:
	void AddWeights(UINT boneIndex, float boneWeights)
	{
		if (boneWeights <= 0.0f) return;

		bool bAdd = false;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (boneWeights > it->second)
			{
				BoneWeights.insert(it, Pair(boneIndex, boneWeights));
				bAdd = true;

				break;
			}

			it++;
		}//	while(it)

		if (bAdd == false)
			BoneWeights.push_back(Pair(boneIndex, boneWeights));
	}

	void GetBlendWeights(FbxBlendWeight& blendWeights)
	{
		for (UINT i = 0; i < BoneWeights.size(); i++)
		{
			if (i >= 4) return;

			blendWeights.Set(i, BoneWeights[i].first, BoneWeights[i].second);
		}
	}

	void Normalize()
	{
		float totalWeight = 0.0f;

		int i = 0;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (i < 4)
			{
				totalWeight += it->second;
				i++; it++;
			}
			else
				it = BoneWeights.erase(it);
		}

		float scale = 1.0f / totalWeight;

		it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			it->second *= scale;
			it++;
		}
	}
};

struct FbxVertex
{
	int ControlPoint;
	string MaterialName;

	VertexTextureNormalTangentBlend Vertex;
};

struct FbxMeshPartData
{
	string MaterialName;

	UINT StartVertex;
	UINT VertexCount;
};

struct FbxMeshData
{
	string Name;
	int ParentBone;

	FbxMesh*Mesh;

	vector<FbxVertex*> Vertices;
	vector<VertexTextureNormalTangentBlend> OutVertices;

	vector<FbxMeshPartData*> MeshParts;

	FbxMeshData() {}
	~FbxMeshData() 
	{
		for (FbxVertex * vertex : Vertices)
			SAFE_DELETE(vertex);

		Mesh->Destroy();
	}
};

struct FbxKeyframeData
{
	float Time;

	D3DXVECTOR3 Scale;
	D3DXQUATERNION Rotation;
	D3DXVECTOR3 Translation;
};

//	Animation datas which a bone has
struct FbxKeyframe
{
	string BoneName;
	int index;
	int parent;

	vector<FbxKeyframeData> Transforms;
};

struct FbxClip
{
	string Name;

	int FrameCount;		
	float FrameRate;	
	float Duration;		

	vector<FbxKeyframe*> KeyFrames;
};
