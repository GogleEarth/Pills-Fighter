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

void CSound::PlayFMODSoundLoop(SoundMaterial *pSoundMaterial)
{
	bool bPlaying = false;
	pSoundMaterial->pChannel->isPlaying(&bPlaying);

	if (!bPlaying) m_pfmodSystem->playSound(pSoundMaterial->pSound, NULL, false, &(pSoundMaterial->pChannel));
	else
	{
		ResumeFMODSound(pSoundMaterial);
	}
}

void CSound::PlayFMODSound(SoundMaterial *pSoundMaterial)
{
	m_pfmodSystem->playSound(pSoundMaterial->pSound, NULL, false, &(pSoundMaterial->pChannel));
}

void CSound::ResumeFMODSound(SoundMaterial *pSoundMaterial)
{
	bool bPause = false;
	pSoundMaterial->pChannel->getPaused(&bPause);

	if (bPause) pSoundMaterial->pChannel->setPaused(false);
}

void CSound::PauseFMODSound(SoundMaterial *pSoundMaterial)
{
	bool bPause = false;
	pSoundMaterial->pChannel->getPaused(&bPause);

	if(!bPause) pSoundMaterial->pChannel->setPaused(true);
}

//////////////////////////////////////////////////////////////////////

CColonySceneSound::CColonySceneSound() : CSound()
{
	m_pfmodSystem->createSound("./Resource/BGM/bgm_pvp_p03.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &(m_BGM.pSound));
}

CColonySceneSound::~CColonySceneSound()
{
	m_BGM.pSound->release();
}

//////////////////////////////////////////////////////////////////////

CRobotObjectSound::CRobotObjectSound() : CSound()
{
	m_pfmodSystem->createSound("./Resource/SE/Gun_Hit.wav", FMOD_DEFAULT, NULL, &(m_GGHit.pSound));
	m_pfmodSystem->createSound("./Resource/SE/Gun_Shot.wav", FMOD_DEFAULT, NULL, &(m_GGShot.pSound));
	m_pfmodSystem->createSound("./Resource/SE/BZK_Hit.wav", FMOD_DEFAULT, NULL, &(m_BZKHit.pSound));
	m_pfmodSystem->createSound("./Resource/SE/BZK_Shot.wav", FMOD_DEFAULT, NULL, &(m_BZKShot.pSound));
	m_pfmodSystem->createSound("./Resource/SE/MG_Shot.wav", FMOD_DEFAULT, NULL, &(m_MGShot.pSound));
	m_pfmodSystem->createSound("./Resource/SE/Move.wav", FMOD_DEFAULT, NULL, &(m_Move.pSound));
	m_pfmodSystem->createSound("./Resource/SE/Booster.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &(m_Booster.pSound));
}

CRobotObjectSound::~CRobotObjectSound()
{
	m_GGHit.pSound->release();
	m_GGShot.pSound->release();
	m_BZKHit.pSound->release();
	m_BZKShot.pSound->release();
	m_MGShot.pSound->release();
	m_Move.pSound->release();
	m_Booster.pSound->release();
}