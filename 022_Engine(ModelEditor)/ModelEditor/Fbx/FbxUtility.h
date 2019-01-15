#pragma once


class FbxUtility
{
public:
	static bool bRightHand;
	static const D3DXMATRIX Negative;

	static FbxAMatrix GetGeometryTransformation(FbxNode * node);

	static D3DXVECTOR2 ToVector2(FbxVector2& vec);
	static D3DXVECTOR3 ToVector3(FbxVector4& vec);
	static D3DXCOLOR ToColor(FbxVector4& vec);
	static D3DXCOLOR ToColor(FbxPropertyT<FbxDouble3>& vec, FbxPropertyT<FbxDouble>& factor);
	static D3DXQUATERNION ToQuaternion(FbxQuaternion& vec);
	static D3DXMATRIX ToMatrix(FbxAMatrix& matrix);

	static D3DXVECTOR3 ToPosition(FbxVector4& vec);
	static D3DXVECTOR3 ToNormal(FbxVector4& vec);

	//FbxAMatrix => 아핀변환찾아보기
	
	static string GetTextureFile(FbxProperty&prop);

	static string GetMaterialName(FbxMesh*mesh, int polygonIndex, int cpIndex);
	static D3DXVECTOR2 GetUv(FbxMesh*mesh, int cpIndex, int uvIndex);
};
