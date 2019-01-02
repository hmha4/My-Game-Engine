#include "Framework.h"
#include "Camera.h"

Camera::Camera()
	: position(0, 0, 0), rotation(0, 0)
	, forward(0, 0, 1), right(1, 0, 0), up(0, 1, 0)
{
	D3DXMatrixIdentity(&matRotation);
	D3DXMatrixIdentity(&matView);

	//	�� �ʱ�ȭ �� ������ �ѹ� ó��
	Move();
	Rotation();
}

Camera::~Camera()
{
}

void Camera::Move()
{
	View();
}

void Camera::Rotation()
{
	D3DXMATRIX x, y;
	D3DXMatrixRotationX(&x, rotation.x);
	D3DXMatrixRotationY(&y, rotation.y);

	matRotation = x * y;

	//	����(forward, right, up)�� ����(matRotation)���� ��ȯ
	//	forward�� matRotation����� ���� ȸ���� ������ ����
	D3DXVec3TransformNormal(&forward, &D3DXVECTOR3(0, 0, 1), &matRotation);
	D3DXVec3TransformNormal(&right, &D3DXVECTOR3(1, 0, 0), &matRotation);
	D3DXVec3TransformNormal(&up, &D3DXVECTOR3(0, 1, 0), &matRotation);
}

void Camera::View()
{
	//	�Լ� �տ� Matrix�� ������ ��ȯ ���� ���
	//	�Լ� �տ� Vector3�� ������ ��ȯ ���� ����3
	D3DXMatrixLookAtLH(&matView, &position, &(position + forward), &up);

	//	front = At - Eye
	//	right = cross(up, front)
	//	up = cross(front, right)
	//	posX = -dot(Eye, right);
	//	posY = -dot(Eye, up);
	//	posZ = -dot(Eye, front);

	//	==> (-)�� �ٿ��ִ� ������ ī�޶� ����������
	//		  �����̸� ȭ��� �̹����� �������� 
	//		  �����̱� �����̴�.


	//	�� ���
	//	front.x, up.x, right.x, 0
	//	front.y, up.y, right.y, 0
	//	front.z, up.z, right.z, 0
	//	posX   , posY, posZ   , 1

	//	==> _11 ~ _33 : ������ ���ڴ� ī�޶��� ���� ǥ��
	//	==> _41 ~ _43 : �������� ������ ī�޶������ ����(��ġ)
}
