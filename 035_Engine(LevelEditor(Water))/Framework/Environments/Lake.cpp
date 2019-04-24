#include "Framework.h"
#include "Lake.h"
#include "Terrain.h"
#include "Flower.h"
#include "Billboard.h"
#include "ScatterSky.h"
#include "Viewer/Frustum.h"

Lake::Lake(ScatterSky * sky, Terrain * terrain)
	: sky(sky), terrain(terrain)
{
	D3DXMatrixIdentity(&world);
	D3DXMatrixIdentity(&reflectionView);
}

Lake::~Lake()
{
	SAFE_DELETE(effect);

	SAFE_DELETE(reflectionRTV);
	SAFE_DELETE(refractionRTV);

	SAFE_DELETE(waterBumpMap);

	SAFE_RELEASE(vertexBuffer);
}

void Lake::Initialize()
{
	waterLevel = 1.0f;
	waveHeight = 0.04f;
	waterSpeed = 0.01f;

	specularLightPowerValue = 50;
	specularLightPerturbationValue = 4;	

	fresnelMode = 1;
	fresnelCount = 3;

	reflectionRefractionRatio = 1.0f;

	xdullBlendFactor = 0.8f;

	for (int i = 0; i < 2; i++)
		renderElements[i] = true;
}

void Lake::Ready()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);
	refractionRTV = new RenderTarget((UINT)desc.Width / 2, (UINT)desc.Height / 2);
	reflectionRTV = new RenderTarget((UINT)desc.Width / 2, (UINT)desc.Height / 2);

	waterBumpMap = new Texture(Textures + L"waterbump.dds");

	effect = new Effect(Effects + L"037_LakeWater.fx");
	InitEffectVariables();
	SetFirstEffectVariables();

	CreateVertices();
}

void Lake::Update()
{
	UpdateReflectionCam();
}

void Lake::PreRender()
{
	if (renderElements[0] == true)
		UpdateReflectionMap();

	if (renderElements[1] == true)
		UpdateRefractionMap();
}

void Lake::Render()
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	worldVar->SetMatrix(world);

	effect->Draw(0, 0, 6);
}

void Lake::ImGuiRender()
{
	ImGui::Begin("Water", 0, NULL);
	{
		ImGui::DragFloat("Speed", &waterSpeed, 0.01f, 1.728f, 10.0f);
		ImGui::DragFloat("Fresnel Term", &reflectionRefractionRatio, 0.01f, 0, 10);
		ImGui::DragFloat("Wave Height", &waveHeight, 0.01f, 0.1f, 4.0f);
		ImGui::SliderFloat("Light Perturbation", &specularLightPerturbationValue, 0, 10);
		ImGui::DragFloat("Dull Blend", &xdullBlendFactor, 0.01f, 0, 2.0f);
		ImGui::SliderFloat("Light Power", &specularLightPowerValue, 0, 10);
		ImGui::Checkbox("Reflection", &renderElements[0]);
		ImGui::Checkbox("Refraction", &renderElements[1]);
		ImGui::SliderInt("Fresnel Mode", &fresnelMode, 0, fresnelCount - 1);

		SetFirstEffectVariables();
	}
	ImGui::End();
}

void Lake::CreateVertices()
{
	VertexTexture * vertices = new VertexTexture[6];

	float width = terrain->Width() * 0.5f;
	float depth = terrain->Depth() * 0.5f;

	vertices[0].Position = D3DXVECTOR3(-width, waterLevel, -depth);
	vertices[0].Uv = D3DXVECTOR2(0, 1);
	vertices[1].Position = D3DXVECTOR3(-width, waterLevel, depth);
	vertices[1].Uv = D3DXVECTOR2(0, 0);
	vertices[2].Position = D3DXVECTOR3(width, waterLevel, -depth);
	vertices[2].Uv = D3DXVECTOR2(1, 1);

	vertices[3].Position = D3DXVECTOR3(width, waterLevel, -depth);
	vertices[3].Uv = D3DXVECTOR2(1, 1);
	vertices[4].Position = D3DXVECTOR3(-width, waterLevel, depth);
	vertices[4].Uv = D3DXVECTOR2(0, 0);
	vertices[5].Position = D3DXVECTOR3(width, waterLevel, width);
	vertices[5].Uv = D3DXVECTOR2(1, 0);

	//Create Vertex Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * 6;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	SAFE_DELETE_ARRAY(vertices);
}

