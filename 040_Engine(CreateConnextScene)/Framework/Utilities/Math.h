#pragma once

// ------------------------------------------------------------------------- //
//	수학 관련 내용
// ------------------------------------------------------------------------- //

class Math
{
public:
	static const float PI;
	static const float PI_2;
	static const float PI_DIV_2;
	static const float PI_DIV_4;
	static const float EPSILON;

	static float cotf(float& c, float& s);

	template<typename T>
	static const bool Equals(const T& v1, const T& v2)
	{
		bool bCheck = true;
		bCheck &= v1 + numeric_limits<T>::epsilon() >= v2;
		bCheck &= v1 - numeric_limits<T>::epsilon() <= v2;

		return bCheck;
	}

	static float Modulo(float val1, float val2);

	static float ToRadian(float degree);
	static float ToDegree(float radian);

	static int Random(int r1, int r2);
	static float Random(float r1, float r2);

	static float Clamp(float value, float min, float max);

	static float GetAngle(D3DXVECTOR3& p1, D3DXVECTOR3& p2);
	static float GetAngleY(D3DXVECTOR3& p1, D3DXVECTOR3& p2);
	static float GetDistanceXZ(D3DXVECTOR3 p1, D3DXVECTOR3 p2);

	static float Distance(D3DXVECTOR3 p1, D3DXVECTOR3 p2);
	static float DistanceSquared(D3DXVECTOR3 p1, D3DXVECTOR3 p2);

	//	My D3DX Fuction
	static void LerpMatrix(OUT D3DXMATRIX& out, D3DXMATRIX& m1, D3DXMATRIX& m2, float amount);
	static void QuatToYawPithRoll(const D3DXQUATERNION& q, float &pitch, float &yaw, float &roll);
	static void Vec3Clamp(D3DXVECTOR3& center, D3DXVECTOR3& min, D3DXVECTOR3& max, OUT D3DXVECTOR3& out);
	static D3DXVECTOR3 SmoothDamp(D3DXVECTOR3 current, D3DXVECTOR3 target, D3DXVECTOR3& currentVelocity, float smoothTime, float maxSpeed = FLT_MAX);
	static D3DXVECTOR3 ClampMagnitude(D3DXVECTOR3 vector, float maxLength);
	static float SqrMagnitude(D3DXVECTOR3 a);
};