#pragma once
#include "stdafx.h"

class CSound
{
public:
	CSound();
	virtual ~CSound();

protected:
	FMOD::System* m_pfmodSystem = NULL;
};

//////////////////////////////////////////////////////////////////////

class CColonySceneSound : public CSound
{
public:
	CColonySceneSound();
	virtual ~CColonySceneSound();

	void PlayBGM() { m_pfmodSystem->playSound(m_pfmodBGM, NULL, 0, &m_pfmodChannelBGM); }
	void ResumeBGM() { m_pfmodChannelBGM->setPaused(false); }
	void StopBGM() { m_pfmodChannelBGM->setPaused(true); }

protected:
	FMOD::Sound* m_pfmodBGM = NULL;
	FMOD::Channel* m_pfmodChannelBGM = NULL;
};

//////////////////////////////////////////////////////////////////////

class CRobotObjectSound : public CSound
{
public:
	CRobotObjectSound();
	virtual ~CRobotObjectSound();

	void PlayGGHit() { m_pfmodSystem->playSound(m_pfmodGGHit, NULL, 0, &m_pfmodChannelGGHit); }
	void PlayGGShot() { m_pfmodSystem->playSound(m_pfmodGGShot, NULL, 0, &m_pfmodChannelGGShot); }

	void PlayBZKHit() { m_pfmodSystem->playSound(m_pfmodBZKHit, NULL, 0, &m_pfmodChannelBZKHit); }
	void PlayBZKShot() { m_pfmodSystem->playSound(m_pfmodBZKShot, NULL, 0, &m_pfmodChannelBZKShot); }

	void PlayMGShot() { m_pfmodSystem->playSound(m_pfmodMGShot, NULL, 0, &m_pfmodChannelMGShot); }

	void PlayMove() { m_pfmodSystem->playSound(m_pfmodMove, NULL, 0, &m_pfmodChannelMove); }
	void PlayBooster();
	void PauseBooster();

protected:
	FMOD::Sound* m_pfmodGGHit = NULL;
	FMOD::Sound* m_pfmodGGShot = NULL;
	FMOD::Sound* m_pfmodBZKHit = NULL;
	FMOD::Sound* m_pfmodBZKShot = NULL;
	FMOD::Sound* m_pfmodMGShot = NULL;
	FMOD::Sound* m_pfmodMove = NULL;
	FMOD::Sound* m_pfmodBooster = NULL;
	bool bPlayBooster = false;

	FMOD::Channel* m_pfmodChannelGGHit = NULL;
	FMOD::Channel* m_pfmodChannelGGShot = NULL;
	FMOD::Channel* m_pfmodChannelBZKHit = NULL;
	FMOD::Channel* m_pfmodChannelBZKShot = NULL;
	FMOD::Channel* m_pfmodChannelMGShot = NULL;
	FMOD::Channel* m_pfmodChannelMove = NULL;
	FMOD::Channel* m_pfmodChannelBooster = NULL;
};


