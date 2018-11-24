#pragma once

class ICollidable
{
public:
	//	obj1.Hits(obj2) : A 가 B를 치다
	virtual void Hits(ICollidable &) = 0;
	//	내가 맞았을 때 
	virtual void Hits_Mine() = 0;
	//	콜라이더의 태그 이름 얻기
	virtual wstring Tag(wstring type) = 0;
	//	태그로 콜라이더 얻기
	virtual DebugLine * GetCollider(wstring name) = 0;
};