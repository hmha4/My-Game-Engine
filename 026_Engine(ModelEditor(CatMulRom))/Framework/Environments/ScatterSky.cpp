#include "Framework.h"
#include "ScatterSky.h"
#include "./Viewer/Camera.h"
#include "Utilities/Xml.h"

ScatterSky::ScatterSky()
	: realTime(true), theta(0.0f), phi(0), MoonAlpha(0.0f)
	, radius(10), slices(32), stacks(16), startTheta(0)
	, timeFactor(0.3f), setAngle(90), isPick(false), isShowLM(false)
	, loadStarMap(L""), loadMoonMap(L""), loadGlowMap(L""), loadcloudMap(L"")
{
	SampleCount = 20;

	WaveLength = D3DXVECTOR3(0.65f, 0.57f, 0.475f);
}

ScatterSky::~ScatterSky()
{
	SAFE_RELEASE(quadBuffer);
	SAFE_DELETE_ARRAY(quadVertices);

	// Texture
	SAFE_DELETE(cloud);
	SAFE_DELETE(moonGlow);
	SAFE_DELETE(moon);
	SAFE_DELETE(starField);

	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(vertexBuffer);

	SAFE_DELETE(effect);

	SAFE_DELETE(rayleighTarget);
	SAFE_DELETE(mieTarget);

	SAFE_DELETE(noise2D);
	SAFE_DELETE(mie2D);
	SAFE_DELETE(rayleigh2D);
}

void ScatterSky::Initialize()
{
}

void ScatterSky::Ready()
{
	mieTarget = new RenderTarget(128, 64);
	rayleighTarget = new RenderTarget(128, 64);

	effect = new Effect(Effects + L"024_ScatterSky.fx");
	worldVariable = effect->AsMatrix("World");
	moonAlphaVar = effect->AsScalar("MoonAlpha");

	effect->AsScalar("SampleCount")->SetInt(SampleCount);
	effect->AsSRV("RayleighMap")->SetResource(rayleighTarget->SRV());
	effect->AsSRV("MieMap")->SetResource(mieTarget->SRV());
	CalcWaveLength();

	GenerateSphere();
	GenerateQuad();

	if (loadStarMap == L"")
		starField = new Texture(Textures + L"Star.png");
	else
		starField = new Texture(loadStarMap);
	effect->AsSRV("StarfieldMap")->SetResource(starField->SRV());

	if (loadMoonMap == L"")
		moon = new Texture(Textures + L"moon.tga");
	else
		moon = new Texture(loadMoonMap);
	effect->AsSRV("MoonMap")->SetResource(moon->SRV());

	if (loadGlowMap == L"")
		moonGlow = new Texture(Textures + L"moonglow.tga");
	else
		moonGlow = new Texture(loadGlowMap);
	effect->AsSRV("MoonGlowMap")->SetResource(moonGlow->SRV());

	rayleigh2D = new Render2D();
	rayleigh2D->Position(0, 100);
	rayleigh2D->Scale(200, 100);

	mie2D = new Render2D();
	mie2D->Position(0, 0);
	mie2D->Scale(200, 100);

	D3DDesc desc;
	D3D::GetDesc(&desc);

	noise2D = new Render2D();
	noise2D->Position(0, 200);
	noise2D->Scale(200, 200);

	int perm[] = { 151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	int gradValues[] = {
		1,1,0,
		-1,1,0, 1,-1,0,
		-1,-1,0, 1,0,1,
		-1,0,1, 1,0,-1,
		-1,0,-1, 0,1,1,
		0,-1,1, 0,1,-1,
		0,-1,-1, 1,1,0,
		0,-1,1, -1,1,0,
		0,-1,-1
	};
	if (loadcloudMap == L"")
		cloud = new Texture(Textures + L"PermTex.png");
	else
		cloud = new Texture(loadcloudMap);

	vector<D3DXCOLOR> pixels;
	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			//int offset = (i * 256 + j);
			int value = perm[(j + perm[i]) & 0xFF];
			D3DXCOLOR color;
			color.r = (float)(gradValues[value & 0x0F] * 64 + 64);
			color.g = (float)(gradValues[value & 0x0F + 1] * 64 + 64);
			color.b = (float)(gradValues[value & 0x0F + 2] * 64 + 64);
			color.a = (float)value;
			pixels.push_back(color);
		}
	}

	cloud->WritePixels(DXGI_FORMAT_R8G8B8A8_UNORM, pixels, true);

	effect->AsSRV("CloudMap")->SetResource(cloud->SRV());
}

