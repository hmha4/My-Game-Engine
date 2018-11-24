#pragma once

struct GlobalLight
{
	D3DXVECTOR3 Direction = D3DXVECTOR3(-1, -1, 1);
	D3DXCOLOR Color = D3DXCOLOR(1, 1, 1, 1);
};

class CBuffer;
class Context
{
public:
	static void Create();
	static void Delete();

	static Context* Get();

private:
	Context();
	~Context();

public:
	void Update();
	void AddShader(Shader* shader);

	class Perspective* GetPerspective();
	void ChangePerspective();

	GlobalLight* GetGlobalLight();
	void ChangeGlobalLight();

	class Viewport* GetViewport();
	class Camera* GetMainCamera();

private:
	static Context* instance;

private:
	class Perspective* perspective;
	class Viewport* viewport;
	class Camera* mainCamera;
	GlobalLight* globalLight;

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

	struct Light
	{
		D3DXCOLOR Color;
		D3DXVECTOR3 Direction;
		
		float Padding;
	};
	Light light;
	map<Shader *, CBuffer*> lightMap;
};