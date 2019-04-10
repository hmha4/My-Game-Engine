#include "Framework.h"
#include "ProgressBar.h"

ProgressBar * ProgressBar::instance = NULL;

ProgressBar::ProgressBar()
	: progress(0.0f)
{
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::Render()
{
	ImGui::Begin("Progress");
	{
		ImGui::ProgressBar(progress);
	}
	ImGui::End();
}

void ProgressBar::Create()
{
	if (instance != NULL)
		return;

	instance = new ProgressBar();
}

void ProgressBar::Delete()
{
	if (instance == NULL)
		return;

	SAFE_DELETE(instance);
}