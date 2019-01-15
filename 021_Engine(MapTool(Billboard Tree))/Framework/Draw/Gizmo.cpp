#include "Framework.h"
#include "Gizmo.h"
#include "GeometryManager.h"
#include "../Objects/IGameObject.h"
#include "../Viewer/Camera.h"


Gizmo::Gizmo(D3DXMATRIX world)
	: sceneWorld(world)
	, isActive(true), enabled(false)
	, position(0, 0, 0), localForward(0, 0, 1), localUp(0, 1, 0)
	, activePivotNum(0)
{
	D3DXMatrixIdentity(&gizmoWorld);

	activeAxis = EGizmoAxis::None;
	activeMode = EGizmoMode::Translate;
	activeSpace = ETransformSpace::Local;
	activePivot = EPivotType::SelectionCenter;

	translationDelta = D3DXVECTOR3(0, 0, 0);
	rotationDelta = D3DXVECTOR3(0, 0, 0);
	scaleDelta = D3DXVECTOR3(0, 0, 0);

	precisionModeEnabled = false;

	geometry = new GeometryManager();
}

Gizmo::~Gizmo()
{
	selection.clear();

	SAFE_DELETE(geometry);
}

void Gizmo::Initialize()
{
	geometry->Initialize();
}

void Gizmo::Ready()
{
	geometry->Ready();
}

