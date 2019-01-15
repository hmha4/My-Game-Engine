#pragma once

class CBuffer;

struct PointLightDesc
{
	D3DXCOLOR Ambient;
	D3DXCOLOR Diffuse;
	D3DXCOLOR Specular;

	D3DXVECTOR3 Position;
	float Range;

	D3DXVECTOR3 Attenuation;
	float Padding;
};

struct SpotLightDesc
{
	D3DXCOLOR Ambient;
	D3DXCOLOR Diffuse;
	D3DXCOLOR Specular;

	D3DXVECTOR3 Position;
	float Padding;

	D3DXVECTOR3 Direction;
	float Spot;

	D3DXVECTOR3 Attenuation;
	float Padding2;
};

class Context
{
private:
	struct GlobalLight;

public:
	static void Create();
	static void Delete();

	static Context* Get();

private:
	Context();
	~Context();

public:
	void Update();
	void UpdateVP(D3DXMATRIX view, D3DXMATRIX proj);
	void InitVP();

	void AddShader(Shader* shader);
	void Erase(CBuffer * buffer);
	void Erase(Shader * shader);

	class Perspective* GetPerspective();
	void ChangePerspective();

	GlobalLight* GetGlobalLight();
	void ChangeGlobalLight();

	void AddPointLight(PointLightDesc& light);
	void AddSpotLight(SpotLightDesc& light);

	class Viewport* GetViewport();
	class Camera* GetMainCamera();


private:
	static Context* instance;

private:
	class Perspective* perspective;
	class Viewport* viewport;
	class Camera* mainCamera;

private:
	struct PerFrame
	{
		D3DXMATRIX View;

		D3DXVECTOR3 ViewDirection;
		float Time;
		D3DXVECTOR3 ViewPosition;

		float Padding;
	};
	PerFrame perFrame;
	map<Shader*, CBuffer*> perFrameMap;

	struct Projection
	{
		D3DXMATRIX Project;
	};
	Projection projection;
	map<Shader *, CBuffer*> projectionMap;

private:
	struct GlobalLight
	{
		D3DXCOLOR SunColor = D3DXCOLOR(1, 1, 1, 1);
		D3DXCOLOR Ambient = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1);
		D3DXCOLOR Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1);
		D3DXCOLOR Specular = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1);
		D3DXVECTOR3 Direction = D3DXVECTOR3(0.57735f, -0.57735f, 0.57735f);

		float Padding;
	};
	GlobalLight light;
	map<Shader *, CBuffer*> lightMap;

private:
	struct PointLightBuffer
	{
		PointLightDesc Lights[16];
		int Count = 0;

		float Padding[3];
	};
	PointLightBuffer pointLight;
	map<Shader *, CBuffer*> pointLightMap;

private:
	struct SpotLightBuffer
	{
		SpotLightDesc Lights[16];
		int Count = 0;

		float Padding[3];
	};
	SpotLightBuffer spotLight;
	map<Shader *, CBuffer*> spotLightMap;
};