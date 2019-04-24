#include "stdafx.h"
#include "TestDeferred.h"
#include "Scene/GameScene.h"
#include "Environments/Shadow.h"
#include "Environments/Lake.h"
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
#include "Particles/Fire.h"

void TestDeferred::Initialize()
{
	
	
}


void TestDeferred::Ready()
{
	objNames.clear();
	gameObjects.clear();

	lineEffect = new Effect(Effects + L"002_Line.fx");
	trailEffect = new Effect(Effects + L"031_TrailRender.fx");

	D3DDesc desc;
	D3D::GetDesc(&desc);

	shadow = new Shadow((UINT)desc.Width, (UINT)desc.Height);
	shadow->Initialize();
	shadow->Ready();

	GameSceneSpec * sceneSpec = (GameSceneSpec*)GameDataSpecManager::Get()->LoadScene(L"Scene.spec");

	sky = new ScatterSky();
	sky->Load(sceneSpec->SkySpec);
	sky->Initialize();
	sky->Ready();
	sky->Name() = L"Sky";
	sky->Tag() = L"Sky";
	gameObjects.insert(make_pair(sky->Name(), (IGameObject*)sky));
	AlphaRenderer::Get()->AddAlphaMapsF((IAlpha*)sky);


	Terrain::InitDesc terrainDesc;
	Terrain * terrain = new Terrain(terrainDesc);
	terrain->Initialize(sky);
	terrain->Load(sceneSpec->TerrainSpec);
	terrain->Ready();
	terrain->SetShadows(shadow);
	terrain->Name() = L"Terrain";
	terrain->Tag() = L"Terrain";
	gameObjects.insert(make_pair(terrain->Name(), (IGameObject*)terrain));

	lake = new Lake(sky, terrain);
	lake->Initialize();
	lake->Ready();
	AlphaRenderer::Get()->AddAlphaMaps((IAlpha*)lake);

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

	gameScene = new GameScene();
	gameScene->Initialize();
	gameScene->InputShadow(shadow);
	gameScene->Ready();
	gameScene->InitObjects(gameObjects);

	/*for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			GameAnimator * model = new GameAnimator
			(
				i * 5 + j, NULL,
				Models + L"skeletonzombie_t_avelange/skeletonzombie_t_avelange.material",
				Models + L"skeletonzombie_t_avelange/skeletonzombie_t_avelange.mesh"
			);
			model->Ready();
			model->Position(i * 2, 0, j * 2);

			model->AddClip(1, Models + L"skeletonzombie_t_avelange/Idle.anim");
			model->Play("Idle", true);

			models.push_back(model);
			shadow->Add(model);
		}
	}

	meshMat = new Material(Effects + L"013_Mesh&Model.fx");
	plane = new MeshGrid(meshMat, 10, 10, 20, 20);
	plane->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
	plane->Position(5, 0, 5);
	shadow->Add(plane);*/

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
		//SAFE_DELETE(objIter->second);
		objIter->second->Delete();
	}
	gameObjects.clear();
	
	objNames.clear();
	objNames.shrink_to_fit();

	SAFE_DELETE(shadow);

	sky = NULL;
	terrain = NULL;

	SAFE_DELETE(fire);
	SAFE_DELETE(lake);
}

void TestDeferred::Update()
{
	gameScene->Update();
	lake->Update();
}

void TestDeferred::PreRender()
{
	gameScene->PreRender();
	lake->PreRender();
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
		desc.dcImg->Scale(125, 125);
		desc.dcImg->Position(37, 33);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"orb_back.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"01Orb_back", desc);
		UIManager::Get()->SetUiRender(L"01Orb_back", true);

		desc.dcImg = new Render2D();
		desc.dcImg->Scale(220, 130);
		desc.dcImg->Position(170, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"orb_back.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"04base_back", desc);
		UIManager::Get()->SetUiRender(L"04base_back", true);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D(Effects + L"040_Health.fx");
		desc.dcImg->Scale(125, 125);
		desc.dcImg->Position(37, 33);
		desc.dcImg->SetUV(L"y", 30, 30);
		desc.dcImg->SetColor(0.9f, 0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Health.png");
		desc.uiImg2 = new Texture(Textures + L"orb_Fill.png");
		UIManager::Get()->InputUI(L"02Health", desc);
		UIManager::Get()->SetUiRender(L"02Health", true);
		//UIManager::Get()->GetRender2D(L"2Health")->SetUV(L"y", 30, 30);


		desc.dcImg = new Render2D(Effects + L"040_Health.fx");
		desc.dcImg->Scale(220, 130);
		desc.dcImg->Position(170, 0);
		desc.dcImg->SetUV(L"x", 30, 30);
		desc.dcImg->SetColor(1.0f, 1.0f, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"Health.png");
		desc.uiImg2 = new Texture(Textures + L"orb_scroll.png");
		UIManager::Get()->InputUI(L"05Stealth", desc);
		UIManager::Get()->SetUiRender(L"05Stealth", true);
		//UIManager::Get()->GetRender2D(L"5Stealth")->SetUV(L"x", 30, 30);
	}
	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D();
		desc.dcImg->Scale(125, 125);
		desc.dcImg->Position(37, 33);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"orb_glass.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"03Orb_glass", desc);
		UIManager::Get()->SetUiRender(L"03Orb_glass", true);

		desc.dcImg = new Render2D();
		desc.dcImg->Scale(220, 130);
		desc.dcImg->Position(170, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"orb_glass.png");
		desc.uiImg2 = NULL;
		UIManager::Get()->InputUI(L"06base_glass", desc);
		UIManager::Get()->SetUiRender(L"06base_glass", true);
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
		UIManager::Get()->SetUiRender(L"10GameSceneUI", true);
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
		UIManager::Get()->SetUiRender(L"09SkillIcon", true);
	}

	for (int i = 0; i < 6; i++)
	{
		{
			UIManager::UIdesc desc;
			desc.dcImg = new Render2D();
			desc.dcImg->Scale(200, 40);
			desc.dcImg->Position(0, 0);
			desc.isRender = true;
			desc.uiImg = new Texture(Textures + L"BarBack.png");
			desc.uiImg2 = NULL;
			UIManager::Get()->InputUI(L"07BarBack" + to_wstring(i), desc);
		}
		{
			UIManager::UIdesc desc;
			desc.dcImg = new Render2D(Effects + L"040_Health.fx");
			desc.dcImg->Scale(200, 40);
			desc.dcImg->Position(0, 0);
			desc.dcImg->SetUV(L"x", 10, 10);
			desc.dcImg->SetColor(1.0f, 1.0f, 1.0f);
			desc.isRender = true;
			desc.uiImg = new Texture(Textures + L"BarFront.png");
			desc.uiImg2 = new Texture(Textures + L"orb_scroll.png");
			UIManager::Get()->InputUI(L"08BarFront" + to_wstring(i), desc);
		}
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
	fire = new Fire();
	fire->Initialize();
	AlphaRenderer::Get()->AddAlphaMaps(fire);
	fire->Setting(0, 4, 0.6f, 1.35f, 1.0f, 1.0f);
	fire->Ready(28.157f, 7.6f, 33.537f);
}
