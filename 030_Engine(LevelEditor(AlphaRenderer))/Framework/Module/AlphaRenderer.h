#pragma once

class AlphaRenderer
{
public:
	AlphaRenderer();
	~AlphaRenderer();

	void Render();
	void AddAlphaMaps(class IAlpha * alphas);

private:
	list<class IAlpha *> alphaMaps;
	list<class IAlpha *>::iterator alphaMapIter;

public:
	static void Create();
	static void Delete();
	static AlphaRenderer * Get();

private:
	static AlphaRenderer * instance;
};