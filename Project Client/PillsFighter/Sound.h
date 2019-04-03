#pragma once
#include "stdafx.h"

class CSound
{
public:
	CSound();
	virtual ~CSound();

	void PlayFMODSound(FMOD::Sound *pSound);
	void PlayFMODSoundLoop(FMOD::Sound *pSound, FMOD::Channel *pChannel);
	void ResumeFMODSound(FMOD::Channel *pChannel);
	void PauseFMODSound(FMOD::Channel *pChannel);

protected:
	FMOD::System* m_pfmodSystem = NULL;
};

//////////////////////////////////////////////////////////////////////

class CColonySceneSound : public CSound
{
public:
	CColonySceneSound();
	virtual ~CColonySceneSound();

	FMOD::Sound *m_pSoundBGM;
	FMOD::Channel *m_pChannelBGM;
};

//////////////////////////////////////////////////////////////////////

class CRobotObjectSound : public CSound
{
public:
	CRobotObjectSound();
	virtual ~CRobotObjectSound();
	
	FMOD::Sound *m_pSoundGGHit;
	FMOD::Sound *m_pSoundGGShot;
	FMOD::Sound *m_pSoundBZKHit;
	FMOD::Sound *m_pSoundBZKShot;
	FMOD::Sound *m_pSoundMGShot;
	FMOD::Sound *m_pSoundMove;

	FMOD::Sound *m_pSoundBooster;
	FMOD::Channel *m_pChannelBooster;
};