#include "stdafx.h"
#include "ModelTool.h"
#include "Fbx/FbxLoader.h"
#include "Environments/Shadow.h"
#include "Collider/ColliderBox.h"
#include "Collider/ColliderSphere.h"

#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"

#include "Draw/GizmoGrid.h"
#include "Module/TrailRenderer.h"
#include "Module/MyBehaviourTree.h"

ModelTool::ModelTool()
	: shadow(NULL), grid(NULL), currentBT(NULL)
	, modelEffect(NULL), animEffect(NULL), lineEffect(NULL), trailEffect(NULL)
	, currGameModel(NULL), currObject(NULL), prevObject(NULL), model(NULL)
	, boneIndex(-1), meshIndex(-1), materialIndex(-1)
	, showBones(false), showBoneInit(false)
	, pickColor(1, 1, 1, 1)
{
	selection_mask = (1 << 2);
}

ModelTool::~ModelTool()
{
	SAFE_DELETE(playButton);
	SAFE_DELETE(pauseButton);
	SAFE_DELETE(stopButton);

	SAFE_DELETE(grid);
	SAFE_DELETE(shadow);
	SAFE_DELETE(modelEffect);
	SAFE_DELETE(animEffect);
	SAFE_DELETE(lineEffect);
	SAFE_DELETE(trailEffect);

	for (DebugLine * line : showBone)
		SAFE_DELETE(line);

	for (Texture * texture : materials)
		SAFE_DELETE(texture);

	for (IGameObject * obj : models)
		obj->Delete();

	for (pair<wstring, IGameObject *> obj : objects)
		obj.second->Delete();

	for (pair<wstring, MyBehaviourTree *> bt : BTs)
		SAFE_DELETE(bt.second);
}

void ModelTool::Initialize()
{
	MyGui::show_app_inspector = true;
	MyGui::show_app_debugLog = true;
	MyGui::show_app_hierarchy = true;
	MyGui::show_app_animation = true;

	currentClipName = "";
	target = TARGET::DEFAULT;
}

void ModelTool::Ready()
{
	modelEffect = new Effect(Effects + L"032_ModelInstance.fx");
	animEffect = new Effect(Effects + L"033_AnimInstance.fx");
	playButton = new Texture(Textures + L"PlayButton.png");
	pauseButton = new Texture(Textures + L"PauseButton.png");
	stopButton = new Texture(Textures + L"StopButton.png");

	lineEffect = new Effect(Effects + L"002_Line.fx");
	trailEffect = new Effect(Effects + L"031_TrailRender.fx");

	grid = new GizmoGrid(1);
	grid->Initialize();
	grid->Ready();
}

void ModelTool::Update()
{
	if (currGameModel == NULL)
		return;

	//	Current Model Update
	currGameModel->Update();

	//	Child Model Update
	for (string name : objNames)
	{
		IGameObject * obj = objects[String::ToWString(name)];
		GameModel * child = dynamic_cast<GameModel*>(obj);

		if (child->ParentName() == currObject->Name())
			child->Update();
	}
}

void ModelTool::PreRender()
{
}

void ModelTool::Render()
{
	grid->Render();

	if (currGameModel != NULL)
	{
		//	Current Model Render
		InstanceManager::Get()->Render(model->GetModelName());
		currGameModel->Render();
		//	Current Model Colliders Render
		RenderColliderBox(currGameModel);
		//	Current Model Bones Render
		if (showBones == true)
			ShowBoneWorld();

		//	Child Model Render
		for (string name : objNames)
		{
			IGameObject * obj = objects[String::ToWString(name)];
			GameModel * child = dynamic_cast<GameModel*>(obj);

			if (child->ParentName() == currObject->Name())
				InstanceManager::Get()->Render(child->GetModel()->GetModelName());
		}
	}

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
			ImGui::MenuItem("Animation", NULL, &MyGui::show_app_animation);
			ImGui::MenuItem("Behaviour", NULL, &MyGui::show_app_behaviour);
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
	//ImGui::ShowDemoWindow();

	if (MyGui::show_app_help) { MyGui::ShowUserGuide(); }
	if (MyGui::show_app_style) { MyGui::ShowStyleEdit(); }
	if (MyGui::show_app_camera) { MyGui::ShowCameraSetting(Context::Get()); }
	if (MyGui::show_app_light) { MyGui::ShowLightSetting(Context::Get()); }
	if (MyGui::show_app_debugLog) { MyGui::DrawLog(); }

	if (MyGui::show_app_hierarchy) { RenderHeirarchy(); RenderProject(); }
	if (MyGui::show_app_inspector) { RenderInspector(); }
	if (MyGui::show_app_animation) { RenderAnimation(); }
	if (MyGui::show_app_behaviour)
	{
		if (currGameModel != NULL)
			currentBT->ImguiRender();
	}

	//	error pop-up
	if (MyGui::show_app_modal == true)
	{
		ImGui::OpenPopup("Error");
		MyGui::show_app_modal = false;
	}
	MyGui::OpenPopupModal(errorMessage.c_str());

	if (currObject != NULL)
		ShowBoneName(boneIndex);
}

void ModelTool::PostRender()
{
}

void ModelTool::Delete()
{
}

void ModelTool::Reset()
{
	currentClipName = "";
	boneIndex = -1;
	meshIndex = -1;
	materialIndex = -1;
	showBones = false;
	showBoneInit = false;
	target = TARGET::DEFAULT;
}

