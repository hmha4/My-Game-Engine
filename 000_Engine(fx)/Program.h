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
	//	Execute를 상속받은 클래스를 넣어주면
	//	그 클래스가 실행되는 클래스가 된다.
	vector<class Execute *> executes;

	
};
