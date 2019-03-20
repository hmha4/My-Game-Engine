#include "stdafx.h"
#include "EditorManager.h"
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
#include "Draw/Gizmo.h"

#include "Utilities/Xml.h"

EditorManager::EditorManager()
	: sky(NULL), shadow(NULL), gizmo(NULL)
	, terrainCount(0), cubeCount(0), sphereCount(0)
	, planeCount(0), quadCount(0), cylCount(0)
{
}

EditorManager::~EditorManager()
{
	SAFE_DELETE(meshMaterial);

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
	SAFE_DELETE(gizmo);
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

	curToolType = EditorType::NONE;

	shadow = new Shadow(2048, 2048);
	shadow->Initialize();

	D3DXMATRIX temp;
	gizmo = new Gizmo(temp);
	gizmo->Initialize();

	MyGui::show_app_inspector = true;
	MyGui::show_app_debugLog = true;
	MyGui::show_app_hierarchy = true;
	MyGui::show_app_animation = true;
}

void EditorManager::Ready()
{
	meshMaterial = new Material(Effects + L"013_Mesh&Model.fx");

	gizmo->Ready();

	sky->Ready();

	toolIter tIter = editors.begin();

	for (; tIter != editors.end(); tIter++)
		tIter->second->Ready();

	shadow->Ready();
}

void EditorManager::Update()
{
	sky->Update();

	toolIter tIter = editors.begin();

	for (; tIter != editors.end(); tIter++)
	{
		tIter->second->Update();
	}

	shadow->Update();
	gizmo->Update();
}

void EditorManager::PreRender()
{
	sky->PreRender();
	shadow->PreRender();
}

void EditorManager::Render()
{
	sky->Render();
	shadow->Render();
	gizmo->Render();

	MyGui::ShowDebugOverlay();

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
				GameSceneSpec * spec = new GameSceneSpec();

				gameObjIter objIter = gameObjects.begin();
				for (; objIter != gameObjects.end(); objIter++)
				{
					objIter->second->Save();

					if (objIter->second->Tag() == L"Sky")
						spec->SkySpec = objIter->second->Name() + L".spec";
					if (objIter->second->Tag() == L"Terrain")
						spec->TerrainSpec = objIter->second->Name() + L".spec";
					if (objIter->second->Tag() == L"Player")
						spec->PlayerSpec = objIter->second->Name() + L".spec";
					if (objIter->second->Tag() == L"Enemy")
					{
						spec->EnemyCount++;
						spec->EnemySpecs.push_back(objIter->second->Name() + L".spec");
					}
					if (objIter->second->Tag() == L"Object")
					{
						spec->ObjectCount++;
						spec->ObjectSpecs.push_back(objIter->second->Name() + L".spec");
					}
				}

				GameDataSpecManager::Get()->WriteScene(L"Scene", spec);
				SAFE_DELETE(spec);
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
			//ImGui::MenuItem("Sky", NULL, &MyGui::show_app_sky);
			ImGui::MenuItem("Light", NULL, &MyGui::show_app_light);
			ImGui::MenuItem("Camera", NULL, &MyGui::show_app_camera);
			ImGui::MenuItem("Inspector", NULL, &MyGui::show_app_inspector);
			ImGui::MenuItem("Hierarchy", NULL, &MyGui::show_app_hierarchy);
			ImGui::MenuItem("Animation", NULL, &MyGui::show_app_animation);
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
	//if (MyGui::show_app_sky) { MyGui::ShowSkySetting(sky); }
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
}

