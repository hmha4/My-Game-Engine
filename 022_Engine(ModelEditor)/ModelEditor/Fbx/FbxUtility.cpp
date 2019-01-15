#include "stdafx.h"
#include "FbxUtility.h"
#include "FbxType.h"

bool FbxUtility::bRightHand = false;

//	Scale(1, 1, -1), RotationY(180.0f)
const D3DXMATRIX FbxUtility::Negative = 
{
	 1,  0,  0,  0,
	 0,	 1,  0,  0,
	 0,  0, -1,  0,
	 0,  0,  0,  1
};

FbxAMatrix FbxUtility::GetGeometryTransformation(FbxNode * node)
{
	if (!node)
	{
		throw std::exception("Null for mesh geometry");
	}

	const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

D3DXVECTOR2 FbxUtility::ToVector2(FbxVector2 & vec)
{
	return D3DXVECTOR2((float)vec.mData[0], (float)vec.mData[1]);
}

D3DXVECTOR3 FbxUtility::ToVector3(FbxVector4 & vec)
{
	return D3DXVECTOR3((float)vec.mData[0], (float)vec.mData[1], (float)vec.mData[2]);
}

D3DXCOLOR FbxUtility::ToColor(FbxVector4 & vec)
{
	return D3DXCOLOR((float)vec.mData[0], (float)vec.mData[1], (float)vec.mData[2], 1);
}

D3DXCOLOR FbxUtility::ToColor(FbxPropertyT<FbxDouble3>& vec, FbxPropertyT<FbxDouble>& factor)
{
	FbxDouble3 color = vec;

	D3DXCOLOR result;
	result.r = (float)color.mData[0];
	result.g = (float)color.mData[1];
	result.b = (float)color.mData[2];
	result.a = (float)factor; // 조명일때 이건 알파값이 아니라 강도로표현

	return result;
}

D3DXQUATERNION FbxUtility::ToQuaternion(FbxQuaternion & vec)
{
	D3DXQUATERNION result;
	result.x = (float)vec.mData[0];
	result.y = (float)vec.mData[1];
	result.z = (float)vec.mData[2];
	result.w = (float)vec.mData[3];

	return result;
}

D3DXMATRIX FbxUtility::ToMatrix(FbxAMatrix & matrix)
{
	D3DXVECTOR3 S = ToVector3(matrix.GetS());
	D3DXVECTOR3 T = ToVector3(matrix.GetT());
	D3DXQUATERNION R = ToQuaternion(matrix.GetQ());

	D3DXMATRIX s, r, t, w;
	D3DXMatrixScaling(&s, S.x, S.y, S.z);
	D3DXMatrixTranslation(&t, T.x, T.y, T.z);
	D3DXMatrixRotationQuaternion(&r, &R);

	w = s * r * t;

	if (bRightHand == true)
		return Negative * w * Negative;

	return w;
}

D3DXVECTOR3 FbxUtility::ToPosition(FbxVector4 & vec)
{
	D3DXVECTOR3 temp = ToVector3(vec);

	if (bRightHand == true)
		D3DXVec3TransformCoord(&temp, &temp, &Negative);

	return temp;
}

D3DXVECTOR3 FbxUtility::ToNormal(FbxVector4 & vec)
{
	D3DXVECTOR3 temp = ToVector3(vec);

	if (bRightHand == true)
		D3DXVec3TransformNormal(&temp, &temp, &Negative);

	return temp;
}


string FbxUtility::GetTextureFile(FbxProperty & prop)
{
	if (prop.IsValid() == true)
	{
		//prop의 오브젝트정보가 0보다클때만
		if (prop.GetSrcObjectCount() > 0)
		{
			//텍스처 오브젝트 정보 받아옴
			FbxFileTexture*texture=prop.GetSrcObject<FbxFileTexture>();
			if (texture != NULL)
				return string(texture->GetFileName());
		}
	}
	return "";
}

string FbxUtility::GetMaterialName(FbxMesh * mesh, int polygonIndex, int cpIndex)
{
	FbxNode*node = mesh->GetNode(); // 메쉬가소속된노드나옴
	if (node == NULL) return "";

	FbxLayerElementMaterial*material = mesh->GetLayer(0)->GetMaterials();
	if (material == NULL) return "";

	FbxLayerElement::EMappingMode mappingMode = material->GetMappingMode();
	FbxLayerElement::EReferenceMode refMode = material->GetReferenceMode();

	int mappingIndex = -1;

	//매핑모드 : 데이터를 어떻게 매핑시킬지
	//
	switch (mappingMode)
	{
		//AllSame:머터리얼번호와 메쉬번호와 정확히 일치할때
		case FbxLayerElement::eAllSame:mappingIndex = 0;break;
		//ByPolygon:삼각형단위로 디자인되어 삼각형단위로 머터리얼을 연결한것
		case FbxLayerElement::eByPolygon: mappingIndex = polygonIndex;break;
		//ByControlPoint:fbx가 따로머터리얼을 찾을때 컨트롤인덱스를 넘겨줌
		case FbxLayerElement::eByControlPoint:mappingIndex = cpIndex;break;
		//ByPolygonVertex:정점기준으로 머터리얼이 다른경우
		case FbxLayerElement::eByPolygonVertex:mappingIndex = polygonIndex * 3;break;
		default:assert(false); break;
	}

	FbxSurfaceMaterial*findMaterial = NULL;
	//위에서 찾은번호로 머터리얼로 바로찾는경우
	if (refMode == FbxLayerElement::eDirect)
	{
		if (mappingIndex < node->GetMaterialCount())
			findMaterial = node->GetMaterial(mappingIndex);
	}
	//매핑인덱스가 참조번호에 저장된 번호에 접근해서 머터리얼을 찾는경우
	else if (refMode == FbxLayerElement::eIndexToDirect)
	{
		FbxLayerElementArrayTemplate<int>& indexArr = material->GetIndexArray();

		if (mappingIndex < indexArr.GetCount())
		{
			int tempIndex = indexArr.GetAt(mappingIndex);

			if (tempIndex < node->GetMaterialCount())
				findMaterial = node->GetMaterial(tempIndex);
		}
	}
	if (findMaterial == NULL)
		return "";

	return findMaterial->GetName();
}

D3DXVECTOR2 FbxUtility::GetUv(FbxMesh * mesh, int cpIndex, int uvIndex)
{
	D3DXVECTOR2 result = D3DXVECTOR2(0, 0);
	
	FbxLayerElementUV*uv = mesh->GetLayer(0)->GetUVs();
	if (uv == NULL)return result;

	FbxLayerElement::EMappingMode mappingMode = uv->GetMappingMode();
	FbxLayerElement::EReferenceMode refMode = uv->GetReferenceMode();

	switch(mappingMode)
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
	//Fbx는 아래서부터위로올라가지만 여기선 위에서아래로내려가서
	//빼주는형식

	if (bRightHand == true)
		return D3DXVECTOR2(result.x, 1.0f - result.y);


	return result;
}