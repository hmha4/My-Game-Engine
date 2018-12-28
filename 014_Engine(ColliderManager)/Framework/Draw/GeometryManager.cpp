#include "Framework.h"
#include "GeometryManager.h"
#include "../Draw/LineMaker.h"

GeometryManager::GeometryManager(float length)
{
	D3DXMatrixIdentity(&initMat);
	currentModelType = ModelType::Translation;
	pickState = PickState::PICK_NONE;

	for (int i = 0; i < 4; i++)
		axisModels[i] = new GizmoAxisModel();

	axisModels[0]->modelType = ModelType::Translation;
	axisModels[1]->modelType = ModelType::Rotate;
	axisModels[2]->modelType = ModelType::Scale;
	axisModels[3]->modelType = ModelType::UniformScale;

	//ScaleModel
	{
		axisModels[3]->xAxisModel = new Geometry_Scale(Geometry::XYZAxis::XAxis, Geometry::GizmoType::Component);
		axisModels[3]->xAxisModel->SetLineLength(length);
		axisModels[3]->xAxisModel->SetColor(1, 0, 0);
		axisModels[3]->xAxisModel->SetTargetWorld(&initMat);
				   
		axisModels[3]->yAxisModel = new Geometry_Scale(Geometry::XYZAxis::YAxis, Geometry::GizmoType::Component);
		axisModels[3]->yAxisModel->SetLineLength(length);
		axisModels[3]->yAxisModel->SetColor(1, 0, 0);
		axisModels[3]->yAxisModel->SetTargetWorld(&initMat);
				   
		axisModels[3]->zAxisModel = new Geometry_Scale(Geometry::XYZAxis::ZAxis, Geometry::GizmoType::Component);
		axisModels[3]->zAxisModel->SetLineLength(length);
		axisModels[3]->zAxisModel->SetColor(1, 0, 0);
		axisModels[3]->zAxisModel->SetTargetWorld(&initMat);
				   
		axisModels[3]->XZPlane = new Geometry_Plane(Geometry::XYZAxis::XZPlane, Geometry::GizmoType::Component);
		axisModels[3]->XZPlane->SetLineLength(length);
		axisModels[3]->XZPlane->SetTargetWorld(&initMat);
				   
		axisModels[3]->XYPlane = new Geometry_Plane(Geometry::XYZAxis::XYPlane, Geometry::GizmoType::Component);
		axisModels[3]->XYPlane->SetLineLength(length);
		axisModels[3]->XYPlane->SetTargetWorld(&initMat);
				   
		axisModels[3]->YZPlane = new Geometry_Plane(Geometry::XYZAxis::YZPlane, Geometry::GizmoType::Component);
		axisModels[3]->YZPlane->SetLineLength(length);
		axisModels[3]->YZPlane->SetTargetWorld(&initMat);
				   
		axisModels[3]->xLine = new Geometry_Line(Geometry::XYZAxis::XLine, Geometry::GizmoType::Component);
		axisModels[3]->xLine->SetLineLength(length);
		axisModels[3]->xLine->SetTargetWorld(&initMat);
				   
		axisModels[3]->yLine = new Geometry_Line(Geometry::XYZAxis::YLine, Geometry::GizmoType::Component);
		axisModels[3]->yLine->SetLineLength(length);
		axisModels[3]->yLine->SetTargetWorld(&initMat);
				   
		axisModels[3]->zLine = new Geometry_Line(Geometry::XYZAxis::ZLine, Geometry::GizmoType::Component);
		axisModels[3]->zLine->SetLineLength(length);
		axisModels[3]->zLine->SetTargetWorld(&initMat);
				   
		axisModels[3]->xBoundingBox = new Geometry_Box(Geometry::XYZAxis::XBox, Geometry::GizmoType::Component);
		axisModels[3]->xBoundingBox->SetLineLength(length);
		axisModels[3]->xBoundingBox->SetTargetWorld(&initMat);
				   
		axisModels[3]->yBoundingBox = new Geometry_Box(Geometry::XYZAxis::YBox, Geometry::GizmoType::Component);
		axisModels[3]->yBoundingBox->SetLineLength(length);
		axisModels[3]->yBoundingBox->SetTargetWorld(&initMat);
				   
		axisModels[3]->zBoundingBox = new Geometry_Box(Geometry::XYZAxis::ZBox, Geometry::GizmoType::Component);
		axisModels[3]->zBoundingBox->SetLineLength(length);
		axisModels[3]->zBoundingBox->SetTargetWorld(&initMat);
	}

	//ScaleModel
	{
		axisModels[2]->xAxisModel = new Geometry_Scale(Geometry::XYZAxis::XAxis, Geometry::GizmoType::Component);
		axisModels[2]->xAxisModel->SetLineLength(length);
		axisModels[2]->xAxisModel->SetTargetWorld(&initMat);

		axisModels[2]->yAxisModel = new Geometry_Scale(Geometry::XYZAxis::YAxis, Geometry::GizmoType::Component);
		axisModels[2]->yAxisModel->SetLineLength(length);
		axisModels[2]->yAxisModel->SetTargetWorld(&initMat);

		axisModels[2]->zAxisModel = new Geometry_Scale(Geometry::XYZAxis::ZAxis, Geometry::GizmoType::Component);
		axisModels[2]->zAxisModel->SetLineLength(length);
		axisModels[2]->zAxisModel->SetTargetWorld(&initMat);

		axisModels[2]->XZPlane = new Geometry_Plane(Geometry::XYZAxis::XZPlane, Geometry::GizmoType::Component);
		axisModels[2]->XZPlane->SetLineLength(length);
		axisModels[2]->XZPlane->SetTargetWorld(&initMat);

		axisModels[2]->XYPlane = new Geometry_Plane(Geometry::XYZAxis::XYPlane, Geometry::GizmoType::Component);
		axisModels[2]->XYPlane->SetLineLength(length);
		axisModels[2]->XYPlane->SetTargetWorld(&initMat);

		axisModels[2]->YZPlane = new Geometry_Plane(Geometry::XYZAxis::YZPlane, Geometry::GizmoType::Component);
		axisModels[2]->YZPlane->SetLineLength(length);
		axisModels[2]->YZPlane->SetTargetWorld(&initMat);

		axisModels[2]->xLine = new Geometry_Line(Geometry::XYZAxis::XLine, Geometry::GizmoType::Component);
		axisModels[2]->xLine->SetLineLength(length);
		axisModels[2]->xLine->SetTargetWorld(&initMat);

		axisModels[2]->yLine = new Geometry_Line(Geometry::XYZAxis::YLine, Geometry::GizmoType::Component);
		axisModels[2]->yLine->SetLineLength(length);
		axisModels[2]->yLine->SetTargetWorld(&initMat);

		axisModels[2]->zLine = new Geometry_Line(Geometry::XYZAxis::ZLine, Geometry::GizmoType::Component);
		axisModels[2]->zLine->SetLineLength(length);
		axisModels[2]->zLine->SetTargetWorld(&initMat);

		axisModels[2]->xBoundingBox = new Geometry_Box(Geometry::XYZAxis::XBox, Geometry::GizmoType::Component);
		axisModels[2]->xBoundingBox->SetLineLength(length);
		axisModels[2]->xBoundingBox->SetTargetWorld(&initMat);

		axisModels[2]->yBoundingBox = new Geometry_Box(Geometry::XYZAxis::YBox, Geometry::GizmoType::Component);
		axisModels[2]->yBoundingBox->SetLineLength(length);
		axisModels[2]->yBoundingBox->SetTargetWorld(&initMat);

		axisModels[2]->zBoundingBox = new Geometry_Box(Geometry::XYZAxis::ZBox, Geometry::GizmoType::Component);
		axisModels[2]->zBoundingBox->SetLineLength(length);
		axisModels[2]->zBoundingBox->SetTargetWorld(&initMat);
	}

	//RotateModel
	{
		axisModels[1]->xAxisModel = new Geometry_Rotate(Geometry::XYZAxis::XAxis, Geometry::GizmoType::Component);
		axisModels[1]->xAxisModel->SetLineLength(length);
		axisModels[1]->xAxisModel->SetTargetWorld(&initMat);

		axisModels[1]->yAxisModel = new Geometry_Rotate(Geometry::XYZAxis::YAxis, Geometry::GizmoType::Component);
		axisModels[1]->yAxisModel->SetLineLength(length);
		axisModels[1]->yAxisModel->SetTargetWorld(&initMat);

		axisModels[1]->zAxisModel = new Geometry_Rotate(Geometry::XYZAxis::ZAxis, Geometry::GizmoType::Component);
		axisModels[1]->zAxisModel->SetLineLength(length);
		axisModels[1]->zAxisModel->SetTargetWorld(&initMat);

		axisModels[1]->XZPlane = new Geometry_Plane( Geometry::XYZAxis::XZPlane, Geometry::GizmoType::Component);
		axisModels[1]->XZPlane->SetLineLength(length);
		axisModels[1]->XZPlane->SetTargetWorld(&initMat);

		axisModels[1]->XYPlane = new Geometry_Plane(Geometry::XYZAxis::XYPlane, Geometry::GizmoType::Component);
		axisModels[1]->XYPlane->SetLineLength(length);
		axisModels[1]->XYPlane->SetTargetWorld(&initMat);

		axisModels[1]->YZPlane = new Geometry_Plane(Geometry::XYZAxis::YZPlane, Geometry::GizmoType::Component);
		axisModels[1]->YZPlane->SetLineLength(length);
		axisModels[1]->YZPlane->SetTargetWorld(&initMat);

		axisModels[1]->xLine = new Geometry_Line(Geometry::XYZAxis::XLine, Geometry::GizmoType::Component);
		axisModels[1]->xLine->SetLineLength(length);
		axisModels[1]->xLine->SetTargetWorld(&initMat);

		axisModels[1]->yLine = new Geometry_Line(Geometry::XYZAxis::YLine, Geometry::GizmoType::Component);
		axisModels[1]->yLine->SetLineLength(length);
		axisModels[1]->yLine->SetTargetWorld(&initMat);

		axisModels[1]->zLine = new Geometry_Line(Geometry::XYZAxis::ZLine, Geometry::GizmoType::Component);
		axisModels[1]->zLine->SetLineLength(length);
		axisModels[1]->zLine->SetTargetWorld(&initMat);

		axisModels[1]->xBoundingBox = new Geometry_Box(Geometry::XYZAxis::XBox, Geometry::GizmoType::Component);
		axisModels[1]->xBoundingBox->SetLineLength(length);
		axisModels[1]->xBoundingBox->SetTargetWorld(&initMat);
				   
		axisModels[1]->yBoundingBox = new Geometry_Box(Geometry::XYZAxis::YBox, Geometry::GizmoType::Component);
		axisModels[1]->yBoundingBox->SetLineLength(length);
		axisModels[1]->yBoundingBox->SetTargetWorld(&initMat);
				   
		axisModels[1]->zBoundingBox = new Geometry_Box(Geometry::XYZAxis::ZBox, Geometry::GizmoType::Component);
		axisModels[1]->zBoundingBox->SetLineLength(length);
		axisModels[1]->zBoundingBox->SetTargetWorld(&initMat);
	}

	//TransModel
	{
		axisModels[0]->xAxisModel = new Geometry_Trans(Geometry::XYZAxis::XAxis, Geometry::GizmoType::Component);
		axisModels[0]->xAxisModel->SetLineLength(length);
		axisModels[0]->xAxisModel->SetTargetWorld(&initMat);

		axisModels[0]->yAxisModel = new Geometry_Trans(Geometry::XYZAxis::YAxis, Geometry::GizmoType::Component);
		axisModels[0]->yAxisModel->SetLineLength(length);
		axisModels[0]->yAxisModel->SetTargetWorld(&initMat);

		axisModels[0]->zAxisModel = new Geometry_Trans(Geometry::XYZAxis::ZAxis, Geometry::GizmoType::Component);
		axisModels[0]->zAxisModel->SetLineLength(length);
		axisModels[0]->zAxisModel->SetTargetWorld(&initMat);

		axisModels[0]->XZPlane = new Geometry_Plane(Geometry::XYZAxis::XZPlane, Geometry::GizmoType::Component);
		axisModels[0]->XZPlane->SetLineLength(length);
		axisModels[0]->XZPlane->SetTargetWorld(&initMat);

		axisModels[0]->XYPlane = new Geometry_Plane(Geometry::XYZAxis::XYPlane, Geometry::GizmoType::Component);
		axisModels[0]->XYPlane->SetLineLength(length);
		axisModels[0]->XYPlane->SetTargetWorld(&initMat);

		axisModels[0]->YZPlane = new Geometry_Plane(Geometry::XYZAxis::YZPlane, Geometry::GizmoType::Component);
		axisModels[0]->YZPlane->SetLineLength(length);
		axisModels[0]->YZPlane->SetTargetWorld(&initMat);

		axisModels[0]->xLine = new Geometry_Line(Geometry::XYZAxis::XLine, Geometry::GizmoType::Component);
		axisModels[0]->xLine->SetLineLength(length);
		axisModels[0]->xLine->SetTargetWorld(&initMat);

		axisModels[0]->yLine = new Geometry_Line(Geometry::XYZAxis::YLine, Geometry::GizmoType::Component);
		axisModels[0]->yLine->SetLineLength(length);
		axisModels[0]->yLine->SetTargetWorld(&initMat);

		axisModels[0]->zLine = new Geometry_Line(Geometry::XYZAxis::ZLine, Geometry::GizmoType::Component);
		axisModels[0]->zLine->SetLineLength(length);
		axisModels[0]->zLine->SetTargetWorld(&initMat);

		axisModels[0]->xBoundingBox = new Geometry_Box(Geometry::XYZAxis::XBox, Geometry::GizmoType::Component);
		axisModels[0]->xBoundingBox->SetLineLength(length);
		axisModels[0]->xBoundingBox->SetTargetWorld(&initMat);
				   
		axisModels[0]->yBoundingBox = new Geometry_Box(Geometry::XYZAxis::YBox, Geometry::GizmoType::Component);
		axisModels[0]->yBoundingBox->SetLineLength(length);
		axisModels[0]->yBoundingBox->SetTargetWorld(&initMat);
				   
		axisModels[0]->zBoundingBox = new Geometry_Box(Geometry::XYZAxis::ZBox, Geometry::GizmoType::Component);
		axisModels[0]->zBoundingBox->SetLineLength(length);
		axisModels[0]->zBoundingBox->SetTargetWorld(&initMat);
	}
}

