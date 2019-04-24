#pragma once

class Shadow
{
public:
	Shadow(UINT width, UINT height);
	~Shadow();

	void Initialize();
	void Ready();
	void Update();
	void PreRender();
	void Render();

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