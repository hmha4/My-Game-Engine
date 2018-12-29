#pragma once


class ModelTweener
{
public:
	ModelTweener();
	~ModelTweener();
	
	bool IsPlay(class ModelClip* clip);

	void Reset(class ModelClip* clip);
	void Pause(class ModelClip* clip);
	void Play
	(
		class ModelClip* clip,
		bool bRepeat = false,
		float blendTime = 0.0f,
		float speed = 1.0f,
		float startTime = 0.0f
	);

	//	�ִϸ��̼� ���� �Լ�
	void UpdateBlending(class ModelBone* bone, float time);

private:
	class ModelClip* current;	//	���� ����
	class ModelClip* next;		//	���� ����

	float blendTime;			//	���� �ð�
	float elapsedTime;			//	���� �����ӿ��� ���� �����ӱ����� �ð�

	bool isPaused;
};