void Gizmo::Update()
{
	geometry->Update();

	// select entities with your cursor (add the desired keys for add-to / remove-from -selection)
	if (Mouse::Get()->Press(2))
	{
		SelectEntities(
			Keyboard::Get()->Down(VK_LCONTROL) || Keyboard::Get()->Down(VK_RCONTROL)
		);
	}

	// set the active mode like translate or rotate
	if (Keyboard::Get()->Down('1'))
	{
		activeMode = EGizmoMode::Translate;
		geometry->SetMode((UINT)activeMode);
	}
	if (Keyboard::Get()->Down('2'))
	{
		activeMode = EGizmoMode::Rotate;
		geometry->SetMode((UINT)activeMode);
	}
	if (Keyboard::Get()->Down('3'))
	{
		activeMode = EGizmoMode::NonUniformScale;
		geometry->SetMode((UINT)activeMode);
	}
	if (Keyboard::Get()->Down('4'))
	{
		activeMode = EGizmoMode::UniformScale;
		geometry->SetMode((UINT)activeMode);
	}


	// toggle precision mode
	if (Keyboard::Get()->Press(VK_LSHIFT) || Keyboard::Get()->Press(VK_RSHIFT))
		precisionModeEnabled = true;
	else
		precisionModeEnabled = false;

	// toggle active space
	if (Keyboard::Get()->Down('O'))
		activeSpace = activeSpace == ETransformSpace::Local ? ETransformSpace::World : ETransformSpace::Local;

	// select pivot types
	if (Keyboard::Get()->Down('P'))
		NextPivotType();

	// clear selection
	if (Keyboard::Get()->Down(VK_ESCAPE))
		selection.clear();

	if (isActive)
	{
		if (Mouse::Get()->Press(0) && activeAxis != EGizmoAxis::None)
		{
			switch (activeMode)
			{
			case Gizmo::EGizmoMode::UniformScale:
			case Gizmo::EGizmoMode::NonUniformScale:
			case Gizmo::EGizmoMode::Translate:
			{
#pragma region Translate & Scale

				D3DXVECTOR3 delta(0, 0, 0);

				switch (activeAxis)
				{
				case Gizmo::EGizmoAxis::XY:
				case Gizmo::EGizmoAxis::X:
					tDelta = 5.5f * Mouse::Get()->GetMoveValue() * Time::Delta();
					delta = (activeAxis == EGizmoAxis::X)
						? D3DXVECTOR3(tDelta.x, 0, 0)
						: D3DXVECTOR3(tDelta.x, -tDelta.y, 0);
					break;
				case Gizmo::EGizmoAxis::Z:
				case Gizmo::EGizmoAxis::YZ:
				case Gizmo::EGizmoAxis::Y:
					tDelta = 5.5f * Mouse::Get()->GetMoveValue() * Time::Delta();
					switch (activeAxis)
					{
					case Gizmo::EGizmoAxis::Y:
						delta = D3DXVECTOR3(0, -tDelta.y, 0);
						break;
					case Gizmo::EGizmoAxis::Z:
						delta = D3DXVECTOR3(0, 0, tDelta.x);
						break;
					default:
						delta = D3DXVECTOR3(0, -tDelta.y, tDelta.x);
						break;
					}
					break;
				case Gizmo::EGizmoAxis::ZX:
					tDelta = 5.5f * Mouse::Get()->GetMoveValue() * Time::Delta();
					delta = D3DXVECTOR3(tDelta.x, 0, -tDelta.y);
					break;
				}

#pragma endregion

				if (precisionModeEnabled)
					delta *= PRECISION_MODE_SCALE;

				if (activeMode == EGizmoMode::Translate)
				{
					// transform (local or world)
					D3DXVec3TransformCoord(&delta, &delta, &rotationMatrix);
					translationDelta = delta * 8;
				}
				else if (activeMode == EGizmoMode::NonUniformScale ||
					activeMode == EGizmoMode::UniformScale)
				{
					// -- Apply Scale -- //
					scaleDelta += delta;
				}
			}
			break;
			case Gizmo::EGizmoMode::Rotate:
#pragma region Rotate
			{
				float rDelta = 5.5f * Mouse::Get()->GetMoveValue().x * Time::Delta();

				if (precisionModeEnabled)
					rDelta *= PRECISION_MODE_SCALE;

				// rotation matrix to transform - if more than one objects selected, always use world-space.
				D3DXMATRIX R;
				switch (activeAxis)
				{
				case Gizmo::EGizmoAxis::X:
					rotationDelta = D3DXVECTOR3(rDelta, 0, 0);
					break;
				case Gizmo::EGizmoAxis::Y:
					rotationDelta = D3DXVECTOR3(0, rDelta, 0);
					break;
				case Gizmo::EGizmoAxis::Z:
					rotationDelta = D3DXVECTOR3(0, 0, rDelta);
					break;
				}
			}
#pragma endregion
			break;
			default:
				break;
			}
		}
		else
		{
			if ((Mouse::Get()->Press(0) && Mouse::Get()->Press(1)) == false)
				SelectAxis();
		}

		SetGizmoPosition();

		if (Mouse::Get()->Press(0))
		{
			D3DXVECTOR3 zero(0, 0, 0);
			D3DXMATRIX identity; D3DXMatrixIdentity(&identity);

			if (translationDelta != zero)
			{
				for (IGameObject * entity : selection)
				{
					D3DXVECTOR3 position;
					entity->Position(&position);

					entity->Position(position + translationDelta);
				}
				translationDelta = zero;
			}
			if (rotationDelta != zero)
			{
				for (IGameObject * entity : selection)
				{
					D3DXVECTOR3 rot;
					entity->Rotation(&rot);

					entity->Rotation(rot + rotationDelta);
				}

				rotationDelta = zero;
			}
			if (scaleDelta != zero)
			{
				for (IGameObject * entity : selection)
				{
					D3DXVECTOR3 scale;
					entity->Scale(&scale);


					if (activeMode == EGizmoMode::UniformScale)
						entity->Scale(scale * (1 + ((scaleDelta.x + scaleDelta.y + scaleDelta.z) / 3)));
					else
						entity->Scale(scale + scaleDelta);
				}
				scaleDelta = zero;
			}

			for (IGameObject * entity : selection)
			{
				//entity->Update();

				D3DXMATRIX world;
				entity->Matrix(&world);
				targetWorld = world;
			}

		}

		if (selection.size() < 1)
		{
			isActive = false;
			enabled = false;
			activeAxis = EGizmoAxis::None;
			return;
		}

		// helps solve visual lag (1-frame-lag) after selecting a new entity
		if (isActive == false)
			SetGizmoPosition();

		isActive = true;

		// -- Scale Gizmo to fit on-screen -- //
		D3DXVECTOR3 camPos;
		Context::Get()->GetMainCamera()->Position(&camPos);

		D3DXVECTOR3 vLength = camPos - position;
		float scaleFactor = 25;

		float length = D3DXVec3Length(&vLength);
		screenScale = length / scaleFactor;

		D3DXMATRIX scale;
		D3DXMatrixScaling(&scale, screenScale, screenScale, screenScale);

		screenScaleMatrix = scale;

		selection[0]->Forward(&localForward);
		selection[0]->Up(&localUp);
		// -- Vector Rotation (Local/World) -- //
		D3DXVec3Normalize(&localForward, &localForward);
		D3DXVec3Cross(&localRight, &localForward, &localUp);
		D3DXVec3Cross(&localUp, &localRight, &localForward);
		D3DXVec3Normalize(&localRight, &localRight);
		D3DXVec3Normalize(&localUp, &localUp);

		// -- Create Both World Matrices -- //
		D3DXMATRIX lookAt;
		D3DXMatrixLookAtLH(&lookAt, &position, &(position + localForward), &localUp);
		objectOrientedWorld = screenScaleMatrix * lookAt;
		D3DXMatrixLookAtLH(&lookAt, &position, &(position + D3DXVECTOR3(0, 0, 1)), &D3DXVECTOR3(0, 1, 0));
		axisAlignedWorld = screenScaleMatrix * lookAt;

		// Assign World
		if (activeSpace == ETransformSpace::World ||
			activeMode == EGizmoMode::Rotate ||
			activeMode == EGizmoMode::NonUniformScale ||
			activeMode == EGizmoMode::UniformScale)
		{
			gizmoWorld = axisAlignedWorld;

			// align lines, boxes etc. with the grid-lines
			D3DXVECTOR3 s, t; D3DXQUATERNION q;
			D3DXMatrixDecompose(&s, &q, &t, &sceneWorld);
			D3DXMatrixRotationQuaternion(&rotationMatrix, &q);
		}
		else
		{
			gizmoWorld = objectOrientedWorld;

			// align lines, boxes etc. with the selected object
			D3DXVECTOR3 s, t; D3DXQUATERNION q;
			D3DXMatrixDecompose(&s, &q, &t, &targetWorld);
			D3DXMatrixRotationQuaternion(&rotationMatrix, &q);
		}

		D3DXVECTOR3 s, t; D3DXQUATERNION q;
		D3DXMatrixDecompose(&s, &q, &t, &gizmoWorld);
		geometry->SetScale(s.x, s.y, s.z);
		geometry->SetLineLength(s.x * 5);
	}
}