GeometryManager::~GeometryManager()
{
	for (int i = 0; i < 4; i++)
	{
		SAFE_DELETE(axisModels[i]->xAxisModel);
		SAFE_DELETE(axisModels[i]->yAxisModel);
		SAFE_DELETE(axisModels[i]->zAxisModel);

		SAFE_DELETE(axisModels[i]->XZPlane);
		SAFE_DELETE(axisModels[i]->XYPlane);
		SAFE_DELETE(axisModels[i]->YZPlane);

		SAFE_DELETE(axisModels[i]->xLine);
		SAFE_DELETE(axisModels[i]->yLine);
		SAFE_DELETE(axisModels[i]->zLine);
	}
}

void GeometryManager::Update()
{
	axisModels[(int)currentModelType]->Update();

	//if (Keyboard::Get()->Down('1'))
	//	currentModelType = ModelType::Scale;
	//if (Keyboard::Get()->Down('2'))
	//	currentModelType = ModelType::Rotate;
	//if (Keyboard::Get()->Down('3'))
	//	currentModelType = ModelType::Translation;

	//ColorPickUpdate();
}

void GeometryManager::Render()
{
	axisModels[(int)currentModelType]->Render();
}

void GeometryManager::SetMode(UINT activeMode)
{
	currentModelType = (ModelType)activeMode;
}

