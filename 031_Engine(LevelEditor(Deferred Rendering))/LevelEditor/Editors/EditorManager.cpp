#include "stdafx.h"
#include "EditorManager.h"
#include "MapEditor.h"
#include "ObjectEditor.h"
#include "Environments/Grass.h"
#include "Environments/Terrain.h"
#include "Environments/TerrainRenderer.h"
#include "Environments/HeightMap.h"
#include "Objects/IGameObject.h"
#include "Environments/Shadow.h"
#include "Environments/ScatterSky.h"
#include "Draw/MeshCube.h"
#include "Draw/MeshSphere.h"
#include "Draw/MeshGrid.h"
#include "Draw/MeshQuad.h"
#include "Draw/MeshCylinder.h"
#include "Collider/ColliderBox.h"
#include "Collider/ColliderSphere.h"
#include "Module/MyBehaviourTree.h"
#include "Module/TrailRenderer.h"
#include "Utilities/Xml.h"

EditorManager::EditorManager()
	: sky(NULL), shadow(NULL)
	, modelEffect(NULL), animEffect(NULL), lineEffect(NULL)
	, terrainCount(0), cubeCount(0), sphereCount(0)
	, planeCount(0), quadCount(0), cylCount(0)
{
	selection_mask = (1 << 2);
}

EditorManager::~EditorManager()
{
	SAFE_DELETE(meshMaterial);
	SAFE_DELETE(modelEffect);
	SAFE_DELETE(animEffect);
	SAFE_DELETE(lineEffect);
	SAFE_DELETE(trailEffect);

	gameObjIter objIter = gameObjects.begin();
	for (; objIter != gameObjects.end(); objIter++)
	{
		objIter->second->Delete();
	}
	gameObjects.clear();

	toolIter toolIter = editors.begin();
	for (; toolIter != editors.end(); toolIter++)
	{
		toolIter->second->Delete();
	}
	editors.clear();

	SAFE_DELETE(shadow);
}

void EditorManager::Initialize()
{
	sky = new ScatterSky();
	sky->Initialize();
	sky->Name() = L"Sky";
	sky->Tag() = L"Sky";
	gameObjects.insert(make_pair(sky->Name(), (IGameObject*)sky));
	objNames.push_back(String::ToString(sky->Name()));
	

	MapEditor * mapEditor = new MapEditor();
	mapEditor->Initalize();
	editors.insert(make_pair(EditorType::TERRAINTOOL, mapEditor));

	ObjectEditor * objectEditor = new ObjectEditor();
	objectEditor->Initalize();
	editors.insert(make_pair(EditorType::OBJECTTOOL, objectEditor));

	curToolType = EditorType::NONE;

	shadow = new Shadow(2048, 2048);
	shadow->Initialize();

	MyGui::show_app_inspector = true;
	MyGui::show_app_debugLog = true;
	MyGui::show_app_hierarchy = true;
}

void EditorManager::Ready()
{
	meshMaterial = new Material(Effects + L"013_Mesh&Model.fx");
	modelEffect = new Effect(Effects + L"032_ModelInstance.fx");
	animEffect = new Effect(Effects + L"033_AnimInstance.fx");
	lineEffect = new Effect(Effects + L"002_Line.fx");
	trailEffect = new Effect(Effects + L"031_TrailRender.fx");

	sky->Ready();

	toolIter tIter = editors.begin();

	for (; tIter != editors.end(); tIter++)
		tIter->second->Ready();

	shadow->Ready();
}

void EditorManager::Update()
{
	if(sky != NULL)
		sky->Update();

	toolIter tIter = editors.begin();

	for (; tIter != editors.end(); tIter++)
	{
		tIter->second->Update();
	}

	gameObjIter oIter = gameObjects.begin();

	UINT temp = 0;
	for (; oIter != gameObjects.end(); oIter++, temp++)
	{
		if (Mouse::Get()->Up(2))
		{
			if (oIter->second->IsPicked())
			{
				curToolType = EditorType::OBJECTTOOL;
				editors[curToolType]->SetGameObjectPtr(oIter->second);
				selection_mask = (1 << temp);

				break;
			}
		}
	}

	shadow->Update();
}

void EditorManager::PreRender()
{
	if (sky != NULL)
		sky->PreRender();
	shadow->PreRender();
}

