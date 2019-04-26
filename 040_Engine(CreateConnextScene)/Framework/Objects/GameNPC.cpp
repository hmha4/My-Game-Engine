#include "Framework.h"
#include "GameNPC.h"
#include "Models/ModelClip.h"
#include "Environments/Terrain.h"
#include "Objects/GamePlayer.h"
#include "Viewer/SmoothFollow.h"

GameNPC::GameNPC(GameAnimator * model)
	: GameUnit(model), terrain(NULL)
	, distance(0), isTalk(false)
	, uiIndex(0), turnTime(0.0f)
	, rotFactor(0)
	, enemyCount(0)
	, dialog13(false)
	, clear(false)
{
	model->Play("Sitting", true, 0.2f);

	spawnT = model->Position();
	spawnR = model->Rotation();
	spawnS = model->Scale();

	state = STATE::SIT;
}

GameNPC::~GameNPC()
{
}

void GameNPC::Initialize()
{
	__super::Initialize();
}

void GameNPC::Ready()
{
	
}

void GameNPC::TitleUpdate()
{

}

void GameNPC::Update()
{
	D3DXVECTOR3 pos = model->Position();
	pos.y = terrain->GetHeight(pos.x, pos.z);

	model->Position(pos);

	switch (state)
	{
	case GameNPC::STATE::IDLE:
		if (player->IsTalk() == true)
		{
			D3DXVECTOR2 rot;
			Context::Get()->GetMainCamera()->Rotation(&rot);
			if (rotFactor >= -50)
			{
				Context::Get()->GetMainCamera()->Rotation(rot.x, rot.y + Math::ToRadian(-1));
				rotFactor--;
			}

			turnTime += 3.0f * Time::Delta();

			if (turnTime <= 1.0f)
			{
				D3DXVECTOR3 rot;
				D3DXVec3Lerp(&rot, &rotFrom, &rotTo, turnTime);

				model->Rotation(rot);
			}
			
			if (Mouse::Get()->Down(0))
			{
				UIManager::Get()->SetUiRender(L"12Dialog" + to_wstring(uiIndex - 1), false);
				UIManager::Get()->SetUiRender(L"12Dialog" + to_wstring(uiIndex++), true);

				if (uiIndex > 4)
				{
					turnTime = 0.0f;
					player->IsTalk(false);
					player->SetUI(true);
					UIManager::Get()->SetUiRender(L"12Dialog" + to_wstring(uiIndex - 1), false);
					state = STATE::IDLE2;
					uiIndex--;
				}
			}

			break;
		}
		if (player->IsTalkable() == true)
		{
			D3DXVECTOR2 rot;
			Context::Get()->GetMainCamera()->Rotation(&rot);
			if (rotFactor < 0)
			{
				Context::Get()->GetMainCamera()->Rotation(rot.x, rot.y + Math::ToRadian(1));
				rotFactor++;
			}

			if (Mouse::Get()->Down(0))
			{
				player->IsTalk(true);
				((SmoothFollow*)Context::Get()->GetMainCamera())->SetDistance(10);
				D3DXVECTOR3 lookPos = player->Position() + model->Position();
				lookPos /= 2;
				Context::Get()->GetMainCamera()->LookAtPosition(lookPos.x, lookPos.y + 2, lookPos.z);
				
				UIManager::Get()->SetUiRender(L"12Dialog" + to_wstring(uiIndex++), true);

				player->SetUI(false);

				rotFrom = model->Rotation();
				D3DXVECTOR3 playerPos = player->Position();
				D3DXVECTOR3 enemyPos = model->Position();
				playerPos.y = 0.0f;
				enemyPos.y = 0.0f;

				rotTo = playerPos - enemyPos;
				D3DXVECTOR3 dir = -model->Forward();
				float rotation = Math::GetAngle(dir, rotTo);

				rotTo = rotFrom;
				rotTo.y += rotation;

				break;
			}
		}
		
		
		break;
	case GameNPC::STATE::IDLE2:
		if (player->IsTalk() == true)
		{
			D3DXVECTOR2 rot;
			Context::Get()->GetMainCamera()->Rotation(&rot);
			if (rotFactor >= -50)
			{
				Context::Get()->GetMainCamera()->Rotation(rot.x, rot.y + Math::ToRadian(-1));
				rotFactor--;
			}

			turnTime += 3.0f * Time::Delta();

			if (turnTime <= 1.0f)
			{
				D3DXVECTOR3 rot;
				D3DXVec3Lerp(&rot, &rotFrom, &rotTo, turnTime);

				model->Rotation(rot);
			}

			if (enemyCount <= 0)
			{
				if (Mouse::Get()->Down(0))
				{
					UIManager::Get()->SetUiRender(L"12Dialog" + to_wstring(uiIndex - 1), false);
					UIManager::Get()->SetUiRender(L"12Dialog" + to_wstring(uiIndex++), true);

					if (uiIndex > 6)
					{
						turnTime = 0.0f;
						player->IsTalk(false);
						player->SetUI(true);
						UIManager::Get()->SetUiRender(L"12Dialog" + to_wstring(uiIndex - 1), false);
						state = STATE::WAVE;
						model->Play("Wave", false, 2.0f);
					}
				}
			}
			else
			{
				if (Mouse::Get()->Down(0))
				{
					turnTime = 0.0f;

					dialog13 = !dialog13;

					player->IsTalk(dialog13);
					player->SetUI(!dialog13);
					UIManager::Get()->SetUiRender(L"13Dialog", dialog13);
				}
			}

			break;
		}
		if (player->IsTalkable() == true)
		{
			D3DXVECTOR2 rot;
			Context::Get()->GetMainCamera()->Rotation(&rot);
			if (rotFactor < 0)
			{
				Context::Get()->GetMainCamera()->Rotation(rot.x, rot.y + Math::ToRadian(1));
				rotFactor++;
			}

			if (Mouse::Get()->Down(0))
			{
				player->IsTalk(true);
				((SmoothFollow*)Context::Get()->GetMainCamera())->SetDistance(10);
				D3DXVECTOR3 lookPos = player->Position() + model->Position();
				lookPos /= 2;
				Context::Get()->GetMainCamera()->LookAtPosition(lookPos.x, lookPos.y + 2, lookPos.z);

				if (enemyCount > 0)
				{
					dialog13 = !dialog13;
					UIManager::Get()->SetUiRender(L"13Dialog", dialog13);
				}
				else
				{
					UIManager::Get()->SetUiRender(L"12Dialog" + to_wstring(uiIndex++), true);
				}

				player->SetUI(false);

				rotFrom = model->Rotation();
				D3DXVECTOR3 playerPos = player->Position();
				D3DXVECTOR3 enemyPos = model->Position();
				playerPos.y = 0.0f;
				enemyPos.y = 0.0f;

				rotTo = playerPos - enemyPos;
				D3DXVECTOR3 dir = -model->Forward();
				float rotation = Math::GetAngle(dir, rotTo);

				rotTo = rotFrom;
				rotTo.y += rotation;

				break;
			}
		}
		break;
	case GameNPC::STATE::SIT:
		if (Mouse::Get()->Press(0))
		{
			state = STATE::IDLE;
			model->Play("Idle", true, 0.2f);
		}
		break;
	case GameNPC::STATE::RUN:
		break;
	case GameNPC::STATE::WAVE:
		if (player->IsTalkable() == true)
		{
			D3DXVECTOR2 rot;
			Context::Get()->GetMainCamera()->Rotation(&rot);
			if (rotFactor < 0)
			{
				Context::Get()->GetMainCamera()->Rotation(rot.x, rot.y + Math::ToRadian(1));
				rotFactor++;
			}
		}
		if (model->GetClip("Wave")->IsPlay() == false)
		{
			clear = true;
			state = STATE::DEFAULT;
		}
		break;
	case GameNPC::STATE::DEFAULT:

		break;
	}
}

