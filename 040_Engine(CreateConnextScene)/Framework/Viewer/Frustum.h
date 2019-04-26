#pragma once

class Frustum
{
public:
	Frustum(float zFar, class Camera* camera = NULL, class Perspective* perspective = NULL);
	~Frustum();

	void ZFar(float val) { zFar = val; }
	void Update();

	void GetPlanes(OUT D3DXPLANE* plane);
	void SetSinglePlane(D3DXPLANE& plane);

	bool ContainPoint(D3DXVECTOR3& position);
	bool ContainRect(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize);
	bool ContainRect(D3DXVECTOR3 center, D3DXVECTOR3 size);
	bool ContainCube(D3DXVECTOR3& center, float radius);

private:
	D3DXPLANE planes[6];

	float zFar;

	class Camera* camera;
	class Perspective* perspective;
};