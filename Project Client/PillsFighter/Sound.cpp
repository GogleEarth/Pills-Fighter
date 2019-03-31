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

//////////////////////////////////////////////////////////////////////

CColonySceneSound::CColonySceneSound() : CSound()
{
	m_pfmodSystem->createSound("./Resource/BGM/bgm_pvp_p03.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pfmodBGM);
}

CColonySceneSound::~CColonySceneSound()
{
	m_pfmodBGM->release();
}

//////////////////////////////////////////////////////////////////////

CRobotObjectSound::CRobotObjectSound() : CSound()
{
	m_pfmodSystem->createSound("./Resource/SE/Gun_Hit.wav", FMOD_DEFAULT, NULL, &m_pfmodGGHit);
	m_pfmodSystem->createSound("./Resource/SE/Gun_Shot.wav", FMOD_DEFAULT, NULL, &m_pfmodGGShot);
	m_pfmodSystem->createSound("./Resource/SE/BZK_Hit.wav", FMOD_DEFAULT, NULL, &m_pfmodBZKHit);
	m_pfmodSystem->createSound("./Resource/SE/BZK_Shot.wav", FMOD_DEFAULT, NULL, &m_pfmodBZKShot);
	m_pfmodSystem->createSound("./Resource/SE/MG_Shot.wav", FMOD_DEFAULT, NULL, &m_pfmodMGShot);
	m_pfmodSystem->createSound("./Resource/SE/Move.wav", FMOD_DEFAULT, NULL, &m_pfmodMove);
	m_pfmodSystem->createSound("./Resource/SE/Booster.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pfmodBooster);
}

CRobotObjectSound::~CRobotObjectSound()
{
	m_pfmodGGHit->release();
	m_pfmodGGShot->release();
	m_pfmodBZKHit->release();
	m_pfmodBZKShot->release();
	m_pfmodMGShot->release();
	m_pfmodMove->release();
	m_pfmodBooster->release();
}

void CRobotObjectSound::PlayBooster() 
{
	if (!bPlayBooster)
	{
		m_pfmodSystem->playSound(m_pfmodBooster, NULL, 0, &m_pfmodChannelBooster);
		bPlayBooster = true;
	}

	bool bPause = false;
	m_pfmodChannelBooster->getPaused(&bPause);

	if(bPause) m_pfmodChannelBooster->setPaused(false);
}

void CRobotObjectSound::PauseBooster()
{
	bool bPause = false;
	m_pfmodChannelBooster->getPaused(&bPause);

	if (!bPause) m_pfmodChannelBooster->setPaused(true);
}