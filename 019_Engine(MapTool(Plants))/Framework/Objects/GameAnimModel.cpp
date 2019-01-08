#include "Framework.h"
#include "GameAnimModel.h"
#include "../Models/ModelBone.h"
#include "../Models/ModelClip.h"
#include "../Models/ModelTweener.h"

GameAnimModel::GameAnimModel(wstring matFolder, wstring matFile, wstring meshFolder, wstring meshFile)
	: GameModel(matFolder, matFile, meshFolder, meshFile)
{
	for (ModelMesh * mesh : model->Meshes())
		mesh->Pass(1);

	tweener = new ModelTweener();
}


GameAnimModel::~GameAnimModel()
{
	for (ModelClip* clip : clips)
		SAFE_DELETE(clip);

	SAFE_DELETE(tweener);
}

void GameAnimModel::Update()
{
	__super::Update();

	if (clips.size() < 1) return;

	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone* bone = model->BoneByIndex(i);
		tweener->UpdateBlending(bone, Time::Delta());
	}
	UpdateWorld();
}

UINT GameAnimModel::AddClip(wstring file)
{
	ModelClip* clip = new ModelClip(file);
	clips.push_back(clip);

	return clips.size() - 1;
}

void GameAnimModel::LockRoot(UINT index, bool val)
{
	clips[index]->LockRoot(val);
}

void GameAnimModel::Repeat(UINT index, bool val)
{
	clips[index]->Repeat(val);
}

void GameAnimModel::Speed(UINT index, float val)
{
	clips[index]->Speed(val);
}

void GameAnimModel::Reset(UINT index)
{
	tweener->Reset(clips[index]);
	D3DXMATRIX matrix;
	D3DXMatrixIdentity(&matrix);
	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		model->BoneByIndex(i)->Local(matrix);
	}
	Update();
}

void GameAnimModel::Pause(UINT index)
{
	tweener->Pause(clips[index]);
}

bool GameAnimModel::IsPlay(UINT index)
{
	return tweener->IsPlay(clips[index]);
}

void GameAnimModel::Play(UINT index, bool bRepeat, float blendTime, float speed, float startTime)
{
	tweener->Play(clips[index], bRepeat, blendTime, speed, startTime);
}

void GameAnimModel::ShowFrameData(UINT index)
{
	clips[index]->PostRender();
}
