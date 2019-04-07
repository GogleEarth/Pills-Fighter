#pragma once
#include "stdafx.h"

class CFMODSound
{
public:
	CFMODSound();
	virtual ~CFMODSound();

	void PlayFMODSound(FMOD::Sound *pSound);
	void PlayFMODSoundLoop(FMOD::Sound *pSound, FMOD::Channel **pChannel);
	void ResumeFMODSound(FMOD::Channel *pChannel);
	void PauseFMODSound(FMOD::Channel *pChannel);

	FMOD::System* m_pfmodSystem = NULL;

	FMOD::Sound *m_pSoundBGM = NULL;
	FMOD::Channel *m_pBGMChannel = NULL;

	FMOD::Sound *m_pSoundGGHit = NULL;
	FMOD::Sound *m_pSoundGGShot = NULL;
	FMOD::Sound *m_pSoundBZKHit = NULL;
	FMOD::Sound *m_pSoundBZKShot = NULL;
	FMOD::Sound *m_pSoundMGShot = NULL;
	FMOD::Sound *m_pSoundMove = NULL;
	FMOD::Sound *m_pSoundBooster = NULL;
};