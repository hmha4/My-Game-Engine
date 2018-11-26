#pragma once
#include "Execute.h"

//	<direct compute>
//	�⺻ rendering pipeline ���� ������ ������
//	
//	cpu - single core
//	
//	<thread>
//	int unix : multi programming(���� ���α׷���)
//		->in window : multi tasking(���� : exe)
//		->exe ���̿����� �޸� ������ �ȵ�(���� �ٸ� �޸� ������ ������ ����
//		->exe �ȿ��� �������� process�� ����
//	
//		* ���ӿ��� ���� task�� ����.(ai, network..)
//	
//	->�ذ��� 1 : IPC(���� ����(pipe line ���), �޸� ��(�ϵ� ���� ��򰡿� ����ó�� ��
//					���� ����� �޸𸮸� �ְ� ����)
//					* ��� �� / ����� �Ͼ(���ͷ�Ʈ�� �߻��� : ó���ӵ��� ������)
//	->�ذ��� 2 : Thread(multi tasking������ thread�� �ɰ�)
//					* ���� exe������ thread�� �����ϱ� ������ �޸� ������ �����ϰ� �Ͼ
//	
//	
//	Task - Game ->	Render		A
//				->	File I / O	B
//				->	AI			C
//				->	...
//	
//	A(0.1)->B(0.1)->C(0.1)->A(0.1)->B(0.1)->C(0.1)
//	
//	���� ���ÿ� ����Ǵ� �� ó�� ���̰� �ϱ� ���ؼ� ������ �ɰ���.
//	���� ���� ������ �ɰ��� ����ڴ� ���� ���ÿ� ����Ǵ� �� ó�� ������.
//	
//	���� �켱 ������ �������� ���� ������ ���ؾ���
//	������ �����층 : �켱 ������ �ű�, ���� �ε�,
//	�� ������ �����층 : �������
//	
//	����ȭ : ���� ������ �����ϳĸ� ����(mutex)

class TestComputeAppend : public Execute
{
public:
	TestComputeAppend();
	~TestComputeAppend();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	void ExecuteCS();

private:
	ID3D11ShaderResourceView *consume;
	ID3D11UnorderedAccessView *append;	//	�����͸� �޾ƿ� �ڷ���

	ID3D11Buffer *outputBuffer;
	ID3D11Buffer *outputDebugBuffer;

private:
	UINT dataSize;
	Shader *shader;

	float timeStep;

private:
	struct Particle
	{
		D3DXVECTOR3 Position;
		D3DXVECTOR3 Velocity;
		D3DXVECTOR3 Acceleration;
	};
};

