#include "Framework.h"
#include "Fire.h"

#define MIN_SAMPLING_RATE   4
#define MAX_SAMPLING_RATE   64

// Fire parameters
#define DEFAULT_JITTER true
#define DEFAULT_SAMPLING_RATE 16
#define DEFAULT_SPEED 0.6f
#define DEFAULT_NOISE_SCALE 1.35f
#define DEFAULT_ROUGHNESS 3.20f
#define DEFAULT_SHAPE_SIZE 3.0f
#define DEFAULT_FREQUENCY1 1.0f
#define DEFAULT_FREQUENCY2 0.5f
#define DEFAULT_FREQUENCY3 0.25f
#define DEFAULT_FREQUENCY4 0.125f
#define DEFAULT_FREQUENCY5 0.0625f

Fire::Fire()
{
	D3DXMatrixIdentity(&world);
}

Fire::~Fire()
{
	SAFE_DELETE(material);
	SAFE_DELETE(meshCube);

	SAFE_DELETE(fireTexture);

	SAFE_RELEASE(noiseTexture);
	SAFE_RELEASE(jitterSRV);
	SAFE_RELEASE(permSRV);
}

void Fire::Initialize()
{
	Reset();
}

void Fire::Ready(float x, float y, float z)
{
	material = new Material(Effects + L"036_PerlinFire.fx");
	meshCube = new MeshCube(material, 1.0f, 1.0f, 1.0f);
	meshCube->Scale(4.0f * shapeSize, 8.0f * shapeSize, 4.0f * shapeSize);
	meshCube->Position(x, y, z);

	worldVar = material->GetWorldVariable();


	screenDepthVar = material->GetEffect()->AsSRV("ScreenDepth");
	fireTexVar = material->GetEffect()->AsSRV("FireShape");
	jitterTexVar = material->GetEffect()->AsSRV("JitterTexture");
	permTexVar = material->GetEffect()->AsSRV("PermTexture");

	stepSizeVar = material->GetEffect()->AsScalar("StepSize");
	noiseScaleVar = material->GetEffect()->AsScalar("NoiseScale");
	roughnessVar = material->GetEffect()->AsScalar("Roughness");
	frequencyWeightsVar = material->GetEffect()->AsScalar("FrequencyWeights");
	speedVar = material->GetEffect()->AsScalar("Speed");
	bJitterVar = material->GetEffect()->AsScalar("Jitter");

	fireTexture = new Texture(Textures + L"Firetex.dds");

	BYTE data[256 * 256];
	for (int i = 0; i < 256 * 256; i++)
		data[i] = (unsigned char)(Math::Random(0, 255));

	CsResource::CreateTexture(256, 256, data, &noiseTexture, false, false, DXGI_FORMAT_R8_TYPELESS, 256);
	CsResource::CreateSRV(noiseTexture, &jitterSRV, DXGI_FORMAT_R8_UNORM);
	CsResource::CreateSRV(noiseTexture, &permSRV, DXGI_FORMAT_R8_UINT);

	fireTexVar->SetResource(fireTexture->SRV());
	jitterTexVar->SetResource(jitterSRV);
	permTexVar->SetResource(permSRV);
	if(DEFERRED == true)
		screenDepthVar->SetResource(DeferredRendering::Get()->DepthSRV());



	noiseScaleVar->SetFloat(noiseScale);
	roughnessVar->SetFloat(roughness);
	frequencyWeightsVar->SetFloatArray(frequencyWeights, 0, 5);
	speedVar->SetFloat(speed);
	stepSizeVar->SetFloat(1.0f / (float)samplingRate);
	bJitterVar->SetBool(bJitter);
}

void Fire::ImGuiRender()
{
	ImGui::Begin("Fire", 0, NULL);
	{
		if (ImGui::Button("Reset"))
			Reset();

		ImGui::Checkbox("Jitter", &bJitter);
		ImGui::SliderFloat("Speed", &speed, 0, 200);
		ImGui::SliderInt("SampleRate", &samplingRate, MIN_SAMPLING_RATE, MAX_SAMPLING_RATE);
		ImGui::SliderFloat("NoiseScale", &noiseScale, 0, 100);
		ImGui::SliderFloat("Roughness", &roughness, 0, 100);
		ImGui::SliderFloat("ShapeSize", &shapeSize, 25, 100);

		ImGui::Text("Octave weights");
		ImGui::SliderFloat("Frequency1", &frequencyWeights[0], 0, 100);
		ImGui::SliderFloat("Frequency2", &frequencyWeights[1], 0, 100);
		ImGui::SliderFloat("Frequency3", &frequencyWeights[2], 0, 100);
		ImGui::SliderFloat("Frequency4", &frequencyWeights[3], 0, 100);
		ImGui::SliderFloat("Frequency5", &frequencyWeights[4], 0, 100);

		noiseScaleVar->SetFloat(noiseScale);
		roughnessVar->SetFloat(roughness);
		frequencyWeightsVar->SetFloatArray(frequencyWeights, 0, 5);
		speedVar->SetFloat(speed);
		stepSizeVar->SetFloat(1.0f / (float)samplingRate);
		bJitterVar->SetBool(bJitter);
	}
	ImGui::End();
}

void Fire::Render()
{
	meshCube->Render(0, 0);
}

void Fire::Setting(bool jitter, int smpRate, float speed, float noiseScale, float shapeSize, float rough)
{
	bJitter = jitter;
	samplingRate = smpRate;
	this->speed = speed;
	this->noiseScale = noiseScale;
	this->shapeSize = shapeSize;
	roughness = rough;
}

void Fire::Reset()
{
	bJitter = DEFAULT_JITTER;
	samplingRate = DEFAULT_SAMPLING_RATE;
	speed = DEFAULT_SPEED;
	noiseScale = DEFAULT_NOISE_SCALE;
	shapeSize = DEFAULT_SHAPE_SIZE;
	roughness = DEFAULT_ROUGHNESS;

	frequencyWeights[0] = DEFAULT_FREQUENCY1;
	frequencyWeights[1] = DEFAULT_FREQUENCY2;
	frequencyWeights[2] = DEFAULT_FREQUENCY3;
	frequencyWeights[3] = DEFAULT_FREQUENCY4;
	frequencyWeights[4] = DEFAULT_FREQUENCY5;
}

