#pragma once
#include "Framework.h"

class IGameObject
{
public:
	virtual wstring & Name() = 0;
	virtual wstring & Tag() = 0;
	virtual void Save() = 0;
	virtual void Load(wstring fileName) = 0;
	virtual void Delete() = 0;
};