void ModelTool::RenderHeirarchy()
{
	ImGui::Begin("Hierarchy", &MyGui::show_app_hierarchy, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save"))
				{
					Save();
				}
				if (ImGui::MenuItem("Load"))
				{
					Path::OpenFileDialog(bind(&ModelTool::Load, this, std::placeholders::_1), Datas + L"Character/");
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		//	Hierarchy Filter
		if (filter.Draw())
		{
		}
		else if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MeshPayload"))
			{
				IM_ASSERT(payload->DataSize == size);
				IGameObject * data = (GameModel*)payload->Data;;

				wstring modelName = data->Name();
				UINT count = 0;
				for (string name : objNames)
				{
					if (name.find(String::ToString(modelName)) != string::npos)
						count++;
				}

				if (count > 0)
					modelName = modelName + L" (" + to_wstring(count) + L")";

				IGameObject * object = NULL;

				data->Clone((void**)&object, data->Name());

				objects.insert(make_pair(modelName, (IGameObject *)object));
				objNames.push_back(String::ToString(modelName));
			}
			ImGui::EndDragDropTarget();
		}

		int node_clicked = -1;
		for (size_t i = 0; i < objNames.size(); i++)
		{
			if (filter.PassFilter(objNames[i].c_str()))
			{
				if (currObject != NULL)
				{
					if (currObject->Name() != String::ToWString(objNames[i]))
						ImGui::SetNextTreeNodeOpen(false);
				}

				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << i)) ? ImGuiTreeNodeFlags_Selected : 0);
				bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, objNames[i].c_str(), i);

				if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
					node_clicked = i;

				if (node_open == true)
				{
					if (currObject != NULL)
					{
						if (ImGui::TreeNode("Bone Datas"))
						{
							if (model->BoneByIndex(1)->ParentIndex() == -1)
								GetBoneName(model->BoneByIndex(1));
							else
								GetBoneName(model->BoneByIndex(0));

							ImGui::TreePop();
						}

						if (ImGui::TreeNode("Mesh Datas"))
						{
							GetMeshData();

							ImGui::TreePop();
						}
						else
							materialIndex = -1;
					}

					ImGui::TreePop();
				}
				else if (ImGui::BeginDragDropSource())
				{
					GameModel * temp1 = dynamic_cast<GameModel*>(objects[String::ToWString(objNames[i])]);

					if (temp1 != NULL)
					{
						size = sizeof(GameModel);
						ImGui::SetDragDropPayload("ChildPayload", temp1, size);
						ImGui::Text(objNames[i].c_str());
					}
					ImGui::EndDragDropSource();
				}
				else if (ImGui::BeginPopupContextItem(string("##" + to_string(i)).c_str()))
				{
					char str[20];
					strcpy(str, objNames[i].c_str());

					if (ImGui::Selectable("Delete"))
					{
						map<wstring, IGameObject*>::iterator iter;
						iter = objects.find(String::ToWString(objNames[i]));

						if (iter != objects.end())
						{
							(*iter).second->Delete();
							iter = objects.erase(iter);
						}

						objNames.erase(objNames.begin() + i);

						currObject = NULL;
						currGameModel = NULL;
						model = NULL;
						Reset();
					}
					ImGui::PushItemWidth(-1);
					ImGui::Text("Name");
					ImGui::PushItemWidth(100);

					if (ImGui::InputText(string("##" + to_string(i)).c_str(), str, 20))
					{
						if (Keyboard::Get()->Down(VK_RETURN))
						{
							map<wstring, IGameObject*>::iterator iter;
							iter = objects.find(String::ToWString(objNames[i]));

							if (iter != objects.end())
							{
								(*iter).second->Name() = String::ToWString(str);
								objects.insert(make_pair(String::ToWString(str), (*iter).second));
								iter = objects.erase(iter);
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
			currObject = objects[String::ToWString(objNames[node_clicked])];
			if (prevObject != currObject)
			{
				model = currObject->GetModel();
				prevObject = currObject;
				currGameModel = dynamic_cast<GameModel*>(currObject);
				currentBT = BTs[currGameModel->BTName()];

				Reset();
			}

			selection_mask = (1 << node_clicked);
		}
	}
	ImGui::End();
}

void ModelTool::RenderProject()
{
	ImGui::Begin("Project", &MyGui::show_app_hierarchy, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::BeginMenu("Export"))
				{
					if (ImGui::BeginMenu("Model"))
					{
						if (ImGui::MenuItem("Bone", "CTR + B"))
							OpenFBXFile(0);
						if (ImGui::MenuItem("Skinned", "CTR + S"))
							OpenFBXFile(1);

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Open"))
				{
					if (ImGui::BeginMenu("Mesh", "CTR + M"))
					{
						if (ImGui::MenuItem("Bone", "CTR + B"))
							OpenMeshFile(0);
						if (ImGui::MenuItem("Skinned", "CTR + S"))
							OpenMeshFile(1);

						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Material", "CTR + ALT + M"))
					{
						OpenMaterialFile();
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Create"))
				{
					if (ImGui::MenuItem("BehaviourTree", "CTR + B"))
					{
						wstring temp = L"Player " + to_wstring(BTs.size());
						BTNames.push_back(String::ToString(temp));
						BTs.insert(make_pair(temp, new MyBehaviourTree()));
						BTs[temp]->Name() = temp;
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Text("Assets");

		if (ImGui::CollapsingHeader("Meshes"))
		{
			for (size_t i = 0; i < models.size(); i++)
			{
				if (ImGui::TreeNode(String::ToString(models[i]->Name()).c_str()))
				{
					ImGui::BulletText("Bone Count : %d", models[i]->GetModel()->BoneCount());
					ImGui::BulletText("Material Count : %d", models[i]->GetModel()->MaterialCount());
					ImGui::BulletText("Mesh Count : %d", models[i]->GetModel()->MeshCount());

					ImGui::TreePop();
				}
				else if (ImGui::BeginPopupContextItem(string("##" + to_string(i)).c_str()))
				{
					char str[20];
					strcpy(str, String::ToString(models[i]->Name()).c_str());

					if (ImGui::Selectable("Delete")) {
						SAFE_DELETE(models[i]);
						models.erase(models.begin() + i);
					}
					ImGui::PushItemWidth(-1);
					ImGui::Text("Name");
					ImGui::PushItemWidth(100);

					if (ImGui::InputText(string("##" + to_string(i)).c_str(), str, 20))
						models[i]->Name() = String::ToWString(str);

					ImGui::PopItemWidth();
					ImGui::EndPopup();
				}
				else if (ImGui::BeginDragDropSource())
				{
					GameModel * temp1 = dynamic_cast<GameModel*>(models[i]);

					if (temp1 != NULL)
					{
						size = sizeof(GameModel);
						ImGui::SetDragDropPayload("MeshPayload", models[i], size);
						ImGui::Text(String::ToString(models[i]->Name()).c_str());
					}
					ImGui::EndDragDropSource();
				}
			}
		}
		if (ImGui::CollapsingHeader("Materials"))
		{
			for (size_t i = 0; i < materials.size(); i++)
			{
				wstring fileName = materials[i]->GetFile();
				string file = String::ToString(Path::GetFileName(fileName));
				if (ImGui::TreeNode(file.c_str()))
				{
					ImGui::Image(materials[i]->SRV(), ImVec2(100, 100));
					ImGui::SameLine();
					ImGui::BeginGroup();
					{
						ImGui::Spacing(); ImGui::Spacing();
						ImGui::Text("Size : %d x %d", materials[i]->GetWidth(), materials[i]->GetHeight()); ImGui::Spacing();
						ImGui::Text("Width : %d", materials[i]->GetWidth()); ImGui::Spacing();
						ImGui::Text("Height : %d", materials[i]->GetHeight()); ImGui::Spacing();
						ImGui::Text("Format : .png");
					}
					ImGui::EndGroup();

					ImGui::TreePop();
				}
				else if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("TexPayload", materials[i], sizeof(Texture));
					ImGui::Text(file.c_str());
					ImGui::EndDragDropSource();
				}
			}
		}
		if (ImGui::CollapsingHeader("Behaviour Trees", ImGuiTreeNodeFlags_DefaultOpen))
		{
			map<wstring, MyBehaviourTree*>::iterator iter;
			for (size_t i = 0; i < BTNames.size(); i++)
			{
				string fileName = BTNames[i];
				if (ImGui::TreeNode(fileName.c_str()))
				{
					ImGui::TreePop();
				}
				else if (ImGui::BeginPopupContextItem(string("##" + fileName).c_str()))
				{
					char str[20];
					strcpy(str, fileName.c_str());

					if (ImGui::Selectable("Delete"))
					{
						iter = BTs.find(String::ToWString(BTNames[i]));

						if (iter != BTs.end())
						{
							SAFE_DELETE((*iter).second);
							BTs.erase(iter);
						}
					}
					ImGui::PushItemWidth(-1);
					ImGui::Text("Name");
					ImGui::PushItemWidth(100);

					if (ImGui::InputText(string("##" + fileName).c_str(), str, 20))
					{
						if (Keyboard::Get()->Down(VK_RETURN))
						{
							iter = BTs.find(String::ToWString(BTNames[i]));

							if (iter != BTs.end())
							{
								(*iter).second->Name() = String::ToWString(str);
								BTs.insert(make_pair(String::ToWString(str), (*iter).second));
								BTs.erase(iter);

								BTNames.erase(BTNames.begin() + i);
								BTNames.push_back(str);
							}
						}
					}

					ImGui::PopItemWidth();
					ImGui::EndPopup();
				}
				else if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("BTPayload", BTs[String::ToWString(BTNames[i])], sizeof(MyBehaviourTree));
					ImGui::Text(fileName.c_str());
					ImGui::EndDragDropSource();
				}
			}
		}
	}
	ImGui::End();
}

void ModelTool::RenderInspector()
{
	ImGui::Begin("Inspector", &MyGui::show_app_inspector, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoMove);
	{
		if (currObject != NULL)
		{
			//	Check to show bones or not
			ImGui::Checkbox("Show Bones", &showBones);
			if (showBones == true && showBoneInit == false)
			{
				InitShowBoneWorld();
				showBoneInit = true;
			}
			else if (showBones == false)
			{
				InitShowBoneWorld();
				showBoneInit = false;
			}

			ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - 100);
			ImGui::PushItemWidth(70);

			//	Writing gameObject's tag name
			char buf[20];
			strcpy(buf, String::ToString(currObject->Tag()).c_str());
			if (ImGui::InputText("Tag", buf, 20))
				currObject->Tag() = String::ToWString(buf);
			ImGui::PopItemWidth();

			switch (target)
			{
			case TARGET::DEFAULT:
			{
				if (currGameModel != NULL)
				{
					if (currGameModel->HasBT() == true)
					{
						ImGui::Spacing();
						ImGui::Spacing();
						if (ImGui::CollapsingHeader("BehaviourTree", ImGuiTreeNodeFlags_DefaultOpen))
						{
							char buf[20];
							strcpy(buf, String::ToString(currGameModel->BTName()).c_str());
							if (ImGui::InputText("Input B Tree", buf, 20))
							{
							}
							else if (ImGui::BeginDragDropTarget())
							{
								//	Filter에 오브젝트가 들어왔을 때
								if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BTPayload"))
								{
									IM_ASSERT(payload->DataSize == sizeof(MyBehaviourTree));
									MyBehaviourTree * data = (MyBehaviourTree*)payload->Data;

									currGameModel->BTName() = data->Name();
									currGameModel->HasBT() = true;

									if (currGameModel != NULL)
										currentBT = BTs[currGameModel->BTName()];
								}
								ImGui::EndDragDropTarget();
							}
						}
					}
				}

				break;
			}
			case TARGET::BONE:
			{
				if (boneIndex != -1)
				{
					ImGui::Separator(); ImGui::Separator();
					ImGui::Text("Bone Inspector");
					ImGui::Separator();
					ImGui::Spacing(); ImGui::Spacing();

					FixBoneData(boneIndex);
					FixColliderBox(boneIndex);
					FixChildObject(boneIndex);
					FixTrail(boneIndex);
				}
			}
			break;

			case TARGET::MESH:
			{
				ImGui::Separator(); ImGui::Separator();
				ImGui::Text("Mesh Inspector");
				ImGui::Separator();
				ImGui::Spacing(); ImGui::Spacing();

				FixMaterialData();
				FixColliderBox(boneIndex);
			}
			break;
			}

			ImGui::Spacing(); ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing(); ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SameLine(ImGui::GetWindowContentRegionWidth() / 2 - 40);

			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("Component");
			if (ImGui::BeginPopup("Component"))
			{
				if (ImGui::BeginMenu("Collider"))
				{
					wstring	boneName;
					switch (target)
					{
					case TARGET::BONE:
						boneName = model->BoneByIndex(boneIndex)->Name();

						if (ImGui::Selectable("Box"))
						{
							InitColliderBox(ColliderElement::EType::EBox, boneName + L" Collider");
							ImGui::CloseCurrentPopup();
						}

						if (ImGui::Selectable("Sphere"))
							InitColliderBox(ColliderElement::EType::ESphere, boneName + L" Collider");

						break;
					case TARGET::MESH:
						if (ImGui::Selectable("Box"))
							InitColliderBox(ColliderElement::EType::EBox, L"Main Collider");
						if (ImGui::Selectable("Sphere"))
							InitColliderBox(ColliderElement::EType::ESphere, L"Main Collider");

						break;
					}
					ImGui::EndMenu();
				}
				if (ImGui::Selectable("Trail"))
				{
					if (target == TARGET::BONE)
						InitTrail();
				}
				if (ImGui::Selectable("Behaviour"))
				{
					if (target == TARGET::DEFAULT)
					{
						GameModel * m = dynamic_cast<GameModel*>(currObject);
						if (m != NULL)
							m->HasBT() = true;
					}
				}

				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();
}

void ModelTool::RenderAnimation()
{
	ImGui::Begin("Animation", &MyGui::show_app_animation, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoMove);
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File##1"))
			{
				if (ImGui::BeginMenu("Export"))
				{
					if (ImGui::MenuItem("FBX", "CTR + F"))
					{
						if (currObject == NULL)
						{
							MyGui::show_app_modal = true;
							errorMessage = "There is no model to skin animation";
						}
						else
							OpenAnimFile();
					}

					if (ImGui::MenuItem("Anim", "CTR + A"))
					{
						if (currObject == NULL)
						{
							MyGui::show_app_modal = true;
							errorMessage = "There is no model to skin animation";
						}
						else
							OpenClipFile();
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (currObject != NULL)
		{
			GameAnimator * anim = dynamic_cast<GameAnimator *>(currObject);
			if (anim != NULL)
			{
				clipNames = anim->ClipNames();
				ImGui::PushItemWidth(160);

				if (ImGui::BeginCombo("Clips", currentClipName.c_str()))
				{
					for (size_t i = 0; i < clipNames.size(); i++)
					{
						bool isSelected = (currentClipName == clipNames[i]);
						if (ImGui::Selectable(clipNames[i].c_str(), isSelected))
							currentClipName = clipNames[i];

						if (isSelected == true)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				float i = ImGui::GetWindowContentRegionWidth();
				ImGui::SameLine((i / 2) - 35);
				if (ImGui::ImageButton(playButton->SRV(), ImVec2(17, 17)))
				{
					anim->Play(currentClipName, true, 0.2f);
					SetTrailRun(true);
				} ImGui::SameLine((i / 2));
				if (ImGui::ImageButton(pauseButton->SRV(), ImVec2(17, 17)))
				{
					anim->Pause();
					SetTrailRun(false);
				}
				ImGui::SameLine((i / 2) + 35);
				if (ImGui::ImageButton(stopButton->SRV(), ImVec2(17, 17)))
				{
					anim->Stop();
					SetTrailRun(false);
				}

				if (currentClipName != "")
					anim->ShowFrameData();
			}
		}
	}
	ImGui::End();
}

void ModelTool::Transform(string transform, ModelBone * bone)
{
	if (ImGui::CollapsingHeader(transform.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		D3DXVECTOR3 pos(0, 0, 0);
		D3DXVECTOR3 rot(0, 0, 0);
		D3DXVECTOR3 scale(1, 1, 1);
		D3DXQUATERNION quaternion(0, 0, 0, 0);

		if (bone != NULL)
		{
			D3DXMatrixDecompose(&scale, &quaternion, &pos, &bone->Root());
			Math::QuatToYawPithRoll(quaternion, rot.x, rot.y, rot.z);
			rot.x = Math::ToDegree(rot.x);
			rot.y = Math::ToDegree(rot.y);
			rot.z = Math::ToDegree(rot.z);
		}
		else
		{
			currObject->Position(&pos);
			currObject->RotationDegree(&rot);
			currObject->Scale(&scale);
		}

		bool b = false;
		b |= ImGui::DragFloat3("Position", (float*)&pos, 0.1f, -200, 200);
		b |= ImGui::DragFloat3("Rotation", (float*)&rot, 0.1f, -180, 180);
		b |= ImGui::DragFloat3("Scale", (float*)&scale, 0.1f, -200, 200);
		if (b == true)
		{
			if (bone != NULL)
			{
				D3DXMATRIX S, R, T;
				D3DXQUATERNION q;
				D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
				D3DXMatrixTranslation(&T, pos.x, pos.y, pos.z);
				D3DXQuaternionRotationYawPitchRoll
				(
					&q,
					Math::ToRadian(rot.y),
					Math::ToRadian(rot.x),
					Math::ToRadian(rot.z)
				);
				D3DXMatrixRotationQuaternion(&R, &q);
				bone->Root(S * R * T);

				GameAnimator * temp1 = dynamic_cast<GameAnimator*>(currObject);
				GameModel * temp2 = dynamic_cast<GameModel*>(currObject);

				if (temp1 != NULL)
					temp1->UpdateBoneWorld();
				else if (temp2 != NULL)
					temp2->UpdateWorld();
			}
			else
			{
				currObject->Position(pos);
				currObject->RotationDegree(rot);
				currObject->Scale(scale);
			}
		}
	}
}

void ModelTool::FixChildObject(int index)
{
	if (index < 0) return;

	ModelBone * bone = model->BoneByIndex(index);

	vector<IGameObject *> object;
	for (string name : objNames)
	{
		GameModel * obj = dynamic_cast<GameModel*>(objects[String::ToWString(name)]);

		if (obj == NULL) continue;
		if (obj->IsChild() == false) continue;
		if (obj->ParentName() != currObject->Name()) continue;
		if (obj->ParentBone()->Name() != bone->Name()) continue;

		object.push_back(obj);
	}

	if (ImGui::CollapsingHeader("ChildObject"))
	{
		for (IGameObject * objs : object)
		{
			GameModel * obj = dynamic_cast<GameModel*>(objs);
			if (obj == NULL) continue;

			string modelName = String::ToString(obj->Name());

			ImGui::Separator();
			ImGui::Text(modelName.c_str());
			ImGui::Separator();
			ImGui::Text((string("Transform##").append(modelName)).c_str());
			ImGui::Spacing();

			D3DXVECTOR3 trans, scale, rot;
			D3DXQUATERNION quat;

			D3DXMatrixDecompose(&scale, &quat, &trans, &obj->RootAxis());
			Math::QuatToYawPithRoll(quat, rot.x, rot.y, rot.z);

			bool check = false;
			check |= ImGui::DragFloat3((string("Position##").append(modelName)).c_str(), (float*)&trans, 0.01f, -100, 100);
			check |= ImGui::DragFloat3((string("Rotation##").append(modelName)).c_str(), (float*)&rot, 0.001f, -Math::PI, Math::PI);
			check |= ImGui::DragFloat3((string("Scale##").append(modelName)).c_str(), (float*)&scale, 0.01f, -10, 10);

			if (check == true)
			{
				D3DXMATRIX S, R, T;
				D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
				D3DXQuaternionRotationYawPitchRoll(&quat, rot.y, rot.x, rot.z);
				D3DXMatrixRotationQuaternion(&R, &quat);
				D3DXMatrixTranslation(&T, trans.x, trans.y, trans.z);

				obj->RootAxis(S * R * T);
				obj->UpdateWorld();
			}

			if (ImGui::Button("Delete"))
			{
				obj->IsChild(false);
				obj->ParentName(L"");
				obj->ParentBone(NULL);
			}
		}
	}
	else if (ImGui::BeginDragDropTarget())
	{
		//	Filter에 오브젝트가 들어왔을 때
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ChildPayload"))
		{
			IM_ASSERT(payload->DataSize == size);
			IGameObject * data = (GameModel*)payload->Data;

			IGameObject * objs = NULL;
			string name = String::ToString(data->Name());
			objs = objects[String::ToWString(name)];

			GameModel * temp = dynamic_cast<GameModel *>(objs);
			if (temp != NULL)
			{
				temp->IsChild(true);
				temp->ParentName(currObject->Name());
				temp->ParentBone(bone);
				temp->ParentModel((GameModel*)currObject);
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (index < 0)
	{
		//	본이 아닐때
	}
	else
	{
		//	본 일때
	}
}

void ModelTool::GetBoneName(ModelBone * bone)
{
	wstring temp = bone->Name();
	string boneName = String::ToString(temp);

	ImGuiTreeNodeFlags nodeFlag = NULL;
	nodeFlag = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	bool b = ImGui::TreeNodeEx(boneName.c_str(), nodeFlag);
	if (b == true)
	{
		boneIndex = bone->Index();
		target = TARGET::BONE;

		for (size_t i = 0; i < bone->ChildCount(); i++)
			GetBoneName(bone->GetChilds()[i]);

		ImGui::TreePop();
	}
}

void ModelTool::ShowBoneName(int index)
{
	if (index < 0) return;
	if (target != TARGET::BONE) return;

	ModelBone * bone = model->BoneByIndex(index);

	D3DXMATRIX view, proj, boneWorld;
	Context::Get()->GetMainCamera()->MatrixView(&view);
	Context::Get()->GetPerspective()->GetMatrix(&proj);
	boneWorld = bone->World();
	D3DXVECTOR3 scale;
	currObject->Scale(&scale);

	D3DXMATRIX S;
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);

	RECT rect;
	rect = DirectWrite::Get()->Get2DPosition(boneWorld * S, view, proj);

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("BCKGND", NULL, ImGui::GetIO().DisplaySize, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
	{
		string str = String::ToString(bone->Name());
		ImGui::GetWindowDrawList()->AddText(ImVec2((float)rect.left, (float)rect.top), ImColor(1.0f, 1.0f, 1.0f, 1.0f), str.c_str());
	}
	ImGui::End();

	//DirectWrite::RenderText(bone->Name().c_str(), rect, 20.0f, L"Roboto-Regular");
}

void ModelTool::InitShowBoneWorld()
{
	for (DebugLine * line : showBone)
		SAFE_DELETE(line);
	showBone.clear();
	showBone.shrink_to_fit();

	if (showBones == false) return;

	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		BSphere * sphere = new BSphere(D3DXVECTOR3(0, 0, 0), 1, 2, 2);
		DebugLine * line = new DebugLine(lineEffect);
		line->Initialize();
		line->Ready();
		line->Color(D3DXCOLOR(1, 0.55f, 0, 1));

		GameModel * obj = dynamic_cast<GameModel *>(currObject);

		D3DXMATRIX W;
		if (obj->Pass() == 1)
			D3DXMatrixIdentity(&W);
		else
			D3DXMatrixScaling(&W, 5, 5, 5);

		line->Draw(W, sphere);
		SAFE_DELETE(sphere);

		showBone.push_back(line);
	}
}

void ModelTool::ShowBoneWorld()
{
	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone * bone = model->BoneByIndex(i);
		showBone[i]->Render(1, &bone->World());
	}

	if (Mouse::Get()->Down(2))
		PickShowBoneWorld();
}

void ModelTool::PickShowBoneWorld()
{
	Ray ray;
	D3DXVECTOR3 camPos;
	Context::Get()->GetMainCamera()->Position(&camPos);

	D3DXMATRIX W, V, P;
	D3DXMatrixIdentity(&W);
	Context::Get()->GetMainCamera()->MatrixView(&V);
	Context::Get()->GetPerspective()->GetMatrix(&P);

	Context::Get()->GetViewport()->GetRay(&ray, camPos, W, V, P);

	GameModel * obj = dynamic_cast<GameModel *>(currObject);
	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone * bone = model->BoneByIndex(i);
		D3DXMATRIX world = bone->World();

		D3DXVECTOR3 position = D3DXVECTOR3(world._41, world._42, world._43);

		BSphere sphere;
		if (obj->Pass() == 1)
			sphere = BSphere(position, 0.01f);
		else
			sphere = BSphere(position, 0.01f * 5);

		float result;
		if (sphere.Intersect(&ray, result))
		{
			boneIndex = i;
			target = TARGET::BONE;
			break;
		}
	}
}

void ModelTool::FixBoneData(int index)
{
	if (index < 0) return;
	if (target != TARGET::BONE) return;

	ModelBone * bone = model->BoneByIndex(index);

	ImGui::Spacing(); ImGui::Spacing();
	ImGui::Text(String::ToString(bone->Name()).c_str());

	//	Fixing Transform of the current bone
	Transform("Transform##01", bone);
}

void ModelTool::GetMeshData()
{
	int meshCount = model->MeshCount();

	for (int i = 0; i < meshCount; i++)
	{
		ModelMesh * mesh = model->Meshes()[i];
		string meshName = String::ToString(mesh->Name());

		if (ImGui::TreeNode(meshName.c_str()))
		{
			target = TARGET::MESH;
			boneIndex = -1;

			meshIndex = i;
			GetMaterialData();

			ImGui::TreePop();
		}
		else if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("IntPayload", &i, sizeof(int));
			ImGui::Text(meshName.c_str());
			ImGui::EndDragDropSource();
		}
		else if (ImGui::BeginDragDropTarget())
		{
			//	Filter에 오브젝트가 들어왔을 때
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IntPayload"))
			{
				IM_ASSERT(payload->DataSize == sizeof(int));
				int data = *(const int*)payload->Data;

				ModelMesh * temp = model->Meshes()[i];
				model->Meshes()[i] = model->Meshes()[data];
				model->Meshes()[data] = temp;
			}
			ImGui::EndDragDropTarget();
		}
	}
}

void ModelTool::GetMaterialData()
{
	ModelMesh * mesh = model->Meshes()[meshIndex];

	int meshPartsCount = mesh->MeshParts().size();

	for (int i = 0; i < meshPartsCount; i++)
	{
		ModelMeshPart * meshParts = mesh->MeshParts()[i];

		//	Diffuse Map
		if (meshParts->GetMaterial()->GetDiffuseMap() != NULL)
		{
			wstring temp = meshParts->GetMaterial()->GetDiffuseMap()->GetFile();
			wstring texture = Path::GetFileName(temp);
			ImGui::BulletText(String::ToString(texture).c_str());
		}
		//	Normal Map
		if (meshParts->GetMaterial()->GetNormalMap() != NULL)
		{
			wstring temp = meshParts->GetMaterial()->GetNormalMap()->GetFile();
			wstring texture = Path::GetFileName(temp);
			ImGui::BulletText(String::ToString(texture).c_str());
		}
		//	Specular Map
		if (meshParts->GetMaterial()->GetSpecularMap() != NULL)
		{
			wstring temp = meshParts->GetMaterial()->GetSpecularMap()->GetFile();
			wstring texture = Path::GetFileName(temp);
			ImGui::BulletText(String::ToString(texture).c_str());
		}
	}
}

void ModelTool::FixMaterialData()
{
	if (meshIndex < 0) return;

	ModelMesh * mesh = model->Meshes()[meshIndex];
	Texture * diffuseTexture = NULL;
	Texture * normalTexture = NULL;
	Texture * specularTexture = NULL;
	Material * material;

	ImGui::Begin("Inspector");
	{
		if (ImGui::CollapsingHeader(String::ToString(mesh->Name() + L" Material").c_str()))
		{
			int meshPartsCount = mesh->MeshParts().size();

			for (int i = 0; i < meshPartsCount; i++)
			{
				ModelMeshPart * meshParts = mesh->MeshParts()[i];

				ImGui::Columns(3);
				ImGui::Text("Diffuse"); ImGui::NextColumn();
				ImGui::Text("Normal"); ImGui::NextColumn();
				ImGui::Text("Specular"); ImGui::NextColumn();
				ImGui::Separator();

				//	Diffuse Map
				material = meshParts->GetMaterial();
				diffuseTexture = material->GetDiffuseMap();

				if (diffuseTexture != NULL)
				{
					if (ImGui::ImageButton(diffuseTexture->SRV(), ImVec2(50, 50)))
						materialIndex = 0;
					else if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TexPayload"))
						{
							IM_ASSERT(payload->DataSize == sizeof(Texture));
							Texture* data = (Texture*)payload->Data;
							material->SetDiffuseMap(data->GetFile());
							diffuseTexture = material->GetDiffuseMap();
						}
						ImGui::EndDragDropTarget();
					}
				}
				else
				{
					if (ImGui::ImageButton(NULL, ImVec2(50, 50)))
					{
					}
					else if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TexPayload"))
						{
							IM_ASSERT(payload->DataSize == sizeof(Texture));
							Texture* data = (Texture*)payload->Data;
							material->SetDiffuseMap(data->GetFile());
							diffuseTexture = material->GetDiffuseMap();
						}
						ImGui::EndDragDropTarget();
					}
				}
				ImGui::NextColumn();

				//	Normal Map
				normalTexture = material->GetNormalMap();
				if (normalTexture != NULL)
				{
					if (ImGui::ImageButton(normalTexture->SRV(), ImVec2(50, 50)))
						materialIndex = 1;
					else if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TexPayload"))
						{
							IM_ASSERT(payload->DataSize == sizeof(Texture));
							Texture* data = (Texture*)payload->Data;
							material->SetNormalMap(data->GetFile());
							normalTexture = material->GetNormalMap();
						}
						ImGui::EndDragDropTarget();
					}
				}
				else
				{
					if (ImGui::ImageButton(NULL, ImVec2(50, 50)))
					{
					}
					else if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TexPayload"))
						{
							IM_ASSERT(payload->DataSize == sizeof(Texture));
							Texture* data = (Texture*)payload->Data;
							material->SetNormalMap(data->GetFile());
							normalTexture = material->GetNormalMap();
						}
						ImGui::EndDragDropTarget();
					}
				}
				ImGui::NextColumn();

				//	Specular Map
				specularTexture = material->GetSpecularMap();
				if (specularTexture != NULL)
				{
					if (ImGui::ImageButton(specularTexture->SRV(), ImVec2(50, 50)))
						materialIndex = 2;
					else if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TexPayload"))
						{
							IM_ASSERT(payload->DataSize == sizeof(Texture));
							Texture* data = (Texture*)payload->Data;
							material->SetSpecularMap(data->GetFile());
							specularTexture = material->GetSpecularMap();
						}
						ImGui::EndDragDropTarget();
					}
				}
				else
				{
					if (ImGui::ImageButton(NULL, ImVec2(50, 50)))
					{
					}
					else if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TexPayload"))
						{
							IM_ASSERT(payload->DataSize == sizeof(Texture));
							Texture* data = (Texture*)payload->Data;
							material->SetSpecularMap(data->GetFile());
							specularTexture = material->GetSpecularMap();
						}
						ImGui::EndDragDropTarget();
					}
				}
			}
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::Spacing(); ImGui::Spacing();

			//if (ImGui::Button("Shaders"))
			//	ImGui::OpenPopup("shader");
			//ImGui::SameLine();
			//wstring temp = currentObject->ModelShader->GetFile();
			//wstring temp2 = Path::GetFileName(temp);
			//string fileName = String::ToString(temp2);
			//ImGui::Text(fileName.c_str());
			//ImGui::Spacing(); ImGui::Spacing();
			//ImGui::Separator();
			//ImGui::Spacing(); ImGui::Spacing();
			//if (ImGui::BeginPopup("shader"))
			//{
			//	ImGui::Text(fileName.c_str());
			//	ImGui::SameLine();
			//	if (ImGui::Button("Change"))
			//		OpenShaderFile();
			//
			//	ImGui::EndPopup();
			//}

			float width = ImGui::GetWindowContentRegionWidth();
			switch (materialIndex)
			{
			case 0:
			{
				D3DXCOLOR diffuse = material->GetDiffuse();
				if (ImGui::ColorEdit4("", (float*)&diffuse))
					material->SetDiffuse(diffuse);
				ImGui::Image(diffuseTexture->SRV(), ImVec2(width, width));
				ImGui::Text(String::ToString(diffuseTexture->GetFile()).c_str());
				ImGui::Spacing(); ImGui::Spacing();
				ImGui::ColorPicker4("", (float*)&pickColor);
				break;
			}
			case 1:
				ImGui::Image(normalTexture->SRV(), ImVec2(width, width));
				ImGui::Text(String::ToString(normalTexture->GetFile()).c_str());
				break;
			case 2:
			{
				D3DXCOLOR specular = material->GetSpecular();
				if (ImGui::ColorEdit3("", (float*)&specular))
					material->SetSpecular(specular);
				if (ImGui::DragFloat("Shininess", &specular.a))
					material->SetSpecular(specular);

				ImGui::Image(specularTexture->SRV(), ImVec2(width, width));
				ImGui::Text(String::ToString(specularTexture->GetFile()).c_str());
				ImGui::Spacing(); ImGui::Spacing();
				ImGui::ColorPicker4("", (float*)&pickColor);
				break;
			}
			}
		}
	}
	ImGui::End();
}

void ModelTool::InitColliderBox(ColliderElement::EType type, wstring colName)
{
	GameModel * render = dynamic_cast<GameModel*>(currObject);
	UINT count = 0;
	for (size_t i = 0; i < render->GetColliders().size(); i++)
	{
		ColliderElement * collider = render->GetColliders()[i];
		if (collider->Name() == colName)
		{
			count++;
		}
	}

	if (count > 0)
		colName = colName + L" (" + to_wstring(count) + L")";

	switch (type)
	{
	case ColliderElement::EType::EBox:
	{
		ColliderBox * collider = new ColliderBox(lineEffect, colName, D3DXVECTOR3(-0.5f, -0.5f, -0.5f), D3DXVECTOR3(0.5f, 0.5f, 0.5f));

		if (colName.find(L"Main") == string::npos)
		{
			size_t i = colName.find(L" ");
			wstring temp = wstring(colName.begin(), colName.begin() + i);

			ModelBone * bone = model->BoneByName(temp);
			UINT index = bone->Index();

			collider->BoneIndex(index);
		}

		render->GetColliders().push_back(collider);
		break;
	}
	case ColliderElement::EType::ESphere:
	{
		ColliderSphere * collider = new ColliderSphere(lineEffect, colName, D3DXVECTOR3(0, 0, 0), 1);

		if (colName.find(L"Main") == string::npos)
		{
			size_t i = colName.find(L" ");
			wstring temp = wstring(colName.begin(), colName.begin() + i);

			ModelBone * bone = model->BoneByName(temp);
			UINT index = bone->Index();

			collider->BoneIndex(index);
		}

		render->GetColliders().push_back(collider);
		break;
	}
	}
}

void ModelTool::RenderColliderBox(GameModel * model)
{
	for (ColliderElement * collider : model->GetColliders())
	{
		int index = collider->BoneIndex();

		if (index != -1)
		{
			ModelBone * bone = this->model->BoneByIndex(index);

			//	실제 게임에서는 Render사용 안함
			//	Transform(bone->World()); 해주면 됨
			collider->Transform(bone->World());
			collider->Render();
		}
		else
		{
			collider->Transform(model->Transformed());
			collider->Render();
		}
	}
}

void ModelTool::FixColliderBox(int index)
{
	//	To push_back colliders if there is more than one collider
	vector<ColliderElement * > ColliderBox;
	if (currGameModel != NULL)
	{
		for (ColliderElement * line : currGameModel->GetColliders())
		{
			if (line->BoneIndex() == index)
				ColliderBox.push_back(line);
		}
	}

	if (ColliderBox.size() == 0) return;

	for (size_t i = 0; i < ColliderBox.size(); i++)
	{
		ColliderElement * colliderBox = ColliderBox[i];
		bool inUse = false;

		if (ImGui::CollapsingHeader(string("Collider##").append(to_string(i)).c_str()))
		{
			//	Check to set usage of the collider box
			inUse = colliderBox->IsDraw();
			if (ImGui::Checkbox(string("Active##").append(to_string(i)).c_str(), &inUse))
				colliderBox->IsDraw(inUse);
			//	Change the name of the collider box
			char buf[64];
			strcpy(buf, String::ToString(colliderBox->Name()).c_str());
			if (ImGui::InputText(string("Name##").append(to_string(i)).c_str(), buf, 64))
			{
				if (Keyboard::Get()->Down(VK_RETURN))
					colliderBox->Name(String::ToWString(buf));
			}

			//	Change the collider's World
			D3DXMATRIX world = colliderBox->Root();

			D3DXVECTOR3 position, scale, rot;
			D3DXQUATERNION quat;
			D3DXMatrixDecompose(&scale, &quat, &position, &world);
			Math::QuatToYawPithRoll(quat, rot.x, rot.y, rot.z);

			bool check = false;
			check |= ImGui::DragFloat3(string("Center##Collider").append(to_string(i)).c_str(), (float*)&position, 0.1f, -200, 200);
			check |= ImGui::DragFloat3(string("Rotation##Collider").append(to_string(i)).c_str(), (float*)&rot, 0.1f, -200, 200);
			check |= ImGui::DragFloat3(string("Size##Collider").append(to_string(i)).c_str(), (float*)&scale, 0.01f, -200, 200);

			if (check == true)
			{
				D3DXMATRIX Result, S, R, T;
				D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
				D3DXMatrixTranslation(&T, position.x, position.y, position.z);
				D3DXQuaternionRotationYawPitchRoll(&quat, rot.y, rot.x, rot.z);
				D3DXMatrixRotationQuaternion(&R, &quat);

				Result = S * R * T;
				colliderBox->Root(Result);
				colliderBox->Transform(colliderBox->Transform());
			}
		}
	}
}

void ModelTool::InitTrail()
{
	GameModel * render = dynamic_cast<GameModel*>(currObject);

	TrailRenderer * trail = new TrailRenderer(model, trailEffect);
	trail->Ready();
	trail->SetBone(boneIndex);
	trail->SetLifeTime(0.2f);

	render->GetTrails().push_back(trail);
}

void ModelTool::FixTrail(int index)
{
	if (index < 0) return;

	vector<TrailRenderer *> trails;
	for (size_t i = 0; i < currGameModel->GetTrails().size(); i++)
	{
		if (currGameModel->GetTrails()[i]->GetBone() == index)
			trails.push_back(currGameModel->GetTrails()[i]);
	}

	for (TrailRenderer * trail : trails)
	{
		trail->ImGuiRender();
	}
}

void ModelTool::SetTrailRun(bool val)
{
	for (TrailRenderer * trail : currGameModel->GetTrails())
		trail->SetRun(val);
}

void ModelTool::ExportModel(wstring file, UINT type)
{
	wstring fileName = Path::GetFileName(file);
	int i = fileName.find(L".fbx", 0);
	int j = fileName.find(L".FBX", 0);

	if (i == string::npos && j == string::npos) return;

	int k = fileName.find(L".", 0);
	wstring temp(fileName.begin(), fileName.begin() + k);
	fileName = temp;

	FbxLoader * loader = NULL;
	loader = new FbxLoader
	(
		file,
		Models + fileName + L"/", fileName
	);

	loader->ExportMaterial();
	loader->ExportMesh();

	SAFE_DELETE(loader);

	GameRender * model;
	switch (type)
	{
	case 0:
		model = new GameModel
		(
			0, modelEffect,
			Models + fileName + L"/" + fileName + L".material",
			Models + fileName + L"/" + fileName + L".mesh"
		);
		model->Ready(false);

		break;
	case 1:
		model = new GameAnimator
		(
			0, animEffect,
			Models + fileName + L"/" + fileName + L".material",
			Models + fileName + L"/" + fileName + L".mesh"
		);
		model->Ready(false);

		break;
	}
	
	model->Name() = fileName;
	model->FileName() = fileName;

	models.push_back((IGameObject *)model);
}

void ModelTool::OpenFBXFile(UINT type)
{
	Path::OpenFileDialog(bind(&ModelTool::ExportModel, this, placeholders::_1, type), Assets);
}

void ModelTool::ExportAnim(wstring file)
{
	wstring fileName = Path::GetFileName(file);
	int i = fileName.find(L".fbx", 0);
	int j = fileName.find(L".FBX", 0);

	if (i == string::npos && j == string::npos) return;

	int k = fileName.find(L".", 0);
	wstring temp(fileName.begin(), fileName.begin() + k);
	fileName = temp;

	wstring modelName = currObject->FileName();

	FbxLoader * loader = NULL;
	loader = new FbxLoader
	(
		file,
		Models + modelName + L"/", fileName
	);
	loader->ExportAnimation(0);

	SAFE_DELETE(loader);

	GameAnimator * model = dynamic_cast<GameAnimator*>(currObject);
	if (model != NULL)
	{
		UINT clipIndex = 0;
		for (size_t i = 0; i < model->ClipCount(); i++)
			clipIndex++;

		model->AddClip(clipIndex + 1, Models + modelName + L"/" + fileName + L".anim");
	}
}

void ModelTool::OpenAnimFile()
{
	Path::OpenFileDialog(bind(&ModelTool::ExportAnim, this, placeholders::_1), Assets);
}

void ModelTool::LoadMaterial(wstring file)
{
	Texture * texture = new Texture(file);

	materials.push_back(texture);
}

void ModelTool::OpenMaterialFile()
{
	Path::OpenFileDialog(bind(&ModelTool::LoadMaterial, this, placeholders::_1), Textures);
}

void ModelTool::LoadMesh(wstring file, UINT type)
{
	wstring fileName = Path::GetFileName(file);
	int i = fileName.find(L".mesh", 0);
	if (i == string::npos) return;

	wstring temp(fileName.begin(), fileName.begin() + i);
	fileName = temp;

	GameRender * model;
	switch (type)
	{
	case 0:
		model = new GameModel
		(
			0, modelEffect,
			Models + fileName + L"/" + fileName + L".material",
			Models + fileName + L"/" + fileName + L".mesh"
		);
		model->Ready(false);
		break;
	case 1:
		model = new GameAnimator
		(
			0, animEffect,
			Models + fileName + L"/" + fileName + L".material",
			Models + fileName + L"/" + fileName + L".mesh"
		);
		model->Ready(false);
		break;
	}
	
	model->Name() = fileName;
	model->FileName() = fileName;

	models.push_back((IGameObject *)model);
}

void ModelTool::OpenMeshFile(UINT type)
{
	Path::OpenFileDialog(bind(&ModelTool::LoadMesh, this, placeholders::_1, type), Models);
}

void ModelTool::LoadClip(wstring file)
{
	wstring fileName = Path::GetFileName(file);
	int i = fileName.find(L".anim", 0);
	if (i == string::npos) return;

	wstring temp(fileName.begin(), fileName.begin() + i);
	fileName = temp;

	wstring modelName = currObject->FileName();

	GameAnimator * model = dynamic_cast<GameAnimator*>(currObject);
	if (model != NULL)
	{
		UINT clipIndex = 0;
		for (size_t i = 0; i < model->ClipCount(); i++)
			clipIndex++;

		model->AddClip(clipIndex + 1, Models + modelName + L"/" + fileName + L".anim");
	}
}

void ModelTool::OpenClipFile()
{
	Path::OpenFileDialog(bind(&ModelTool::LoadClip, this, placeholders::_1), Models);
}

void ModelTool::Save()
{
	for (string name : objNames)
	{
		SaveMesh(name);
		SaveMaterial(name);

		IGameObject * obj = objects[String::ToWString(name)];

		GameAnimator * temp1 = dynamic_cast<GameAnimator *>(obj);
		GameModel * temp2 = dynamic_cast<GameModel *>(obj);

		GameCharacterSpec * spec = new GameCharacterSpec();

		if (temp1 != NULL)
		{
			spec->ModelType = ModelType::EGameAnimator;

			spec->ClipCount = temp1->ClipNames().size();
			for (string name : temp1->ClipNames())
				spec->ClipNames.push_back(name);
		}
		else
		{
			spec->ModelType = ModelType::EGameModel;
		}

		spec->Name = temp2->Name();
		spec->Tag = temp2->Tag();

		spec->ColliderCount = temp2->GetColliders().size();
		for (ColliderElement * col : temp2->GetColliders())
		{
			GameCharacterSpec::Collider collider;
			collider.Name = col->Name();
			collider.Type = col->Type();
			collider.BoneIndex = col->BoneIndex();
			collider.RootMatrix = col->Root();

			spec->Colliders.push_back(collider);
		}

		spec->IsChild = (int)temp2->IsChild();
		spec->RootMatrix = temp2->RootAxis();
		if (temp2->IsChild() == true)
		{
			spec->ParentBoneIndex = temp2->ParentBone()->Index();
			spec->ParentName = String::ToString(temp2->ParentName());
		}

		if (temp2->HasBT() == true)
			spec->BT = BTs[temp2->BTName()];

		spec->SourceFilePath = temp2->FileName();

		GameDataSpecManager::Get()->WriteCharacter(String::ToWString(name + ".spec"), spec);
		SAFE_DELETE(spec);
	}
}

void ModelTool::Load(wstring fileName)
{
	wstring temp = Path::GetFileName(fileName);

	GameCharacterSpec * spec = NULL;
	GameDataSpec * gamedata = GameDataSpecManager::Get()->Find(temp);
	if (gamedata == NULL)
		spec = (GameCharacterSpec*)GameDataSpecManager::Get()->LoadCharacter(temp);
	else
		spec = (GameCharacterSpec*)gamedata;

	wstring filePath = Models + spec->SourceFilePath + L"/";

	GameModel * model;
	switch (spec->ModelType)
	{
	case ModelType::EGameModel:
		model = new GameModel
		(
			0, modelEffect, 
			filePath + spec->Name + L".material",
			filePath + spec->Name + L".mesh"
		);
		model->Ready();
		break;
	case ModelType::EGameAnimator:
		model = new GameAnimator
		(
			0, animEffect,
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
	
	model->Name() = spec->Name;
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

			model->GetColliders().push_back(colliderBox);
		}
		else if (spec->Colliders[i].Type == (int)ColliderElement::EType::ESphere)
		{
			ColliderSphere * colliderSphere = new ColliderSphere
			(
				lineEffect, spec->Colliders[i].Name,
				D3DXVECTOR3(0, 0, 0), 1
			);

			colliderSphere->BoneIndex(spec->Colliders[i].BoneIndex);
			colliderSphere->Root(spec->Colliders[i].RootMatrix);
			colliderSphere->IsDraw(true);

			model->GetColliders().push_back(colliderSphere);
		}
	}

	model->IsChild(spec->IsChild);
	model->RootAxis(spec->RootMatrix);
	if ((bool)spec->IsChild == true)
	{
		GameModel * parent = (GameModel*)objects[String::ToWString(spec->ParentName)];
		model->ParentBone(parent->GetModel()->BoneByIndex(spec->ParentBoneIndex));
		model->ParentModel(parent);
		model->ParentName(String::ToWString(spec->ParentName));
	}

	model->BTName() = spec->BT->Name();
	if (model->BTName() != L"")
	{
		model->HasBT() = true;

		map<wstring, MyBehaviourTree*>::iterator iter;
		iter = BTs.find(model->BTName());
		if (iter == BTs.end())
		{
			BTNames.push_back(String::ToString(model->BTName()));
			BTs.insert(make_pair(model->BTName(), spec->BT));
		}
	}
	else
	{
		SAFE_DELETE(spec->BT);
	}

	objNames.push_back(String::ToString(model->Name()));
	objects.insert(make_pair(model->Name(), (IGameObject*)model));
}

void ModelTool::SaveMesh(string objName)
{
	IGameObject * obj = objects[String::ToWString(objName)];
	GameModel * gameModel = (GameModel *)obj;
	Model * model = gameModel->GetModel();

	wstring folderName = Models + gameModel->FileName();
	wstring fileName = gameModel->Name() + L".mesh";

	//	Create a folder
	Path::CreateFolder(folderName);

	BinaryWriter* w = new BinaryWriter();

	w->Open(folderName + L"/" + fileName);

	w->UInt(model->BoneCount());
	for (ModelBone * bone : model->Bones())
	{
		w->Int(bone->Index());
		w->String(String::ToString(bone->Name()));
		w->Int(bone->ParentIndex());

		w->Matrix(bone->Local());
		w->Matrix(bone->Global());
	}

	w->UInt(model->MeshCount());
	for (ModelMesh * meshData : model->Meshes())
	{
		w->String(String::ToString(meshData->Name()));
		w->Int(meshData->ParentBoneIndex());

		w->UInt(meshData->VertexCount());
		w->Byte(&meshData->Vertices()[0], sizeof(VertexTextureNormalTangentBlend) * meshData->VertexCount());

		w->UInt(meshData->MeshParts().size());
		for (ModelMeshPart * part : meshData->MeshParts())
		{
			w->String(String::ToString(part->MaterialName()));

			w->UInt(part->StartVertex());
			w->UInt(part->VertexCount());
		}
	}

	w->Close();
	SAFE_DELETE(w);
}

void ModelTool::SaveMaterial(string objName)
{
	IGameObject * obj = objects[String::ToWString(objName)];
	GameModel * gameModel = (GameModel *)obj;

	wstring folderName = Models + gameModel->FileName();
	//	Creates a folder in a specified directory
	Path::CreateFolder(folderName);

	//	All nodes are connected to XMLDocument
	//	If the document is delete all nodes are deleted
	Xml::XMLDocument * document = new Xml::XMLDocument();
	//	Declaration is the first entry point
	Xml::XMLDeclaration * decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement * root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	Model * model = gameModel->GetModel();
	string mapName = "";

	for (Material * material : model->Materials())
	{
		Xml::XMLElement* node = document->NewElement("Material");
		root->LinkEndChild(node);

		Xml::XMLElement* element = NULL;

		element = document->NewElement("Name");
		element->SetText(String::ToString(material->Name()).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("DiffuseFile");
		if (material->GetDiffuseMap() != NULL)
		{
			mapName = String::ToString(Path::GetFileName(material->GetDiffuseMap()->GetFile()));
			element->SetText(mapName.c_str());
		}
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		if (material->GetSpecularMap() != NULL)
		{
			mapName = String::ToString(Path::GetFileName(material->GetSpecularMap()->GetFile()));
			element->SetText(mapName.c_str());
		}
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		if (material->GetNormalMap() != NULL)
		{
			mapName = String::ToString(Path::GetFileName(material->GetNormalMap()->GetFile()));
			element->SetText(mapName.c_str());
		}
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		node->LinkEndChild(element);

		Xml::XMLElement * newColor = NULL;

		WriteXmlColor(element, document, material->GetDiffuse());

		element = document->NewElement("Specular");
		node->LinkEndChild(element);

		WriteXmlColor(element, document, material->GetSpecular());

		element = document->NewElement("SpecularExp");
		element->SetText(material->GetSpecular().a);
		node->LinkEndChild(element);
	}

	wstring fileName = gameModel->Name() + L".material";
	//	Save file to a specified directory
	string file = String::ToString(folderName + L"/" + fileName);
	document->SaveFile(file.c_str());

	SAFE_DELETE(document);
}

void ModelTool::WriteXmlColor(Xml::XMLElement * element, Xml::XMLDocument * document, D3DXCOLOR color)
{
	Xml::XMLElement * newColor = NULL;

	newColor = document->NewElement("R");
	newColor->SetText(color.r);
	element->LinkEndChild(newColor);

	newColor = document->NewElement("G");
	newColor->SetText(color.g);
	element->LinkEndChild(newColor);

	newColor = document->NewElement("B");
	newColor->SetText(color.b);
	element->LinkEndChild(newColor);

	newColor = document->NewElement("A");
	newColor->SetText(color.a);
	element->LinkEndChild(newColor);
}