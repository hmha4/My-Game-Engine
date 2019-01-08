#pragma once
#include "stdafx.h"

class GameEditor
{
public:
	GameEditor(wstring name) : name(name) {}
	virtual ~GameEditor() {}

	virtual void Initalize() = 0;
	virtual void Ready() = 0;
	virtual void Update() = 0;
	virtual void PreRender() = 0;
	virtual void Render() = 0;
	virtual void HierarchyRender() = 0;
	virtual void ProjectRender() = 0;
	virtual void InspectorRender() = 0;
	virtual void AssetRender() = 0;
	virtual void Delete() = 0;

	virtual void SetGameObjectPtr(class IGameObject * gameObject) = 0;
	virtual void SetType(UINT type) = 0;
protected:
	wstring name;

};