#pragma once
#include "Systems/IExecute.h"

class Main : public IExecute
{
public:
	//<summary>
	//	객체 생성용
	//</summary>
	void Initialize() override;
	//<summary>
	//	버퍼 생성용(Update직전에 콜)
	//</summary>
	void Ready() override;
	//<summary>
	//	객체 및 버퍼 삭제
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