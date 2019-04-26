#pragma once

class AlphaRenderer
{
public:
	AlphaRenderer();
	~AlphaRenderer();

	void RenderD();
	void RenderF();
	void AddAlphaMaps(class IAlpha * alphas);
	void AddAlphaMapsF(class IAlpha * alphas);

	void RemoveMaps();

private:
	vector<class IAlpha *> alphaMaps;
	vector<class IAlpha *>::iterator alphaMapIter;

	list<class IAlpha *> forwardMaps;
	list<class IAlpha *>::iterator forwardMapIter;

public:
	static void Create();
	static void Delete();
	static AlphaRenderer * Get();

private:
	static AlphaRenderer * instance;
};