#include "Framework.h"
#include "CollisionContext.h"
#include "ColliderElement.h"

// ------------------------------------------------------------------------- //
//	CollisionLayer
// ------------------------------------------------------------------------- //
CollisionLayer::CollisionLayer()
	: name(L"")
{
}

CollisionLayer::~CollisionLayer()
{
}

void CollisionLayer::AddCollider(ColliderElement * collider)
{
	for (ColliderElement * col : colliderContainer)
	{
		if (col->Name() == collider->Name())
			return;
	}

	colliderContainer.push_back(collider);
	collider->ParentLayer(this);
}

ColliderElement * CollisionLayer::GetCollider(UINT index)
{
	return colliderContainer[index];
}

ColliderElement * CollisionLayer::FindCollider(wstring name)
{
	for (ColliderElement * col : colliderContainer)
	{
		if (col->Name() == name)
			return col;
	}

	return nullptr;
}

bool CollisionLayer::IsContain(ColliderElement * collider)
{
	for (ColliderElement * col : colliderContainer)
	{
		if (col->Name() == collider->Name())
			return true;
	}

	return false;
}

void CollisionLayer::RemoveCollider(ColliderElement * collider)
{
	for (size_t i = 0; i < colliderContainer.size();)
	{
		ColliderElement * col = colliderContainer[i];

		if (col->Name() == collider->Name())
			colliderContainer.erase(colliderContainer.begin() + i);
		else
			i++;
	}
}

void CollisionLayer::RemoveCollider(UINT index)
{
	colliderContainer.erase(colliderContainer.begin() + index);
}

void CollisionLayer::RemoveAll()
{
	colliderContainer.clear();
}

void CollisionLayer::Name(wstring val)
{
	name = val;
}

wstring CollisionLayer::Name()
{
	return name;
}

// ------------------------------------------------------------------------- //
//	CollisionContext
// ------------------------------------------------------------------------- //
CollisionContext * CollisionContext::instance = NULL;

CollisionContext::CollisionContext()
{
	tempResult = new CollisionResult();
}

CollisionContext::~CollisionContext()
{
	for (CollisionLayer * layer : colliderLayerContainer)
		SAFE_DELETE(layer);

	SAFE_DELETE(tempResult);
}

void CollisionContext::Create()
{
	if (instance != NULL)
		assert("instance already exists");

	instance = new CollisionContext();
}

void CollisionContext::Delete()
{
	if (instance == NULL)
		assert("instance does not exists");

	SAFE_DELETE(instance);
}

CollisionContext * CollisionContext::Get()
{
	return instance;
}

CollisionLayer * CollisionContext::AddLayer(wstring name)
{
	CollisionLayer * newLayer = new CollisionLayer();
	newLayer->Name(name);

	colliderLayerContainer.push_back(newLayer);

	return newLayer;
}

CollisionLayer * CollisionContext::GetLayer(UINT id)
{
	return nullptr;
}

CollisionLayer * CollisionContext::GetLayer(wstring layerName)
{
	for (CollisionLayer * layer : colliderLayerContainer)
	{
		if (layer->Name() == layerName)
			return layer;
	}

	return nullptr;
}

#include "Collider/ColliderSphere.h"
#include "Collider/ColliderBox.h"
#include "Collider/ColliderRay.h"
bool CollisionContext::TestBoxintersectBox(ColliderBox * srcCollider, ColliderBox * dstCollider, OUT CollisionResult * result)
{
	totalCollidingCount++;

	BBox * src = srcCollider->BoundingBox();
	BBox * dst = dstCollider->BoundingBox();

	if (src->Intersect(dst))
	{
		if (result != NULL)
		{
			D3DXVECTOR3 srcCenter = (src->Min + src->Max) * 0.5f;
			D3DXVECTOR3 dstCenter = (dst->Min + dst->Max) * 0.5f;

			float distance = Math::Distance(srcCenter, dstCenter);

			result->distance = distance;
			result->detectedCollider = dstCollider;
			result->collideCount++;
		}

		return true;
	}

	return false;
}
bool CollisionContext::TestSphereintersectSphere(ColliderSphere * srcCollider, ColliderSphere * dstCollider, OUT CollisionResult * result)
{
	totalCollidingCount++;

	BSphere * src = srcCollider->BoundingSphere();
	BSphere * dst = dstCollider->BoundingSphere();

	if (dstCollider->IsActive() == false)
		return false;

	if (src->Intersect(dst))
	{
		if (result != NULL)
		{
			float distance = Math::Distance(src->Center, dst->Center);

			D3DXVECTOR3 direction = dst->Center - src->Center;
			D3DXVec3Normalize(&direction, &direction);

			result->distance = distance;
			result->intersect = direction * result->distance;
			result->collideCount++;
			result->detectedCollider = dstCollider;
		}

		return true;
	}

	return false;
}

