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
	this->width = width;			//	ȭ�� ���� ũ��
	this->height = height;			//	ȭ�� ���� ũ��
	this->fov = fov;				//	�� ���������� ����� ����(near, far�� ��)	
	this->aspect = width / height;	//	ȭ�� ����
	this->zNear = zNear;			//	near
	this->zFar = zFar;				//	far

	D3DXMatrixPerspectiveFovLH(&projection, fov, aspect, zNear, zFar);

	//	(P)
	//	W	0	0	0
	//	0	H	0	0
	//	0	0	A	1(���߿� z�� ������ ���ؼ� �ʿ�)
	//	0	0	B	0
	//	_34 : 1. ���߿� z�� ������ ���ؼ� ���� �����ؾ��Ѵ�.
	//			 ���� ��ǥ���� w���� �� ���� ��� ���� �� �а�
	//		: 2. �� * �������� ��ȯ�� �Ͼ �� ��� ��ġ

	//	(x, y, z, 1) * (P) = (Wx, Hy, Az + B, Z)

	//	a(near), b(far)�� ����ȭ���� �鿡 ���� �����ȴ�.
	//	Near�� ��� 0, Far�� ��� 1�� �ǹǷ�
	//	(0, 0, Z_Near), (0, 0, Z_Far)�� ��쿡

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
