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

	//	애니메이션 섞는 함수
	void UpdateBlending(class ModelBone* bone, float time);

private:
	class ModelClip* current;	//	현재 동작
	class ModelClip* next;		//	다음 동작

	float blendTime;			//	섞일 시간
	float elapsedTime;			//	이전 프레임에서 현재 프레임까지의 시간

	bool isPaused;
};