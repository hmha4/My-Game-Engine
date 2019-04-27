#include "stdafx.h"
#include "TestLightning.h"
#include "Particles/LightningSeed.h"
#include "Draw/GizmoGrid.h"

#include <fstream>
#include <sstream>

void TestLightning::Initialize()
{

}

void TestLightning::Ready()
{
	m_lightning_renderer = new LightningRenderer();
	m_lightning_renderer->Initialize();
	m_lightning_renderer->Ready();
	CreateLightning();

	//vector < pair<D3DXVECTOR3, D3DXVECTOR3>> temp;
	//temp.push_back(make_pair(D3DXVECTOR3(0, 100, 0), D3DXVECTOR3(0, 0, 0)));
	//CreateThunder(temp, Type::EChain, 5, 1.0f);

	grid = new GizmoGrid(1);
	grid->Initialize();
	grid->Ready();
}

void TestLightning::Destroy()
{
	m_lightning_renderer->DestroyLightning(m_fence_lightning);
	m_lightning_renderer->DestroyLightning(m_inter_coil_lightning);
	m_lightning_renderer->DestroyLightning(m_coil_helix_lightning);
	m_lightning_renderer->DestroyLightning(m_chain_lightning);
	SAFE_DELETE(m_lightning_renderer);

	SAFE_DELETE(grid);
}

void TestLightning::Update()
{
	do_lightning = Settings.Fence || Settings.InterCoil || Settings.CoilHelix || Settings.Chain;
}

void TestLightning::PreRender()
{
	if (do_lightning == false) return;

	m_lightning_renderer->PreRender();
}

void TestLightning::Render()
{
	grid->Render();
	ImGuiRender();

	if (do_lightning == false) return;

	if (Settings.Fence)
		m_lightning_renderer->Render(m_fence_lightning, m_red_beam, 1.0f, Settings.AnimationSpeed);

	if (Settings.InterCoil)
		m_lightning_renderer->Render(m_inter_coil_lightning, m_blue_beam, 1.0f, Settings.AnimationSpeed);

	if (Settings.CoilHelix)
		m_lightning_renderer->Render(m_coil_helix_lightning, m_blue_cyan_beam, 1.0f, Settings.AnimationSpeed);

	if (Settings.Chain)
	{
		m_chain_lightning->Properties.ChainSource = D3DXVECTOR3(0, 25, 31);
		m_chain_lightning->Properties.NumTargets = 4;

		const UINT NumTargets = 4;
		float s[NumTargets] = { 1.00f, -0.5f, 0.5f, 2.0f };
		float r[NumTargets] = { 7.0f, 9.0f, 11.0f, 13.0f };
		float d[NumTargets] = { 5.0f, 10.0f, 15.0f, 20.0f };

		D3DXVECTOR4 m_target_positions[4];

		for (int i = 0; i < NumTargets; ++i)
		{
			float angle = s[i] * Time::Get()->Running();

			m_target_positions[i] = D3DXVECTOR4(r[i] * sin(angle), d[i], r[i] * cos(angle), 1);
		}

		for (int i = 0; i < m_chain_lightning->Properties.NumTargets; ++i)
			m_chain_lightning->Properties.ChainTargetPositions[i] = m_target_positions[i];

		m_lightning_renderer->Render(m_chain_lightning, Settings.Beam, 1.0f, Settings.AnimationSpeed);
	}

}

void TestLightning::PostRender()
{
	if (do_lightning == false) return;

	m_lightning_renderer->PostRender(Settings.Glow, Settings.BlurSigma);
}

void TestLightning::ResizeScreen()
{
}

