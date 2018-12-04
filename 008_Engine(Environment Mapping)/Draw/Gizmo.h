#pragma once

/// <summary>
/// The value to adjust all transformation when precisionMode is active.
/// </summary>
#define PRECISION_MODE_SCALE 0.1f

class Gizmo
{
private:
	enum class EGizmoAxis;
	enum class EGizmoMode;
	enum class ETransformSpace;
	enum class EPivotType;

	struct Quad;
public:
	Gizmo(D3DXMATRIX world);
	~Gizmo();

	void Update();
	void Render();

	void SetSelectionPool(class GameModel *entity);

private:
	void NextPivotType();

	void SelectEntities(bool addToSelection);
	void PickObject();
	void ResetDeltas();

	/// <summary>
	/// Per-frame check to see if mouse is hovering over any axis.
	/// </summary>
	void SelectAxis();
	void SetGizmoPosition();

private:
	/// <summary>
	/// 하나라도 선택 됐을 때만 active
	/// </summary> 
	bool isActive;

	/// <summary>
	/// Enabled if gizmo should be able to select objects and axis.
	/// </summary>
	bool enabled;

	// -- Screen Scale -- //
	D3DXMATRIX	screenScaleMatrix;
	float		screenScale;

	// -- Position - Rotation -- //
	D3DXVECTOR3 position;
	D3DXMATRIX	rotationMatrix;

	D3DXVECTOR3 localForward;
	D3DXVECTOR3 localUp;
	D3DXVECTOR3 localRight;

	// -- Matrices -- //
	D3DXMATRIX	objectOrientedWorld;
	D3DXMATRIX	axisAlignedWorld;

	// used for all drawing, assigned by local- or world-space matrices
	D3DXMATRIX gizmoWorld;

	// the matrix used to apply to your whole scene, usually matrix.identity (default scale, origin on 0,0,0 etc.)
	D3DXMATRIX	sceneWorld;

	// -- Modes & Selections -- //
	EGizmoAxis		activeAxis;
	EGizmoMode		activeMode;
	ETransformSpace activeSpace;
	EPivotType		activePivot;
	UINT			activePivotNum;

	// -- Selection -- //
	vector<class GameModel *> selection;
	vector<class GameModel *> selectionPool;

	// -- Transform Deltas -- //
	D3DXVECTOR3 translationDelta;
	D3DXVECTOR3 rotationDelta;
	D3DXVECTOR3 scaleDelta;

	// -- Translation Variables -- //
	D3DXVECTOR3 tDelta;

	bool  precisionModeEnabled;

	//	-- Gizmo Geometry -- //
	D3DXMATRIX targetWorld;
	class GeometryManager *geometry;
private:
#pragma region Gizmo Enums

	enum class EGizmoAxis
	{
		X,
		Y,
		Z,
		XY,
		ZX,
		YZ,
		None
	};

	enum class EGizmoMode
	{
		Translate,
		Rotate,
		NonUniformScale,
		UniformScale
	};

	enum class ETransformSpace
	{
		Local,
		World
	};

	enum class EPivotType
	{
		ObjectCenter = 0,
		SelectionCenter,
		WorldOrigin
	};

#pragma endregion
};