#include "Framework.h"
#include "InstanceManager.h"

InstanceManager*InstanceManager::instance = NULL;

void InstanceManager::Create()
{
	if (instance == NULL)
		instance = new InstanceManager();
}

void InstanceManager::Delete()
{
	SAFE_DELETE(instance);
}

InstanceManager::InstanceManager()
{
}

InstanceManager::~InstanceManager()
{
	modelsIter msIter = modelInstance.begin();

	for (; msIter != modelInstance.end(); msIter++)
	{
		SAFE_DELETE(msIter->second);
	}

	animsIter asIter = animInstance.begin();

	for (; asIter != animInstance.end(); asIter++)
	{
		SAFE_DELETE(asIter->second);
	}
}

void InstanceManager::AddModel(GameModel * model, wstring shaderFile, InstanceType instType)
{
	Model *m = model->GetModel();
	wstring modelName = m->GetModelName();
	switch (instType)
	{
		case InstanceManager::InstanceType::MODEL:
		{
			modelsIter msIter = modelInstance.find(modelName);
			if (msIter != modelInstance.end()) // 기존에 해당모델의 인스턴스가 존재하면
			{
				msIter->second->AddWorld(model->Transformed());
			}
			else
			{
				ModelInstance *mInst = new ModelInstance(model->GetModel(), shaderFile);
				mInst->Ready();
				mInst->AddWorld(model->Transformed());

				modelInstance[modelName] = mInst;
			}
		}
		break;
		case InstanceManager::InstanceType::ANIMATION:
		{
			animsIter asIter = animInstance.find(modelName);
			if (asIter != animInstance.end()) // 기존에 해당모델의 인스턴스가 존재하면
			{
				asIter->second->AddWorld((GameAnimator*)model);
			}
			else
			{
				AnimInstance *aInst = new AnimInstance(model->GetModel(), shaderFile);
				aInst->Ready();
				aInst->AddWorld((GameAnimator*)model);

				animInstance[modelName] = aInst;
			}
		}
		break;
	}
}

void InstanceManager::AddModel(GameModel * model, Effect * effect, InstanceType instType)
{
	Model *m = model->GetModel();
	wstring modelName = m->GetModelName();
	switch (instType)
	{
		case InstanceManager::InstanceType::MODEL:
		{
			modelsIter msIter = modelInstance.find(modelName);
			if (msIter != modelInstance.end()) // 기존에 해당모델의 인스턴스가 존재하면
			{
				msIter->second->AddWorld(model->World());
			}
			else
			{
				ModelInstance *mInst = new ModelInstance(model->GetModel(), effect);
				mInst->Ready();
				mInst->AddWorld(model->World());

				modelInstance[modelName] = mInst;
			}
		}
		break;
		case InstanceManager::InstanceType::ANIMATION:
		{
			animsIter asIter = animInstance.find(modelName);
			if (asIter != animInstance.end()) // 기존에 해당모델의 인스턴스가 존재하면
			{
				asIter->second->AddWorld((GameAnimator*)model);
			}
			else
			{
				AnimInstance *aInst = new AnimInstance(model->GetModel(), effect);
				aInst->Ready();
				aInst->AddWorld((GameAnimator*)model);

				animInstance[modelName] = aInst;
			}
		}
		break;
	}
}

void InstanceManager::DeleteModel(GameModel * model, UINT instanceID, InstanceType instType)
{
	Model *m = model->GetModel();
	wstring modelName = m->GetModelName();
	switch (instType)
	{
		case InstanceManager::InstanceType::MODEL:
		{
			modelsIter msIter = modelInstance.find(modelName);
			if (msIter != modelInstance.end()) // 기존에 해당모델의 인스턴스가 존재하면
			{
				msIter->second->DeleteWorld(instanceID);
			}
		}
		break;
		case InstanceManager::InstanceType::ANIMATION:
		{
			animsIter asIter = animInstance.find(modelName);
			if (asIter != animInstance.end()) // 기존에 해당모델의 인스턴스가 존재하면
			{
				asIter->second->DeleteWorld(instanceID);
			}
		}
		break;
	}
}

void InstanceManager::UpdateWorld(GameModel * model, UINT instNum, D3DXMATRIX & world, InstanceType instType)
{
	Model *m = model->GetModel();
	wstring modelName = m->GetModelName();

	switch (instType)
	{
		case InstanceManager::InstanceType::MODEL:
		{
			modelsIter msIter = modelInstance.find(modelName);
			if (msIter != modelInstance.end()) // 기존에 해당모델의 인스턴스가 존재하면
			{
				msIter->second->UpdateWorld(instNum, world);
			}
		}
		break;
		case InstanceManager::InstanceType::ANIMATION:
		{

			animsIter asIter = animInstance.find(modelName);
			if (asIter != animInstance.end()) // 기존에 해당모델의 인스턴스가 존재하면
			{
				asIter->second->UpdateWorld(instNum, world);
			}
		}
		break;
	}
}

ModelInstance * InstanceManager::FindModel(wstring name)
{
	modelsIter msIter = modelInstance.find(name);

	if (msIter != modelInstance.end())
		return msIter->second;
	else
		return NULL;
}

AnimInstance * InstanceManager::FindAnim(wstring name)
{
	animsIter asIter = animInstance.find(name);

	if (asIter != animInstance.end())
		return asIter->second;
	else
		return NULL;
}

void InstanceManager::Render(UINT tech)
{
	modelsIter msIter = modelInstance.begin();

	for (; msIter != modelInstance.end(); msIter++)
	{
		msIter->second->Render(tech);
	}

	animsIter asIter = animInstance.begin();

	for (; asIter != animInstance.end(); asIter++)
	{
		asIter->second->Render(tech);
	}
}

void InstanceManager::Render(wstring name)
{
	modelsIter msIter = modelInstance.find(name);
	if (msIter != modelInstance.end())
		msIter->second->Render();

	animsIter asIter = animInstance.find(name);
	if (asIter != animInstance.end())
		asIter->second->Render();
}

void InstanceManager::Update()
{
	modelsIter msIter = modelInstance.begin();

	for (; msIter != modelInstance.end(); msIter++)
	{
		msIter->second->Update();
	}

	animsIter asIter = animInstance.begin();

	for (; asIter != animInstance.end(); asIter++)
	{
		asIter->second->Update();
	}
}

void InstanceManager::Empty()
{
	modelsIter msIter = modelInstance.begin();

	for (; msIter != modelInstance.end(); msIter++)
	{
		SAFE_DELETE(msIter->second);
	}
	modelInstance.clear();

	animsIter asIter = animInstance.begin();

	for (; asIter != animInstance.end(); asIter++)
	{
		SAFE_DELETE(asIter->second);
	}
	animInstance.clear();
}

