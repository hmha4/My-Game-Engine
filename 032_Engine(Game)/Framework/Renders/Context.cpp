#include "Framework.h"
#include "Context.h"
#include "../Viewer/Perspective.h"
#include "../Viewer/Viewport.h"
#include "../Viewer/Freedom.h"
#include "../Viewer/Orbit.h"
#include "../Viewer/Frustum.h"

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

	for (pair<Effect *, CBuffer*> temp : perFrameMap)
		temp.second->Change();

	CBuffers::Update();
}

void Context::UpdateVP(D3DXMATRIX view, D3DXMATRIX proj)
{
	perFrame.View = view;
	projection.Project = proj;

	for (pair<Effect *, CBuffer*> temp : perFrameMap)
		temp.second->Change();
	ChangePerspective();

	CBuffers::Update();
}

void Context::InitVP()
{
	mainCamera->Update();
	mainCamera->MatrixView(&perFrame.View);
	perspective->GetMatrix(&projection.Project);

	for (pair<Effect *, CBuffer*> temp : perFrameMap)
		temp.second->Change();
	ChangePerspective();

	CBuffers::Update();
}

Context::Context()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	perspective = new Perspective(desc.Width, desc.Height);
	perspective->GetMatrix(&projection.Project);

	cameras.push_back(new Freedom());
	cameras.push_back(new Orbit(20.0f));
	mainCamera = cameras[0];

	viewport = new Viewport(desc.Width, desc.Height);
}

Context::~Context()
{
	for (pair<Effect *, CBuffer*> temp : perFrameMap)
		SAFE_DELETE(temp.second);

	for (pair<Effect *, CBuffer*> temp : projectionMap)
		SAFE_DELETE(temp.second);

	for (pair<Effect *, CBuffer*> temp : lightMap)
		SAFE_DELETE(temp.second);

	for (pair<Effect *, CBuffer*> temp : pointLightMap)
		SAFE_DELETE(temp.second);

	for (pair<Effect *, CBuffer*> temp : spotLightMap)
		SAFE_DELETE(temp.second);

	for(Camera * camera : cameras)
		SAFE_DELETE(camera);

	SAFE_DELETE(perspective);
	SAFE_DELETE(viewport);
}

void Context::AddEffect(Effect * effect)
{
	CBuffer* cbPerFrame = new CBuffer(effect, "CB_PerFrame", &perFrame, sizeof(PerFrame));
	perFrameMap.insert(pair<Effect *, CBuffer*>(effect, cbPerFrame));

	CBuffer* cbProjection = new CBuffer(effect, "CB_Projection", &projection, sizeof(Projection));
	projectionMap.insert(pair<Effect *, CBuffer*>(effect, cbProjection));

	CBuffer* cbLight = new CBuffer(effect, "CB_Light", &light, sizeof(GlobalLight));
	lightMap.insert(pair<Effect *, CBuffer*>(effect, cbLight));

	CBuffer* cbPointLight = new CBuffer(effect, "CB_PointLight", &pointLight, sizeof(PointLightBuffer));
	pointLightMap.insert(pair<Effect *, CBuffer*>(effect, cbPointLight));

	CBuffer* cbSpotLight = new CBuffer(effect, "CB_SpotLight", &spotLight, sizeof(SpotLightBuffer));
	spotLightMap.insert(pair<Effect *, CBuffer*>(effect, cbSpotLight));
}

void Context::Erase(CBuffer * buffer)
{
	CBuffers::Erase(buffer);
}

void Context::Erase(Effect * effect)
{
	CBuffers::Erase(effect);

	map<Effect *, CBuffer*>::iterator iter;

	//	Erase PerFrame Buffer
	{
		iter = perFrameMap.begin();
		for (; iter != perFrameMap.end();)
		{
			if ((*iter).first == effect)
			{
				iter = perFrameMap.erase(iter);
			}
			else iter++;
		}
	}
	//	Erase Projection Buffer
	{
		iter = projectionMap.begin();
		for (; iter != projectionMap.end();)
		{
			if ((*iter).first == effect)
			{
				iter = projectionMap.erase(iter);
			}
			else iter++;
		}
	}

	//	Erase Light Buffer
	{
		iter = lightMap.begin();
		for (; iter != lightMap.end();)
		{
			if ((*iter).first == effect)
			{
				iter = lightMap.erase(iter);
			}
			else iter++;
		}
	}

	//	Erase PointLight Buffer
	{
		iter = pointLightMap.begin();
		for (; iter != pointLightMap.end();)
		{
			if ((*iter).first == effect)
			{
				iter = pointLightMap.erase(iter);
			}
			else iter++;
		}
	}

	//	Erase SpotLight Buffer
	{
		iter = spotLightMap.begin();
		for (; iter != spotLightMap.end();)
		{
			if ((*iter).first == effect)
			{
				iter = spotLightMap.erase(iter);
			}
			else iter++;
		}
	}
}

Perspective * Context::GetPerspective()
{
	return perspective;
}

void Context::ChangePerspective()
{
	for (pair<Effect *, CBuffer*> temp : projectionMap)
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

void Context::ChangeCameraType(UINT val)
{
	mainCamera = cameras[val];
}

Context::GlobalLight * Context::GetGlobalLight()
{
	return &light;
}

void Context::ChangeGlobalLight()
{
	for (pair<Effect *, CBuffer*> temp : lightMap)
		temp.second->Change();
}

void Context::AddPointLight(PointLightDesc & light)
{
	pointLight.Lights[pointLight.Count] = light;
	pointLight.Count++;

	for (pair<Effect *, CBuffer*> temp : pointLightMap)
		temp.second->Change();
}

void Context::AddSpotLight(SpotLightDesc & light)
{
	spotLight.Lights[pointLight.Count] = light;
	spotLight.Count++;

	for (pair<Effect *, CBuffer*> temp : spotLightMap)
		temp.second->Change();
}