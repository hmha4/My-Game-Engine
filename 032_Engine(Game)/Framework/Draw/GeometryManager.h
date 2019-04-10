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

	void Initialize();
	void Ready();
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

		void Initialize()
		{
			xAxisModel->Initialize();
			yAxisModel->Initialize();
			zAxisModel->Initialize();

			XZPlane->Initialize();
			XYPlane->Initialize();
			YZPlane->Initialize();

			xLine->Initialize();
			yLine->Initialize();
			zLine->Initialize();

			xBoundingBox->Initialize();
			yBoundingBox->Initialize();
			zBoundingBox->Initialize();
		}
		void Ready()
		{
			xAxisModel->Ready();
			yAxisModel->Ready();
			zAxisModel->Ready();

			XZPlane->Ready();
			XYPlane->Ready();
			YZPlane->Ready();

			xLine->Ready();
			yLine->Ready();
			zLine->Ready();

			xBoundingBox->Ready();
			yBoundingBox->Ready();
			zBoundingBox->Ready();
		}
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

		void Delete()
		{
			SAFE_DELETE(xAxisModel);
			SAFE_DELETE(yAxisModel);
			SAFE_DELETE(zAxisModel);

			SAFE_DELETE(XZPlane);
			SAFE_DELETE(XYPlane);
			SAFE_DELETE(YZPlane);

			SAFE_DELETE(xLine);
			SAFE_DELETE(yLine);
			SAFE_DELETE(zLine);

			SAFE_DELETE(xBoundingBox);
			SAFE_DELETE(yBoundingBox);
			SAFE_DELETE(zBoundingBox);
		}
	};
private:
	//¸ðµ¨+¹æÇâ¼±
	GizmoAxisModel * axisModels[4];

	Effect *gizmoEffect;
	Effect *lineEffect;

	ModelType currentModelType;
	PickState pickState;

	D3DXMATRIX*targetWorld;
	D3DXMATRIX initMat;

	float lineLength;
};
