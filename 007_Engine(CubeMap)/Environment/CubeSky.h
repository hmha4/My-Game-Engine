#pragma once

class CubeSky
{
public:
	CubeSky(wstring cubeFile);
	~CubeSky();

	void Update();
	void Render();

private:
	Material * material;

	MeshSphere* sphere;

	ID3D11ShaderResourceView* srv;
};