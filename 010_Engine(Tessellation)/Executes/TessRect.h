#pragma once
#include "Execute.h"
//	IA -> VS ----------> GS -> SO -> PS -> OM
//				ㄴ> HS -----> DS ->
//						ㄴ> Tessellation
//				HS : 어떻게 자르는 구간
//					ㄴ Constant : 정점을 제어점으로 어떻게 나눌것인지 처리
//					ㄴ Control Point : 제어점의 위치만 가지고 있는데이터
//				DS : Bilinear 보간, HS 단계를 거친 정보들을 가지고 동차 변환을 한다

class TessRect : public Execute
{
public:
	TessRect();
	~TessRect();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	Shader * shader;
	ID3D11Buffer * vertexBuffer;
};


