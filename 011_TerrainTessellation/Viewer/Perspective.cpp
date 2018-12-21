#include "stdafx.h"
#include "Perspective.h"

Perspective::Perspective(float width, float height, float fov, float zNear, float zFar)
{
	Set(width, height, fov, zNear, zFar);
}

Perspective::~Perspective()
{
}

void Perspective::Set(float width, float height, float fov, float zNear, float zFar)
{
	this->width = width;			//	화면 가로 크기
	this->height = height;			//	화면 세로 크기
	this->fov = fov;				//	뷰 프러스텀의 모양을 결정(near, far의 각)	
	this->aspect = width / height;	//	화면 비율
	this->zNear = zNear;			//	near
	this->zFar = zFar;				//	far

	D3DXMatrixPerspectiveFovLH(&projection, fov, aspect, zNear, zFar);

	//	(P)
	//	W	0	0	0
	//	0	H	0	0
	//	0	0	A	1(나중에 z로 나누기 위해서 필요)
	//	0	0	B	0
	//	_34 : 1. 나중에 z로 나누기 위해서 값이 존재해야한다.
	//			 동차 좌표상의 w값에 이 값을 잠시 복사 해 둔것
	//		: 2. 뷰 * 프로젝션 변환이 일어난 후 상대 위치

	//	(x, y, z, 1) * (P) = (Wx, Hy, Az + B, Z)

	//	a(near), b(far)는 가상화면의 면에 의해 결정된다.
	//	Near일 경우 0, Far일 경우 1이 되므로
	//	(0, 0, Z_Near), (0, 0, Z_Far)의 경우에

	//	a*Znear + b = 0 => a + b/zNear = 0
	//	a*Zfar + b = 1 => a + b/zFar = 0

	//	==> a = zNear / (zFar - zNear)
	//	==> b = a * -zFar

	//	w	0	0		0
	//	0	h	0		0
	//	0	0	q		1
	//	0	0	-q*near	0

	//	h : 1 / tan(fovY / 2) == cot(fovY / 2)
	//	w : h / aspect
	//	q : zFar / zFar - zNear
}
