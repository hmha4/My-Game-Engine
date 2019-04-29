#pragma once

#include "../inc/fmod.hpp"
#pragma comment(lib,"fmodex_vc.lib")
using namespace FMOD;

#define SOUNDBUFFER               500
#define EXTRACHANNELBUFFER      250
#define TOTALSOUNDBUFFER SOUNDBUFFER+EXTRACHANNELBUFFER

class SoundManager
{
public:
	static void Create();
	static void Delete();

	static SoundManager* Get() { return instance; }
private:
	static SoundManager*instance;


private:
	typedef map<string, Sound**>                  arrSounds;
	typedef map<string, Sound**>::iterator      arrSoundsIter;

	typedef map<string, Channel**>               arrChannels;
	typedef map<string, Channel**>::iterator      arrChannelsIter;

private:
	System * _system;
	Sound**            _sound;
	Channel**         _channel;

	arrSounds         _mTotalSounds;
	int countIdx;
public:
	SoundManager();
	~SoundManager();

	HRESULT init();
	void release();
	void update();

	void addSound(string keyName, string soundName, bool bgm, bool loop);

	void play(string keyName, float volume = 1.0f);
	void stop(string keyName);
	void pause(string keyName);
	void resume(string keyName);
	void Update();
	void SetVolume(string keyName, float volume);


	bool isPlaySound(string keyName);
	bool isPauseSound(string keyName);
};