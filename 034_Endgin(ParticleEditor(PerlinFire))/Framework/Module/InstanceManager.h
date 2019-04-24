#pragma once

class InstanceManager
{
public:
	enum class InstanceType
	{
		MODEL,
		ANIMATION
	};
public:
	static void Create();
	static void Delete();
	static InstanceManager *Get() { return instance; }

private:
	static InstanceManager *instance;

private:
	typedef unordered_map<wstring,class ModelInstance*>::iterator modelsIter;
	typedef unordered_map<wstring, class AnimInstance*>::iterator animsIter;

public:
	InstanceManager();
	~InstanceManager();

	unordered_map<wstring,class ModelInstance*> modelInstance;
	unordered_map<wstring, class AnimInstance*> animInstance;

	void AddModel(class GameModel *model, wstring shaderFile, InstanceType instType);
	void AddModel(class GameModel *model, Effect *effect, InstanceType instType);

	void DeleteModel(class GameModel *model,UINT instanceID, InstanceType instType);

	void UpdateWorld(class GameModel *model, UINT instNum, D3DXMATRIX &world, InstanceType instType);
	class ModelInstance *FindModel(wstring name);
	class AnimInstance *FindAnim(wstring name);

	void Render(UINT tech);
	void Render(wstring name);
	void Update();
};
