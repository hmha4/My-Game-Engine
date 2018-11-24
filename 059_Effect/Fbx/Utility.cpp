#include "stdafx.h"
#include "Utility.h"

D3DXMATRIX Fbx::Utility::Negative()
{
	D3DXMATRIX scaleX;
	D3DXMatrixScaling(&scaleX, -1, 1, 1);


	D3DXMATRIX rotation;
	D3DXMatrixRotationY(&rotation, Math::ToRadian(180));

	return scaleX * rotation;
};

D3DXVECTOR2 Fbx::Utility::ToVector2(FbxVector2 & vec)
{
	return D3DXVECTOR2((float)vec.mData[0], (float)vec.mData[1]);
}

D3DXVECTOR3 Fbx::Utility::ToVector3(FbxVector4 & vec)
{
	return D3DXVECTOR3((float)vec.mData[0], (float)vec.mData[1], (float)vec.mData[2]);
}

D3DXCOLOR Fbx::Utility::ToColor(FbxVector4 & vec)
{
	return D3DXCOLOR((float)vec.mData[0], (float)vec.mData[1], (float)vec.mData[2], 1);
}

D3DXCOLOR Fbx::Utility::ToColor(FbxPropertyT<FbxDouble3>& vec, FbxPropertyT<FbxDouble>& factor)
{
	FbxDouble3 color = vec;

	D3DXCOLOR result;
	result.r = (float)color.mData[0];
	result.g = (float)color.mData[1];
	result.b = (float)color.mData[2];
	result.a = (float)factor;	//	빛의 강도

	return result;
}

D3DXMATRIX Fbx::Utility::ToMatrix(FbxAMatrix & matrix)
{
	FbxVector4 r1 = matrix.GetRow(0);
	FbxVector4 r2 = matrix.GetRow(1);
	FbxVector4 r3 = matrix.GetRow(2);
	FbxVector4 r4 = matrix.GetRow(3);

	D3DXMATRIX origin = D3DXMATRIX
	(
		(float)r1.mData[0], (float)r1.mData[1], (float)r1.mData[2], (float)r1.mData[3],
		(float)r2.mData[0], (float)r2.mData[1], (float)r2.mData[2], (float)r2.mData[3],
		(float)r3.mData[0], (float)r3.mData[1], (float)r3.mData[2], (float)r3.mData[3],
		(float)r4.mData[0], (float)r4.mData[1], (float)r4.mData[2], (float)r4.mData[3]
	);

	return Negative() * origin * Negative();
}

string Fbx::Utility::GetTextureFile(FbxProperty & prop)
{
	if (prop.IsValid() == true)
	{
		//	오브젝트 정보가 0보다 클때
		if (prop.GetSrcObjectCount() > 0)
		{
			//	텍스쳐 오브젝트를 가져옴
			//	팩토리 패턴, 템플릿 팩토리 패턴
			FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>();

			if (texture != NULL)
				return string(texture->GetFileName());
		}
	}

	return "";
}


string Fbx::Utility::GetMaterialName(FbxMesh * mesh, int polygonIndex, int cpIndx)
{
	//	현재 메쉬가 소속되어있는 노드를 반환
	FbxNode* node = mesh->GetNode();
	if (node == NULL) return "";

	//	FbxLayerElementMaterial : FbxSurfaceMaterial의 자식
	//	GetLayer(0) : 계층
	//	3D 모델은 계층이 있는데 게임에서는 
	//	계층을 하나만 사용 하도록 0(맨 아래) 계층만 사용한다.
	FbxLayerElementMaterial* material = mesh->GetLayer(0)->GetMaterials();
	if (material == NULL) return "";

	//	데이터를 어떻게 매핑 시켜서 가져 올꺼냐
	FbxLayerElement::EMappingMode mappingMode = material->GetMappingMode();
	FbxLayerElement::EReferenceMode refMode = material->GetReferenceMode();

	int mappingIndex = -1;
	switch (mappingMode)
	{
		//	머터리얼과 메시가 일치할때 AllSame
		//	 => 메쉬 마다 머터리얼이 다른 경우
	case FbxLayerElement::eAllSame: mappingIndex = 0; break;
		//	삼각형을 기준으로 디자인을 해서 머터리얼이 연결된 경우
		//	 => 폴리곤 마다 머터리얼이 다른 경우
	case FbxLayerElement::eByPolygon: mappingIndex = polygonIndex; break;
		//	Fbx자체에서 처리
	case FbxLayerElement::eByControlPoint: mappingIndex = cpIndx; break;
		//	정점을 기준으로 디자인을 해서 머터리얼이 연결된 경우
		//	 => 정점 마다 머터리얼이 다른 경우
	case FbxLayerElement::eByPolygonVertex: mappingIndex = polygonIndex * 3; break;
	default: assert(false); break;
	}

	//	Mapping Index를 가지고 머터리얼을 찾는다.
	FbxSurfaceMaterial* findMaterial = NULL;
	//	MappingIndex로 바로 머터리얼에 접근 가능 할 때
	//	MappingIndex -> 머터리얼
	if (refMode == FbxLayerElement::eDirect)
	{
		if (mappingIndex < node->GetMaterialCount())
			findMaterial = node->GetMaterial(mappingIndex);
	}
	//	참조 번호를 통해 머터리얼에 접근 가능 할 때
	//	Mapping Index -> 참조 번호 -> 머터리얼
	else if (refMode == FbxLayerElement::eIndexToDirect)
	{
		FbxLayerElementArrayTemplate<int>& indexArr = material->GetIndexArray();

		if (mappingIndex < indexArr.GetCount())
		{
			int tempIndex = indexArr.GetAt(mappingIndex);

			if (tempIndex < node->GetMaterialCount())
				findMaterial = node->GetMaterial(tempIndex);
		}//	if(mappingIndex)
	}//	if(refMode)

	if (findMaterial == NULL)
		return "";

	return findMaterial->GetName();
}

D3DXVECTOR2 Fbx::Utility::GetUv(FbxMesh * mesh, int cpIndex, int uvIndex)
{
	D3DXVECTOR2 result = D3DXVECTOR2(0, 0);

	FbxLayerElementUV * uv = mesh->GetLayer(0)->GetUVs();
	if (uv == NULL) return result;

	FbxLayerElement::EMappingMode mappingMode = uv->GetMappingMode();
	FbxLayerElement::EReferenceMode refMode = uv->GetReferenceMode();

	switch (mappingMode)
	{
	case FbxLayerElement::eByControlPoint:
	{
		if (refMode == FbxLayerElement::eDirect)
		{
			result.x = (float)uv->GetDirectArray().GetAt(cpIndex).mData[0];
			result.y = (float)uv->GetDirectArray().GetAt(cpIndex).mData[1];
		}
		else if (refMode == FbxLayerElement::eIndexToDirect)
		{
			int index = uv->GetIndexArray().GetAt(cpIndex);

			result.x = (float)uv->GetDirectArray().GetAt(index).mData[0];
			result.y = (float)uv->GetDirectArray().GetAt(index).mData[1];
		}
	}
	break;

	case FbxLayerElement::eByPolygonVertex:
	{
		result.x = (float)uv->GetDirectArray().GetAt(uvIndex).mData[0];
		result.y = (float)uv->GetDirectArray().GetAt(uvIndex).mData[1];
	}
	break;
	}

	//	Fbx는 밑에서 위로 올라가는 식
	//	그래서 위에서 아래로 내려가는 식으로 변경
	result.y = 1.0f - result.y;

	return result;
}
