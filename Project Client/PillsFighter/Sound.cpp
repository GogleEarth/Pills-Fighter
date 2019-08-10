#include "stdafx.h"
#include "Sound.h"

CFMODSound::CFMODSound()
{
	FMOD::System_Create(&m_pfmodSystem);

	m_pfmodSystem->init(FMOD_MAX_CHANNEL_WIDTH, FMOD_INIT_NORMAL, NULL);

	m_pfmodSystem->createSound("./Resource/BGM/Title.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pSoundTitleBGM);
	m_pfmodSystem->createSound("./Resource/BGM/Lobby.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pSoundLobbyBGM);
	m_pfmodSystem->createSound("./Resource/BGM/Colony.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pSoundColonyBGM);
	m_pfmodSystem->createSound("./Resource/BGM/Space.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pSoundSpaceBGM);

	m_pfmodSystem->createSound("./Resource/SE/Gun_Hit.wav", FMOD_DEFAULT, NULL, &m_pSoundGGHit);
	m_pfmodSystem->createSound("./Resource/SE/Gun_Shot.wav", FMOD_DEFAULT, NULL, &m_pSoundGGShot);
	m_pfmodSystem->createSound("./Resource/SE/BZK_Hit.wav", FMOD_DEFAULT, NULL, &m_pSoundBZKHit);
	m_pfmodSystem->createSound("./Resource/SE/BZK_Shot.wav", FMOD_DEFAULT, NULL, &m_pSoundBZKShot);
	m_pfmodSystem->createSound("./Resource/SE/MG_Shot.wav", FMOD_DEFAULT, NULL, &m_pSoundMGShot);
	m_pfmodSystem->createSound("./Resource/SE/Move.wav", FMOD_DEFAULT, NULL, &m_pSoundMove);
	m_pfmodSystem->createSound("./Resource/SE/PickAmmo.wav", FMOD_DEFAULT, NULL, &m_pSoundPickAmmo);
	m_pfmodSystem->createSound("./Resource/SE/PickHeal.wav", FMOD_DEFAULT, NULL, &m_pSoundPickHeal);
	m_pfmodSystem->createSound("./Resource/SE/SaberAttack.wav", FMOD_DEFAULT, NULL, &m_pSoundSaberAttack);
	m_pfmodSystem->createSound("./Resource/SE/SaberHit1.wav", FMOD_DEFAULT, NULL, &m_pSoundSaberHit1);
	m_pfmodSystem->createSound("./Resource/SE/Booster.wav", FMOD_LOOP_NORMAL | FMOD_DEFAULT, NULL, &m_pSoundBooster);
	m_pfmodSystem->createSound("./Resource/SE/Alert.wav", FMOD_DEFAULT, NULL, &m_pSoundAlert);
	m_pfmodSystem->createSound("./Resource/SE/BeamRifle.wav", FMOD_DEFAULT, NULL, &m_pSoundBeamRifle);
	m_pfmodSystem->createSound("./Resource/SE/Destroy.wav", FMOD_DEFAULT, NULL, &m_pSoundDestroy);
	m_pfmodSystem->createSound("./Resource/SE/BattleAlert.wav", FMOD_DEFAULT, NULL, &m_pSoundBattleAlert);
}

CFMODSound::~CFMODSound()
{
	m_pSoundTitleBGM->release();
	m_pSoundLobbyBGM->release();
	m_pSoundColonyBGM->release();
	m_pSoundSpaceBGM->release();
	m_pSoundGGHit->release();
	m_pSoundGGShot->release();
	m_pSoundBZKHit->release();
	m_pSoundBZKShot->release();
	m_pSoundMGShot->release();
	m_pSoundMove->release();
	m_pSoundBooster->release();
	m_pSoundPickAmmo->release();
	m_pSoundPickHeal->release();
	m_pSoundSaberAttack->release();
	m_pSoundAlert->release();
	m_pSoundBeamRifle->release();
	m_pSoundSaberHit1->release();
	m_pSoundDestroy->release();
	m_pSoundBattleAlert->release();

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

	pChannel->setPosition(0, FMOD_TIMEUNIT_MS);
}

void CFMODSound::StopFMODSound(FMOD::Channel *pChannel)
{
	if (!pChannel) return;

	pChannel->stop();
}