void GameNPC::Render()
{
	if (state != STATE::IDLE && state != STATE::IDLE2) return;

	if (player->IsTalk() == false)
	{
		D3DDesc desc;
		D3D::GetDesc(&desc);

		RECT rect;
		rect.left = (LONG)(desc.Width - (desc.Width / 6.4f));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("BCKGND", NULL, ImGui::GetIO().DisplaySize, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			rect.top = (LONG)(desc.Height / 7.5f);
			string temp = "Quests";
			ImGui::GetWindowDrawList()->AddText(ImVec2((float)rect.left, (float)rect.top), ImColor(1.0f, 1.0f, 0.0f, 1.0f), temp.c_str());
		}
		ImGui::End();

		if (state == STATE::IDLE2)
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::Begin("BCKGND", NULL, ImGui::GetIO().DisplaySize, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
			{
				rect.top = (LONG)(desc.Height / 6);
				string temp = " - Eliminate Creatures (" + to_string(enemyCount) + "/6)";
				ImGui::GetWindowDrawList()->AddText(ImVec2((float)rect.left, (float)rect.top), ImColor(1.0f, 1.0f, 1.0f, 1.0f), temp.c_str());
			}
			ImGui::End();
		}
	}
	

	distance = Math::GetDistanceXZ(model->Position(), player->Position());
	if (distance <= 10)
	{
		D3DXMATRIX view, proj, world;
		Context::Get()->GetMainCamera()->MatrixView(&view);
		Context::Get()->GetPerspective()->GetMatrix(&proj);
		world = model->Transformed();
		world._42 += 4;
		world._41 += 1;

		RECT rect;
		rect = DirectWrite::Get()->Get2DPosition(world, view, proj);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("BCKGND", NULL, ImGui::GetIO().DisplaySize, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			ImGui::GetWindowDrawList()->AddText(ImVec2((float)rect.left, (float)rect.top), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "Talk");
		}
		ImGui::End();

		player->IsTalkable(true);

	}
	else
	{
		player->IsTalkable(false);
	}
}

void GameNPC::Clear()
{
	model->Stop();

	model->Position(spawnT);
	model->Rotation(spawnR);
	model->Scale(spawnS);
}

void GameNPC::InputTerrain(Terrain * terrain)
{
	this->terrain = terrain;

	D3DXVECTOR3 pos = model->Position();
	pos.y = terrain->GetHeight(pos.x, pos.z);

	model->Position(pos);
}

void GameNPC::InputPlayer(GamePlayer * player)
{
	this->player = player;
}
