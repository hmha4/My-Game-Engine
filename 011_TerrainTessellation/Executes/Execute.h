#pragma once

//	추상 클래스 : 하나라도 순수 가상함수가 있기 때문에
//	어떤 기능이 포함 될지는 알지만 지금 상황에서 군현 할 수 없을 때 사용
//	이름 통일(오버라이딩)을 위해 사용
//	추상 클래스로 만들어 놓은 객체를 업캐스팅을 한 다음 넘겨주면 다운 캐스팅을 언제든지 사용 가능

class Execute
{
public:
	Execute()
	{

	}

	//	가상 소멸자를 사용하지 않으면 호출 안된다.
	//	상속을 시키기 때문에
	//	하지만 여기서는 사용하지 않아도된다.
	//	=> 이유는. 추상 클래스는 virtual로 쓰지 않아도 자동으로 이루어진다.
	virtual ~Execute() {}

	virtual void Update() = 0;

	virtual void PreRender() = 0;		//	전처리 렌더
	virtual void Render() = 0;			//	일반적인 렌더
	virtual void PostRender() = 0;		//	모든 렌더링 후에 렌더링(UI)

	virtual void ResizeScreen() = 0;

};