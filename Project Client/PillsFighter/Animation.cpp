#include "stdafx.h"
#include "Animation.h"
#include "Model.h"
#include "Sound.h"

extern CFMODSound gFmodSound;

void CAnimationCallbackHandler::HandleCallback(CALLBACKDATA *pData)
{
	switch (pData->m_nType)
	{
	case CALLBACK_TYPE_SOUND:
	{
		UINT nSoundType = *((int*)(pData->m_pData));

		switch (nSoundType)
		{
		case CALLBACK_TYPE_SOUND_MOVE:
			gFmodSound.PlayFMODSound(gFmodSound.m_pSoundMove);
			break;
		case CALLBACK_TYPE_SOUND_SABER_ATTACK:
			gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberAttack);
			break;
		}
		break;
	}
	}
}

///////////////////////////////////////////////////////////////////

CAnimation::CAnimation()
{
	ZeroMemory(m_pstrAnimationName, sizeof(m_pstrAnimationName));
	m_fAnimationLength = 0.0f;
	m_nKeyFrameTransforms = 0;
	m_pfKeyFrameTransformTimes = NULL;
	m_ppxmf4x4KeyFrameTransforms = NULL;
	m_fAnimationTimePosition = 0.0f;
	m_nAnimationType = ANIMATION_TYPE_LOOP;
	m_nCallbackKeys = 0;
	m_pCallbackKeys = NULL;
	m_pAnimationCallbackHandler = NULL;
}

CAnimation::~CAnimation()
{
	if (m_pfKeyFrameTransformTimes)
	{
		delete[] m_pfKeyFrameTransformTimes;
		m_pfKeyFrameTransformTimes = NULL;
	}

	if (m_ppxmf4x4KeyFrameTransforms)
	{
		for (int j = 0; j < m_nKeyFrameTransforms; j++)
		{
			if (m_ppxmf4x4KeyFrameTransforms[j])
			{
				delete[] m_ppxmf4x4KeyFrameTransforms[j];
				m_ppxmf4x4KeyFrameTransforms[j] = NULL;
			}
		}

		delete[] m_ppxmf4x4KeyFrameTransforms;
		m_ppxmf4x4KeyFrameTransforms = NULL;
	}

	if (m_ppxmf4x4KeyFrameTransforms)
	{
		delete[] m_ppxmf4x4KeyFrameTransforms;
		m_ppxmf4x4KeyFrameTransforms = NULL;
	}

	if (m_pCallbackKeys)
	{
		delete[] m_pCallbackKeys;
		m_pCallbackKeys = NULL;
	}

	if (m_pAnimationCallbackHandler)
	{
		delete m_pAnimationCallbackHandler;
		m_pAnimationCallbackHandler = NULL;
	}
}

CALLBACKDATA *CAnimation::GetCallbackData()
{
	for (int i = 0; i < m_nCallbackKeys; i++)
	{
		if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fAnimationTimePosition, ANIMATION_CALLBACK_EPSILON))
		{
			return  &m_pCallbackKeys[i].m_CallBackData;
		}
	}

	return NULL;
}