void GeometryManager::SetWorld(D3DXMATRIX * world)
{
	this->targetWorld = world;

	//ScaleModel
	{
		axisModels[0]->xAxisModel->SetTargetWorld(targetWorld);
		axisModels[0]->yAxisModel->SetTargetWorld(targetWorld);
		axisModels[0]->zAxisModel->SetTargetWorld(targetWorld);
		axisModels[0]->XZPlane->SetTargetWorld(targetWorld);
		axisModels[0]->XYPlane->SetTargetWorld(targetWorld);
		axisModels[0]->YZPlane->SetTargetWorld(targetWorld);
		axisModels[0]->xLine->SetTargetWorld(targetWorld);
		axisModels[0]->yLine->SetTargetWorld(targetWorld);
		axisModels[0]->zLine->SetTargetWorld(targetWorld);
		axisModels[0]->xBoundingBox->SetTargetWorld(targetWorld);
		axisModels[0]->yBoundingBox->SetTargetWorld(targetWorld);
		axisModels[0]->zBoundingBox->SetTargetWorld(targetWorld);
	}

	//RotateModel
	{
		axisModels[1]->xAxisModel->SetTargetWorld(targetWorld);
		axisModels[1]->yAxisModel->SetTargetWorld(targetWorld);
		axisModels[1]->zAxisModel->SetTargetWorld(targetWorld);
		axisModels[1]->XZPlane->SetTargetWorld(targetWorld);
		axisModels[1]->XYPlane->SetTargetWorld(targetWorld);
		axisModels[1]->YZPlane->SetTargetWorld(targetWorld);
		axisModels[1]->xLine->SetTargetWorld(targetWorld);
		axisModels[1]->yLine->SetTargetWorld(targetWorld);
		axisModels[1]->zLine->SetTargetWorld(targetWorld);
		axisModels[1]->xBoundingBox->SetTargetWorld(targetWorld);
		axisModels[1]->yBoundingBox->SetTargetWorld(targetWorld);
		axisModels[1]->zBoundingBox->SetTargetWorld(targetWorld);
	}

	//TransModel
	{
		axisModels[2]->xAxisModel->SetTargetWorld(targetWorld);
		axisModels[2]->yAxisModel->SetTargetWorld(targetWorld);
		axisModels[2]->zAxisModel->SetTargetWorld(targetWorld);
		axisModels[2]->XZPlane->SetTargetWorld(targetWorld);
		axisModels[2]->XYPlane->SetTargetWorld(targetWorld);
		axisModels[2]->YZPlane->SetTargetWorld(targetWorld);
		axisModels[2]->xLine->SetTargetWorld(targetWorld);
		axisModels[2]->yLine->SetTargetWorld(targetWorld);
		axisModels[2]->zLine->SetTargetWorld(targetWorld);
		axisModels[2]->xBoundingBox->SetTargetWorld(targetWorld);
		axisModels[2]->yBoundingBox->SetTargetWorld(targetWorld);
		axisModels[2]->zBoundingBox->SetTargetWorld(targetWorld);
	}

	//TransModel
	{
		axisModels[3]->xAxisModel->SetTargetWorld(targetWorld);
		axisModels[3]->yAxisModel->SetTargetWorld(targetWorld);
		axisModels[3]->zAxisModel->SetTargetWorld(targetWorld);
		axisModels[3]->XZPlane->SetTargetWorld(targetWorld);
		axisModels[3]->XYPlane->SetTargetWorld(targetWorld);
		axisModels[3]->YZPlane->SetTargetWorld(targetWorld);
		axisModels[3]->xLine->SetTargetWorld(targetWorld);
		axisModels[3]->yLine->SetTargetWorld(targetWorld);
		axisModels[3]->zLine->SetTargetWorld(targetWorld);
		axisModels[3]->xBoundingBox->SetTargetWorld(targetWorld);
		axisModels[3]->yBoundingBox->SetTargetWorld(targetWorld);
		axisModels[3]->zBoundingBox->SetTargetWorld(targetWorld);
	}
}