void Lake::UpdateReflectionCam()
{
	D3DXVECTOR3 camPos, targetPos;
	D3DXVECTOR3 right;
	Context::Get()->GetMainCamera()->Position(&camPos);
	Context::Get()->GetMainCamera()->Forward(&targetPos);
	targetPos = camPos + targetPos;
	Context::Get()->GetMainCamera()->Right(&right);

	//	reflection is rendered from the (x,-y, z) point if (x,y,z) is the original position of the camera.
	//	calculate the '-y' value and the (x,-y,z) vector.
	float reflectionCamYCoord = -camPos.y + 2 * waterLevel;
	D3DXVECTOR3 reflectionCamPos = D3DXVECTOR3(camPos.x, reflectionCamYCoord, camPos.z);

	//	Update values for reflection rendering
	float reflectionTargetYCoord = -targetPos.y + 2 * waterLevel;
	D3DXVECTOR3 reflectionCamTarget = D3DXVECTOR3(targetPos.x, reflectionTargetYCoord, targetPos.z);

	D3DXVECTOR3 forward = reflectionCamTarget - reflectionCamPos;
	D3DXVECTOR3 reflectionCamUp;
	D3DXVec3Cross(&reflectionCamUp, &right, &forward);

	D3DXMatrixLookAtLH(&reflectionView, &reflectionCamPos, &reflectionCamTarget, &reflectionCamUp);
	reflectionViewVar->SetMatrix(reflectionView);
}

void Lake::UpdateReflectionMap()
{
	reflectionRTV->Set();
	sky->RenderReflection(reflectionView);
	terrain->RenderReflection(reflectionView);
	terrain->GetBillboard()->RenderReflection(reflectionView);
}

void Lake::UpdateRefractionMap()
{
	D3DXMATRIX V;
	Context::Get()->GetMainCamera()->MatrixView(&V);

	refractionRTV->Set();
	terrain->RenderReflection(V);
}

void Lake::InitEffectVariables()
{
	worldVar = effect->AsMatrix("World");
	reflectionViewVar = effect->AsMatrix("ReflectionView");
	drawModeVar = effect->AsScalar("DrawMode");
	fresnelModeVar = effect->AsScalar("FresnelMode");
	xdullBlendFactorVar = effect->AsScalar("DullBlendFactor");

	waterSpeedVar = effect->AsScalar("WaterSpeed");
	waveLengthVar = effect->AsScalar("WaveLength");
	waveHeightVar = effect->AsScalar("WaveHeight");
	windForceVar = effect->AsScalar("WindForce");
	windDirectionVar = effect->AsMatrix("WindDirection");
	specPowerVar = effect->AsScalar("SpecPower");
	specPerturbationVar = effect->AsScalar("SpecPerturb");
}

void Lake::SetFirstEffectVariables()
{
	effect->AsSRV("ReflectionMap")->SetResource(reflectionRTV->SRV());
	effect->AsSRV("RefractionMap")->SetResource(refractionRTV->SRV());
	effect->AsSRV("WaterBumpMap")->SetResource(waterBumpMap->SRV());
	effect->AsSRV("ScreenDepth")->SetResource(DeferredRendering::Get()->DepthSRV());

	reflectionViewVar->SetMatrix(reflectionView);
	drawModeVar->SetFloat(reflectionRefractionRatio);
	fresnelModeVar->SetInt(fresnelMode);
	xdullBlendFactorVar->SetFloat(xdullBlendFactor);

	waterSpeedVar->SetFloat(waterSpeed);
	waveLengthVar->SetFloat(0.1f);
	waveHeightVar->SetFloat(waveHeight);
	windForceVar->SetFloat(20.0f);
	D3DXMATRIX R;
	D3DXMatrixRotationY(&R, 0);
	windDirectionVar->SetMatrix(R);
	specPowerVar->SetFloat(specularLightPowerValue);
	specPerturbationVar->SetFloat(specularLightPerturbationValue);
}
