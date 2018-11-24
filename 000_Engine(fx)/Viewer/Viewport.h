#pragma once

//////////////////////////////////////////////////////////
//	����Ʈ : ���� Ÿ���� ������ ������ ���� ����
//
//	���� Ÿ�ٿ� �������� �ϱ� ���ؼ��� ����Ʈ�� ������ �ʿ���
//	���� Ÿ���� ����, ����, ���� ������ ������ ������ ����
//	�� ����Ÿ�ٺ��� ���� �ؾ��Ѵ�.
//	==> ����Ÿ���� 8���̸� ����Ʈ�� 8�� �����ؾ���
//
//	����Ʈ�� Ư���� �������̽��� �ִ� ���� �ƴ϶�
//	���� �����ϴ� ���̱� ������
//	����Ʈ ����ü�� ���� ä���, ���� Ÿ�ٿ� ���� ���ֱ⸸ �ϸ� �ȴ�.
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