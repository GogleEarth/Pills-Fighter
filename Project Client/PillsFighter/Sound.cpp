#include "stdafx.h"
#include "Sound.h"

CFMODSound::CFMODSound()
{
	FMOD::System_Create(&m_pfmodSystem);

	m_pfmodSystem->init(FMOD_MAX_CHANNEL_WIDTH, FMOD_INIT_NORMAL, NULL);

	m_pfmodSystem->createSound("./Resource/BGM/bgm_pvp_p03.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pSoundBGM);

	m_pfmodSystem->createSound("./Resource/SE/Gun_Hit.wav", FMOD_DEFAULT, NULL, &m_pSoundGGHit);
	m_pfmodSystem->createSound("./Resource/SE/Gun_Shot.wav", FMOD_DEFAULT, NULL, &m_pSoundGGShot);
	m_pfmodSystem->createSound("./Resource/SE/BZK_Hit.wav", FMOD_DEFAULT, NULL, &m_pSoundBZKHit);
	m_pfmodSystem->createSound("./Resource/SE/BZK_Shot.wav", FMOD_DEFAULT, NULL, &m_pSoundBZKShot);
	m_pfmodSystem->createSound("./Resource/SE/MG_Shot.wav", FMOD_DEFAULT, NULL, &m_pSoundMGShot);
	m_pfmodSystem->createSound("./Resource/SE/Move.wav", FMOD_DEFAULT, NULL, &m_pSoundMove);
	m_pfmodSystem->createSound("./Resource/SE/PickAmmo.wav", FMOD_DEFAULT, NULL, &m_pSoundPickAmmo);
	m_pfmodSystem->createSound("./Resource/SE/PickHeal.wav", FMOD_DEFAULT, NULL, &m_pSoundPickHeal);
	m_pfmodSystem->createSound("./Resource/SE/Booster.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pSoundBooster);
}

CFMODSound::~CFMODSound()
{
	m_pSoundBGM->release();
	m_pSoundGGHit->release();
	m_pSoundGGShot->release();
	m_pSoundBZKHit->release();
	m_pSoundBZKShot->release();
	m_pSoundMGShot->release();
	m_pSoundMove->release();
	m_pSoundBooster->release();
	m_pSoundPickAmmo->release();
	m_pSoundPickHeal->release();

	m_pfmodSystem->close();
	m_pfmodSystem->release();
}

void CFMODSound::PlayFMODSoundLoop(FMOD::Sound *pSound, FMOD::Channel **pChannel)
{
	bool bPlaying = false;

	if (*pChannel) (*pChannel)->isPlaying(&bPlaying);

	if (bPlaying) ResumeFMODSound(*pChannel); 
	else m_pfmodSystem->playSound(pSound, NULL, false, pChannel);
}

void CFMODSound::PlayFMODSound(FMOD::Sound *pSound)
{
	FMOD::Channel *pChannel;

	m_pfmodSystem->playSound(pSound, NULL, false, &pChannel);

	m_pfmodSystem->update();
}

void CFMODSound::ResumeFMODSound(FMOD::Channel *pChannel)
{
	if (!pChannel) return;

	bool bPause = false;
	pChannel->getPaused(&bPause);

	if (bPause) pChannel->setPaused(false);
}

void CFMODSound::PauseFMODSound(FMOD::Channel *pChannel)
{
	if (!pChannel) return;

	bool bPause = false;

	pChannel->getPaused(&bPause);

	if (!bPause) pChannel->setPaused(true);
}
