#include "stdafx.h"
#include "Viewport.h"

Viewport::Viewport(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	Set(width, height, x, y, minDepth, maxDepth);
}

Viewport::~Viewport()
{
}

void Viewport::RSSetViewport()
{
	//	RS(Rasterizer State)
	//	래스터라이져는 뷰포트 변환을 통해서
	//	3D좌표 위치로 부터 2D좌표를 계산한다.

	D3D::GetDC()->RSSetViewports(1, &viewport);

	//	뷰포트의 변환 공식
	//	X = (x - 1) * width * 0.5 + TopLeftX
	//	Y = (1 - y) * height * 0.5 + TopLeftY
	//	Z = MinDepth + z * (MaxDepth - MinDepth)
}

void Viewport::Set(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	viewport.TopLeftX = this->x = x;				//	뷰포트 영역의 좌상 x좌표
	viewport.TopLeftY = this->y = y;				//	뷰포트 영역의 좌상 y좌표
	viewport.Width = this->width = width;			//	뷰포트 영역의 넓이
	viewport.Height = this->height = height;		//	뷰포트 영역의 높이
	viewport.MinDepth = this->maxDepth = minDepth;	//	뷰포트 영역의 깊이값의 최소값
	viewport.MaxDepth = this->maxDepth = maxDepth;	//	뷰포트 영역의 깊이값의 최대값

	RSSetViewport();
}


void Viewport::GetRay(Ray * ray, D3DXVECTOR3 & start, D3DXMATRIX & world, D3DXMATRIX & view, D3DXMATRIX & projection)
{
	D3DXVECTOR2 screenSize;
	screenSize.x = width;
	screenSize.y = height;

	D3DXVECTOR3 mouse = Mouse::Get()->GetPosition();


	D3DXVECTOR2 point;
	//Inv Viewport
	{
		point.x = ((2.0f * mouse.x) / screenSize.x) - 1.0f;
		point.y = (((2.0f * mouse.y) / screenSize.y) - 1.0f) * -1.0f;
	}

	//Inv Projection
	{
		point.x = point.x / projection._11;
		point.y = point.y / projection._22;
	}

	D3DXVECTOR3 direction;
	//Inv View
	{
		D3DXMATRIX invView;
		D3DXMatrixInverse(&invView, NULL, &view);

		D3DXVec3TransformNormal(&direction, &D3DXVECTOR3(point.x, point.y, 1), &invView);
		D3DXVec3Normalize(&direction, &direction);
	}

	//Inv World
	D3DXVECTOR3 tempStart;
	{
		D3DXMATRIX invWorld;
		D3DXMatrixInverse(&invWorld, NULL, &world);

		D3DXVec3TransformCoord(&tempStart, &start, &invWorld);
		D3DXVec3TransformNormal(&direction, &direction, &invWorld);
		D3DXVec3Normalize(&direction, &direction);
	}

	ray->Position = tempStart;
	ray->Direction = direction;
}