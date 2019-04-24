#pragma once
#include "Systems/IExecute.h"

class Main : public IExecute
{
public:
	//<summary>
	//	��ü ������
	//</summary>
	void Initialize() override;
	//<summary>
	//	���� ������(Update������ ��)
	//</summary>
	void Ready() override;
	//<summary>
	//	��ü �� ���� ����
	//</summary>
	void Destroy() override;

	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	void ResizeScreen() override;

private:
	void Push(IExecute * execute);

private:
	vector<IExecute *> executes;
};