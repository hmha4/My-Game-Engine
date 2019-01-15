#include "Framework.h"
#include "Math.h"

const float Math::PI = 3.14159265358979323846264338327950288f;
const float Math::PI_2 = 6.283185307f;
const float Math::PI_DIV_2 = 1.570796327f;
const float Math::PI_DIV_4 = 0.785398163f;
const float Math::EPSILON = 0.000001f;

float Math::cotf(float & c, float & s)
{
	return cosf(c) / sinf(s);
}

float Math::Modulo(float val1, float val2)
{
	while (val1 - val2 >= 0)
		val1 -= val2;

	return val1;
}

float Math::ToRadian(float degree)
{
	return degree * PI / 180.0f;
}

float Math::ToDegree(float radian)
{
	return radian * 180.0f / PI;
}

float Math::Random(float r1, float r2)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = r2 - r1;
	float val = random * diff;
	
	return r1 + val;
}

float Math::Clamp(float value, float min, float max)
{
	value = value > max ? max : value;
	value = value < min ? min : value;
	
	return value;
}

float Math::GetDistanceXZ(D3DXVECTOR3 p1, D3DXVECTOR3 p2)
{
	float dX = fabsf(p1.x - p2.x);
	float dZ = fabsf(p1.z - p2.z);

	float distance = sqrtf(dX * dX + dZ * dZ);

	return distance;
}

float Math::Distance(D3DXVECTOR3 p1, D3DXVECTOR3 p2)
{
	float dX = fabsf(p1.x - p2.x);
	float dY = fabsf(p1.y - p2.y);
	float dZ = fabsf(p1.z - p2.z);

	float distance = sqrtf(dX * dX + dY * dY + dZ * dZ);

	return distance;
}

float Math::DistanceSquared(D3DXVECTOR3 p1, D3DXVECTOR3 p2)
{
	float dX = fabsf(p1.x - p2.x);
	float dY = fabsf(p1.y - p2.y);
	float dZ = fabsf(p1.z - p2.z);

	return dX * dX + dY * dY + dZ * dZ;
}

void Math::LerpMatrix(OUT D3DXMATRIX & out, D3DXMATRIX & m1, D3DXMATRIX & m2, float amount)
{
	out._11 = m1._11 + (m2._11 - m1._11) * amount;
	out._12 = m1._12 + (m2._12 - m1._12) * amount;
	out._13 = m1._13 + (m2._13 - m1._13) * amount;
	out._14 = m1._14 + (m2._14 - m1._14) * amount;

	out._21 = m1._21 + (m2._21 - m1._21) * amount;
	out._22 = m1._22 + (m2._22 - m1._22) * amount;
	out._23 = m1._23 + (m2._23 - m1._23) * amount;
	out._24 = m1._24 + (m2._24 - m1._24) * amount;

	out._31 = m1._31 + (m2._31 - m1._31) * amount;
	out._32 = m1._32 + (m2._32 - m1._32) * amount;
	out._33 = m1._33 + (m2._33 - m1._33) * amount;
	out._34 = m1._34 + (m2._34 - m1._34) * amount;

	out._41 = m1._41 + (m2._41 - m1._41) * amount;
	out._42 = m1._42 + (m2._42 - m1._42) * amount;
	out._43 = m1._43 + (m2._43 - m1._43) * amount;
	out._44 = m1._44 + (m2._44 - m1._44) * amount;
}

void Math::QuatToYawPithRoll(const D3DXQUATERNION & q, float & pitch, float & yaw, float & roll)
{
	float qw = q.w * q.w;
	float qx = q.x * q.x;
	float qy = q.y * q.y;
	float qz = q.z * q.z;

	pitch = asinf(2.0f * (q.w * q.x - q.y * q.z));
	yaw = atan2f(2.0f * (q.x * q.z + q.w * q.y), (-qx - qy + qz + qw));
	roll = atan2f(2.0f * (q.x * q.y + q.w * q.z), (-qx + qy - qz + qw));
}

void Math::Vec3Clamp(D3DXVECTOR3 & center, D3DXVECTOR3 & min, D3DXVECTOR3 & max, OUT D3DXVECTOR3 & out)
{
	float x = center.x;
	x = (x > max.x ? max.x : x);
	x = (x < min.x ? min.x : x);

	float y = center.y;
	y = (y > max.y ? max.y : y);
	y = (y < min.y ? min.y : y);

	float z = center.z;
	z = (z > max.z ? max.z : z);
	z = (z < min.z ? min.z : z);

	out.x = x;
	out.y = y;
	out.z = z;
}


int Math::Random(int r1, int r2)
{
	return (int)(rand() % (r2 - r1 + 1)) + r1;
}