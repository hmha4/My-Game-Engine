#include "stdafx.h"
#include "MapEditor.h"
#include "Environments/Billboard.h"
#include "Environments/Grass.h"
#include "Environments/Terrain.h"
#include "Environments/TerrainRenderer.h"
#include "Environments/HeightMap.h"
#include "Objects/IGameObject.h"

MapEditor::MapEditor()
	: GameEditor(L"MapEditor")
	, terrain(NULL)
{
	for (int i = 0; i < 3; i++)
		layerImg[i] = NULL;
	
	materialIndex = -1;
	errorMsg = "";
}

MapEditor::~MapEditor()
{
	SAFE_DELETE(brushShader);
	SAFE_RELEASE(srv);
	SAFE_RELEASE(uav);

	for (int i = 0; i < 3; i++)
		SAFE_DELETE(layerImg[i]);
}

void MapEditor::Initalize()
{
}

void MapEditor::Ready()
{
	brushShader = new Shader(Effects + L"023_TerrainBrush.fx", true);

	CreateComputeTexture();
}

void MapEditor::Update()
{
	switch (mSetType)
	{
	case MapEditor::MAP_SETTYPE::SET_NONE:
		break;
	case MapEditor::MAP_SETTYPE::SET_DETAIL_TERRAIN:
	{
	}
	break;
	case MapEditor::MAP_SETTYPE::SET_BRUSH_TERRAIN:
	{
		if (terrain->brush.Type != 0)
		{
			D3DXVECTOR3 position, pixelPos;
			bool bPicked = terrain->Picking(&pixelPos, &position);
			if (bPicked)
			{
				terrain->Desc().material->GetShader()->AsVector("Location")->SetFloatVector(position);
				terrain->Desc().material->GetShader()->AsVector("Color")->SetFloatVector(terrain->brush.Color);
				terrain->Desc().material->GetShader()->AsScalar("Range")->SetInt(terrain->brush.Range);

				if (Mouse::Get()->Press(2))
				{
					brushShader->AsScalar("BrushType")->SetInt(terrain->brush.Type);
					brushShader->AsScalar("BrushRange")->SetInt(terrain->brush.Range);
					brushShader->AsScalar("BrushPower")->SetFloat(terrain->brush.BrushPower);
					brushShader->AsVector("PixelPosition")->SetFloatVector(pixelPos);
					brushShader->AsScalar("BrushValue")->SetInt((int)mBrushType);
					
					terrain->GetTerrainRender()->AdjustY(brushShader, srv, uav);
				}
			}
		}
	}
	break;
	case MapEditor::MAP_SETTYPE::SET_PLANTS_TERRAIN:
	{

	}
	break;
	}
}

void MapEditor::PreRender()
{
	
}

void MapEditor::Render()
{

}

void MapEditor::HierarchyRender()
{
}

void MapEditor::ProjectRender()
{
}