void ScatterSky::Update()
{
	D3DXMATRIX V;
	D3DXVECTOR3 pos;
	Context::Get()->GetMainCamera()->Position(&pos);
	D3DXMatrixTranslation(&V, pos.x, pos.y, pos.z);

	world = V;
	worldVariable->SetMatrix(world);

	theta += Time::Delta() * timeFactor;
	if (theta > Math::PI * 2.0f) theta -= Math::PI * 2.0f;

	if (timeFactor > 0.0f)
	{
		lightDirection = GetDirection();
		Context::Get()->GetGlobalLight()->Direction = lightDirection;

		sunColor = GetSunColor(-theta, 2);
		Context::Get()->GetGlobalLight()->SunColor = sunColor;
		Context::Get()->ChangeGlobalLight();
	}
}

void ScatterSky::PreRender()
{
	if (prevTheta == theta && prevPhi == phi) return;

	mieTarget->Set();
	rayleighTarget->Set();

	ID3D11RenderTargetView* rtvs[2];
	rtvs[0] = rayleighTarget->RTV();
	rtvs[1] = mieTarget->RTV();

	ID3D11DepthStencilView* dsv;
	dsv = rayleighTarget->DSV();

	D3D::Get()->SetRenderTargets(2, rtvs, dsv);

	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &quadBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	effect->Draw(0, 1, 6);
}

void ScatterSky::Render()
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	effect->DrawIndexed(0, 0, indexCount);

	D3DXVECTOR3 camPos;
	Context::Get()->GetMainCamera()->Position(&camPos);
	// Moon
	{
		D3DXMATRIX R;
		Context::Get()->GetMainCamera()->MatrixView(&R);
		D3DXMatrixInverse(&R, NULL, &R);

		float x = asin(-R._32);
		float y = atan2(R._31, R._33);
		float z = atan2(R._12, R._22);

		D3DXVECTOR3 rotation(x, y, z);

		D3DXVECTOR3 position;
		position.x = Context::Get()->GetGlobalLight()->Direction.x * 150;
		position.y = Context::Get()->GetGlobalLight()->Direction.y * 150;
		position.z = Context::Get()->GetGlobalLight()->Direction.z * 150;

		D3DXMATRIX S, RX, RY, RZ, T1, T2;
		D3DXMatrixScaling(&S, 20, 20, 20);
		D3DXMatrixRotationX(&RX, rotation.x);
		D3DXMatrixRotationY(&RY, rotation.y);
		D3DXMatrixRotationZ(&RZ, rotation.z);
		D3DXMatrixTranslation(&T1, position.x, position.y, position.z);
		D3DXMatrixTranslation(&T2, camPos.x, camPos.y, camPos.z);

		D3DXMATRIX world = S * RX * RY * RZ * T1 * T2;

		if (theta < Math::PI * 0.5f || theta > Math::PI * 1.5f)
			MoonAlpha = fabs(sinf(theta + (float)Math::PI / 2.0f));
		else
			MoonAlpha = 0.0f;

		moonAlphaVar->SetFloat(MoonAlpha);

		UINT stride = sizeof(VertexTexture);
		UINT offset = 0;

		{
			// Moon
			D3D::GetDC()->IASetVertexBuffers(0, 1, &quadBuffer, &stride, &offset);
			D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			worldVariable->SetMatrix(world);

			effect->Draw(0, 2, 6);
		}

		{
			//Moon Glow
			D3DXMatrixScaling(&S, 2, 2, 1);
			D3DXMATRIX matF = S * world;

			worldVariable->SetMatrix(matF);

			effect->Draw(0, 3, 6);
		}
	}

	// Cloud
	{
		UINT stride = sizeof(VertexTexture);
		UINT offset = 0;
		D3D::GetDC()->IASetVertexBuffers(0, 1, &quadBuffer, &stride, &offset);
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3DXMATRIX sc, rX, rY, dist, cam, matF;

		D3DXMatrixScaling(&sc, 900, 900, 1);
		D3DXMatrixTranslation(&dist, 0, 0, -200.0f);
		D3DXMatrixRotationX(&rX, Math::PI * 0.5f);
		D3DXMatrixTranslation(&cam, camPos.x, camPos.y, camPos.z);
		matF = sc * dist * rX * cam;

		worldVariable->SetMatrix(matF);

		//shader->AsScalar("NumTiles")->SetFloat(16.25f);
		//shader->AsScalar("NoiseTime")->SetFloat(Time::Get()->Running() / 8.0f);
		effect->Draw(0, 4, 6);
	}
}

void ScatterSky::RayleighMieRender()
{
	rayleigh2D->SRV(rayleighTarget->SRV());
	rayleigh2D->Update();
	rayleigh2D->Render();

	mie2D->SRV(mieTarget->SRV());
	mie2D->Update();
	mie2D->Render();

	noise2D->SRV(cloud->SRV());
	noise2D->Update();
	noise2D->Render();
}

void ScatterSky::PostRender()
{
}

void ScatterSky::ResizeScreen()
{
}