bool CollisionContext::TestSphereintersectBox(ColliderSphere * srcCollider, ColliderBox * dstCollider, OUT CollisionResult * result)
{
	totalCollidingCount++;

	BSphere * src = srcCollider->BoundingSphere();
	BBox * dst = dstCollider->BoundingBox();

	if (src->Intersect(dst))
	{
		if (result != NULL)
		{
			D3DXVECTOR3 center = (dst->Min + dst->Max) * 0.5f;
			float distance = Math::Distance(src->Center, center) - src->Radius;

			result->distance = distance;
			result->detectedCollider = dstCollider;
			result->collideCount++;
		}
	}
	return false;
}
bool CollisionContext::TestRayintersectSphere(ColliderRay * srcCollider, ColliderSphere * dstCollider, OUT CollisionResult * result)
{
	totalCollidingCount++;

	Ray * src = srcCollider->GetRay();
	BSphere * dst = dstCollider->BoundingSphere();

	float distance;
	if (dst->Intersect(src, distance))
	{
		if (result != NULL)
		{
			D3DXVECTOR3 direction = src->Position - dst->Center;
			D3DXVec3Normalize(&direction, &direction);

			D3DXVECTOR3 length = direction * dst->Radius;

			result->distance = distance;
			result->detectedCollider = dstCollider;
			result->intersect = dst->Center + length;
			result->collideCount++;
		}

		return true;
	}

	return false;
}

bool CollisionContext::TestRayintersectBox(ColliderRay * srcCollider, ColliderBox * dstCollider, OUT CollisionResult * result)
{
	totalCollidingCount++;

	Ray * src = srcCollider->GetRay();
	BBox * dst = dstCollider->BoundingBox();

	float distance;
	if (dst->Intersect(src, distance))
	{
		if (result != NULL)
		{
			result->distance = distance;
			result->detectedCollider = dstCollider;
			result->collideCount++;
		}

		return true;
	}

	return false;
}

CollisionResult * CollisionContext::HitTest(ColliderElement * collider, wstring name, CollisionResult::ResultType resultType)
{
	CollisionLayer * layer = GetLayer(name);

	return HitTest(collider, layer, resultType);
}