void MapEditor::InspectorRender()
{
	if (terrain == NULL) return;

	//	오류 방지 Popup
	if (MyGui::show_app_modal == true)
	{
		ImGui::OpenPopup("Error");
		MyGui::show_app_modal = false;
	}
	MyGui::OpenPopupModal(errorMsg.c_str());

	//	Map Editor
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File##1"))
			{
				if (ImGui::MenuItem("Open"))
				{
					
				}
				if (ImGui::MenuItem("Save"))
				{
					if (terrain == NULL)
					{
						MyGui::show_app_modal = true;
						errorMsg = "There is no model or animation to retarget";
					}
					else
						terrain->Save();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::TextColored(ImVec4(0, 0, 0, 1), "%s Inspector", String::ToString(terrain->Name()).c_str());
		ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - ImGui::GetItemRectSize().x);
		bool wireFrame = terrain->GetTerrainRender()->WireFrameMode();
		if (ImGui::RadioButton("WireFrame", wireFrame))
			terrain->GetTerrainRender()->WireFrameMode() = wireFrame == true ? 0 : 1;

		ImGui::Separator();
		ImGui::Separator();

		SelectSetType();
		
		switch (mSetType)
		{
		case MapEditor::MAP_SETTYPE::SET_NONE:
		{
			BasicSetRender();
		}
		break;

		case MapEditor::MAP_SETTYPE::SET_DETAIL_TERRAIN:
		{
			if (ImGui::CollapsingHeader("Detail Map", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Separator();
				ImGui::Separator();
				if (terrain->Desc().material->GetDetailMap() != NULL)
				{
					if (ImGui::ImageButton(terrain->Desc().material->GetDetailMap()->SRV(), ImVec2(140, 140)))
						materialIndex = 3;
				}
				else
				{
					if (ImGui::ImageButton(0, ImVec2(140, 140)))
					{
						materialIndex = 3;
						Path::OpenFileDialog(bind(&MapEditor::LoadTerrainMapFile, this, placeholders::_1, 3), Textures);
					}
				}
				ImGui::SameLine();
				ImGui::BeginGroup();
				{
					ImGui::PushItemWidth(110);
					ImGui::Text("Intensity");
					if (ImGui::DragFloat("##Intensity", &terrain->detail.DetailIntensity, 0.01f, 1.0f, 10.0f))
						terrain->Desc().material->GetShader()->AsScalar("DetailIntensity")->SetFloat(terrain->detail.DetailIntensity);
					ImGui::Spacing();

					ImGui::Text("Depth Value");
					if (ImGui::DragFloat("##Depth Value", &terrain->detail.DetailValue, 0.00001f, 0.0f, 1.0f, "%.4f"))
						terrain->Desc().material->GetShader()->AsScalar("DetailValue")->SetFloat(terrain->detail.DetailValue);
					ImGui::Spacing();

					ImGui::Text("TextureSize");
					ImGui::DragInt("##TextureSize", (int*)&terrain->GetTerrainRender()->GetBuffer().TexScale, 1, 1, 64);
					ImGui::Spacing();
					ImGui::PopItemWidth();
				}
				ImGui::EndGroup();

				ImGui::Separator();
				ImGui::Separator();


				float width = ImGui::GetWindowContentRegionWidth();

				if (materialIndex == 3)
				{
					if (ImGui::Button("Change"))
						Path::OpenFileDialog(bind(&MapEditor::LoadTerrainMapFile, this, placeholders::_1, 3), Textures);

					ImGui::Image(terrain->Desc().material->GetDetailMap()->SRV(), ImVec2(width, width));
				}
			}
		}
		break;
		case MapEditor::MAP_SETTYPE::SET_BRUSH_TERRAIN:
		{
			if (ImGui::CollapsingHeader("Line", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Separator();
				ImGui::Separator();

				bool b = false;
				b |= ImGui::SliderInt("Type##Line", &terrain->tline.LineType, 0, 2);
				b |= ImGui::ColorEdit3("Color##Line", terrain->tline.LineColor);
				b |= ImGui::SliderInt("Spacing##Line", &terrain->tline.Spacing, 1, 10);
				b |= ImGui::DragFloat("Thickness##Line", &terrain->tline.Thickness, 0.1f, 0.1f, 10.0f);

				if(b == true)
					terrain->LineUpdate();

				ImGui::Separator();
				ImGui::Separator();
			}

			if (ImGui::CollapsingHeader("Brush", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Separator();
				ImGui::Separator();

				ImGui::Spacing();
				ImGui::RadioButton("None", (int*)&mBrushType, 0);
				ImGui::SameLine();
				ImGui::RadioButton("Up", (int*)&mBrushType, 1);
				ImGui::SameLine();
				ImGui::RadioButton("Down", (int*)&mBrushType, 2);
				ImGui::Spacing();
				ImGui::Separator();

				if(ImGui::SliderInt("Type##Brush", &terrain->brush.Type, 0, 2))
					terrain->Desc().material->GetShader()->AsScalar("Type")->SetInt(terrain->brush.Type);
				ImGui::ColorEdit3("Color##Brush", terrain->brush.Color);
				ImGui::SliderInt("Range##Brush", &terrain->brush.Range, 1, 100);
				ImGui::DragFloat("Power##Brush", &terrain->brush.BrushPower, 0.5f, 10, 100);

				ImGui::Separator();
				ImGui::Separator();
			}

			if (ImGui::CollapsingHeader("HeightMap", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Current HeightMap");

				float width = ImGui::GetWindowContentRegionWidth();
				if (terrain->GetTerrainRender()->GetHeightMapSRV() != NULL)
				{
					if (ImGui::ImageButton(terrain->GetTerrainRender()->GetHeightMapSRV(), ImVec2(width - 10, width - 10)))
					{
						Path::OpenFileDialog(bind(&MapEditor::LoadHeightMapFile, this, placeholders::_1), Textures + L"HeightMap/");
					}
				}
				else
				{
					if (ImGui::ImageButton(0, ImVec2(width - 10, width - 10)))
					{
						Path::OpenFileDialog(bind(&MapEditor::LoadHeightMapFile, this, placeholders::_1), Textures);
					}
				}

				ImGui::Separator();
				if (ImGui::Button("Save HeightMap"))
				{

				}
				ImGui::Separator();
				ImGui::Separator();
			}
		}
		break;
		case MapEditor::MAP_SETTYPE::SET_BLEND_TERRAIN:
		{
			if (ImGui::CollapsingHeader("Blend", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Separator();
				ImGui::Separator();

				ImGui::Columns(3);
				ImGui::Text("Middle"); ImGui::NextColumn();
				ImGui::Text("Top"); ImGui::NextColumn();
				ImGui::Text("Bottom"); ImGui::NextColumn();
				ImGui::Separator();

				if (layerImg[0] != NULL)
				{
					if (ImGui::ImageButton(layerImg[0]->SRV(), ImVec2(50, 50)))
						mBlendType = MAP_BLENDTYPE::BLEND1;
				}
				else
				{
					if (ImGui::ImageButton(0, ImVec2(50, 50)))
					{
						mBlendType = MAP_BLENDTYPE::BLEND1;
						Path::OpenFileDialog(bind(&MapEditor::LoadBlendMapFile, this, placeholders::_1, 0), Textures);
					}
				}
				ImGui::NextColumn();

				if (layerImg[1] != NULL)
				{
					if (ImGui::ImageButton(layerImg[1]->SRV(), ImVec2(50, 50)))
						mBlendType = MAP_BLENDTYPE::BLEND2;
				}
				else
				{
					if (ImGui::ImageButton(0, ImVec2(50, 50)))
					{
						mBlendType = MAP_BLENDTYPE::BLEND2;
						Path::OpenFileDialog(bind(&MapEditor::LoadBlendMapFile, this, placeholders::_1, 1), Textures);
					}
				}
				ImGui::NextColumn();

				if (layerImg[2] != NULL)
				{
					if (ImGui::ImageButton(layerImg[2]->SRV(), ImVec2(50, 50)))
						mBlendType = MAP_BLENDTYPE::BLEND3;
				}
				else
				{
					if (ImGui::ImageButton(0, ImVec2(50, 50)))
					{
						mBlendType = MAP_BLENDTYPE::BLEND3;
						Path::OpenFileDialog(bind(&MapEditor::LoadBlendMapFile, this, placeholders::_1, 2), Textures);
					}
				}
				ImGui::Columns(1);

				ImGui::Separator();
				ImGui::Separator();

				float width = ImGui::GetWindowContentRegionWidth();
				if (mBlendType == MAP_BLENDTYPE::BLEND1)
				{
					ImGui::Text("Middle Blend Texture");

					if (ImGui::Button("Change"))
						Path::OpenFileDialog(bind(&MapEditor::LoadBlendMapFile, this, placeholders::_1, 0), Textures);

					ImGui::Image(layerImg[0]->SRV(), ImVec2(width, width));
					ImGui::Separator();

					ImGui::Checkbox("TurnOffBlend", (bool*)&terrain->Desc().activeBlend.x);
					ImGui::DragFloat("BlendHeight", &terrain->Desc().blendHeight.x, 0.001f, -1, 1);
					terrain->GetTerrainRender()->BlendSet();

					ImGui::Separator();
					ImGui::Separator();
				}
				else if (mBlendType == MAP_BLENDTYPE::BLEND2)
				{
					ImGui::Text("Top Blend Texture");

					if (ImGui::Button("Change"))
						Path::OpenFileDialog(bind(&MapEditor::LoadBlendMapFile, this, placeholders::_1, 1), Textures);

					ImGui::Image(layerImg[1]->SRV(), ImVec2(width, width));
					ImGui::Separator();

					ImGui::Checkbox("TurnOffBlend", (bool*)&terrain->Desc().activeBlend.y);
					ImGui::DragFloat("BlendHeight", &terrain->Desc().blendHeight.y, 0.001f, -1, 1);
					terrain->GetTerrainRender()->BlendSet();

					ImGui::Separator();
					ImGui::Separator();
				}
				else if (mBlendType == MAP_BLENDTYPE::BLEND3)
				{
					ImGui::Text("Bottom Blend Texture");

					if (ImGui::Button("Change"))
						Path::OpenFileDialog(bind(&MapEditor::LoadBlendMapFile, this, placeholders::_1, 2), Textures);

					ImGui::Image(layerImg[2]->SRV(), ImVec2(width, width));
					ImGui::Separator();

					ImGui::Checkbox("TurnOffBlend", (bool*)&terrain->Desc().activeBlend.z);
					ImGui::DragFloat("BlendHeight", &terrain->Desc().blendHeight.z, 0.001f, -1, 1);
					terrain->GetTerrainRender()->BlendSet();

					ImGui::Separator();
					ImGui::Separator();
				}
			}
		}
		break;
		case MapEditor::MAP_SETTYPE::SET_FOG_TERRAIN:
		{
			if (ImGui::CollapsingHeader("Fog", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat("FogStart", &terrain->GetTerrainRender()->GetBuffer().FogStart, 10.0f, 0.0f, 600.0f);
				ImGui::DragFloat("FogRange", &terrain->GetTerrainRender()->GetBuffer().FogRange, 10.0f, 0.0f, 600.0f);
				ImGui::ColorEdit3("FogColor", terrain->GetTerrainRender()->GetBuffer().FogColor);
			}
		}
		break;

		case MapEditor::MAP_SETTYPE::SET_PLANTS_TERRAIN:
		{
			if (ImGui::CollapsingHeader("Grass"))
			{
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::ColorEdit3("Color##Grass", terrain->brush.Color);
				ImGui::SliderInt("Range##Grass", &terrain->brush.Range, 1, 100);
				ImGui::Spacing();

				terrain->GetGrass()->ImGuiRender();

			}


			if (ImGui::CollapsingHeader("BillBoard"))
			{
				terrain->GetBillboard()->ImGuiRender();
				D3DXVECTOR3 pos;
				if (Mouse::Get()->Down(2))
				{
					if (terrain->Picking(NULL, &pos))
					{
						terrain->GetBillboard()->BuildBillboard(pos);
					}
				}

			}
		}
		break;
		}
	}
}

void MapEditor::AssetRender()
{
}

void MapEditor::Delete()
{
	if (this != NULL)
		delete this;
}

void MapEditor::SetGameObjectPtr(IGameObject * gameObject)
{
	terrain = (Terrain*)gameObject;

	if (terrain != NULL)
	{
		renderer = terrain->GetTerrainRender();
		for (int i = 0; i < 3; i++)
		{
			if (layerImg[i] == NULL)
				layerImg[i] = new Texture(terrain->Desc().layerMapFile[i]);
		}
	}
}

void MapEditor::SetType(UINT type)
{
	mSetType = (MapEditor::MAP_SETTYPE)type;
}

void MapEditor::LoadTerrainMapFile(wstring filename, int type)
{
	renderer->ChangeImageMap(terrain->Desc().material, filename, type);
	terrain->SetMaterial(type, filename);
}

void MapEditor::LoadHeightMapFile(wstring file)
{
	terrain->Desc().HeightMapInit(file, terrain->Desc().HeightScale);

	terrain->GetHeightMap()->Load(terrain->Desc().HeightMap);
	renderer->Initialize();

	terrain->Ready();
}

void MapEditor::LoadBlendMapFile(wstring file, int bNum)
{
	SAFE_DELETE(layerImg[bNum]);

	layerImg[bNum] = new Texture(file);
	terrain->Desc().BlendMapInit(file, bNum);

	terrain->Ready();
}

void MapEditor::BasicSetRender()
{
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Separator();
		ImGui::Separator();

		ImGui::DragFloat3("Position", (float*)&terrain->GetTerrainRender()->GetSRT().trans, 0.05f, -128, 128);
		ImGui::DragFloat3("Scale", (float*)&terrain->GetTerrainRender()->GetSRT().scale, 0.05f, -10, 10);
		ImGui::DragFloat3("Rotation", (float*)&terrain->GetTerrainRender()->GetSRT().rotate, 0.05f, -3.14f, 3.14f);

		ImGui::Separator();
		ImGui::Separator();
	}

	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Separator();
		ImGui::Separator();

		D3DXCOLOR ambientColor = terrain->Desc().material->GetAmbient();
		if(ImGui::ColorEdit3("Ambient", (float*)&ambientColor))
			terrain->Desc().material->SetAmbient(ambientColor);

		ImGui::Columns(3);
		ImGui::Text("Diffuse"); ImGui::NextColumn();
		ImGui::Text("Normal"); ImGui::NextColumn();
		ImGui::Text("Specular"); ImGui::NextColumn();
		ImGui::Separator();

		if (terrain->Desc().material->GetDiffuseMap() != NULL)
		{
			if (ImGui::ImageButton(terrain->Desc().material->GetDiffuseMap()->SRV(), ImVec2(50, 50)))
				materialIndex = 0;
		}
		else
		{
			if (ImGui::ImageButton(0, ImVec2(50, 50)))
			{
				materialIndex = 0;
				Path::OpenFileDialog(bind(&MapEditor::LoadTerrainMapFile, this, placeholders::_1, 0), Textures);
			}
		}
		ImGui::NextColumn();

		if (terrain->Desc().material->GetNormalMap() != NULL)
		{
			if (ImGui::ImageButton(terrain->Desc().material->GetNormalMap()->SRV(), ImVec2(50, 50)))
				materialIndex = 1;
		}
		else
		{
			if (ImGui::ImageButton(0, ImVec2(50, 50)))
			{
				materialIndex = 1;
				Path::OpenFileDialog(bind(&MapEditor::LoadTerrainMapFile, this, placeholders::_1, 2), Textures);
			}
		}
		ImGui::NextColumn();

		if (terrain->Desc().material->GetSpecularMap() != NULL)
		{
			if (ImGui::ImageButton(terrain->Desc().material->GetSpecularMap()->SRV(), ImVec2(50, 50)))
				materialIndex = 2;
		}
		else
		{
			if (ImGui::ImageButton(0, ImVec2(50, 50)))
			{
				materialIndex = 2;
				Path::OpenFileDialog(bind(&MapEditor::LoadTerrainMapFile, this, placeholders::_1, 1), Textures);
			}
		}
		ImGui::Columns(1);

		ImGui::Separator();
		ImGui::Separator();

		

		float width = ImGui::GetWindowContentRegionWidth();
		switch (materialIndex)
		{
		case 0:
		{
			if (ImGui::Button("Change"))
				Path::OpenFileDialog(bind(&MapEditor::LoadTerrainMapFile, this, placeholders::_1, 0), Textures);

			ImGui::Image(terrain->Desc().material->GetDiffuseMap()->SRV(), ImVec2(width, width));

			D3DXCOLOR diffuseColor = terrain->Desc().material->GetDiffuse();
			ImGui::ColorEdit3("Color", (float*)&diffuseColor);
			terrain->Desc().material->SetDiffuse(diffuseColor);
		}
		break;

		case 1:
		{
			if (ImGui::Button("Change"))
				Path::OpenFileDialog(bind(&MapEditor::LoadTerrainMapFile, this, placeholders::_1, 2), Textures);

			ImGui::Image(terrain->Desc().material->GetNormalMap()->SRV(), ImVec2(width, width));
		}
		break;

		case 2:
		{
			if (ImGui::Button("Change"))
				Path::OpenFileDialog(bind(&MapEditor::LoadTerrainMapFile, this, placeholders::_1, 1), Textures);

			ImGui::Image(terrain->Desc().material->GetSpecularMap()->SRV(), ImVec2(width, width));

			D3DXCOLOR specularColor = terrain->Desc().material->GetSpecular();
			ImGui::ColorEdit3("Color", (float*)&specularColor);
			terrain->Desc().material->SetSpecular(specularColor);
			float shininess = terrain->Desc().material->GetSpecular().a;
			ImGui::DragFloat("Shininess", &shininess, 0.01f, 1, 60);
			D3DXCOLOR specular = terrain->Desc().material->GetSpecular();
			specular.a = shininess;
			terrain->Desc().material->SetSpecular(specular);
		}
		break;
		}
	}
}

void MapEditor::CreateComputeTexture()
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = 2049;
	desc.Height = 2049;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	ID3D11Texture2D*texture;
	hr = D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture);
	assert(SUCCEEDED(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R16_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	hr = D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv);
	assert(SUCCEEDED(hr));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R16_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	hr = D3D::GetDevice()->CreateUnorderedAccessView(texture, &uavDesc, &uav);
	assert(SUCCEEDED(hr));

	SAFE_RELEASE(texture);
}

void MapEditor::SelectSetType()
{
	ImGui::BeginTabBar("Terrain Bars");
	{
		if (ImGui::BeginTabItem("General"))
		{
			mSetType = MAP_SETTYPE::SET_NONE;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Detail"))
		{
			mSetType = MAP_SETTYPE::SET_DETAIL_TERRAIN;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Height"))
		{
			mSetType = MAP_SETTYPE::SET_BRUSH_TERRAIN;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Blend"))
		{
			mSetType = MAP_SETTYPE::SET_BLEND_TERRAIN;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Fog"))
		{
			mSetType = MAP_SETTYPE::SET_FOG_TERRAIN;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Plants"))
		{
			mSetType = MAP_SETTYPE::SET_PLANTS_TERRAIN;
			terrain->brush.Type = 1;
			terrain->Desc().material->GetShader()->AsScalar("Type")->SetInt(terrain->brush.Type);
			ImGui::EndTabItem();
		}
		else
		{
			terrain->brush.Type = 0;
			terrain->Desc().material->GetShader()->AsScalar("Type")->SetInt(terrain->brush.Type);
		}
	}
	ImGui::EndTabBar();
}
