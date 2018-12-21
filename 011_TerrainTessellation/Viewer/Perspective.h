#pragma once

class Perspective
{
public:
	Perspective(float width, float height, float fov = (float)D3DX_PI * 0.25f, float zNear = 0.1f, float zFar = 1000.0f);
	~Perspective();

	void GetMatrix(D3DXMATRIX* projection) { *projection = this->projection; }

	void Set(float width, float height, float fov = (float)D3DX_PI * 0.25f, float zNear = 0.1f, float zFar = 1000.0f);

	float Width() { return width; }
	float Height() { return height; }
	float Fov() { return fov; }
	float Aspect() { return aspect; }
	float Near() { return zNear; }
	float Far() { return zFar; }
private:
	float width;
	float height;
	float fov;
	float aspect;
	float zNear;
	float zFar;

	D3DXMATRIX projection;
};