#pragma once
#include "Execute.h"

//	<direct compute>
//	기본 rendering pipeline 과는 별개로 동작함
//	
//	cpu - single core
//	
//	<thread>
//	int unix : multi programming(병렬 프로그래밍)
//		->in window : multi tasking(단위 : exe)
//		->exe 사이에서는 메모리 공유가 안됨(서로 다른 메모리 구역을 가지고 있음
//		->exe 안에는 여러가지 process가 있음
//	
//		* 게임에는 많은 task가 돈다.(ai, network..)
//	
//	->해경방안 1 : IPC(내부 소켓(pipe line 통신), 메모리 맵(하드 공간 어딘가에 파일처럼 공
//					간을 만들고 메모리를 주고 받음)
//					* 모두 입 / 출력이 일어남(인터럽트가 발생함 : 처리속도가 느려짐)
//	->해결방안 2 : Thread(multi tasking단위를 thread로 쪼갬)
//					* 같은 exe내에서 thread가 동작하기 때문에 메모리 공유가 원할하게 일어남
//	
//	
//	Task - Game ->	Render		A
//				->	File I / O	B
//				->	AI			C
//				->	...
//	
//	A(0.1)->B(0.1)->C(0.1)->A(0.1)->B(0.1)->C(0.1)
//	
//	거의 동시에 실행되는 것 처럼 보이게 하기 위해서 단위를 쪼갠다.
//	아주 작은 단위로 쪼개면 사용자는 거의 동시에 실행되는 것 처럼 느낀다.
//	
//	누가 우선 순위가 높은지를 위한 기준을 정해야함
//	선점형 스케쥴링 : 우선 수위를 매김, 라운드 로딩,
//	비 선점형 스케쥴링 : 순서대로
//	
//	동기화 : 누가 변수를 접근하냐를 제어(mutex)

class TestComputeAdd : public Execute
{
public:
	TestComputeAdd();
	~TestComputeAdd();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	void ExecuteCS();

private:
	ID3D11ShaderResourceView *srv1;
	ID3D11ShaderResourceView *srv2;
	ID3D11UnorderedAccessView *uav;	//	데이터를 받아올 자료형

	ID3D11Buffer *outputBuffer;
	ID3D11Buffer *outputDebugBuffer;

private:
	UINT dataSize;
	Shader *shader;

private:
	struct Data
	{
		D3DXVECTOR3 V1;
		D3DXVECTOR2 V2;
	};
};


