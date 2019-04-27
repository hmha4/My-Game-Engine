#pragma once

class ProgressBar
{
public:
	ProgressBar();
	~ProgressBar();

	void Progress(float val) { progress = val; }
	float Progress() { return progress; }

	void Render();

public:
	static void Create();
	static void Delete();
	static ProgressBar * Get() { return instance; }

private:
	static ProgressBar * instance;

private:
	float progress;
};