void EditorManager::Render()
{
	if (sky != NULL)
		sky->Render();
	shadow->Render();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				Reset();
				Open();
			}
			if (ImGui::MenuItem("Save"))
			{
				Save();
			}
			if (ImGui::MenuItem("Exit"))
			{
				PostQuitMessage(0);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::MenuItem("Style", NULL, &MyGui::show_app_style);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::MenuItem("Light", NULL, &MyGui::show_app_light);
			ImGui::MenuItem("Camera", NULL, &MyGui::show_app_camera);
			ImGui::MenuItem("Inspector", NULL, &MyGui::show_app_inspector);
			ImGui::MenuItem("Hierarchy", NULL, &MyGui::show_app_hierarchy);
			ImGui::MenuItem("Debug Log", NULL, &MyGui::show_app_debugLog);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("help", NULL, &MyGui::show_app_help);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	//ImGui::ShowTestWindow();

	if (MyGui::show_app_help) { MyGui::ShowUserGuide(); }
	if (MyGui::show_app_style) { MyGui::ShowStyleEdit(); }
	if (MyGui::show_app_camera) { MyGui::ShowCameraSetting(Context::Get()); }
	if (MyGui::show_app_light) { MyGui::ShowLightSetting(Context::Get()); }
	if (MyGui::show_app_debugLog) { MyGui::DrawLog(); }

	if (MyGui::show_app_hierarchy) { RenderHeirarchy(); RenderProject(); }
	if (MyGui::show_app_inspector) { RenderInspector(); }
	if (MyGui::show_app_animation) { RenderAnimation(); }
}

void EditorManager::PostRender()
{
}

void EditorManager::Reset()
{
	objNames.clear();
	objNames.shrink_to_fit();
	gameObjIter objIter = gameObjects.begin();
	for (; objIter != gameObjects.end(); objIter++)
	{
		objIter->second->Delete();
	}
	gameObjects.clear();

	shadow->RemoveAll();
}

void EditorManager::Open()
{
	GameSceneSpec * sceneSpec = (GameSceneSpec*)GameDataSpecManager::Get()->LoadScene(L"Scene.spec");

	sky = new ScatterSky();
	sky->Load(sceneSpec->SkySpec);
	sky->Initialize();
	sky->Name() = L"Sky";
	sky->Tag() = L"Sky";

	Terrain::InitDesc desc;
	Terrain * terrain = new Terrain(desc);
	terrain->Initialize(sky);
	terrain->Load(sceneSpec->TerrainSpec);
	terrain->Name() = L"Terrain";
	terrain->Tag() = L"Terrain";

	terrain->Ready();
	sky->Ready();

	objNames.push_back(String::ToString(sky->Name()));
	objNames.push_back(String::ToString(terrain->Name()));

	gameObjects.insert(make_pair(sky->Name(), sky));
	gameObjects.insert(make_pair(terrain->Name(), (IGameObject*)terrain));

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
}

void EditorManager::Save()
{
	GameSceneSpec * spec = new GameSceneSpec();

	gameObjIter objIter = gameObjects.begin();
	for (; objIter != gameObjects.end(); objIter++)
	{
		objIter->second->Save();

		if (objIter->second->Tag() == L"Sky")
			spec->SkySpec = objIter->second->Name() + L".spec";
		else if (objIter->second->Tag() == L"Terrain")
			spec->TerrainSpec = objIter->second->Name() + L".spec";
		else if (objIter->second->Tag() == L"Player")
		{
			spec->PlayerSpec.Name = objIter->second->FileName() + L".spec";
			objIter->second->Position(&spec->PlayerSpec.Position);
			objIter->second->Rotation(&spec->PlayerSpec.Rotation);
			objIter->second->Scale(&spec->PlayerSpec.Scale);
		}
		else if (objIter->second->Tag() == L"Enemy")
		{
			spec->EnemyCount++;
			GameSceneSpec::Spec enemySpec;
			enemySpec.Name = objIter->second->FileName() + L".spec";
			objIter->second->Position(&enemySpec.Position);
			objIter->second->Rotation(&enemySpec.Rotation);
			objIter->second->Scale(&enemySpec.Scale);
			spec->EnemySpecs.push_back(enemySpec);

		}
		else
		{
			spec->ObjectCount++;
			GameSceneSpec::Spec objSpec;
			objSpec.Name = objIter->second->FileName() + L".spec";
			objIter->second->Position(&objSpec.Position);
			objIter->second->Rotation(&objSpec.Rotation);
			objIter->second->Scale(&objSpec.Scale);
			spec->ObjectSpecs.push_back(objSpec);
		}
	}

	GameDataSpecManager::Get()->WriteScene(L"Scene", spec);
	SAFE_DELETE(spec);
}