void CAnimation::SetTimePosition(float fTrackTimePosition)
{
	m_fAnimationTimePosition = fTrackTimePosition;

	switch (m_nAnimationType)
	{
	case ANIMATION_TYPE_LOOP:
	{
		m_fAnimationTimePosition = fmod(fTrackTimePosition, m_fAnimationLength); // m_fPosition = fTrackPosition - int(fTrackPosition / m_pfKeyFrameTransformTimes[m_nKeyFrameTransforms-1]) * m_pfKeyFrameTransformTimes[m_nKeyFrameTransforms-1];

		//	m_fPosition = fmod(fTrackPosition, m_fLength); //if (m_fPosition < 0) m_fPosition += m_fLength;
	//	m_fPosition = fTrackPosition - int(fTrackPosition / m_fLength) * m_fLength;
		break;
	}
	case ANIMATION_TYPE_ONCE:
	{
		m_fAnimationTimePosition = m_fAnimationTimePosition > m_fAnimationLength ? m_fAnimationLength : m_fAnimationTimePosition;

		//TCHAR pstrDebug[256] = { 0 };
		//_stprintf_s(pstrDebug, 256, _T("Length : %f, Time Position : %f\n"), m_fAnimationLength, m_fAnimationTimePosition);
		//OutputDebugString(pstrDebug);
		break;
	}
	case ANIMATION_TYPE_PINGPONG:
	{
		//m_fAnimationTimePosition = abs(((int(fTrackTimePosition / m_fAnimationLength) & 0x1) *m_fAnimationLength) - fmod(fTrackTimePosition, m_fAnimationLength));

		m_fAnimationTimePosition = abs(int(fTrackTimePosition / m_fAnimationLength) * m_fAnimationLength - fmod(fTrackTimePosition, m_fAnimationLength));
		m_fAnimationTimePosition = m_fAnimationTimePosition > m_fAnimationLength ? 0.0f : m_fAnimationTimePosition;
		
		break;
	}
	}

	if (m_pAnimationCallbackHandler)
	{
		CALLBACKDATA *pData = GetCallbackData();

		if(pData) 
			m_pAnimationCallbackHandler->HandleCallback(pData);
	}
}

XMFLOAT4X4 CAnimation::GetSRT(int nFrame)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();

	for (int i = 0; i < (m_nKeyFrameTransforms - 1); i++)
	{
		if ((m_pfKeyFrameTransformTimes[i] <= m_fAnimationTimePosition) && (m_fAnimationTimePosition <= m_pfKeyFrameTransformTimes[i + 1]))
		{
			float t = (m_fAnimationTimePosition - m_pfKeyFrameTransformTimes[i]) / (m_pfKeyFrameTransformTimes[i + 1] - m_pfKeyFrameTransformTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nFrame], m_ppxmf4x4KeyFrameTransforms[i + 1][nFrame], t);
			break;
		}
	}

	return(xmf4x4Transform);
}

void CAnimation::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
	ZeroMemory(m_pCallbackKeys, sizeof(CALLBACKKEY) * nCallbackKeys);
}

void CAnimation::SetCallbackKey(int nKeyIndex, float fKeyTime, CALLBACKDATA callbackData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_CallBackData = callbackData;
}

void CAnimation::SetAnimationCallbackHandler(CAnimationCallbackHandler *pCallbackHandler)
{
	m_pAnimationCallbackHandler = pCallbackHandler;
}

void CAnimation::LoadAnimationFromFile(FILE *pfile, int nFrames)
{
	BYTE nstrLength;
	char pstrToken[64] = { 0 };

	fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
	fread_s(m_pstrAnimationName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
	fread_s(&m_fAnimationLength, sizeof(float), sizeof(float), 1, pfile);
	fread_s(&m_nKeyFrameTransforms, sizeof(int), sizeof(int), 1, pfile);

	m_pfKeyFrameTransformTimes = new float[m_nKeyFrameTransforms];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4*[m_nKeyFrameTransforms];

	std::string name = m_pstrAnimationName;
	if (name.find("ONE") != name.npos)
		m_nAnimationType = ANIMATION_TYPE_ONCE;
	else if (name.find("LOOP") != name.npos)
		m_nAnimationType = ANIMATION_TYPE_LOOP;
	else if (name.find("PINGPONG") != name.npos)
		m_nAnimationType = ANIMATION_TYPE_PINGPONG;

	for (int i = 0; i < m_nKeyFrameTransforms; i++)
	{
		fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
		fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);  // <Transforms>:
		pstrToken[nstrLength] = '\0';

		fread_s(&m_pfKeyFrameTransformTimes[i], sizeof(float), sizeof(float), 1, pfile);

		m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nFrames];
		fread_s(m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4) * nFrames, sizeof(XMFLOAT4X4), nFrames, pfile);
	}

	m_fAnimationLength = m_pfKeyFrameTransformTimes[m_nKeyFrameTransforms - 1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationSet::CAnimationSet(int nAnimations)
{
	m_nAnimations = nAnimations;
	m_pAnimations = new CAnimation[nAnimations];

	m_nAnimationFrames = 0;
	m_ppAnimationFrameCaches = NULL;
}

CAnimationSet::~CAnimationSet()
{
	if (m_pAnimations)
	{
		delete[] m_pAnimations;
		m_pAnimations = NULL;
	}
	if (m_ppAnimationFrameCaches)
	{
		delete[] m_ppAnimationFrameCaches;
		m_ppAnimationFrameCaches = NULL;
	}
}

void CAnimationSet::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	m_pAnimations[nAnimationSet].SetCallbackKeys(nCallbackKeys);
}

