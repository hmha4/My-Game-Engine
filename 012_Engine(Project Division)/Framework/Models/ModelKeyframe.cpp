#include "Framework.h"
#include "ModelKeyframe.h"

ModelKeyframe::ModelKeyframe()
{
}

ModelKeyframe::~ModelKeyframe()
{
}

D3DXMATRIX ModelKeyframe::GetInterpolatedMatrix(float time, bool bRepeat, OUT int &currFrame)
{
	UINT index1 = 0, index2 = 0;
	float interpolatedTime = 0.0f;

	CalcKeyframeIndex(time, bRepeat, index1, index2, interpolatedTime);

	currFrame = index1;
	return GetInterpolatedMatrix(index1, index2, interpolatedTime);
}


UINT ModelKeyframe::GetKeyframeIndex(float time)
{
	UINT start = 0, end = frameCount - 1;
	if (time >= transforms[end].Time)
		return end;

	do
	{
		UINT middle = (start + end) / 2;

		if (end - start <= 1)
		{
			break;
		}
		else if (transforms[middle].Time < time)
		{
			start = middle;
		}
		else if (transforms[middle].Time > time)
		{
			end = middle;
		}
		else
		{
			start = middle;

			break;
		}
	} while ((end - start) > 1);

	return start;
}

void ModelKeyframe::CalcKeyframeIndex(float time, bool bRepeat, OUT UINT & index1, OUT UINT & index2, OUT float & interpolatedTime)
{
	index1 = index2 = 0;
	interpolatedTime = 0.0f;

	//	전달 받은 시간 대에 맞는 start 프레임 번호 반환
	index1 = GetKeyframeIndex(time);
	index2 = index1 + 1;
/*
	if (bRepeat == false)
		index2 = (index1 >= frameCount - 1) ? index1 : index1 + 1;
	else
		index2 = (index1 >= frameCount - 1) ? 0 : index1 + 1;
*/
	if (index1 >= frameCount - 1)
	{
		index1 = index2 = frameCount - 1;

		interpolatedTime = 1.0f;
	}
	else
	{
		float time1 = time - transforms[index1].Time;
		float time2 = transforms[index2].Time - transforms[index1].Time;

		interpolatedTime = time1 / time2;
	}
}

D3DXMATRIX ModelKeyframe::GetInterpolatedMatrix(UINT index1, UINT index2, float t)
{
	D3DXVECTOR3 scale;
	D3DXMATRIX S;
	D3DXVec3Lerp(&scale, &transforms[index1].S, &transforms[index2].S, t);
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);

	D3DXQUATERNION q;
	D3DXMATRIX R;
	D3DXQuaternionSlerp(&q, &transforms[index1].R, &transforms[index2].R, t);
	D3DXMatrixRotationQuaternion(&R, &q);

	D3DXVECTOR3 trans;
	D3DXMATRIX T;
	D3DXVec3Lerp(&trans, &transforms[index1].T, &transforms[index2].T, t);
	D3DXMatrixTranslation(&T, trans.x, trans.y, trans.z);

	return S * R * T;
}


