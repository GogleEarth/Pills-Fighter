#include "stdafx.h"
#include "Sound.h"

CSound::CSound()
{
	FMOD::System_Create(&m_pfmodSystem);

	m_pfmodSystem->init(FMOD_MAX_CHANNEL_WIDTH, FMOD_INIT_NORMAL, NULL);
}

CSound::~CSound()
{
	m_pfmodSystem->close();
	m_pfmodSystem->release();
}

void CSound::PlayFMODSoundLoop(FMOD::Sound *pSound, FMOD::Channel *pChannel)
{
	bool bPlaying = false;
	pChannel->isPlaying(&bPlaying);

	if (!bPlaying) m_pfmodSystem->playSound(pSound, NULL, false, &pChannel);
	else
	{
		ResumeFMODSound(pChannel);
	}
}

void CSound::PlayFMODSound(FMOD::Sound *pSound)
{
	FMOD::Channel *pChannel;

	m_pfmodSystem->playSound(pSound, NULL, false, &pChannel);

	m_pfmodSystem->update();
}

void CSound::ResumeFMODSound(FMOD::Channel *pChannel)
{
	bool bPause = false;
	pChannel->getPaused(&bPause);

	if (bPause) pChannel->setPaused(false);
}

void CSound::PauseFMODSound(FMOD::Channel *pChannel)
{
	bool bPause = false;
	pChannel->getPaused(&bPause);

	if(!bPause) pChannel->setPaused(true);
}

//////////////////////////////////////////////////////////////////////

CColonySceneSound::CColonySceneSound() : CSound()
{
	m_pfmodSystem->createSound("./Resource/BGM/bgm_pvp_p03.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pSoundBGM);
}

CColonySceneSound::~CColonySceneSound()
{
	m_pSoundBGM->release();
}

//////////////////////////////////////////////////////////////////////

CRobotObjectSound::CRobotObjectSound() : CSound()
{
	m_pfmodSystem->createSound("./Resource/SE/Gun_Hit.wav", FMOD_DEFAULT, NULL, &m_pSoundGGHit);
	m_pfmodSystem->createSound("./Resource/SE/Gun_Shot.wav", FMOD_DEFAULT, NULL, &m_pSoundGGShot);
	m_pfmodSystem->createSound("./Resource/SE/BZK_Hit.wav", FMOD_DEFAULT, NULL, &m_pSoundBZKHit);
	m_pfmodSystem->createSound("./Resource/SE/BZK_Shot.wav", FMOD_DEFAULT, NULL, &m_pSoundBZKShot);
	m_pfmodSystem->createSound("./Resource/SE/MG_Shot.wav", FMOD_DEFAULT, NULL, &m_pSoundMGShot);
	m_pfmodSystem->createSound("./Resource/SE/Move.wav", FMOD_DEFAULT, NULL, &m_pSoundMove);
	m_pfmodSystem->createSound("./Resource/SE/Booster.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pSoundBooster);
}

CRobotObjectSound::~CRobotObjectSound()
{
	m_pSoundGGHit->release();
	m_pSoundGGShot->release();
	m_pSoundBZKHit->release();
	m_pSoundBZKShot->release();
	m_pSoundMGShot->release();
	m_pSoundMove->release();
	m_pSoundBooster->release();
}