void TestLightning::ImGuiRender()
{
	ImGui::Begin("Lightning", 0, NULL);
	{
		ImGui::SliderFloat("Anim Speed", &Settings.AnimationSpeed, 0, 100);
		ImGui::DragFloat("Bolt Width", &Settings.Beam.BoltWidth.x, 0.01f, 0, 1);
		ImGui::SliderFloat("Width Falloff", &Settings.Beam.BoltWidth.y, 0, 100);
		ImGui::ColorEdit3("Color", (float*)&Settings.Beam.ColorOutside);
		ImGui::SliderFloat("Color Exp", &Settings.Beam.ColorFallOffExponent, 1, 50);
		ImGui::Checkbox("Chain", &Settings.Chain);
		ImGui::Checkbox("Fence", &Settings.Fence);
		ImGui::Checkbox("Coil", &Settings.CoilHelix);
		ImGui::Checkbox("Connector", &Settings.InterCoil);
		ImGui::Checkbox("Glow", &Settings.Glow);
		ImGui::Checkbox("Show Lines", &Settings.Lines);

		ImGui::SliderFloat("Glow Sigma R", &Settings.BlurSigma.x, 0, 200);
		ImGui::SliderFloat("Glow Sigma G", &Settings.BlurSigma.y, 0, 200);
		ImGui::SliderFloat("Glow Sigma B", &Settings.BlurSigma.z, 0, 200);
	}
	ImGui::End();
}




void TestLightning::CreateThunder(vector<pair<D3DXVECTOR3, D3DXVECTOR3>>& posSeeds, Type type, int rep, float jit)
{
	D3DXVECTOR3 up(0, 1, 0);
	vector<LightningPathSegment> inter_coil_seeds;

	int replicates = rep;
	float jitter = jit;

	for (int j = 0; j < replicates; ++j)
	{
		for (pair<D3DXVECTOR3, D3DXVECTOR3> seed : posSeeds)
		{
			inter_coil_seeds.push_back
			(
				LightningPathSegment
				(
					seed.first,
					seed.second,
					up
				)
			);
		}
	}

	for (size_t i = 0; i < inter_coil_seeds.size(); ++i)
	{
		inter_coil_seeds.at(i).Start += jitter * D3DXVECTOR3(Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f));
		inter_coil_seeds.at(i).End += jitter * D3DXVECTOR3(Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f));
	}

	
	switch (type)
	{
	case TestLightning::Type::EConnector:
		Settings.InterCoil = true;
		m_inter_coil_lightning = m_lightning_renderer->CreatePathLightning(inter_coil_seeds, 0x08, 5);
		m_inter_coil_lightning->Structure = structure[(UINT)type];
		break;
	case TestLightning::Type::EFence:
		Settings.Fence = true;
		m_fence_lightning = m_lightning_renderer->CreatePathLightning(inter_coil_seeds, 0x08, 5);
		m_fence_lightning->Structure = structure[(UINT)type];
		break;
	case TestLightning::Type::ECoil:
		Settings.CoilHelix = true;
		m_coil_helix_lightning = m_lightning_renderer->CreatePathLightning(inter_coil_seeds, 0x08, 5);
		m_coil_helix_lightning->Structure = structure[(UINT)type];
		break;
	case TestLightning::Type::EChain:
		Settings.Chain = true;
		m_chain_lightning = m_lightning_renderer->CreateChainLightning(0x08, 5);
		m_chain_lightning->Structure = structure[(UINT)type];
		break;
	case TestLightning::Type::Num:
		break;
	default:
		break;
	}
}