void CAnimationSet::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, int nType, int nKeyType, void *pData)
{
	float fTimePos;

	switch (nKeyType)
	{
	case CALLBACK_POSITION_TIME:
		fTimePos = fKeyTime;
		break;
	case CALLBACK_POSITION_END:
		fTimePos = m_pAnimations[nAnimationSet].GetLength();
		break;
	case CALLBACK_POSITION_MIDDLE:
		fTimePos = m_pAnimations[nAnimationSet].GetLength() * 0.5f;
		break;
	}

	m_pAnimations[nAnimationSet].m_pCallbackKeys[nKeyIndex].m_fTime = fTimePos;
	m_pAnimations[nAnimationSet].m_pCallbackKeys[nKeyIndex].m_CallBackData = CALLBACKDATA{ nType, pData };
}

void CAnimationSet::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler)
{
	m_pAnimations[nAnimationSet].SetAnimationCallbackHandler(pCallbackHandler);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CAnimationTrack::CAnimationTrack()
{
	m_bEnable = true;
	m_fSpeed = 1.0f;
	m_fPosition = 0.0f;
	m_fWeight = 1.0f;

	m_pAnimation = NULL;
	m_nAnimationState = 0;
}

CAnimationTrack::~CAnimationTrack()
{

}

void CAnimationTrack::SetPosition(float fPosition)
{ 
	switch (m_pAnimation->GetAnimationType())
	{
	case ANIMATION_TYPE_LOOP:
	{

		m_fPosition = (fPosition) > GetLength() ? 0.0f : fPosition;
		break;
	}
	case ANIMATION_TYPE_ONCE:
	{
		m_fPosition = (fPosition) > GetLength() ? GetLength() : fPosition;
		break;
	}
	case ANIMATION_TYPE_PINGPONG:
	{
		//m_fAnimationTimePosition = abs(int(fTrackTimePosition / m_fAnimationLength) * m_fAnimationLength - fmod(fTrackTimePosition, m_fAnimationLength));
		//m_fAnimationTimePosition = m_fAnimationTimePosition > m_fAnimationLength ? 0.0f : m_fAnimationTimePosition;
		break;
	}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CAnimationController::CAnimationController(int nAnimationTracks, CAnimationSet *pAnimationSet)
{
	m_fTime = 0.0f;

	m_nAnimationTracks = nAnimationTracks;
	m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];

	m_xmf4x4BoneTransforms = new XMFLOAT4X4[pAnimationSet->GetAnimationFrames()];
	ZeroMemory(m_xmf4x4BoneTransforms, sizeof(XMFLOAT4X4) * pAnimationSet->GetAnimationFrames());

	SetAnimationSet(pAnimationSet);
}

CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks)
	{
		delete[] m_pAnimationTracks;
		m_pAnimationTracks = NULL;
	}
	if (m_xmf4x4BoneTransforms)
	{
		delete[] m_xmf4x4BoneTransforms;
		m_xmf4x4BoneTransforms = NULL;
	}
}

