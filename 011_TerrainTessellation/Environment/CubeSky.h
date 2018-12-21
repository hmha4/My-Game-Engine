#pragma once

class CubeSky
{
public:
	CubeSky(wstring cubeFile);
	~CubeSky();

	void Update();
	void Render();

	ID3D11ShaderResourceView* SRV() { return srv; }
private:
	Material * material;

	MeshSphere* sphere;

	ID3D11ShaderResourceView* srv;
};