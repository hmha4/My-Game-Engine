#include "Framework.h"
#include "ModelClip.h"
#include "ModelBone.h"
#include "../Utilities/BinaryFile.h"

ModelClip::ModelClip(wstring file)
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
			if (i == 64)
				int a = 10;
			keyframe->Transforms.assign(size, ModelKeyframeData());

			void * ptr = (void *)&keyframe->Transforms[0];
			r->Byte(&ptr, sizeof(ModelKeyframeData) * size);
		}

		keyframeMap[keyframe->BoneName] = keyframe;
	}

	r->Close();
	SAFE_DELETE(r);
}

ModelClip::~ModelClip()
{
	for (keyframePair keyframe : keyframeMap)
		SAFE_DELETE(keyframe.second);
}

ModelKeyframe * ModelClip::Keyframe(wstring name)
{
	if (keyframeMap.count(name) < 1)
		return NULL;

	return keyframeMap[name];
}
