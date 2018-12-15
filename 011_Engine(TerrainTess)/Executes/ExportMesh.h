#pragma once
#include "Execute.h"

class ExportMesh : public Execute
{
public:
	ExportMesh();
	~ExportMesh();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();
	
private:
	vector<class GameModel*> models;
};


