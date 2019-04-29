#include "Framework.h"
#include "SoundManager.h"


SoundManager*SoundManager::instance = NULL;

void SoundManager::Create()
{
	if (instance == NULL)
	{
		instance = new SoundManager();
		instance->init();
	}
}

void SoundManager::Delete()
{
	SAFE_DELETE(instance);
}

SoundManager::SoundManager()
	:_system(NULL), _sound(NULL), _channel(NULL)
{
}


SoundManager::~SoundManager()
{
}

HRESULT SoundManager::init()
{
	//fmod½Ã½ºÅÛ µî·Ï
	System_Create(&_system);

	_system->init(TOTALSOUNDBUFFER, FMOD_INIT_NORMAL, NULL);

	_sound = new Sound*[TOTALSOUNDBUFFER];
	_channel = new Channel*[TOTALSOUNDBUFFER];

	ZeroMemory(_sound, sizeof(_sound));
	ZeroMemory(_channel, sizeof(_channel));

	return S_OK;
}

void SoundManager::release()
{
	
}

void SoundManager::update()
{
	_system->update();
}

void SoundManager::addSound(string keyName, string soundName, bool bgm, bool loop)
{
	if (loop)
	{
		if (bgm)
		{
			//±ä À½¾ÇÀÏ¶§´Â createStreamÀ¸·Î 
			_system->createStream(soundName.c_str(), FMOD_LOOP_NORMAL, NULL, &_sound[_mTotalSounds.size()]);
		}
		else
		{
			_system->createSound(soundName.c_str(), FMOD_LOOP_NORMAL, NULL, &_sound[_mTotalSounds.size()]);
		}
	}
	else
	{
		if (bgm)
		{
			_system->createStream(soundName.c_str(), FMOD_DEFAULT, NULL, &_sound[_mTotalSounds.size()]);
		}
		else
		{
			_system->createSound(soundName.c_str(), FMOD_DEFAULT, NULL, &_sound[_mTotalSounds.size()]);
		}
	}
	_mTotalSounds.insert(make_pair(keyName, &_sound[_mTotalSounds.size()]));
}

void SoundManager::play(string keyName, float volume)
{
	arrSoundsIter iter = _mTotalSounds.begin();

	int count = 0;

	for (iter; iter != _mTotalSounds.end(); iter++, count++)
	{
		if (keyName == iter->first)
		{
			_system->playSound(FMOD_CHANNEL_FREE, _sound[count], false, &_channel[count]);

			_channel[count]->setVolume(volume);
			break;
		}
	}
}

void SoundManager::stop(string keyName)
{
	arrSoundsIter iter = _mTotalSounds.begin();

	int count = 0;

	for (iter; iter != _mTotalSounds.end(); iter++, count++)
	{
		if (keyName == iter->first)
		{
			//³ë·¡¸¦ ¾Æ¾Ö Á¾·áÇÏ´Â °³³ä
			_channel[count]->stop();
			break;
		}
	}
}

void SoundManager::pause(string keyName)
{
	arrSoundsIter iter = _mTotalSounds.begin();

	int count = 0;

	for (iter; iter != _mTotalSounds.end(); iter++, count++)
	{
		if (keyName == iter->first)
		{
			//³ë·¡¸¦ Àá½Ã¸ØÃã
			_channel[count]->setPaused(true);
			break;
		}
	}
}

void SoundManager::resume(string keyName)
{
	arrSoundsIter iter = _mTotalSounds.begin();

	int count = 0;

	for (iter; iter != _mTotalSounds.end(); iter++, count++)
	{
		if (keyName == iter->first)
		{
			//³ë·¡¸¦ Àá½Ã¸ØÃã
			_channel[count]->setPaused(false);
			break;
		}
	}
}

void SoundManager::Update()
{
	_system->update();
}

void SoundManager::SetVolume(string keyName, float volume)
{
	arrSoundsIter iter = _mTotalSounds.begin();

	int count = 0;
	float value = 0;
	for (iter; iter != _mTotalSounds.end(); iter++, count++)
	{
		if (keyName == iter->first)
		{
			_channel[count]->getVolume(&value);
			if (value == volume)break;

			_channel[count]->setVolume(volume);
			break;
		}
	}
}


bool SoundManager::isPlaySound(string keyName)
{
	bool isPlay;

	arrSoundsIter iter = _mTotalSounds.begin();

	int count = 0;

	for (iter; iter != _mTotalSounds.end(); iter++, count++)
	{
		if (keyName == iter->first)
		{
			_channel[count]->isPlaying(&isPlay);
			break;
		}
	}

	return isPlay;
}

bool SoundManager::isPauseSound(string keyName)
{
	bool isPause;

	arrSoundsIter iter = _mTotalSounds.begin();

	int count = 0;

	for (iter; iter != _mTotalSounds.end(); iter++, count++)
	{
		if (keyName == iter->first)
		{
			_channel[count]->getPaused(&isPause);
			break;
		}
	}

	return isPause;
}