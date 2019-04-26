#include "stdafx.h"
#include "TestDeferred.h"
#include "Scene/GameScene.h"
#include "Environments/Shadow.h"
#include "Draw/MeshGrid.h"
#include "Environments/ScatterSky.h"
#include "Environments/Terrain.h"
#include "Environments/TerrainRenderer.h"
#include "Collider/ColliderBox.h"
#include "Collider/ColliderSphere.h"
#include "Module/MyBehaviourTree.h"
#include "Module/TrailRenderer.h"
#include "Objects/IGameObject.h"
#include "Scene/GameScene.h"


void TestDeferred::Initialize()
{
	sceneSpec = (GameSceneSpec*)GameDataSpecManager::Get()->LoadScene(L"Scene.spec");
	ProgressBar::Get()->Progress(0.2f);

	D3DDesc desc;
	D3D::GetDesc(&desc);

	shadow = new Shadow((UINT)desc.Width, (UINT)desc.Height);
	shadow->Initialize();
	ProgressBar::Get()->Progress(0.4f);

	sky = new ScatterSky();
	sky->Load(sceneSpec->SkySpec);
	sky->Initialize();
	sky->Name() = L"Sky";
	sky->Tag() = L"Sky";
	ProgressBar::Get()->Progress(0.6f);

	Terrain::InitDesc terrainDesc;
	terrain = new Terrain(terrainDesc);
	terrain->Initialize(sky);
	terrain->Load(sceneSpec->TerrainSpec);
	terrain->Name() = L"Terrain";
	terrain->Tag() = L"Terrain";
	ProgressBar::Get()->Progress(0.8f);

	gameScene = new GameScene();
	gameScene->InputShadow(shadow);
	ProgressBar::Get()->Progress(1.0f);
}