CollisionResult * CollisionContext::HitTest(ColliderElement * collider, CollisionLayer * targetLayer, CollisionResult::ResultType resultType)
{
	CollisionResult * result = NULL;
	tempResult->Clear();
	totalCollidingCount = 0;

	if (isActive == false)
		return nullptr;
	if (collider == NULL)
		assert("No collider");
	if (targetLayer == NULL)
		assert("No targetLayer");

	//  checking all collisions in current collision layer
	for (UINT i = 0; i < targetLayer->ColliderCount(); i++)
	{
		ColliderElement * targetCollider = targetLayer->GetCollider(i);

		//  Skip ifself
		if (collider->Name() == targetCollider->Name())
			continue;

		//   If source collision is BoundingSphere
		if (collider->Type() == (UINT)ColliderElement::EType::ESphere)
		{
			//  Test with target sphere
			if (targetCollider->Type() == (UINT)ColliderElement::EType::ESphere)
			{
				//	Sphere to sphere collision
				ColliderSphere * src = dynamic_cast<ColliderSphere*>(collider);
				ColliderSphere * dst = dynamic_cast<ColliderSphere*>(targetCollider);

				if (src == NULL || dst == NULL)
					assert("dynamic casting error");

				TestSphereintersectSphere(src, dst, tempResult);
			}
			//  Test with target box
			else if (targetCollider->Type() == (UINT)ColliderElement::EType::EBox)
			{
				//	Sphere to Box Collision
				ColliderSphere * src = dynamic_cast<ColliderSphere*>(collider);
				ColliderBox * dst = dynamic_cast<ColliderBox*>(targetCollider);

				if (src == NULL || dst == NULL)
					assert("dynamic casting error");

				TestSphereintersectBox(src, dst, tempResult);
			}

			//  Test with target ray
			if (targetCollider->Type() == (UINT)ColliderElement::EType::ERay)
			{
				//	Ray to Sphere
				ColliderRay * src = dynamic_cast<ColliderRay*>(targetCollider);
				ColliderSphere * dst = dynamic_cast<ColliderSphere*>(collider);

				if (src == NULL || dst == NULL)
					assert("dynamic casting error");

				TestRayintersectSphere(src, dst, tempResult);
			}
		}
		//   If source collision is Ray
		else if (collider->Type() == (UINT)ColliderElement::EType::ERay)
		{
			//  Test with target sphere
			if (targetCollider->Type() == (UINT)ColliderElement::EType::ESphere)
			{
				//	Ray to sphere collision
				ColliderRay * src = dynamic_cast<ColliderRay*>(collider);
				ColliderSphere * dst = dynamic_cast<ColliderSphere*>(targetCollider);

				if (src == NULL || dst == NULL)
					assert("dynamic casting error");

				TestRayintersectSphere(src, dst, tempResult);
			}
			//  Test with target box
			else if (targetCollider->Type() == (UINT)ColliderElement::EType::EBox)
			{
				//	Ray to Box Collision
				ColliderRay * src = dynamic_cast<ColliderRay*>(collider);
				ColliderBox * dst = dynamic_cast<ColliderBox*>(targetCollider);

				if (src == NULL || dst == NULL)
					assert("dynamic casting error");

				TestRayintersectBox(src, dst, tempResult);
			}
		}
		//	 if source collision is Box
		else if (collider->Type() == (UINT)ColliderElement::EType::EBox)
		{
			//  Test with target sphere
			if (targetCollider->Type() == (UINT)ColliderElement::EType::ESphere)
			{
				//	Box to sphere collision
				ColliderBox * src = dynamic_cast<ColliderBox*>(collider);
				ColliderSphere * dst = dynamic_cast<ColliderSphere*>(targetCollider);

				if (src == NULL || dst == NULL)
					assert("dynamic casting error");

				TestSphereintersectBox(dst, src, tempResult);
			}
			//  Test with target box
			else if (targetCollider->Type() == (UINT)ColliderElement::EType::EBox)
			{
				//	Box to Box Collision
				ColliderBox * src = dynamic_cast<ColliderBox*>(collider);
				ColliderBox * dst = dynamic_cast<ColliderBox*>(targetCollider);

				if (src == NULL || dst == NULL)
					assert("dynamic casting error");

				TestBoxintersectBox(src, dst, tempResult);
			}
			//  Test with target ray
			else if (targetCollider->Type() == (UINT)ColliderElement::EType::ERay)
			{
				//	Ray to Box
				ColliderRay * src = dynamic_cast<ColliderRay*>(targetCollider);
				ColliderBox * dst = dynamic_cast<ColliderBox*>(collider);

				if (src == NULL || dst == NULL)
					assert("dynamic casting error");

				TestRayintersectBox(src, dst, tempResult);
			}
		}

		//  To find the nearest detected collision.
		if (resultType == CollisionResult::ResultType::ENearestOne)
		{
			if (tempResult->collideCount > 0)
			{
				float distance = FLT_MAX;

				if (distance > tempResult->distance)
					result = tempResult;
			}
		}
	}

	return result;
}

void CollisionContext::Render()
{
	for (CollisionLayer * layer : colliderLayerContainer)
	{
		for (ColliderElement * collider : layer->Colliders())
			collider->Render();
	}
}

// ------------------------------------------------------------------------- //
//	CollisionResult
// ------------------------------------------------------------------------- //
CollisionResult::CollisionResult()
	: distance(0.0f), collideCount(0)
	, detectedCollider(NULL)
{
	intersect = D3DXVECTOR3(0, 0, 0);
	normal = D3DXVECTOR3(0, 0, 0);
}

CollisionResult::~CollisionResult()
{
}

void CollisionResult::CopyTo(CollisionResult * target)
{
	target->distance = this->distance;
	target->collideCount = this->collideCount;
	target->detectedCollider = this->detectedCollider;
	target->intersect = this->intersect;
	target->normal = this->normal;
}

void CollisionResult::Clear()
{
	distance = 0.0f;
	collideCount = 0;
	detectedCollider = NULL;
	intersect = D3DXVECTOR3(0, 0, 0);
	normal = D3DXVECTOR3(0, 0, 0);
}