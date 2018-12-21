#pragma once
#include "Framework.h"
#include "../Draw/ILine.h"

struct BBox : public ILine
{
	BBox()
	{
		Min = Max = D3DXVECTOR3(0, 0, 0);
	}
	
	BBox(D3DXVECTOR3& min, D3DXVECTOR3& max)
		: Min(min), Max(max)
	{
		
	}

	bool Intersect(Ray* ray, OUT float& result)
	{
		result = 0.0f;
		float minValue = 0.0f, maxValue = FLT_MAX;

		//Check X
		if (fabsf(ray->Direction.x) >= 1e-6f)
		{
			float value = 1.0f / ray->Direction.x;
			float minX = (Min.x - ray->Position.x) * value;
			float maxX = (Max.x - ray->Position.x) * value;

			if (minX > maxX)
			{
				float temp = minX;
				minX = maxX;
				maxX = temp;
			}

			minValue = max(minX, minValue);
			maxValue = min(maxX, maxValue);

			if (minValue > maxValue)
				return false;
		}
		else if (ray->Position.x < Min.x || ray->Position.x > Max.x)
			return false;

		//Check Y
		if (fabsf(ray->Direction.y) >= 1e-6f)
		{
			float value = 1.0f / ray->Direction.y;
			float minY = (Min.y - ray->Position.y) * value;
			float maxY = (Max.y - ray->Position.y) * value;

			if (minY > maxY)
			{
				float temp = minY;
				minY = maxY;
				maxY = temp;
			}

			minValue = max(minY, minValue);
			maxValue = min(maxY, maxValue);

			if (minValue > maxValue)
				return false;
		}
		else if (ray->Position.y < Min.y || ray->Position.y > Max.y)
			return false;


		//Check Z
		if (fabsf(ray->Direction.z) >= 1e-6f)
		{
			float value = 1.0f / ray->Direction.z;
			float minZ = (Min.z - ray->Position.z) * value;
			float maxZ = (Max.z - ray->Position.z) * value;

			if (minZ > maxZ)
			{
				float temp = minZ;
				minZ = maxZ;
				maxZ = temp;
			}

			minValue = max(minZ, minValue);
			maxValue = min(maxZ, maxValue);

			if (minValue > maxValue)
				return false;
		}
		else if (ray->Position.z < Min.z || ray->Position.z > Max.z)
			return false;

		result = minValue;
		return true;
	}

	void GetLine(D3DXMATRIX& world, vector<D3DXVECTOR3>& lines)
	{
		D3DXVECTOR3 tempMin;
		D3DXVECTOR3 tempMax;

		D3DXVec3TransformCoord(&tempMin, &Min, &world);
		D3DXVec3TransformCoord(&tempMax, &Max, &world);


		//Front
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMin.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMin.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMax.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMax.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMin.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMax.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMin.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMax.y, tempMin.z));

		//Back
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMin.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMin.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMax.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMax.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMin.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMax.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMin.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMax.y, tempMax.z));

		//Middle
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMin.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMin.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMin.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMin.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMax.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMin.x, tempMax.y, tempMax.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMax.y, tempMin.z));
		lines.push_back(D3DXVECTOR3(tempMax.x, tempMax.y, tempMax.z));
	}

public:
	D3DXVECTOR3 Min;
	D3DXVECTOR3 Max;
};