void TestDeferred::Ready()
{
	objNames.clear();
	gameObjects.clear();

	lineEffect = new Effect(Effects + L"002_Line.fx");
	trailEffect = new Effect(Effects + L"031_TrailRender.fx");

	//	shadow
	shadow->Ready();

	//	sky
	sky->Ready();
	gameObjects.insert(make_pair(sky->Name(), (IGameObject*)sky));
	AlphaRenderer::Get()->AddAlphaMapsF((IAlpha*)sky);

	//	terrain
	terrain->Ready();
	terrain->SetShadows(shadow);
	gameObjects.insert(make_pair(terrain->Name(), (IGameObject*)terrain));

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

	if (sceneSpec->NpcSpec.Name != L"")
	{
		LoadObjectTransform
		(
			sceneSpec->NpcSpec.Name,
			sceneSpec->NpcSpec.Position,
			sceneSpec->NpcSpec.Rotation,
			sceneSpec->NpcSpec.Scale
		);
	}

	if (sceneSpec->EnemyCount > 0)
	{
		for (UINT i = 0; i < sceneSpec->EnemyCount; i++)
		{
			LoadObjectTransform
			(
				sceneSpec->EnemySpecs[i].Name,
				sceneSpec->EnemySpecs[i].Position,
				sceneSpec->EnemySpecs[i].Rotation,
				sceneSpec->EnemySpecs[i].Scale
			);
		}
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

	gameScene->InitObjects(gameObjects);


	InitParticles();
	InitLights();
	InitUI();
}

void TestDeferred::Destroy()
{
	//gameScene->ClearObjects();
	SAFE_DELETE(gameScene);

	SAFE_DELETE(lineEffect);
	SAFE_DELETE(trailEffect);

	gameObjIter objIter = gameObjects.begin();
	for (; objIter != gameObjects.end(); objIter++)
	{
		//objIter->second->Delete();
		SAFE_DELETE(objIter->second);
	}
	gameObjects.clear();
	
	objNames.clear();
	objNames.shrink_to_fit();

	SAFE_DELETE(shadow);

	sky = NULL;
	terrain = NULL;
}

void TestDeferred::Update()
{
	gameScene->Update();
}

void TestDeferred::PreRender()
{
	gameScene->PreRender();
}

void TestDeferred::Render()
{
	gameScene->Render();

	ImGui::Begin("Test");
	{
		int fps = (int)Time::Get()->FPS();
		ImGui::Text("FPS : %d", fps);
	}
	ImGui::End();
}


void TestDeferred::PostRender()
{
	
}

void TestDeferred::ResizeScreen()
{
	//gameScene->ResizeScreen();
}

void TestDeferred::LoadObjectTransform(wstring fileName, D3DXVECTOR3& p, D3DXVECTOR3& r, D3DXVECTOR3& s)
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

	for (UINT i = 0; i < spec->ColliderCount; i++)
	{
		if (spec->Colliders[i].Type == (int)ColliderElement::EType::EBox)
		{
			ColliderBox * colliderBox = new ColliderBox
			(
				lineEffect, spec->Colliders[i].Name,
				D3DXVECTOR3(-0.5f, -0.5f, -0.5f),
				D3DXVECTOR3(0.5f, 0.5f, 0.5f)
			);

			colliderBox->BoneIndex(spec->Colliders[i].BoneIndex);
			colliderBox->Root(spec->Colliders[i].RootMatrix);
			colliderBox->IsDraw(true);

			if (colliderBox->Name().find(L"Main") != wstring::npos)
				model->GetMainCollider() = colliderBox;
			model->GetColliders().push_back(colliderBox);

			if (model->Tag() == L"Weapon")
				CollisionContext::Get()->GetLayer(L"Collision Weapon")->AddCollider(colliderBox);
			else if (model->Tag() == L"Player")
				CollisionContext::Get()->GetLayer(L"Collision Player")->AddCollider(colliderBox);
			else if (model->Tag() == L"Enemy")
			{
				if (colliderBox->Name().find(L"RightHand") != wstring::npos)
					CollisionContext::Get()->GetLayer(L"Collision Enemy Weapon")->AddCollider(colliderBox);
				else
					CollisionContext::Get()->GetLayer(L"Collision Enemy")->AddCollider(colliderBox);
			}
			else
				CollisionContext::Get()->GetLayer(L"Collision World")->AddCollider(colliderBox);
		}
		else if (spec->Colliders[i].Type == (int)ColliderElement::EType::ESphere)
		{
			ColliderSphere * colliderSphere = new ColliderSphere
			(
				lineEffect, spec->Colliders[i].Name + model->Name(),
				D3DXVECTOR3(0, 0, 0), 1
			);

			colliderSphere->BoneIndex(spec->Colliders[i].BoneIndex);
			colliderSphere->Root(spec->Colliders[i].RootMatrix);
			colliderSphere->Transform(colliderSphere->Transform());
			colliderSphere->IsDraw(true);
			if (colliderSphere->Name().find(L"Main") != wstring::npos)
				model->GetMainCollider() = colliderSphere;

			model->GetColliders().push_back(colliderSphere);

			if (model->Tag() == L"Weapon")
				CollisionContext::Get()->GetLayer(L"Collision Weapon")->AddCollider(colliderSphere);
			else if (model->Tag() == L"Player")
			{
				CollisionContext::Get()->GetLayer(L"Collision Player")->AddCollider(colliderSphere);
				if (colliderSphere->Name().find(L"Skill") != wstring::npos)
					CollisionContext::Get()->GetLayer(L"Collision Weapon")->AddCollider(colliderSphere);
			}
			else if (model->Tag() == L"Enemy")
			{
				if (colliderSphere->Name().find(L"RightHand") != wstring::npos)
					CollisionContext::Get()->GetLayer(L"Collision Enemy Weapon")->AddCollider(colliderSphere);
				else
					CollisionContext::Get()->GetLayer(L"Collision Enemy")->AddCollider(colliderSphere);
			}
			else
				CollisionContext::Get()->GetLayer(L"Collision World")->AddCollider(colliderSphere);
		}
	}

	GameAnimator * animator = dynamic_cast<GameAnimator*>(model);
	if (animator != NULL)
	{
		for (UINT i = 0; i < spec->TrailCount; i++)
		{
			TrailRenderer * trail = new TrailRenderer(model, trailEffect);
			trail->BoneIndex(spec->Trails[i].BoneIndex);
			trail->LifeTime(spec->Trails[i].LifeTime);
			trail->DiffuseMap(spec->Trails[i].DiffuseMap);
			trail->AlphaMap(spec->Trails[i].AlphaMap);
			trail->StartMat(spec->Trails[i].StartMat);
			trail->EndMat(spec->Trails[i].EndMat);
			if (i == 1)
				trail->UseDissolve(true);

			model->GetTrails().push_back(trail);
			AlphaRenderer::Get()->AddAlphaMaps(trail);
		}
	}
	
	model->IsChild((bool)spec->IsChild);
	model->RootAxis(spec->RootMatrix);
	if ((bool)spec->IsChild == true)
	{
		GameModel * parent = (GameModel*)gameObjects[String::ToWString(spec->ParentName)];
		model->ParentModel(parent);
		model->ParentBone(parent->GetModel()->BoneByIndex(spec->ParentBoneIndex));
	}

	if (spec->BT != NULL)
		model->BTName() = spec->BT->Name();

	if (model->BTName() != L"")
		model->HasBT() = true;
	else
		SAFE_DELETE(spec->BT);

	objNames.push_back(String::ToString(model->Name()));
	gameObjects.insert(make_pair(model->Name(), (IGameObject*)model));


	shadow->Add((IShadow*)model);
}

