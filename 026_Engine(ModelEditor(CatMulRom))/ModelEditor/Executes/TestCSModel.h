#pragma once
#include "Systems/IExecute.h"


//	FbxType : Remove Indices
//			  Vertices to array
//	FbxLoader : Remove Indices
//			    Vertices to array
//	ModelReader
//	ModelMeshPart
//	ModelMesh
//	GameModel : UpdateVertex


class TestCSModel : public IExecute
{
public:
	void Initialize() override;
	void Ready() override;
	void Destroy() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	void ResizeScreen() override;

private:
	GameModel * model;
};