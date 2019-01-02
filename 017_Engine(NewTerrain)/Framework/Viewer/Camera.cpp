#include "Framework.h"
#include "Camera.h"

Camera::Camera()
	: position(0, 0, 0), rotation(0, 0)
	, forward(0, 0, 1), right(1, 0, 0), up(0, 1, 0)
{
	D3DXMatrixIdentity(&matRotation);
	D3DXMatrixIdentity(&matView);

	//	값 초기화 한 값으로 한번 처리
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

	//	방향(forward, right, up)을 공간(matRotation)으로 변환
	//	forward에 matRotation행렬을 통해 회전된 방향을 구함
	D3DXVec3TransformNormal(&forward, &D3DXVECTOR3(0, 0, 1), &matRotation);
	D3DXVec3TransformNormal(&right, &D3DXVECTOR3(1, 0, 0), &matRotation);
	D3DXVec3TransformNormal(&up, &D3DXVECTOR3(0, 1, 0), &matRotation);
}

void Camera::View()
{
	//	함수 앞에 Matrix가 붙으면 반환 값이 행렬
	//	함수 앞에 Vector3이 붙으면 반환 값이 벡터3
	D3DXMatrixLookAtLH(&matView, &position, &(position + forward), &up);

	//	front = At - Eye
	//	right = cross(up, front)
	//	up = cross(front, right)
	//	posX = -dot(Eye, right);
	//	posY = -dot(Eye, up);
	//	posZ = -dot(Eye, front);

	//	==> (-)를 붙여주는 이유는 카메라가 오른쪽으로
	//		  움직이면 화면상 이미지는 왼쪽으로 
	//		  움직이기 때문이다.


	//	뷰 행렬
	//	front.x, up.x, right.x, 0
	//	front.y, up.y, right.y, 0
	//	front.z, up.z, right.z, 0
	//	posX   , posY, posZ   , 1

	//	==> _11 ~ _33 : 까지의 인자는 카메라의 축을 표현
	//	==> _41 ~ _43 : 원점으로 부터의 카메라까지의 벡터(위치)
}
