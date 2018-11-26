#include "stdafx.h"
#include "Context.h"
#include "../Viewer/Perspective.h"
#include "../Viewer/Viewport.h"
#include "../Viewer/Freedom.h"

Context* Context::instance = NULL;

void Context::Create()
{
	assert(instance == NULL);

	instance = new Context();
}

void Context::Delete()
{
	SAFE_DELETE(instance);
}

Context * Context::Get()
{
	return instance;
}

void Context::Update()
{
	mainCamera->Update();
	mainCamera->MatrixView(&perFrame.View);
	mainCamera->Position(&perFrame.ViewPosition);
	mainCamera->Forward(&perFrame.ViewDirection);

	perFrame.Time = Time::Get()->Running();

	for (pair<Shader *, CBuffer*> temp : perFrameMap)
		temp.second->Change();

	CBuffers::Update();
}

Context::Context()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	perspective = new Perspective(desc.Width, desc.Height);
	perspective->GetMatrix(&projection.Project);

	mainCamera = new Freedom();
	viewport = new Viewport(desc.Width, desc.Height);
}

Context::~Context()
{
	for (pair<Shader *, CBuffer*> temp : perFrameMap)
		SAFE_DELETE(temp.second);

	for (pair<Shader *, CBuffer*> temp : projectionMap)
		SAFE_DELETE(temp.second);

	for (pair<Shader *, CBuffer*> temp : lightMap)
		SAFE_DELETE(temp.second);


	SAFE_DELETE(mainCamera);
	SAFE_DELETE(perspective);
	SAFE_DELETE(viewport);
}

void Context::AddShader(Shader * shader)
{
	CBuffer* cbPerFrame = new CBuffer(shader, "CB_PerFrame", &perFrame, sizeof(PerFrame));
	perFrameMap.insert(pair<Shader *, CBuffer*>(shader, cbPerFrame));

	CBuffer* cbProjection = new CBuffer(shader, "CB_Projection", &projection, sizeof(Projection));
	projectionMap.insert(pair<Shader *, CBuffer*>(shader, cbProjection));

	CBuffer* cbLight = new CBuffer(shader, "CB_Light", &light, sizeof(GlobalLight));
	lightMap.insert(pair<Shader *, CBuffer*>(shader, cbLight));
}

Perspective * Context::GetPerspective()
{
	return perspective;
}

void Context::ChangePerspective()
{
	for (pair<Shader *, CBuffer*> temp : projectionMap)
		temp.second->Change();
}

Viewport * Context::GetViewport()
{
	return viewport;
}

Camera * Context::GetMainCamera()
{
	return mainCamera;
}

Context::GlobalLight * Context::GetGlobalLight()
{
	return &light;
}

void Context::ChangeGlobalLight()
{
	for (pair<Shader *, CBuffer*> temp : lightMap)
		temp.second->Change();
}