void Gizmo::Render()
{
	if (isActive == false) return;

	geometry->Render();
}


void Gizmo::NextPivotType()
{
	if (activePivotNum == (UINT)EPivotType::WorldOrigin)
		activePivotNum = (UINT)EPivotType::ObjectCenter;
	else
		activePivotNum++;
}

void Gizmo::SetSelectionPool(class IGameObject *entity)
{
	selectionPool.push_back(entity);
}

void Gizmo::SelectEntities(bool addToSelection)
{
	if (activeAxis == EGizmoAxis::None)
	{
		if (!addToSelection)
			selection.clear();
		PickObject();
	}
	ResetDeltas();
}


void Gizmo::PickObject()
{
	IGameObject *obj = NULL;
	bool picked = false;

	for (size_t i = 0; i < selectionPool.size(); i++)
	{
		IGameObject * entity = selectionPool[i];

		if (entity->IsPicked())
		{
			bool b = false;
			for (IGameObject * find : selection)
			{
				if (find->Name() == entity->Name())
				{
					b = true;
					break;
				}
			}

			if (b == true) continue;

			obj = entity;
			picked = true;
		}
	}

	if (obj != NULL)
	{
		//obj->Update();
		selection.push_back(obj);
		obj->Matrix(&targetWorld);
		geometry->SetWorld(&targetWorld);

		enabled = true;
	}

	if (picked == false)
		enabled = false;

	if (selection.size() > 0)
		isActive = true;
}

void Gizmo::ResetDeltas()
{
	tDelta = D3DXVECTOR3(0, 0, 0);
}

void Gizmo::SelectAxis()
{
	if (enabled == false) return;

	UINT axis;
	geometry->ColorPickUpdate(&axis);
	activeAxis = (EGizmoAxis)axis;
}

void Gizmo::SetGizmoPosition()
{
	switch (activePivot)
	{
		case Gizmo::EPivotType::ObjectCenter:
			if (selection.size() > 0)
				selection[0]->Position(&position);
			break;
		case Gizmo::EPivotType::SelectionCenter:
		{
			if (selection.size() == 0)
				position = D3DXVECTOR3(0, 0, 0);

			D3DXVECTOR3 center(0, 0, 0);
			for (IGameObject * selected : selection)
			{
				D3DXVECTOR3 temp;
				selected->Position(&temp);
				center += temp;
			}

			center /= (float)selection.size();
			position = center;
		}
		break;
		case Gizmo::EPivotType::WorldOrigin:
			position = D3DXVECTOR3(sceneWorld._41, sceneWorld._42, sceneWorld._43);
			break;
	}

	position += translationDelta;
}
