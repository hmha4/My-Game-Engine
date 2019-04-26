#pragma once
#include "GameEditor.h"

class ObjectEditor : public GameEditor
{
public:
	ObjectEditor();
	~ObjectEditor();

	// GameEditor을(를) 통해 상속됨
	void Initalize() override;
	void Ready() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void HierarchyRender() override;
	void ProjectRender() override;
	void InspectorRender() override;
	void AssetRender() override;
	void Delete() override;

	void SetGameObjectPtr(IGameObject * gameObject) override;
	void SetType(UINT type) override;

private:
	void Transform();
	void Information();

private:
	IGameObject * object;
	GameModel * model;
};