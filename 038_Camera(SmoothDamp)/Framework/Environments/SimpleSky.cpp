#include "Framework.h"
#include "SimpleSky.h"
#include "../Viewer/Camera.h"

SimpleSky::SimpleSky()
	: apex(0xFF9BCDFF), center(0xFF0080FF), height(4.5f)
{
	material = new Material(Effects + L"012_SimpleSky.fx");
	sphere = new MeshSphere(material, 0.5f, 20, 20);

	material->GetEffect()->AsVector("Apex")->SetFloatVector(apex);
	material->GetEffect()->AsVector("Center")->SetFloatVector(center);
	material->GetEffect()->AsScalar("Height")->SetFloat(height);
}

SimpleSky::~SimpleSky()
{
	SAFE_DELETE(sphere);
	SAFE_DELETE(material);
}

void SimpleSky::Update()
{
	D3DXVECTOR3 position;
	Context::Get()->GetMainCamera()->Position(&position);

	sphere->Position(position);
}

void SimpleSky::Render()
{
	sphere->Render();
}