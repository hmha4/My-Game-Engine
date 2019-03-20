#include "Framework.h"
#include "ModelClip.h"
#include "../Utilities/BinaryFile.h"

ModelClip::ModelClip(wstring file)
	: startTime(0.0f), speed(1.0f)
	, bLockRoot(false), bRepeat(false)
	, isPlay(false)
{
	BinaryReader * r = new BinaryReader();
	r->Open(file);

	name = String::ToWString(r->String());
	duration = r->Float();
	frameRate = r->Float();
	frameCount = r->UInt();

	UINT keyframesCount = r->UInt();
	for (UINT i = 0; i < keyframesCount; i++)
	{
		ModelKeyframe * keyframe = new ModelKeyframe();
		keyframe->BoneName = String::ToWString(r->String());

		r->UInt();
		r->UInt();

		UINT size = r->UInt();
		if (size > 0)
		{
			keyframe->Transforms.assign(size, ModelKeyframeData());

			void * ptr = (void *)&keyframe->Transforms[0];
			r->Byte(&ptr, sizeof(ModelKeyframeData) * size);
		}

		keyframeMap[keyframe->BoneName] = keyframe;
		keyframeVector.push_back(keyframeMap[keyframe->BoneName]);
	}

	r->Close();
	SAFE_DELETE(r);
}

ModelClip::~ModelClip()
{
	for (keyframePair keyframe : keyframeMap)
		SAFE_DELETE(keyframe.second);
}

void ModelClip::Reset()
{
	bRepeat = false;
	speed = 0.0f;
	startTime = 0.0f;
	isPlay = false;
}

ModelKeyframe * ModelClip::Keyframe(wstring name)
{
	if (keyframeMap.count(name) < 1)
		return NULL;

	return keyframeMap[name];
}

ModelKeyframe * ModelClip::Keyframe(UINT idx)
{
	if (keyframeVector.size() <= idx)
		return NULL;

	//TODO: fix if the head bone index is different
	return keyframeVector[idx];
}