#pragma once

//////////////////////////
//	수학 관련 내용
//////////////////////////

class Math
{
public:
	static const float PI;

	static float Modulo(float val1, float val2);

	static float ToRadian(float degree);
	static float ToDegree(float radian);

	static int Random(int r1, int r2);
	static float Random(float r1, float r2);

	static float Clamp(float value, float min, float max);

	static float GetDistanceXZ(D3DXVECTOR3 p1, D3DXVECTOR3 p2);
	static float GetDistanceXYZ(D3DXVECTOR3 p1, D3DXVECTOR3 p2);

	//	My D3DX Fuction
	static void LerpMatrix(OUT D3DXMATRIX& out, D3DXMATRIX& m1, D3DXMATRIX& m2, float amount);
	static void QuatToYawPithRoll(const D3DXQUATERNION& q, float &pitch, float &yaw, float &roll);
};