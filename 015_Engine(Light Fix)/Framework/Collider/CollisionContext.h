#pragma once


class ColliderElement;
// <summary>
// result report of collision
// </summary>
class CollisionResult
{
public:
	friend class CollisionContext;

public:
	enum class ResultType
	{
		// <summary>
		// To find the nearest collision from itself.
		// </summary>
		ENearestOne = 0,
	};

	CollisionResult();
	~CollisionResult();

	void CopyTo(CollisionResult * target);
	void Clear();

	UINT CollideCount() { return collideCount; }

	void Distance(float val) { distance = val; }
	float Distance() { return distance; }

private:
	// <summary>
	// Distance between detected collision
	// </summary>
	float distance;
	// <summary>
	// Detection object count
	// </summary>
	UINT collideCount;
	// <summary>
	// Detected object element
	// </summary>
	ColliderElement * detectedCollider;
	// <summary>
	// intersect point
	// </summary>
	D3DXVECTOR3 intersect;
	// <summary>
	// intersect normal
	// </summary>
	D3DXVECTOR3 normal;
};

// <summary>
// This layer groups collision elements that need to be processed 
// for the collision collectively.
// </summary>
//#include "INamed.h"
class CollisionLayer// : public INamed
{
public:
	CollisionLayer();
	~CollisionLayer();

	/// <summary>
	///	Add a collsion element.
	/// </summary>
	void AddCollider(ColliderElement * collider);
	/// <summary>
	/// Get collision element using the index.
	/// </summary>
	ColliderElement * GetCollider(UINT index);
	/// <summary>
	/// Find a collision element using the name.
	/// </summary>
	ColliderElement * FindCollider(wstring name);
	/// <summary>
	/// it checks whether the collision element which has been included.
	/// </summary>
	bool IsContain(ColliderElement * collider);
	/// <summary>
	/// Remove the collision element.
	/// </summary>
	void RemoveCollider(ColliderElement * collider);
	/// <summary>
	/// Remove a collision element using the index.
	/// </summary>
	void RemoveCollider(UINT index);
	/// <summary>
	/// Remove all collision elements.
	/// </summary>
	void RemoveAll();

	UINT ColliderCount() { return colliderContainer.size(); }
private:
	wstring name;

	vector<ColliderElement *> colliderContainer;

public:
	// INamed을(를) 통해 상속됨
	void Name(wstring val);// override;
	wstring Name();// override;
};


// ------------------------------------------------------------------------- //
//	It tests for collision again the registered collision elements.
//	When you request CollisionContext a collision test with the source 
//	CollideElement, a result from a collision test would be returned with 
//	all CollideElements that have been registered to the specific collision
//	layer as the target.
//	It supports the following collision types:  ray, model, box, and sphere.
// ------------------------------------------------------------------------- //
class CollisionContext
{
public:
	CollisionContext();
	~CollisionContext();

	static void Create();
	static void Delete();
	static CollisionContext * Get();
public:
	size_t LayerCount() { return colliderLayerContainer.size(); }
	UINT TotalCollidingCount() { return totalCollidingCount; }

	/// <summary>
	/// Creates a new collision layer using the name.
	/// </summary>
	/// <param name="name">The layer name</param>
	CollisionLayer * AddLayer(wstring name);
	/// <summary>
	/// Get a collison layer using the ID number.
	/// </summary>
	/// <param name="id">ID number</param>
	CollisionLayer * GetLayer(UINT id);
	/// <summary>
	/// Get a collison layer using the name.
	/// </summary>
	/// <param name="layerName">The layer name</param>
	CollisionLayer * GetLayer(wstring layerName);
	/// <summary>
	/// Remove all collsion layers.
	/// </summary>
	void ClearAllLayer() { colliderLayerContainer.clear(); }

private:
	// --------------------------------------------------------------------- //
	//	Collision Detection
	// --------------------------------------------------------------------- //
	/// <summary>
	/// It checks for two collision boxes.
	/// </summary>
	/// <param name="sourceCollide">Source collision box</param>
	/// <param name="targetCollide">Target collision box</param>
	/// <param name="result">A result report</param>
	/// <returns>True if there is a collision</returns>
	bool TestBoxintersectBox(class ColliderBox * srcCollider, class ColliderBox * dstCollider, OUT CollisionResult * result);
	/// <summary>
	/// It checks for the collision between two collision spheres.
	/// </summary>
	/// <param name="sourceCollide">Source collision sphere</param>
	/// <param name="targetCollide">Target collision sphere</param>
	/// <param name="result">A result report</param>
	/// <returns>True if there is a collision</returns>
	bool TestSphereintersectSphere(class ColliderSphere * srcCollider, class ColliderSphere * dstCollider, OUT CollisionResult * result);
	/// <summary>
	/// It checks for the collision between a collision sphere and a collision box.
	/// </summary>
	/// <param name="sourceCollide">Source collision ray</param>
	/// <param name="targetCollide">Target collision box</param>
	/// <param name="result">A result report</param>
	/// <returns>True if there is a collision</returns>
	bool TestSphereintersectBox(class ColliderSphere * srcCollider, class ColliderBox * dstCollider, OUT CollisionResult * result);
	/// <summary>
	/// It checks for the collision between a collision ray and a collision sphere.
	/// </summary>
	/// <param name="sourceCollide">Source collision ray</param>
	/// <param name="targetCollide">Target collision sphere</param>
	/// <param name="result">A result report</param>
	/// <returns>True if there is a collision</returns>
	bool TestRayintersectSphere(class ColliderRay * srcCollider, class ColliderSphere * dstCollider, OUT CollisionResult * result);
	/// <summary>
	/// It checks for the collision between a collision ray and a collision box.
	/// </summary>
	/// <param name="sourceCollide">Source collision ray</param>
	/// <param name="targetCollide">Target collision box</param>
	/// <param name="result">A result report</param>
	/// <returns>True if there is a collision</returns>
	bool TestRayintersectBox(class ColliderRay * srcCollider, class ColliderBox * dstCollider, OUT CollisionResult * result);


public:
	/// <summary>
	/// It tests for collision among the collision elements which 
	/// have been registered to the collision layer and returns the result.
	/// </summary>
	/// <param name="collide">Source collsion element</param>
	/// <param name="idLayer">Destination collison layer ID number</param>
	/// <param name="resultType">type of result</param>
	/// <returns>A result report</returns>
	CollisionResult * HitTest(ColliderElement * collider, wstring name, CollisionResult::ResultType resultType);
	CollisionResult * HitTest(ColliderElement * collider, CollisionLayer * targetLayer, CollisionResult::ResultType resultType);

private:
	static CollisionContext * instance;

	bool isActive;

	vector<CollisionLayer *> colliderLayerContainer;
	CollisionResult * tempResult;

	UINT totalCollidingCount;
};