void CAnimationController::SetAnimationSet(CAnimationSet *pAnimationSet)
{
	m_pAnimationSet = pAnimationSet;
}

void CAnimationController::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	if (m_pAnimationSet) m_pAnimationSet->SetCallbackKeys(nAnimationSet, nCallbackKeys);
}

void CAnimationController::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, int nType, int nKeyType, void *pData)
{
	if (m_pAnimationSet) m_pAnimationSet->SetCallbackKey(nAnimationSet, nKeyIndex, fKeyTime, nType, nKeyType, pData);
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler)
{
	if (m_pAnimationSet) m_pAnimationSet->SetAnimationCallbackHandler(nAnimationSet, pCallbackHandler);
}

void CAnimationController::SetTrackAnimation(int nAnimationTrackIndex, int nAnimationIndex)
{
	if (m_pAnimationSet && (nAnimationIndex < m_pAnimationSet->GetAnimationCount()))
	{
		if (m_pAnimationTracks)
		{
			m_pAnimationTracks[nAnimationTrackIndex].SetAnimation(m_pAnimationSet->GetAnimation(nAnimationIndex));
			m_pAnimationTracks[nAnimationTrackIndex].SetAnimationState(nAnimationIndex);
		}
	}
}

void CAnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetEnable(bEnable);
}

void CAnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetPosition(fPosition);
}

void CAnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
}

void CAnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetWeight(fWeight);
}

void CAnimationController::SetTrackAnimationType(int nAnimationTrack, int nType)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetAnimationType(nType);
}

void CAnimationController::AdvanceTime(float fTimeElapsed)
{
	m_fTime += fTimeElapsed;

	if (m_pAnimationSet && m_pAnimationTracks)
	{
		for (int i = 0; i < m_pAnimationSet->GetAnimationFrames(); i++)
		{
			CModel *pFrame = m_pAnimationSet->GetCachedFrame(i);
			pFrame->SetToParent(Matrix4x4::Zero());
		}

		for (int j = 0; j < m_nAnimationTracks; j++)
		{
			if (m_pAnimationTracks[j].IsEnable())
			{
				float fPosition = m_pAnimationTracks[j].GetPosition();
				float fSpeed = m_pAnimationTracks[j].GetSpeed();
				float fNewPosition = fPosition + (fTimeElapsed * fSpeed);
				m_pAnimationTracks[j].SetPosition(fNewPosition);

				CAnimation *pAnimation = m_pAnimationTracks[j].GetAnimation();
				pAnimation->SetTimePosition(fPosition);

				for (int i = 0; i < m_pAnimationSet->GetAnimationFrames(); i++)
				{
					CModel *pFrame = m_pAnimationSet->GetCachedFrame(i);
					XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Scale(pAnimation->GetSRT(i), m_pAnimationTracks[j].GetWeight());
					pFrame->SetToParent(Matrix4x4::Add(pFrame->GetToParent(), xmf4x4Transform));
				}
			}
		}
	}
}

void CAnimationController::AfterAdvanceTime()
{
	if (m_pAnimationSet && m_pAnimationTracks)
	{
		for (int i = 0; i < m_pAnimationSet->GetAnimationFrames(); i++)
		{
			CModel *pFrame = m_pAnimationSet->GetCachedFrame(i);

			m_xmf4x4BoneTransforms[i] = pFrame->GetToParent();
		}
	}
}

void CAnimationController::ApplyTransform()
{
	if (m_pAnimationSet && m_pAnimationTracks)
	{
		for (int j = 0; j < m_nAnimationTracks; j++)
		{
			if (m_pAnimationTracks[j].IsEnable())
			{
				for (int i = 0; i < m_pAnimationSet->GetAnimationFrames(); i++)
				{
					CModel *pFrame = m_pAnimationSet->GetCachedFrame(i);
					pFrame->SetToParent(m_xmf4x4BoneTransforms[i]);
				}
			}
		}
	}
}