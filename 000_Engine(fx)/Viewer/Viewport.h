#pragma once

//////////////////////////////////////////////////////////
//	뷰포트 : 렌더 타겟의 렌더링 영역에 관한 설정
//
//	렌더 타겟에 렌더링을 하기 위해서는 뷰포트의 설정이 필요함
//	렌더 타겟의 넓이, 높이, 깊이 값으로 렌더링 영역을 설정
//	각 렌더타겟별로 설정 해야한다.
//	==> 렌더타겟이 8개이면 뷰포트도 8개 설정해야함
//
//	뷰포트는 특별한 인터페이스가 있는 것이 아니라
//	값을 설정하는 것이기 때문에
//	뷰포트 구조체에 값을 채우고, 렌더 타겟에 세팅 해주기만 하면 된다.
//////////////////////////////////////////////////////////
class Viewport
{
public:
	Viewport(float width, float height, float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1);
	~Viewport();

	void RSSetViewport();
	void Set(float width, float height, float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1);

	void GetRay(struct Ray* ray, D3DXVECTOR3& start, D3DXMATRIX& world, D3DXMATRIX& view, D3DXMATRIX& projection);

	float GetWidth() { return width; }
	float GetHeight() { return height; }
private:
	float x, y;
	float width, height;
	float minDepth, maxDepth;

	D3D11_VIEWPORT viewport;
};