void GeometryManager::SetScale(float x, float y, float z)
{
	for (int i = 0; i < 4; i++)
	{
		axisModels[i]->xAxisModel->Scale(x, y, z);
		axisModels[i]->yAxisModel->Scale(x, y, z);
		axisModels[i]->zAxisModel->Scale(x, y, z);
		axisModels[i]->XZPlane->Scale(x, y, z);
		axisModels[i]->XYPlane->Scale(x, y, z);
		axisModels[i]->YZPlane->Scale(x, y, z);
		axisModels[i]->xLine->Scale(x, y, z);
		axisModels[i]->yLine->Scale(x, y, z);
		axisModels[i]->zLine->Scale(x, y, z);
		axisModels[i]->xBoundingBox->Scale(x, y, z);
		axisModels[i]->yBoundingBox->Scale(x, y, z);
		axisModels[i]->zBoundingBox->Scale(x, y, z);
	}
}

void GeometryManager::SetLineLength(float length)
{
	for (int i = 0; i < 4; i++)
	{
		axisModels[i]->xAxisModel->SetLineLength(length);
		axisModels[i]->yAxisModel->SetLineLength(length);
		axisModels[i]->zAxisModel->SetLineLength(length);
		axisModels[i]->XZPlane->SetLineLength(length);
		axisModels[i]->XYPlane->SetLineLength(length);
		axisModels[i]->YZPlane->SetLineLength(length);
		axisModels[i]->xLine->SetLineLength(length);
		axisModels[i]->yLine->SetLineLength(length);
		axisModels[i]->zLine->SetLineLength(length);
	}
}