void TestLightning::ReadSeed(wstring fileName)
{
	ifstream file(fileName.c_str());
	D3DXVECTOR3 up(0, 1, 0);
	string cur_seed;

	while (file)
	{
		char line_buffer[512] = { 0 };
		file.getline(line_buffer, 512);

		stringstream line;

		line << line_buffer;

		std::string cur_command;

		line >> cur_command;

		if ("*NODE_NAME" == cur_command)
		{
			std::string name;
			line >> name;
			name.erase(name.size() - 1, 1);
			name.erase(0, 1);

			cur_seed = name;

			seeds[cur_seed].Name = name;
		}
		else if ("*SHAPE_CLOSED" == cur_command)
		{
			seeds[cur_seed].Closed = true;
		}
		else if ("*SHAPE_VERTEX_KNOT" == cur_command)
		{
			D3DXVECTOR3 vertex;

			int dummy;
			line >> dummy >> vertex.x >> vertex.z >> vertex.y;
			seeds[cur_seed].Vertices.push_back(vertex);
			seeds[cur_seed].InterpolatedVertices.push_back(vertex);
		}
		else if ("*SHAPE_VERTEX_INTERP" == cur_command)
		{
			D3DXVECTOR3 vertex;

			int dummy;
			line >> dummy >> vertex.x >> vertex.z >> vertex.y;
			seeds[cur_seed].InterpolatedVertices.push_back(vertex);
		}
	}

	std::vector<LightningPathSegment> the_seeds;

	std::vector<LightningPathSegment> fence_seeds;
	std::vector<LightningPathSegment> inter_coil_seeds;

	std::vector<LightningPathSegment> coil_helix_seeds;

	for (std::map<std::string, SeedRecord>::iterator it = seeds.begin(); it != seeds.end(); ++it)
	{
		std::vector<LightningPathSegment>* the_seeds = 0;
		std::vector<D3DXVECTOR3>* seeds = &(it->second.Vertices);

		if (it->first.find("CoilConnector") != std::string::npos && it->second.InterpolatedVertices.size() > 1)
		{
			seeds = &(it->second.InterpolatedVertices);
			the_seeds = &inter_coil_seeds;
		}
		else if (it->first.find("Fence") != std::string::npos)
		{
			the_seeds = &fence_seeds;
		}
		else if (it->first.find("TeslaCoilHelix") != std::string::npos)
		{
			the_seeds = &coil_helix_seeds;
		}
		else
		{
			continue;
		}

		// We duplicate the seed lines for the beams between the tesla coils in order to make the beam thicker

		int replicates = 1;
		float jitter = 0.0f;

		if (it->first.find("CoilConnector") != std::string::npos)
		{
			jitter = 1.0f;
			replicates = 10;
		}

		for (int j = 0; j < replicates; ++j)
		{
			if (seeds->size() > 1)
			{
				for (size_t i = 0; i <seeds->size() - 1; ++i)
				{
					the_seeds->push_back
					(
						LightningPathSegment
						(
						(*seeds)[i],
							(*seeds)[i + 1],
							up
						)
					);
				}

				if (it->second.Closed)
				{
					the_seeds->push_back
					(
						LightningPathSegment
						(
						(*seeds)[seeds->size() - 1],
							(*seeds)[0],
							up
						)
					);
				}
			}
		}
		
		
		for (size_t i = 0; i < the_seeds->size(); ++i)
		{
			the_seeds->at(i).Start += jitter * D3DXVECTOR3(Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f));
			the_seeds->at(i).End += jitter * D3DXVECTOR3(Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f));
		}
	}

	m_fence_lightning = m_lightning_renderer->CreatePathLightning(fence_seeds, 0x00, 5);
	m_inter_coil_lightning = m_lightning_renderer->CreatePathLightning(inter_coil_seeds, 0x08, 5);
	m_coil_helix_lightning = m_lightning_renderer->CreatePathLightning(coil_helix_seeds, 0x03, 5);
	m_chain_lightning = m_lightning_renderer->CreateChainLightning(0x0C, 5);
}