void ScatterSky::CalcWaveLength()
{
	InvWaveLength.x = 1.0f / powf(WaveLength.x, 4.0f);
	InvWaveLength.y = 1.0f / powf(WaveLength.y, 4.0f);
	InvWaveLength.z = 1.0f / powf(WaveLength.z, 4.0f);

	WaveLengthMie.x = powf(WaveLength.x, -0.84f);
	WaveLengthMie.y = powf(WaveLength.y, -0.84f);
	WaveLengthMie.z = powf(WaveLength.z, -0.84f);

	effect->AsVector("WaveLength")->SetFloatVector(WaveLength);
	effect->AsVector("InvWaveLength")->SetFloatVector(InvWaveLength);
	effect->AsVector("WaveLengthMie")->SetFloatVector(WaveLengthMie);
}

void ScatterSky::GenerateSphere()
{
#if false
	UINT radius = 10, slices = 32, stacks = 16;

	float sliceStep = (Math::PI * 2) / slices;
	float stackStep = (Math::PI) / stacks;

	///Create Vertex Data
	vertexCount = (slices + 1) * (stacks + 1);
	VertexTexture* vertices = new VertexTexture[vertexCount];
	{
		float stackAngle = 0.0f; // - (Math::PI / 2);
		for (UINT y = 0; y <= stacks; y++) // 위도, altitude
		{
			float sliceAngle = 0.0f;
			for (UINT xz = 0; xz <= slices; xz++) // 경도, longitude
			{
				UINT index = (slices + 1) * y + xz;

				vertices[index].Position.x = (float)(radius * cosf(stackAngle) * cosf(sliceAngle));
				vertices[index].Position.y = (float)(radius * sinf(stackAngle));
				vertices[index].Position.z = (float)(radius * cosf(stackAngle) * sinf(sliceAngle));

				vertices[index].Uv.y = (1 / (float)slices) * xz;
				vertices[index].Uv.x = (1 / (float)stacks) * y;

				sliceAngle += sliceStep;
			}
			stackAngle += stackStep;
		}
	}

	///Create Index Data
	indexCount = slices * stacks * 6;
	UINT* indices = new UINT[indexCount];
	{
		UINT index = 0;
		for (UINT y = 0; y < stacks; y++)
		{
			for (UINT x = 0; x < slices; x++)
			{
				indices[index + 0] = (slices + 1) * y + x; //0
				indices[index + 1] = (slices + 1) * (y + 1) + x; //1
				indices[index + 2] = (slices + 1) * y + (x + 1); //2

				indices[index + 3] = (slices + 1) * y + (x + 1); //2
				indices[index + 4] = (slices + 1) * (y + 1) + x; //1
				indices[index + 5] = (slices + 1) * (y + 1) + (x + 1); //3

				index += 6;
			}
		}//for(z)
	}
#else
	UINT latitude = 32 / 2; // 위도
	UINT longitude = 32; // 경도

	vertexCount = longitude * latitude * 2;
	indexCount = (longitude - 1) * (latitude - 1) * 2 * 8;

	VertexTexture* vertices = new VertexTexture[vertexCount];

	UINT index = 0;
	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = Math::PI * j / (latitude - 1);

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz);
			vertices[index].Position.z = sinf(xz) * sinf(y);
			vertices[index].Position *= 10.0f; // 크기를 키우려고 임의의 값 곱한거

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		} // for(j)
	}  // for(i)

	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = (Math::PI * 2.0f) - (Math::PI * j / (latitude - 1));

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz);
			vertices[index].Position.z = sinf(xz) * sinf(y);
			vertices[index].Position *= 10.0f; // 크기를 키우려고 임의의 값 곱한거

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		} // for(j)
	}  // for(i)

	index = 0;
	UINT* indices = new UINT[indexCount * 3];

	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = i * latitude + j;
			indices[index++] = (i + 1) * latitude + j;
			indices[index++] = (i + 1) * latitude + (j + 1);

			indices[index++] = (i + 1) * latitude + (j + 1);
			indices[index++] = i * latitude + (j + 1);
			indices[index++] = i * latitude + j;
		}
	}

	UINT offset = latitude * longitude;
	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = offset + i * latitude + j;
			indices[index++] = offset + (i + 1) * latitude + (j + 1);
			indices[index++] = offset + (i + 1) * latitude + j;

			indices[index++] = offset + i * latitude + (j + 1);
			indices[index++] = offset + (i + 1) * latitude + (j + 1);
			indices[index++] = offset + i * latitude + j;
		}
	}
#endif

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * vertexCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(UINT) * indexCount;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = indices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &indexBuffer);
		assert(SUCCEEDED(hr));
	}

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);
}

