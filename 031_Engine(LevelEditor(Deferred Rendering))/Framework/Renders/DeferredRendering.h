#pragma once

class DeferredRendering
{
public:
	enum Target
	{
		Position = 0,
		Normal,
		Albedo,
		Specular,
		Depth,

		Count
	};
public:
	DeferredRendering(float width, float height);
	~DeferredRendering();
	
	void Set();
	void Render();
	Effect * GetEffect() { return effect->GetEffect(); }
	ID3D11ShaderResourceView *& DSVSRV() { return srv[Depth]; }

private:
	void CreateTextures();
	void CreateRTV();
	void CreateDSV();
	void CreateSRV();

private:
	float width;
	float height;

	Material * effect;

	ID3D11Texture2D * texture[4];
	ID3D11RenderTargetView * rtv[4];
	ID3D11ShaderResourceView * srv[5];
	ID3D11Texture2D * depthBuffer;
	ID3D11DepthStencilView * dsv;

	Viewport * viewport;

private:
	Render2D * render[5];

public:
	static void Create(float width, float height);
	static void Delete();
	static DeferredRendering * Get();

private:
	static DeferredRendering * instance;
};