void EditorManager::CreateTerrain(wstring fileName)
{
	Terrain::InitDesc desc;
	desc.HeightMapInit(fileName, 50.0f);

	vector<wstring> layerMaps;
	layerMaps.push_back(Textures + L"Dirt2.png");
	layerMaps.push_back(Textures + L"stone.png");
	layerMaps.push_back(Textures + L"snow.png");

	desc.BlendMapInit(layerMaps);

	Terrain * terrain = new Terrain(desc);
	terrain->Initialize(sky);
	terrain->Name() = L"Terrain";

	string temp = String::ToString(terrain->Name());
	UINT count = 0;
	for (string name : objNames)
	{
		if (name == temp)
			count++;
	}

	if (count > 0)
		terrain->Name() = L"Terrain" + wstring(L" (") + to_wstring(terrainCount) + wstring(L")");
	else
		terrain->Name() = L"Terrain";

	terrain->Tag() = L"Terrain";
	terrain->Ready();

	gameObjects.insert(make_pair(terrain->Name(), (IGameObject*)terrain));
	objNames.push_back(String::ToString(terrain->Name()));
	terrainCount++;

	terrain->SetShadows(shadow);
}

void EditorManager::RenderHeirarchy()
{
	ImGui::Begin("Hierarchy", &MyGui::show_app_hierarchy, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar);
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Terrain"))
				{
					Path::OpenFileDialog(bind(&EditorManager::CreateTerrain, this, placeholders::_1));
				}
				if (ImGui::BeginMenu("Object"))
				{
					if (ImGui::MenuItem("Open"))
					{
						Path::OpenFileDialog(bind(&EditorManager::LoadObject, this, placeholders::_1), Datas + L"Character/");
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Mesh"))
				{
					if (ImGui::MenuItem("Cube")) { LoadMesh(0); }
					if (ImGui::MenuItem("Sphere")) { LoadMesh(1); }
					if (ImGui::MenuItem("Plane")) { LoadMesh(2); }
					if (ImGui::MenuItem("Quad")) { LoadMesh(3); }
					if (ImGui::MenuItem("Cylinder")) { LoadMesh(4); }

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Text("Hierarchy");

		//	Heirarchy Filter
		if (filter.Draw())
		{
			ImGui::Text("");
		}

		//	게임 오브젝트의 이름 나열
		int node_clicked = -1;

		for (size_t i = 0; i < objNames.size(); i++)
		{
			//	이름들의 필터(검색 기능)
			if (filter.PassFilter(objNames[i].c_str()))
			{
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << i)) ? ImGuiTreeNodeFlags_Selected : 0);
				bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, objNames[i].c_str(), i);

				if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
					node_clicked = i;

				if (node_open == true)
					ImGui::TreePop();

				//	우 클릭 했을 때
				if (ImGui::BeginPopupContextItem(string("##" + to_string(i)).c_str()))
				{
					char str[20];
					strcpy(str, objNames[i].c_str());

					if (ImGui::Selectable("Delete"))
					{
						gameObjIter iter = gameObjects.begin();
						for (; iter != gameObjects.end(); )
						{
							if ((*iter).second == gameObjects[String::ToWString(objNames[i])])
							{
								if (objNames[i].find("Terrain") != string::npos)
								{
									Terrain * terrain = (Terrain*)((*iter).second);

									shadow->Remove((i - 1) + 2);
									shadow->Remove((i - 1) + 1);
									shadow->Remove(i - 1);
								}
								else
									shadow->Remove(i + 1);

								GameModel * model = dynamic_cast<GameModel*>((*iter).second);
								GameAnimator * anim = dynamic_cast<GameAnimator*>((*iter).second);

								if (anim != NULL)
								{
									InstanceManager::Get()->DeleteModel
									(
										model, model->InstanceID(),
										InstanceManager::InstanceType::ANIMATION
									);
								}
								else if (model != NULL)
								{
									InstanceManager::Get()->DeleteModel
									(
										model, model->InstanceID(), 
										InstanceManager::InstanceType::MODEL
									);
								}

								(*iter).second->Delete();
								iter = gameObjects.erase(iter);

								curToolType = EditorType::NONE;

								break;
							}
							else
								iter++;
						}

						objNames.erase(objNames.begin() + i);
					}
					ImGui::PushItemWidth(-1);
					ImGui::Text("Name");
					ImGui::PushItemWidth(100);

					if (ImGui::InputText("", str, 20))
					{
						if (Keyboard::Get()->Down(VK_RETURN))
						{
							gameObjIter iter = gameObjects.begin();
							for (; iter != gameObjects.end(); )
							{
								if ((*iter).second == gameObjects[String::ToWString(objNames[i])])
								{
									(*iter).second->Name() = String::ToWString(str);
									gameObjects.insert(make_pair(String::ToWString(str), (*iter).second));
									iter = gameObjects.erase(iter);
									break;
								}
								else
									iter++;
							}

							objNames[i] = str;
						}
					}

					ImGui::PopItemWidth();
					ImGui::EndPopup();
				}
			}
		}

		if (node_clicked != -1)
		{
			if (objNames[node_clicked] == "Sky")
			{
				curToolType = EditorType::SKYTOOL;
			}
			else if (objNames[node_clicked].find("Terrain") != string::npos)
			{
				curToolType = EditorType::TERRAINTOOL;
				IGameObject * object = gameObjects[String::ToWString(objNames[node_clicked])];
				editors[curToolType]->SetGameObjectPtr(object);
				editors[curToolType]->SetType(0);
			}
			else
			{
				curToolType = EditorType::OBJECTTOOL;
				IGameObject * object = gameObjects[String::ToWString(objNames[node_clicked])];
				editors[curToolType]->SetGameObjectPtr(object);
			}

			selection_mask = (1 << node_clicked);
		}
	}
	ImGui::End();
}

