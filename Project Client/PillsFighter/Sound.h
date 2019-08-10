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
	void StopFMODSound(FMOD::Channel *pChannel);

	FMOD::System* m_pfmodSystem = NULL;

	FMOD::Sound *m_pSoundTitleBGM = NULL;
	FMOD::Sound *m_pSoundLobbyBGM = NULL;
	FMOD::Sound *m_pSoundColonyBGM = NULL;
	FMOD::Sound *m_pSoundSpaceBGM = NULL;
	FMOD::Channel *m_pBGMChannel = NULL;

	FMOD::Sound *m_pSoundGGHit = NULL;
	FMOD::Sound *m_pSoundGGShot = NULL;
	FMOD::Sound *m_pSoundBZKHit = NULL;
	FMOD::Sound *m_pSoundBZKShot = NULL;
	FMOD::Sound *m_pSoundMGShot = NULL;
	FMOD::Sound *m_pSoundMove = NULL;
	FMOD::Sound *m_pSoundBooster = NULL;
	FMOD::Sound *m_pSoundPickAmmo = NULL;
	FMOD::Sound *m_pSoundPickHeal = NULL;
	FMOD::Sound *m_pSoundSaberAttack = NULL;
	FMOD::Sound *m_pSoundSaberHit1 = NULL;
	FMOD::Sound *m_pSoundAlert = NULL;
	FMOD::Sound *m_pSoundBeamRifle = NULL;
	FMOD::Sound *m_pSoundDestroy = NULL;
	FMOD::Sound *m_pSoundBattleAlert = NULL;
};