void TestLightning::CreateLightning()
{
	//	Connector
	{
		structure[(UINT)Type::EConnector].ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EConnector].ZigZagDeviationRight = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::EConnector].ZigZagDeviationUp = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::EConnector].ZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EConnector].ForkFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EConnector].ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EConnector].ForkDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkDeviationForward = D3DXVECTOR2(0.0f, 1.0f);
		structure[(UINT)Type::EConnector].ForkDeviationDecay = 0.5f;
		structure[(UINT)Type::EConnector].ForkLength = D3DXVECTOR2(1.0f, 2.0f);
		structure[(UINT)Type::EConnector].ForkLengthDecay = 0.01f;
	}

	//	Fence
	{
		structure[(UINT)Type::EFence].ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EFence].ZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EFence].ForkFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EFence].ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EFence].ForkDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkDeviationForward = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EFence].ForkDeviationDecay = 0.5f;
		structure[(UINT)Type::EFence].ForkLength = D3DXVECTOR2(1.0f, 2.0f);
		structure[(UINT)Type::EFence].ForkLengthDecay = 0.01f;
	}
	
	//	Coil
	{
		structure[(UINT)Type::ECoil].ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::ECoil].ZigZagDeviationRight = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::ECoil].ZigZagDeviationUp = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::ECoil].ZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::ECoil].ForkFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::ECoil].ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::ECoil].ForkDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkDeviationForward = D3DXVECTOR2(0.0f, 1.0f);
		structure[(UINT)Type::ECoil].ForkDeviationDecay = 0.5f;
		structure[(UINT)Type::ECoil].ForkLength = D3DXVECTOR2(1.0f, 2.0f);
		structure[(UINT)Type::ECoil].ForkLengthDecay = 0.01f;
	}

	//	Chain
	{
		structure[(UINT)Type::EChain].ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EChain].ZigZagDeviationRight = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::EChain].ZigZagDeviationUp = D3DXVECTOR2(-5.0f, 5.0f);
		structure[(UINT)Type::EChain].ZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EChain].ForkFraction = D3DXVECTOR2(0.45f, 0.55f);
		structure[(UINT)Type::EChain].ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkZigZagDeviationDecay = 0.5f;
		structure[(UINT)Type::EChain].ForkDeviationRight = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkDeviationUp = D3DXVECTOR2(-1.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkDeviationForward = D3DXVECTOR2(0.0f, 1.0f);
		structure[(UINT)Type::EChain].ForkDeviationDecay = 0.5f;
		structure[(UINT)Type::EChain].ForkLength = D3DXVECTOR2(1.0f, 2.0f);
		structure[(UINT)Type::EChain].ForkLengthDecay = 0.01f;
	}

	{
		m_blue_beam.BoltWidth = D3DXVECTOR2(0.125f, 0.5f);
		m_blue_beam.ColorInside = D3DXVECTOR3(1, 1, 1);
		m_blue_beam.ColorOutside = D3DXVECTOR3(0, 0, 1);
		m_blue_beam.ColorFallOffExponent = 2.0f;
	}

	{
		m_blue_cyan_beam.BoltWidth = D3DXVECTOR2(0.25f, 0.5f);
		m_blue_cyan_beam.ColorInside = D3DXVECTOR3(0, 1, 1);
		m_blue_cyan_beam.ColorOutside = D3DXVECTOR3(0, 0, 1);
		m_blue_cyan_beam.ColorFallOffExponent = 5.0f;
	}

	{
		m_red_beam.BoltWidth = D3DXVECTOR2(0.5f, 0.5f);
		m_red_beam.ColorInside = D3DXVECTOR3(1, 1, 1);
		m_red_beam.ColorOutside = D3DXVECTOR3(1, 0, 0);
		m_red_beam.ColorFallOffExponent = 5.0f;
	}
	{
		Settings.Beam.BoltWidth = D3DXVECTOR2(0.5f, 0.5f);
		Settings.Beam.ColorInside = D3DXVECTOR3(1, 1, 1);
		Settings.Beam.ColorOutside = D3DXVECTOR3(1, 1, 0);
		Settings.Beam.ColorFallOffExponent = 5.0f;
	}

	{
		ReadSeed(Textures + L"seeds.ASE");
		m_inter_coil_lightning->Structure = structure[(UINT)Type::EConnector];
		m_fence_lightning->Structure = structure[(UINT)Type::EFence];
		m_coil_helix_lightning->Structure = structure[(UINT)Type::ECoil];
		
		m_chain_lightning->Structure = structure[(UINT)Type::EChain];
	}
}