void EditorManager::RenderProject()
{
	
}

void EditorManager::RenderInspector()
{
	ImGui::Begin("Inspector", &MyGui::show_app_inspector, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);
	{
		if (curToolType == EditorType::SKYTOOL)
			MyGui::ShowSkySetting(sky);
		else if (curToolType != EditorType::NONE)
			editors[curToolType]->InspectorRender();
	}
	ImGui::End();
}

void EditorManager::RenderAnimation()
{
	
}

void EditorManager::LoadObject(wstring fileName)
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
			count, modelEffect,
			filePath + spec->Name + L".material",
			filePath + spec->Name + L".mesh"
		);
		model->Ready();
		break;
	case ModelType::EGameAnimator:
		model = new GameAnimator
		(
			count, animEffect,
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

	
	if(count > 0)
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

void EditorManager::LoadObjectTransform(wstring fileName, D3DXVECTOR3 & p, D3DXVECTOR3 & r, D3DXVECTOR3 & s)
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
			count, modelEffect,
			filePath + spec->Name + L".material",
			filePath + spec->Name + L".mesh"
		);
		model->Ready();
		
		break;
	case ModelType::EGameAnimator:
		model = new GameAnimator
		(
			count, animEffect,
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
				if(colliderSphere->Name().find(L"Skill") != wstring::npos)
					CollisionContext::Get()->GetLayer(L"Collision Weapon")->AddCollider(colliderSphere);
			}
			else if (model->Tag() == L"Enemy")
			{
				if(colliderSphere->Name().find(L"RightHand") != wstring::npos)
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

void EditorManager::LoadMesh(UINT type)
{
	Mesh * mesh = NULL;

	switch (type)
	{
		case 0:
		{
			mesh = new MeshCube(meshMaterial, 1, 1, 1);
			mesh->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
			mesh->Position(0, 0, 0);
			if (cubeCount > 0)
				mesh->Name() = L"Cube" + wstring(L" (") + to_wstring(cubeCount) + wstring(L")");
			else
				mesh->Name() = L"Cube";
			mesh->Tag() = L"Object";

			cubeCount++;
			
			break;
		}
		case 1:
		{
			mesh = new MeshSphere(meshMaterial, 1);
			mesh->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
			mesh->Position(0, 0, 0);
			if (sphereCount > 0)
				mesh->Name() = L"Sphere" + wstring(L" (") + to_wstring(sphereCount) + wstring(L")");
			else
				mesh->Name() = L"Sphere";
			mesh->Tag() = L"Object";

			sphereCount++;

			break;
		}
		case 2:
		{
			mesh = new MeshGrid(meshMaterial, 1, 1, 1, 1);
			mesh->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
			mesh->Position(0, 0, 0);
			if (sphereCount > 0)
				mesh->Name() = L"Plane" + wstring(L" (") + to_wstring(planeCount) + wstring(L")");
			else
				mesh->Name() = L"Plane";
			mesh->Tag() = L"Object";

			planeCount++;

			break;
		}
		case 3:
		{
			mesh = new MeshQuad(meshMaterial);
			mesh->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
			mesh->Position(0, 0, 0);
			if (sphereCount > 0)
				mesh->Name() = L"Quad" + wstring(L" (") + to_wstring(quadCount) + wstring(L")");
			else
				mesh->Name() = L"Quad";
			mesh->Tag() = L"Object";

			quadCount++;

			break;
		}
		case 4:
		{
			mesh = new MeshCylinder(meshMaterial, 1, 1, 1);
			mesh->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
			mesh->Position(0, 0, 0);
			if (sphereCount > 0)
				mesh->Name() = L"Cylinder" + wstring(L" (") + to_wstring(cylCount) + wstring(L")");
			else
				mesh->Name() = L"Cylinder";
			mesh->Tag() = L"Object";

			cylCount++;
			
			break;
		}
	}
	
	if (mesh == NULL) return;

	gameObjects.insert(make_pair(mesh->Name(), (IGameObject*)mesh));
	objNames.push_back(String::ToString(mesh->Name()));
	shadow->Add(mesh);
}
