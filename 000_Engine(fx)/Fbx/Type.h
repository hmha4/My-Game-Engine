#pragma once
#include "stdafx.h"

namespace Fbx
{
	struct FbxMaterial
	{
		string Name;

		D3DXCOLOR Diffuse;

		string DiffuseFile;
		string NormalFile;
		string SpecularFile;
	};

	struct FbxBoneData
	{
		int Index;					//	자신의 인덱스
		string Name;				//	자신의 이름

		int Parent;					//	부모의 인덱스

		D3DXMATRIX LocalTransform;	//	상대 월드
		D3DXMATRIX GlobalTransform;	//	절대 월드
	};

	struct FbxVertex
	{
		int ControlPoint;
		string MaterialName;

		//	VertexTextureNormal
		ModelVertexType Vertex;
	};

	struct FbxMeshPartData
	{
		string MaterialName;

		vector<ModelVertexType> Vertices;
		vector<UINT> Indices;
	};

	struct FbxMeshData
	{
		string Name;
		int ParentBone;

		FbxMesh* Mesh;

		vector<FbxVertex *> Vertices;
		vector<FbxMeshPartData *> MeshParts;
	};

	struct FbxKeyframeData
	{
		float Time;

		D3DXVECTOR3 Scale;
		D3DXQUATERNION Rotation;
		D3DXVECTOR3 Translation;
	};

	struct FbxKeyframe
	{
		string BoneName;

		int Index;
		int ParentIndex;

		vector<FbxKeyframeData> Transforms;
	};

	struct FbxClip
	{
		string Name;

		int FrameCount;
		float FrameRate;
		float Duration;

		vector<FbxKeyframe *> Keyframes;
	};

	struct FbxBlendWeight
	{
		//	가중치의 갯수는 4개 이하이기 때문에 
		//	D3DXVECTOR4로 만듬 편의상
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
		//	pair<본 인덱스, 가중치 값>
		typedef pair<int, float> Pair;
		vector<Pair> BoneWeights;

	public:
		//	전체 가중치의 갯수는 4개 이하
		//	4개 이상으로 해도 상관없지만
		//	4개면 충분하다.
		//	전체 가중치 값의 합은 1
		void AddWeights(UINT boneIndex, float boneWeights)
		{
			if (boneWeights <= 0.0f) return;

			bool bAdd = false;
			vector<Pair>::iterator it = BoneWeights.begin();

			//	가중치가 작은 순으로 저장
			while (it != BoneWeights.end())
			{
				if (boneWeights > it->second)
				{
					BoneWeights.insert(it, Pair(boneIndex, boneWeights));
					bAdd = true;

					break;
				}

				it++;
			}//while(it)

			if (bAdd == false)
				BoneWeights.push_back(Pair(boneIndex, boneWeights));
		}

		//	가중치 FbxBlendWeight에 세팅
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
					i++, it++;
				}
				else
					//	가중치의 갯수는 4새 이하 이기 때문에
					//	4개 보다 큰 것들은 모두 제거한다.
					it = BoneWeights.erase(it);
			}

			float scale = 1.0f / totalWeight;

			//	가중치의 합을 1로 맞춤
			it = BoneWeights.begin();
			while (it != BoneWeights.end())
			{
				it->second *= scale;
				it++;
			}
		}
	};
}