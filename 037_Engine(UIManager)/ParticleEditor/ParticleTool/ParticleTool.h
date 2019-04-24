#pragma once


class ParticleTool
{
public:
	ParticleTool();
	~ParticleTool();

	void Initialize();
	void Ready();
	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void Delete();
	void Reset();

private:
	void RenderHeirarchy();
	void RenderInspector();

private:
	class ParticleSystem*pSystem;
	class GizmoGrid * grid;

private:
	void Save();
	void Load(wstring fileName);

};