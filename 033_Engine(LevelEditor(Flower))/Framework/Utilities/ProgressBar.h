#pragma once

class ProgressBar
{
public:
	ProgressBar();
	~ProgressBar();

	void Progress(float val) { progress = val; }

	void Render();

public:
	static void Create();
	static void Delete();
	static ProgressBar * Get() { return instance; }

private:
	static ProgressBar * instance;

	float progress;
};