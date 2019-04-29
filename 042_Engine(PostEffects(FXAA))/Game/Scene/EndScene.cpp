#include "stdafx.h"
#include "EndScene.h"
#include "Environments/Shadow.h"
#include "Environments/ScatterSky.h"
#include "Environments/Terrain.h"
#include "Module/MyBehaviourTree.h"
#include "Particles/Fire.h"
#include "Environments/Lake.h"
#include "Viewer/SmoothFollow.h"

void EndScene::Initialize()
{
	sceneSpec = (GameSceneSpec*)GameDataSpecManager::Get()->LoadScene(L"Scene1.spec");
	ProgressBar::Get()->Progress(0.2f);

	D3DDesc desc;
	D3D::GetDesc(&desc);

	shadow = new Shadow((UINT)desc.Width, (UINT)desc.Height);
	shadow->Initialize(28.157f, 7.6f, 33.537f, 50.0f);
	ProgressBar::Get()->Progress(0.4f);

	sky = new ScatterSky();
	sky->Load(sceneSpec->SkySpec);
	sky->Initialize();
	ProgressBar::Get()->Progress(0.6f);

	Terrain::InitDesc terrainDesc;
	terrain = new Terrain(terrainDesc);
	terrain->Initialize(sky);
	terrain->Load(sceneSpec->TerrainSpec);
	ProgressBar::Get()->Progress(0.8f);

	PointLightDesc pDesc;
	pDesc.Ambient = D3DXCOLOR(0, 0, 0, 1);
	pDesc.Diffuse = D3DXCOLOR(1, 0.349f, 0, 1);
	pDesc.Specular = D3DXCOLOR(1, 0.349f, 0, 1);

	pDesc.Position = D3DXVECTOR3(12.203f, 7.6f, 21.895f);
	pDesc.Range = 50;
	pDesc.Attenuation = D3DXVECTOR3(0.01f, 0.01f, 0.01f);
	Context::Get()->AddPointLight(pDesc);

	Context::Get()->ChangeCameraType(2);
	Context::Get()->GetMainCamera()->LookAtPosition(15.203f, 7.6f, 21.895f);
	Context::Get()->GetMainCamera()->Rotation(0, Math::ToRadian(50));
	((SmoothFollow*)Context::Get()->GetMainCamera())->SetDistance(15);

	offset1 = -desc.Height;
	offset2 = -(desc.Height * 2);
	ProgressBar::Get()->Progress(1.0f);
}

void EndScene::Ready()
{
	shadow->Ready();

	sky->Ready();
	AlphaRenderer::Get()->AddAlphaMapsF((IAlpha*)sky);

	terrain->Ready();
	terrain->SetShadows(shadow);

	if (sceneSpec->PlayerSpec.Name != L"")
	{
		LoadObjectTransform
		(
			sceneSpec->PlayerSpec.Name,
			sceneSpec->PlayerSpec.Position,
			sceneSpec->PlayerSpec.Rotation,
			sceneSpec->PlayerSpec.Scale
		);
	}

	if (sceneSpec->ObjectCount > 0)
	{
		for (UINT i = 0; i < sceneSpec->ObjectCount; i++)
		{
			LoadObjectTransform
			(
				sceneSpec->ObjectSpecs[i].Name,
				sceneSpec->ObjectSpecs[i].Position,
				sceneSpec->ObjectSpecs[i].Rotation,
				sceneSpec->ObjectSpecs[i].Scale
			);
		}
	}

	lake = new Lake(sky, terrain);
	lake->Initialize(0);
	lake->Ready();
	AlphaRenderer::Get()->AddAlphaMaps((IAlpha*)lake);

	fire = new Fire();
	fire->Initialize();
	AlphaRenderer::Get()->AddAlphaMaps(fire);
	fire->Setting(0, 4, 0.6f, 1.35f, 1.0f, 1.0f);
	fire->Ready(12.203f, 7.6f, 21.895f);

	InitUI();
	state = STATE::DEFAULT;
	offsetSpeed = 20.0f;

	if (SoundManager::Get()->isPlaySound("00MainBGM") == false)
	{
		SoundManager::Get()->play("00MainBGM", 1.0f);
		SoundManager::Get()->play("21CampFire", 0.2f);
	}
}

void EndScene::Destroy()
{
	SAFE_DELETE(lake);
	SAFE_DELETE(fire);
	SAFE_DELETE(shadow);
	SAFE_DELETE(sky);
	SAFE_DELETE(terrain);
	SAFE_DELETE(player);
	for (GameModel *model : objects)
		SAFE_DELETE(model);
	objects.clear();
	objects.shrink_to_fit();

	objNames.clear();
	objNames.shrink_to_fit();
}

