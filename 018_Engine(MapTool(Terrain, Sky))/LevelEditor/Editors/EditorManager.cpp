#include "stdafx.h"
#include "EditorManager.h"
#include "Environments/Terrain.h"
#include "Environments/TerrainRenderer.h"
#include "Environments/HeightMap.h"
#include "Objects/IGameObject.h"
#include "Environments/Shadow.h"
#include "Environments/ScatterSky.h"

#include "Utilities/Xml.h"

EditorManager::EditorManager()
	: sky(NULL), shadow(NULL)
{
	
}

EditorManager::~EditorManager()
{
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

	curToolType = EditorType::NONE;

	shadow = new Shadow(2048, 2048);
	shadow->Initialize();

	MyGui::show_app_inspector = true;
	MyGui::show_app_debugLog = true;
	MyGui::show_app_hierarchy = true;
	MyGui::show_app_animation = true;
}

void EditorManager::Ready()
{
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

	MyGui::ShowDebugOverlay();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{

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

void EditorManager::CreateTerrain(wstring fileName)
{
	static UINT terrainCount = 0;

	Terrain::InitDesc desc;
	desc.HeightMapInit(fileName, 60.0f);

	vector<wstring> layerMaps;
	layerMaps.push_back(Textures + L"darkdirt.png");
	layerMaps.push_back(Textures + L"stone.png");
	layerMaps.push_back(Textures + L"snow.png");

	desc.BlendMapInit(layerMaps);

	Terrain * terrain = new Terrain(desc);
	terrain->Initialize(sky);
	if (terrainCount > 0)
		terrain->Name() = L"Terrain" + wstring(L" (") + to_wstring(terrainCount) + wstring(L")");
	else
		terrain->Name() = L"Terrain";

	terrain->Tag() = L"Terrain";
	terrain->Ready();

	gameObjects.insert(make_pair(terrain->Name(), (IGameObject*)terrain));
	objNames.push_back(String::ToString(terrain->Name()));
	terrainCount++;

	shadow->Add(terrain);
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
								(*iter).second->Delete();
								iter = gameObjects.erase(iter);

								shadow->Remove(i + 1);

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
	ImGui::Begin("Animation", &MyGui::show_app_animation, ImGuiWindowFlags_MenuBar/* | ImGuiWindowFlags_NoMove*/);
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
