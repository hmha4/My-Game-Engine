#pragma once

class Program
{
public:
	Program();
	~Program();

	void Update();

	void PreRender();
	void Render();
	void PostRender();

	void ResizeScreen();

private:
	//	Execute�� ��ӹ��� Ŭ������ �־��ָ�
	//	�� Ŭ������ ����Ǵ� Ŭ������ �ȴ�.
	vector<class Execute *> executes;

	
};