void EndScene::Update()
{
	sky->Update();
	shadow->Update();
	lake->Update();

	D3DXVECTOR3 pos = player->Position();
	pos.y = terrain->GetHeight(pos.x, pos.z);
	player->Position(pos);

	if (Keyboard::Get()->Press(VK_SPACE))
		offsetSpeed = 60.0f;
	else
		offsetSpeed = 20.0f;
	

	D3DDesc desc;
	D3D::GetDesc(&desc);
	switch (state)
	{
	case EndScene::STATE::DEFAULT:
		UIManager::Get()->UseFadeIn(-3.0f);
		state = STATE::FADEIN;
		
		break;
	case EndScene::STATE::FADEIN:
		offset1 += offsetSpeed * Time::Delta();
		offset2 += offsetSpeed * Time::Delta();

		if (UIManager::Get()->EndFadeIn() == true)
		{
			state = STATE::ENDING;
			UIManager::Get()->ResetFadeOut();
		}
		break;
	case EndScene::STATE::ENDING:
		offset1 += offsetSpeed * Time::Delta();
		offset2 += offsetSpeed * Time::Delta();

		if (offset2 >= desc.Height)
		{
			state = STATE::FADEOUT;
			UIManager::Get()->UseFadeOut(0.3f);
		}

		if (Keyboard::Get()->Down(VK_ESCAPE))
		{
			state = STATE::FADEOUT;
			UIManager::Get()->UseFadeOut(3.0f);
		}
		break;
	case EndScene::STATE::FADEOUT:
		if (UIManager::Get()->EndFadeOut() == true)
		{
			UIManager::Get()->ResetFadeOut();
			SceneManager::Get()->ChangeScene(SceneManager::SCENETYPE::MainScene);
			SoundManager::Get()->stop("00MainBGM");
			SoundManager::Get()->stop("21CampFire");
			return;
		}
		break;
	}

	UIManager::Get()->GetRender2D(L"01Ending1")->Position(offsetX, offset1);
	UIManager::Get()->GetRender2D(L"01Ending2")->Position(offsetX, offset2);
}

void EndScene::PreRender()
{
	sky->PreRender();
	shadow->PreRender();
	lake->PreRender();
}

void EndScene::Render()
{
	shadow->Render();

	ImGui::Begin("Test");
	{
		int fps = (int)Time::Get()->FPS();
		ImGui::Text("FPS : %d", fps);
	}
	ImGui::End();
}

void EndScene::PostRender()
{
}

void EndScene::ResizeScreen()
{
}

void EndScene::LoadObjectTransform(wstring fileName, D3DXVECTOR3 & p, D3DXVECTOR3 & r, D3DXVECTOR3 & s)
{
	wstring temp = Path::GetFileName(fileName);

	GameCharacterSpec * spec = NULL;
	GameDataSpec * gamedata = GameDataSpecManager::Get()->Find(temp);
	if (gamedata == NULL)
		spec = (GameCharacterSpec*)GameDataSpecManager::Get()->LoadCharacter(temp);
	else
		spec = (GameCharacterSpec*)gamedata;

	spec->bDelete = true;

	wstring filePath = Models + spec->SourceFilePath + L"/";

	UINT count = 0;
	for (string name : objNames)
	{
		if (name.find(String::ToString(spec->Name)) != string::npos)
			count++;
	}

	GameModel * model;
	switch (spec->ModelType)
	{
	case ModelType::EGameModel:
		model = new GameModel
		(
			count, NULL,
			filePath + spec->Name + L".material",
			filePath + spec->Name + L".mesh"
		);
		model->Ready();

		objects.push_back(model);

		break;
	case ModelType::EGameAnimator:
		model = new GameAnimator
		(
			count, NULL,
			filePath + spec->Name + L".material",
			filePath + spec->Name + L".mesh"
		);
		model->Ready();

		GameAnimator * animator = (GameAnimator *)model;
		for (UINT i = 0; i < spec->ClipCount; i++)
		{
			wstring file = filePath + String::ToWString(spec->ClipNames[i] + ".anim");
			animator->AddClip(i + 1, file);
		}

		player = animator;
		player->Play("Sitting Idle", true, 0.2f);

		break;
	}


	model->Position(p);
	model->Rotation(r);
	model->Scale(s);

	if (count > 0)
		model->Name() = spec->Name + L" (" + to_wstring(count) + L")";
	else
		model->Name() = spec->Name;

	spec->Name = model->Name();

	model->Tag() = spec->Tag;
	model->FileName() = spec->SourceFilePath;

	if (spec->BT != NULL)
		model->BTName() = spec->BT->Name();

	if (model->BTName() != L"")
		model->HasBT() = true;
	else
		SAFE_DELETE(spec->BT);

	objNames.push_back(String::ToString(model->Name()));

	shadow->Add((IShadow*)model);
}

void EndScene::InitUI()
{
	D3DDesc d3dDesc;
	D3D::GetDesc(&d3dDesc);

	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width / 3.56f, d3dDesc.Height);
		desc.dcImg->Position(d3dDesc.Width - (d3dDesc.Width / 3.56f), 0);
		offsetX = d3dDesc.Width - (d3dDesc.Width / 3.56f);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Ending1.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"01Ending1", desc);
		UIManager::Get()->SetUiRender(L"01Ending1", true);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width / 3.56f, d3dDesc.Height);
		desc.dcImg->Position(d3dDesc.Width - (d3dDesc.Width / 3.56f), -d3dDesc.Height);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Ending2.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"01Ending2", desc);
		UIManager::Get()->SetUiRender(L"01Ending2", true);
	}
}

void EndScene::InitLights()
{
}

void EndScene::InitParticles()
{
}