void TestDeferred::InitUI()
{
	D3DDesc d3dDesc;
	D3D::GetDesc(&d3dDesc);

	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width / 12.8f, d3dDesc.Height / 7.2f);
		desc.dcImg->Position(d3dDesc.Width / 43.24f, d3dDesc.Height / 27.27f);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"orb_back.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"01Orb_back", desc);
		UIManager::Get()->SetUiRender(L"01Orb_back", false);

		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width / 7.27f, d3dDesc.Height / 6.92f);
		desc.dcImg->Position(d3dDesc.Width / 9.41f, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"orb_back.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"04base_back", desc);
		UIManager::Get()->SetUiRender(L"04base_back", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D(Effects + L"040_Health.fx");
		desc.dcImg->Scale(d3dDesc.Width / 12.8f, d3dDesc.Height / 7.2f);
		desc.dcImg->Position(d3dDesc.Width / 43.24f, d3dDesc.Height / 27.27f);
		desc.dcImg->SetUV(L"y", 30, 30);
		desc.dcImg->SetColor(0.9f, 0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Health.png");
		desc.uiImg2 = new Texture(Textures + L"orb_Fill.png");
		UIManager::Get()->InputUI(L"02Health", desc);
		UIManager::Get()->SetUiRender(L"02Health", false);


		desc.dcImg = new Render2D(Effects + L"040_Health.fx");
		desc.dcImg->Scale(d3dDesc.Width / 7.27f, d3dDesc.Height / 6.92f);
		desc.dcImg->Position(d3dDesc.Width / 9.41f, 0);
		desc.dcImg->SetUV(L"x", 30, 30);
		desc.dcImg->SetColor(1.0f, 1.0f, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Health.png");
		desc.uiImg2 = new Texture(Textures + L"orb_scroll.png");
		UIManager::Get()->InputUI(L"05Stealth", desc);
		UIManager::Get()->SetUiRender(L"05Stealth", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width / 12.8f, d3dDesc.Height / 7.2f);
		desc.dcImg->Position(d3dDesc.Width / 43.24f, d3dDesc.Height / 27.27f);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"orb_glass.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"03Orb_glass", desc);
		UIManager::Get()->SetUiRender(L"03Orb_glass", false);

		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width / 7.27f, d3dDesc.Height / 6.92f);
		desc.dcImg->Position(d3dDesc.Width / 9.41f, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"orb_glass.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"06base_glass", desc);
		UIManager::Get()->SetUiRender(L"06base_glass", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"GameSceneUI.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"10GameSceneUI", desc);
		UIManager::Get()->SetUiRender(L"10GameSceneUI", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"SkillIcon.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"09SkillIcon", desc);
		UIManager::Get()->SetUiRender(L"09SkillIcon", false);
	}

	for (int i = 0; i < 6; i++)
	{
		{
			UIManager::UIdesc desc;
			desc.dcImg = new Render2D();
			desc.dcImg->Scale(d3dDesc.Width / 8, d3dDesc.Height / 22.5f);
			desc.dcImg->Position(0, 0);
			desc.isRender = true;
			desc.uiImg = new Texture(Textures + L"BarBack.png");
			desc.uiImg2 = NULL;
			UIManager::Get()->InputUI(L"07BarBack" + to_wstring(i), desc);
		}
		{
			UIManager::UIdesc desc;
			desc.dcImg = new Render2D(Effects + L"040_Health.fx");
			desc.dcImg->Scale(d3dDesc.Width / 8, d3dDesc.Height / 22.5f);
			desc.dcImg->Position(0, 0);
			desc.dcImg->SetUV(L"x", 10, 10);
			desc.dcImg->SetColor(1.0f, 1.0f, 1.0f);
			desc.isRender = true;
			desc.uiImg = new Texture(Textures + L"BarFront.png");
			desc.uiImg2 = new Texture(Textures + L"orb_scroll.png");
			UIManager::Get()->InputUI(L"08BarFront" + to_wstring(i), desc);
		}
	}

	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width / 1.975f, d3dDesc.Height / 2.018f);
		desc.dcImg->Position((d3dDesc.Width * 0.5f) - (d3dDesc.Width / 1.975f) * 0.5f, (d3dDesc.Height * 0.5f) + 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Title.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"11GameTitleUI", desc);
		UIManager::Get()->SetUiRender(L"11GameTitleUI", true);
	}

	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Dialog0.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"12Dialog0", desc);
		UIManager::Get()->SetUiRender(L"12Dialog0", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Dialog1.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"12Dialog1", desc);
		UIManager::Get()->SetUiRender(L"12Dialog1", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Dialog2.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"12Dialog2", desc);
		UIManager::Get()->SetUiRender(L"12Dialog2", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Dialog3.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"12Dialog3", desc);
		UIManager::Get()->SetUiRender(L"12Dialog3", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Dialog4.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"12Dialog4", desc);
		UIManager::Get()->SetUiRender(L"12Dialog4", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Dialog5.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"12Dialog5", desc);
		UIManager::Get()->SetUiRender(L"12Dialog5", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Dialog13.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"13Dialog", desc);
		UIManager::Get()->SetUiRender(L"13Dialog", false);
	}

	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width / 2.192f, d3dDesc.Height / 1.23f);
		desc.dcImg->Position((d3dDesc.Width * 0.5f) - (d3dDesc.Width / 2.192f) * 0.5f, (d3dDesc.Height * 0.5f) - (d3dDesc.Height / 1.23f) * 0.5f);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"GameClear.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"14GameClear", desc);
		UIManager::Get()->SetUiRender(L"14GameClear", false);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(d3dDesc.Width / 6.154f, d3dDesc.Height / 1.822f);
		desc.dcImg->Position((d3dDesc.Width) - (d3dDesc.Width / 5.93f), (d3dDesc.Height) - (d3dDesc.Height / 1.5f));
		desc.dcImg->SetAlpha(0.5f);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"TextBox.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"09TextBox", desc);
		UIManager::Get()->SetUiRender(L"09TextBox", false);
	}
}

void TestDeferred::InitLights()
{
	PointLightDesc pDesc;
	pDesc.Ambient = D3DXCOLOR(0, 0, 0, 1);
	pDesc.Diffuse = D3DXCOLOR(1, 0.349f, 0, 1);
	pDesc.Specular = D3DXCOLOR(1, 0.349f, 0, 1);

	pDesc.Position = D3DXVECTOR3(28.157f, 7.6f, 33.537f);
	pDesc.Range = 50;
	pDesc.Attenuation = D3DXVECTOR3(0.01f, 0.01f, 0.01f);
	Context::Get()->AddPointLight(pDesc);
}

void TestDeferred::InitParticles()
{
	
}
