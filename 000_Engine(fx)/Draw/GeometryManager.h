#pragma once
#include "GizmoModel.h"
class GeometryManager
{
public:
	enum class ModelType
	{
		Translation,
		Rotate,
		Scale,
		UniformScale
	};
	enum class PickState
	{
		PICK_X,
		PICK_Y,
		PICK_Z,
		PICK_XY,
		PICK_XZ,
		PICK_ZY,
		PICK_NONE
	};
public:
	GeometryManager(float length = 0);
	~GeometryManager();

	void Update();
	void Render();

	void SetMode(UINT activeMode);
	void SetWorld(D3DXMATRIX * world);

	void ColorPickUpdate(IN OUT UINT* activeAxis);

	void SetScale(float x, float y, float z);
	void SetLineLength(float length);
	void SetColor(float x, float y, float z);
private:

private:
	struct GizmoAxisModel
	{
		ModelType modelType;

		Geometry*xAxisModel;
		Geometry*yAxisModel;
		Geometry*zAxisModel;
		Geometry*XZPlane;
		Geometry*XYPlane;
		Geometry*YZPlane;
		Geometry*xLine;
		Geometry*yLine;
		Geometry*zLine;
		Geometry*xBoundingBox;
		Geometry*yBoundingBox;
		Geometry*zBoundingBox;

		void Update()
		{
			xAxisModel->Update();
			yAxisModel->Update();
			zAxisModel->Update();
			XZPlane->Update();
			XYPlane->Update();
			YZPlane->Update();
			xLine->Update();
			yLine->Update();
			zLine->Update();
			xBoundingBox->Update();
			yBoundingBox->Update();
			zBoundingBox->Update();
		}

		void Render()
		{
			xAxisModel->Render();
			yAxisModel->Render();
			zAxisModel->Render();
			XZPlane->Render();
			XYPlane->Render();
			YZPlane->Render();
			xLine->Render();
			yLine->Render();
			zLine->Render();
			xBoundingBox->Render();
			yBoundingBox->Render();
			zBoundingBox->Render();
		}
	};
private:
	//모델+방향선
	GizmoAxisModel * axisModels[4];

	ModelType currentModelType;
	PickState pickState;

	D3DXMATRIX*targetWorld;
	D3DXMATRIX initMat;
};