void EditorManager::CreateTerrain(wstring fileName)
{
	Terrain::InitDesc desc;
	desc.HeightMapInit(fileName, 50.0f);

	vector<wstring> layerMaps;
	layerMaps.push_back(Textures + L"GrassNDeadLeafs.png");
	layerMaps.push_back(Textures + L"GroundNWeeds.png");
	layerMaps.push_back(Textures + L"GrassNRock.png");

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
	//shadow->Add(terrain->GetGrass());
	//shadow->Add(terrain);
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
				if (ImGui::BeginMenu("Mesh"))
				{
					if (ImGui::MenuItem("Cube"))
					{
						MeshCube * cube = new MeshCube(meshMaterial, 1, 1, 1);
						cube->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
						cube->Position(0, 0, 0);
						if (cubeCount > 0)
							cube->Name() = L"Cube" + wstring(L" (") + to_wstring(cubeCount) + wstring(L")");
						else
							cube->Name() = L"Cube";
						cube->Tag() = L"Object";

						gameObjects.insert(make_pair(cube->Name(), (IGameObject*)cube));
						objNames.push_back(String::ToString(cube->Name()));
						shadow->Add(cube);

						cubeCount++;
					}

					if (ImGui::MenuItem("Sphere"))
					{
						MeshSphere * sphere = new MeshSphere(meshMaterial, 1);
						sphere->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
						sphere->Position(0, 0, 0);
						if (sphereCount > 0)
							sphere->Name() = L"Sphere" + wstring(L" (") + to_wstring(sphereCount) + wstring(L")");
						else
							sphere->Name() = L"Sphere";
						sphere->Tag() = L"Object";

						gameObjects.insert(make_pair(sphere->Name(), (IGameObject*)sphere));
						objNames.push_back(String::ToString(sphere->Name()));
						shadow->Add(sphere);

						sphereCount++;
					}

					if (ImGui::MenuItem("Plane"))
					{
						MeshGrid * grid = new MeshGrid(meshMaterial, 1, 1, 1, 1);
						grid->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
						grid->Position(0, 0, 0);
						if (sphereCount > 0)
							grid->Name() = L"Plane" + wstring(L" (") + to_wstring(planeCount) + wstring(L")");
						else
							grid->Name() = L"Plane";
						grid->Tag() = L"Object";

						gameObjects.insert(make_pair(grid->Name(), (IGameObject*)grid));
						objNames.push_back(String::ToString(grid->Name()));
						shadow->Add(grid);

						planeCount++;
					}

					if (ImGui::MenuItem("Quad"))
					{
						MeshQuad * quad = new MeshQuad(meshMaterial);
						quad->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
						quad->Position(0, 0, 0);
						if (sphereCount > 0)
							quad->Name() = L"Quad" + wstring(L" (") + to_wstring(quadCount) + wstring(L")");
						else
							quad->Name() = L"Quad";
						quad->Tag() = L"Object";

						gameObjects.insert(make_pair(quad->Name(), (IGameObject*)quad));
						objNames.push_back(String::ToString(quad->Name()));
						shadow->Add(quad);

						quadCount++;
					}

					if (ImGui::MenuItem("Cylinder"))
					{
						MeshCylinder * cyl = new MeshCylinder(meshMaterial, 1, 1, 1);
						cyl->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
						cyl->Position(0, 0, 0);
						if (sphereCount > 0)
							cyl->Name() = L"Cylinder" + wstring(L" (") + to_wstring(cylCount) + wstring(L")");
						else
							cyl->Name() = L"Cylinder";
						cyl->Tag() = L"Object";

						gameObjects.insert(make_pair(cyl->Name(), (IGameObject*)cyl));
						objNames.push_back(String::ToString(cyl->Name()));
						shadow->Add(cyl);

						cylCount++;
					}

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
		for (size_t i = 0; i < objNames.size(); i++)
		{
			//	이름들의 필터(검색 기능)
			if (filter.PassFilter(objNames[i].c_str()))
			{
				if (ImGui::TreeNode(objNames[i].c_str()))
				{
					if (objNames[i] == "Sky")
					{
						curToolType = EditorType::SKYTOOL;
					}
					else if (objNames[i].find("Terrain") != string::npos)
					{
						curToolType = EditorType::TERRAINTOOL;
						editors[curToolType]->SetGameObjectPtr((IGameObject*)gameObjects[String::ToWString(objNames[i])]);
						editors[curToolType]->SetType(0);
					}
					else
					{
						curToolType = EditorType::NONE;
						gizmo->SetSelectionPool((IGameObject*)gameObjects[String::ToWString(objNames[i])]);
						gameObjects[String::ToWString(objNames[i])]->SetPickState(true);
					}

					ImGui::TreePop();
				}
				//	우 클릭 했을 때
				else if (ImGui::BeginPopupContextItem(string("##" + to_string(i)).c_str()))
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
									shadow->Remove(i);

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
	}
	ImGui::End();
}

void EditorManager::RenderProject()
{
	ImGui::Begin("Project", &MyGui::show_app_hierarchy, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::BeginMenu("Export"))
				{
					if (ImGui::MenuItem("Model", "CTR + F"))
					{
						//OpenFBXFile();
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Open"))
				{
					if (ImGui::MenuItem("Open Mesh", "CTR + M"))
					{
						//OpenMeshFile();
					}
					if (ImGui::MenuItem("Open Material", "CTR + ALT + M"))
					{
						//OpenMaterialFile();
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Text("Assets");

		if (ImGui::CollapsingHeader("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
		{
		}
		if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
		{
		}
		if (ImGui::CollapsingHeader("Animations", ImGuiTreeNodeFlags_DefaultOpen))
		{
		}
	}
	ImGui::End();
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
	ImGui::Begin("Animation", &MyGui::show_app_animation, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File##1"))
			{
				if (ImGui::BeginMenu("Export"))
				{
					if (ImGui::MenuItem("Anim", "CTR + A"))
					{
						//if (currentObject == NULL)
						//{
						//	MyGui::show_app_modal = true;
						//	errorMessage = "There is no model to skin animation";
						//}
						//else
						//	OpenAnimFile();
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Load"))
				{
					if (ImGui::MenuItem("Clip", "CTR + C"))
					{
						//if (currentObject == NULL)
						//{
						//	MyGui::show_app_modal = true;
						//	errorMessage = "There is no model to skin animation";
						//}
						//else
						//	OpenClipFile();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
}