void GeometryManager::SetColor(float x, float y, float z)
{
	for (int i = 0; i < 4; i++)
	{
		axisModels[i]->xAxisModel->SetColor(x,y,z);
		axisModels[i]->yAxisModel->SetColor(x, y, z);
		axisModels[i]->zAxisModel->SetColor(x, y, z);
		axisModels[i]->XZPlane->SetColor(x,y,z);
		axisModels[i]->XYPlane->SetColor(x,y,z);
		axisModels[i]->YZPlane->SetColor(x,y,z);
		axisModels[i]->xLine->SetColor(x,y,z);
		axisModels[i]->yLine->SetColor(x,y,z);
		axisModels[i]->zLine->SetColor(x,y,z);
	}
}


void GeometryManager::ColorPickUpdate(IN OUT UINT* activeAxis)
{
	if (axisModels[(int)currentModelType]->xAxisModel->IsPicked() == false &&
		axisModels[(int)currentModelType]->yAxisModel->IsPicked() == false &&
		axisModels[(int)currentModelType]->zAxisModel->IsPicked() == false &&
		axisModels[(int)currentModelType]->XZPlane->IsPicked() == false &&
		axisModels[(int)currentModelType]->XYPlane->IsPicked() == false &&
		axisModels[(int)currentModelType]->YZPlane->IsPicked() == false &&
		axisModels[(int)currentModelType]->xBoundingBox->IsPicked() == false &&
		axisModels[(int)currentModelType]->yBoundingBox->IsPicked() == false &&
		axisModels[(int)currentModelType]->zBoundingBox->IsPicked() == false)
		pickState = PickState::PICK_NONE;

	//=====================Axis=====================//
	if (axisModels[(int)currentModelType]->xAxisModel->IsPicked()|| 
		axisModels[(int)currentModelType]->xBoundingBox->IsPicked())
	{
		if (axisModels[(int)currentModelType]->xAxisModel->IsPicked())
		{
			axisModels[(int)currentModelType]->xLine->SetColor(1, 1, 0);
			pickState = PickState::PICK_X;
		}
		else if (axisModels[(int)currentModelType]->xBoundingBox->IsPicked())
		{
			axisModels[(int)currentModelType]->xLine->SetColor(1, 1, 0);
			axisModels[(int)currentModelType]->xAxisModel->SetColor(1, 1, 0);
			pickState = PickState::PICK_X;
		}
	}
	else
	{
		if (axisModels[(int)currentModelType]->XZPlane->IsPicked() ||
			axisModels[(int)currentModelType]->XYPlane->IsPicked())//축선택안하고 면선택할때
		{
			axisModels[(int)currentModelType]->xAxisModel->SetColor(1, 1, 0);

			pickState = axisModels[(int)currentModelType]->XZPlane->IsPicked() == true
				? PickState::PICK_XZ : PickState::PICK_XY;
		}
		else//축선택안하고 아무것도안할때는 기본색
		{
			axisModels[(int)currentModelType]->xAxisModel->SetColor(1, 0, 0);
		}

		axisModels[(int)currentModelType]->xLine->SetColor(1, 0, 0);
	}

	if (axisModels[(int)currentModelType]->yAxisModel->IsPicked() ||
		axisModels[(int)currentModelType]->yBoundingBox->IsPicked())
	{
		if (axisModels[(int)currentModelType]->yAxisModel->IsPicked())
		{
			axisModels[(int)currentModelType]->yLine->SetColor(1, 1, 0);
			pickState = PickState::PICK_Y;
		}
		else if (axisModels[(int)currentModelType]->yBoundingBox->IsPicked())
		{
			axisModels[(int)currentModelType]->yLine->SetColor(1, 1, 0);
			axisModels[(int)currentModelType]->yAxisModel->SetColor(1, 1, 0);
			pickState = PickState::PICK_Y;
		}
	}
	else
	{
		if (axisModels[(int)currentModelType]->XYPlane->IsPicked() ||
			axisModels[(int)currentModelType]->YZPlane->IsPicked())//축선택안하고 면선택할때
		{
			axisModels[(int)currentModelType]->yAxisModel->SetColor(1, 1, 0);
			pickState = axisModels[(int)currentModelType]->XYPlane->IsPicked() == true
				? PickState::PICK_XY : PickState::PICK_ZY;
		}
		else//축선택안하고 아무것도안할때는 기본색
		{
			if(currentModelType!=ModelType::UniformScale)
				axisModels[(int)currentModelType]->yAxisModel->SetColor(0, 1, 0);
			else
				axisModels[(int)currentModelType]->yAxisModel->SetColor(1, 0, 0);
		}

		if (currentModelType != ModelType::UniformScale)
			axisModels[(int)currentModelType]->yLine->SetColor(0, 1, 0);
		else
			axisModels[(int)currentModelType]->yLine->SetColor(1, 0, 0);
	}

	if (axisModels[(int)currentModelType]->zAxisModel->IsPicked() ||
		axisModels[(int)currentModelType]->zBoundingBox->IsPicked())
	{
		if (axisModels[(int)currentModelType]->zAxisModel->IsPicked())
		{
			axisModels[(int)currentModelType]->zLine->SetColor(1, 1, 0);
			pickState = PickState::PICK_Z;
		}
		else if (axisModels[(int)currentModelType]->zBoundingBox->IsPicked())
		{
			axisModels[(int)currentModelType]->zLine->SetColor(1, 1, 0);
			axisModels[(int)currentModelType]->zAxisModel->SetColor(1, 1, 0);
			pickState = PickState::PICK_Z;
		}
	}
	else
	{
		if (axisModels[(int)currentModelType]->YZPlane->IsPicked() ||
			axisModels[(int)currentModelType]->XZPlane->IsPicked())//축선택안하고 면선택할때
		{
			axisModels[(int)currentModelType]->zAxisModel->SetColor(1, 1, 0);
			pickState = axisModels[(int)currentModelType]->YZPlane->IsPicked() == true
				? PickState::PICK_ZY : PickState::PICK_XZ;
		}
		else//축선택안하고 아무것도안할때는 기본색
		{
			if (currentModelType != ModelType::UniformScale)
				axisModels[(int)currentModelType]->zAxisModel->SetColor(0, 0, 1);
			else
				axisModels[(int)currentModelType]->zAxisModel->SetColor(1, 0, 0);
		}

		if (currentModelType != ModelType::UniformScale)
			axisModels[(int)currentModelType]->zLine->SetColor(0, 0, 1);
		else
			axisModels[(int)currentModelType]->zLine->SetColor(1, 0, 0);
	}


	*activeAxis = (UINT)pickState;
	//==============================================//
}

