#pragma once

class Shadow
{
public:
	Shadow(UINT width, UINT height);
	~Shadow();

	void Initialize(float x = 0, float y = 0, float z = 0, float radius = 100);
	void Ready();
	void Update();
	void PreRender();
	void Render();
	void ResizeScreen();

public:
	void Add(class IShadow * shadow);
	void Remove(UINT index);
	void RemoveAll();

private:
	void BuildShadowTransform();

private:
	vector<IShadow *> objs;

	UINT width;
	UINT height;

	Viewport * viewport;
	BSphere * sceneBounds;

	D3DXMATRIX lightView;
	D3DXMATRIX lightProjection;
	D3DXMATRIX shadowTransform;

	DepthStencilView * depthMapDSV;
	ID3D11ShaderResourceView * depthMapSRV;
};