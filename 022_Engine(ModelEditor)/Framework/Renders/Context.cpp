#include "Framework.h"
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

void Context::UpdateVP(D3DXMATRIX view, D3DXMATRIX proj)
{
	perFrame.View = view;
	projection.Project = proj;

	for (pair<Shader *, CBuffer*> temp : perFrameMap)
		temp.second->Change();
	ChangePerspective();

	CBuffers::Update();
}

void Context::InitVP()
{
	mainCamera->Update();
	mainCamera->MatrixView(&perFrame.View);
	perspective->GetMatrix(&projection.Project);

	for (pair<Shader *, CBuffer*> temp : perFrameMap)
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

	for (pair<Shader *, CBuffer*> temp : pointLightMap)
		SAFE_DELETE(temp.second);

	for (pair<Shader *, CBuffer*> temp : spotLightMap)
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

	CBuffer* cbPointLight = new CBuffer(shader, "CB_PointLight", &pointLight, sizeof(PointLightBuffer));
	pointLightMap.insert(pair<Shader *, CBuffer*>(shader, cbPointLight));

	CBuffer* cbSpotLight = new CBuffer(shader, "CB_SpotLight", &spotLight, sizeof(SpotLightBuffer));
	spotLightMap.insert(pair<Shader *, CBuffer*>(shader, cbSpotLight));
}

void Context::Erase(CBuffer * buffer)
{
	CBuffers::Erase(buffer);
}

void Context::Erase(Shader * shader)
{
	CBuffers::Erase(shader);

	map<Shader *, CBuffer*>::iterator iter;

	//	Erase PerFrame Buffer
	{
		iter = perFrameMap.begin();
		for (; iter != perFrameMap.end();)
		{
			if ((*iter).first == shader)
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
			if ((*iter).first == shader)
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
			if ((*iter).first == shader)
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
			if ((*iter).first == shader)
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
			if ((*iter).first == shader)
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

void Context::AddPointLight(PointLightDesc & light)
{
	pointLight.Lights[pointLight.Count] = light;
	pointLight.Count++;

	for (pair<Shader *, CBuffer*> temp : pointLightMap)
		temp.second->Change();
}

void Context::AddSpotLight(SpotLightDesc & light)
{
	spotLight.Lights[pointLight.Count] = light;
	spotLight.Count++;

	for (pair<Shader *, CBuffer*> temp : spotLightMap)
		temp.second->Change();
}