void ScatterSky::GenerateQuad()
{

	quadVertices = new VertexTexture[6];

	quadVertices[0].Position = D3DXVECTOR3(-1.0f, -1.0f, 0);
	quadVertices[1].Position = D3DXVECTOR3(-1.0f, 1.0f, 0);
	quadVertices[2].Position = D3DXVECTOR3(1.0f, -1.0f, 0);
	quadVertices[3].Position = D3DXVECTOR3(1.0f, -1.0f, 0);
	quadVertices[4].Position = D3DXVECTOR3(-1.0f, 1.0f, 0);
	quadVertices[5].Position = D3DXVECTOR3(1.0f, 1.0f, 0);

	quadVertices[0].Uv = D3DXVECTOR2(0, 1); // 0
	quadVertices[1].Uv = D3DXVECTOR2(0, 0); // 1
	quadVertices[2].Uv = D3DXVECTOR2(1, 1); // 2
	quadVertices[3].Uv = D3DXVECTOR2(1, 1); // 2
	quadVertices[4].Uv = D3DXVECTOR2(0, 0); // 1
	quadVertices[5].Uv = D3DXVECTOR2(1, 0); // 3

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * 6;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = quadVertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &quadBuffer);
		assert(SUCCEEDED(hr));
	}
}

float ScatterSky::GetStarIntensity()
{
	return 0.0f;
}

D3DXVECTOR3 ScatterSky::GetDirection()
{
	float y = (float)cosf(theta);
	float x = (float)(sinf(theta) * cosf(phi));
	float z = (float)(sinf(Math::ToRadian(setAngle)));

	return D3DXVECTOR3(x, y, z);
}

D3DXCOLOR ScatterSky::GetSunColor(float fTheta, int nTurbidity)
{
	float fBeta = 0.04608365822050f * nTurbidity - 0.04586025928522f;
	float fTauR, fTauA;
	float fTau[3];

	float coseno = (float)cos((double)fTheta + Math::PI);
	double factor = (double)fTheta / Math::PI * 180.0;
	double jarl = pow(93.885 - factor, -1.253);
	float potencia = (float)jarl;
	float m = 1.0f / (coseno + 0.15f * potencia);

	int i;
	float fLambda[3];
	fLambda[0] = WaveLength.x;
	fLambda[1] = WaveLength.y;
	fLambda[2] = WaveLength.z;

	for (i = 0; i < 3; i++)
	{
		potencia = (float)pow((double)fLambda[i], 4.0);
		fTauR = (float)exp((double)(-m * 0.008735f * potencia));

		const float fAlpha = 1.3f;
		potencia = (float)pow((double)fLambda[i], (double)-fAlpha);
		if (m < 0.0f)
			fTau[i] = 0.0f;
		else
		{
			fTauA = (float)exp((double)(-m * fBeta * potencia));
			fTau[i] = fTauR * fTauA;
		}
	}

	D3DXCOLOR vAttenuation = D3DXCOLOR(fTau[0], fTau[1], fTau[2], 1.0f);
	return vAttenuation;
}

void ScatterSky::SetTextures(wstring fileName, UINT type)
{
	switch (type)
	{
	case 0:
		loadStarMap = fileName;
		break;
	case 1:
		loadMoonMap = fileName;
		break;
	case 2:
		loadGlowMap = fileName;
		break;
	case 3:
		loadcloudMap = fileName;
		break;
	}
}

wstring & ScatterSky::Name()
{
	return skyName;
}

wstring & ScatterSky::Tag()
{
	return tag;
}

void ScatterSky::Save()
{
	GameSkySpec * spec = new GameSkySpec();

	spec->LoadStarMap = loadStarMap;
	spec->LoadMoonMap = loadMoonMap;
	spec->LoadGlowMap = loadGlowMap;
	spec->LoadCloudMap = loadcloudMap;
	spec->SetAngle = setAngle;
	spec->TimeFactor = timeFactor;
	spec->SampleCount = SampleCount;
	spec->WaveLength = WaveLength;
	spec->StartTheta = startTheta;

	GameDataSpecManager::Get()->WriteSky(skyName, spec);
	SAFE_DELETE(spec);
}

void ScatterSky::Load(wstring fileName)
{
	GameSkySpec * spec = NULL;
	spec = (GameSkySpec*)GameDataSpecManager::Get()->LoadSky(fileName);

	loadStarMap = spec->LoadStarMap;
	loadMoonMap = spec->LoadMoonMap;
	loadGlowMap = spec->LoadGlowMap;
	loadcloudMap = spec->LoadCloudMap;
	setAngle = spec->SetAngle;
	timeFactor = spec->TimeFactor;
	SampleCount = spec->SampleCount;
	WaveLength = spec->WaveLength;
	startTheta = spec->StartTheta;
	theta = startTheta;
}

void ScatterSky::Delete()
{
	if (this != NULL)
		delete this;
}