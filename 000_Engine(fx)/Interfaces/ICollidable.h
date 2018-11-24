#pragma once

class ICollidable
{
public:
	//	obj1.Hits(obj2) : A �� B�� ġ��
	virtual void Hits(ICollidable &) = 0;
	//	���� �¾��� �� 
	virtual void Hits_Mine() = 0;
	//	�ݶ��̴��� �±� �̸� ���
	virtual wstring Tag(wstring type) = 0;
	//	�±׷� �ݶ��̴� ���
	virtual DebugLine * GetCollider(wstring name) = 0;
};