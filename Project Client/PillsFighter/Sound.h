#pragma once
#include "stdafx.h"

struct SoundMaterial
{
	FMOD::Sound* pSound = NULL;
	FMOD::Channel* pChannel = NULL;
};

class CSound
{
public:
	CSound();
	virtual ~CSound();

	void PlayFMODSound(SoundMaterial *pSoundMaterial);
	void PlayFMODSoundLoop(SoundMaterial *pSoundMaterial);
	void ResumeFMODSound(SoundMaterial *pSoundMaterial);
	void PauseFMODSound(SoundMaterial *pSoundMaterial);

protected:
	FMOD::System* m_pfmodSystem = NULL;
};

//////////////////////////////////////////////////////////////////////

class CColonySceneSound : public CSound
{
public:
	CColonySceneSound();
	virtual ~CColonySceneSound();

	SoundMaterial m_BGM;
};

//////////////////////////////////////////////////////////////////////

class CRobotObjectSound : public CSound
{
public:
	CRobotObjectSound();
	virtual ~CRobotObjectSound();
	
	SoundMaterial m_GGHit;
	SoundMaterial m_GGShot;
	SoundMaterial m_BZKHit;
	SoundMaterial m_BZKShot;
	SoundMaterial m_MGShot;
	SoundMaterial m_Move;
	SoundMaterial m_Booster;
};