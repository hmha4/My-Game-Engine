#include "Framework.h"
#include "AlphaRenderer.h"
#include "Interfaces/IAlpha.h"

AlphaRenderer * AlphaRenderer::instance = NULL;
void AlphaRenderer::Create()
{
	if (instance != NULL)
		return;

	instance = new AlphaRenderer();
}

void AlphaRenderer::Delete()
{
	if (instance == NULL)
		return;

	SAFE_DELETE(instance);
}

AlphaRenderer * AlphaRenderer::Get()
{
	return instance;
}

AlphaRenderer::AlphaRenderer()
{
	alphaMaps.clear();
	alphaMapIter = alphaMaps.begin();

	forwardMaps.clear();
	forwardMapIter = forwardMaps.begin();
}

AlphaRenderer::~AlphaRenderer()
{
}

void AlphaRenderer::RenderD()
{
	for (IAlpha * alpha : alphaMaps)
		alpha->Render();
}

void AlphaRenderer::RenderF()
{
	for (IAlpha * alpha : forwardMaps)
		alpha->Render();
}

void AlphaRenderer::AddAlphaMaps(IAlpha * alphas)
{
	alphaMapIter = alphaMaps.insert(alphaMapIter, alphas);
}

void AlphaRenderer::AddAlphaMapsF(IAlpha * alphas)
{
	forwardMapIter = forwardMaps.insert(forwardMapIter, alphas);
}


