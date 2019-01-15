#pragma once

class ModelTool
{
	enum class TARGET
	{
		DEFAULT = 0, BONE, MESH,
	};
public:
	ModelTool();
	~ModelTool();

	void Initialize();
	void Ready();
	void Update();
	void PreRender();
	void Render();
	void Delete();

private:
	void RenderHeirarchy();
	void RenderInspector();
	void RenderAnimation();

	void GetBoneName(ModelBone *bone);
	void GetMeshData();
	void GetMaterialData();

private:
	class Shadow * shadow;

	//	Current Object
	IGameObject * object;
	Model * model;
	int boneIndex;
	int meshIndex;
	int materialIndex;


	TARGET target;

	//	ImGui
	ImGuiTextFilter	filter;
};