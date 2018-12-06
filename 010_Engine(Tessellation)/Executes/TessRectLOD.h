#pragma once
#include "Execute.h"
//	IA -> VS ----------> GS -> SO -> PS -> OM
//				��> HS -----> DS ->
//						��> Tessellation
//				HS : ��� �ڸ��� ����
//					�� Constant : ������ ���������� ��� ���������� ó��
//					�� Control Point : �������� ��ġ�� ������ �ִµ�����
//				DS : Bilinear ����, HS �ܰ踦 ��ģ �������� ������ ���� ��ȯ�� �Ѵ�

class TessRectLOD : public Execute
{
public:
	TessRectLOD();
	~TessRectLOD();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	Shader * shader;
	ID3D11Buffer * vertexBuffer;
};


