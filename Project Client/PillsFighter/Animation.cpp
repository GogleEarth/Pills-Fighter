#include "stdafx.h"
#include "Animation.h"
#include "Model.h"

CAnimation::CAnimation()
{
}

CAnimation::~CAnimation()
{
	if (m_pfKeyFrameTransformTimes) delete[] m_pfKeyFrameTransformTimes;
	for (int j = 0; j < m_nKeyFrameTransforms; j++) if (m_ppxmf4x4KeyFrameTransforms[j]) delete[] m_ppxmf4x4KeyFrameTransforms[j];
	if (m_ppxmf4x4KeyFrameTransforms) delete[] m_ppxmf4x4KeyFrameTransforms;
}

void CAnimation::GetCallbackData(CSound **ppCallbackSound, FMOD::Sound **ppfmodCallbackSound)
{
	for (int i = 0; i < m_nCallbackKeys; i++)
	{
		if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fAnimationTimePosition, ANIMATION_CALLBACK_EPSILON))
		{
			*ppfmodCallbackSound = m_pCallbackKeys[i].m_pfmodCallbackSound;
			*ppCallbackSound = m_pCallbackKeys[i].m_pCallbackSound;
			
			return;
		}
	}

	*ppfmodCallbackSound = NULL;
	*ppCallbackSound = NULL;
}

void CAnimation::SetTimePosition(float fTrackTimePosition)
{
	m_fAnimationTimePosition = fTrackTimePosition;

	switch (m_nAnimationType)
	{
	case ANIMATION_TYPE_LOOP:
	{	
		m_fAnimationTimePosition = fmod(fTrackTimePosition, m_pfKeyFrameTransformTimes[m_nKeyFrameTransforms - 1]); // m_fPosition = fTrackPosition - int(fTrackPosition / m_pfKeyFrameTransformTimes[m_nKeyFrameTransforms-1]) * m_pfKeyFrameTransformTimes[m_nKeyFrameTransforms-1];
	//	m_fPosition = fmod(fTrackPosition, m_fLength); //if (m_fPosition < 0) m_fPosition += m_fLength;
	//	m_fPosition = fTrackPosition - int(fTrackPosition / m_fLength) * m_fLength;
		break;
	}
	case ANIMATION_TYPE_ONCE:
		break;
	case ANIMATION_TYPE_PINGPONG:
		break;
	}

	if (m_pAnimationCallbackHandler)
	{
		CSound *pSound;
		FMOD::Sound *pfmodSound;
		GetCallbackData(&pSound, &pfmodSound);

		if (pSound) m_pAnimationCallbackHandler->HandleCallback(pSound, pfmodSound);
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
}

void CAnimation::SetCallbackKey(int nKeyIndex, float fKeyTime, CSound *pCallbackSound, FMOD::Sound *pfmodCallbackSound)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackSound = pCallbackSound;
	m_pCallbackKeys[nKeyIndex].m_pfmodCallbackSound = pfmodCallbackSound;
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

	for (int i = 0; i < m_nKeyFrameTransforms; i++)
	{
		fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
		fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);  // <Transforms>:
		pstrToken[nstrLength] = '\0';

		fread_s(&m_pfKeyFrameTransformTimes[i], sizeof(float), sizeof(float), 1, pfile);

		m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nFrames];
		fread_s(m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4) * nFrames, sizeof(XMFLOAT4X4), nFrames, pfile);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CAnimationSet::CAnimationSet(int nAnimations)
{
	m_nAnimations = nAnimations;
	m_pAnimations = new CAnimation[m_nAnimations];
}

CAnimationSet::~CAnimationSet()
{
	if (m_pAnimations) delete[] m_pAnimations;
	if (m_ppAnimationFrameCaches) delete[] m_ppAnimationFrameCaches;
}

void CAnimationSet::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	m_pAnimations[nAnimationSet].m_nCallbackKeys = nCallbackKeys;
	m_pAnimations[nAnimationSet].m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationSet::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, CSound *pCallbackSound, FMOD::Sound *pfmodCallbackSound)
{
	m_pAnimations[nAnimationSet].m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pAnimations[nAnimationSet].m_pCallbackKeys[nKeyIndex].m_pCallbackSound = pCallbackSound;
	m_pAnimations[nAnimationSet].m_pCallbackKeys[nKeyIndex].m_pfmodCallbackSound = pfmodCallbackSound;
}

void CAnimationSet::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler)
{
	m_pAnimations[nAnimationSet].SetAnimationCallbackHandler(pCallbackHandler);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CAnimationController::CAnimationController(int nAnimationTracks, CAnimationSet *pAnimationSet)
{
	m_nAnimationTracks = nAnimationTracks;
	m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];
	m_xmf4x4BoneTransforms = new XMFLOAT4X4[pAnimationSet->GetAnimationFrames()];

	SetAnimationSet(pAnimationSet);
}

CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks) delete[] m_pAnimationTracks;
	if (m_xmf4x4BoneTransforms) delete[] m_xmf4x4BoneTransforms;
}

void CAnimationController::SetAnimationSet(CAnimationSet *pAnimationSet)
{
	m_pAnimationSet = pAnimationSet;
}

void CAnimationController::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	if (m_pAnimationSet) m_pAnimationSet->SetCallbackKeys(nAnimationSet, nCallbackKeys);
}

void CAnimationController::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, CSound *pCallbackSound, FMOD::Sound *pfmodCallbackSound)
{
	if (m_pAnimationSet) m_pAnimationSet->SetCallbackKey(nAnimationSet, nKeyIndex, fKeyTime, pCallbackSound, pfmodCallbackSound);
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler)
{
	if (m_pAnimationSet) m_pAnimationSet->SetAnimationCallbackHandler(nAnimationSet, pCallbackHandler);
}

void CAnimationController::SetTrackAnimation(int nAnimationTrackIndex, int nAnimationIndex)
{
	if (m_pAnimationSet && (nAnimationIndex < m_pAnimationSet->GetAnimationCount()))
	{
		if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrackIndex].SetAnimation(m_pAnimationSet->GetAnimation(nAnimationIndex));
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
			float fPosition = m_pAnimationTracks[j].GetPosition();
			float fSpeed = m_pAnimationTracks[j].GetSpeed();
			float fNewPosition = fPosition + (fTimeElapsed * fSpeed);
			m_pAnimationTracks[j].SetPosition(fNewPosition);

			CAnimation *pAnimation = m_pAnimationTracks[j].GetAnimation();
			pAnimation->SetTimePosition(fPosition);

			if (m_pAnimationTracks[j].IsEnable())
			{
				for (int i = 0; i < m_pAnimationSet->GetAnimationFrames(); i++)
				{
					CModel *pFrame = m_pAnimationSet->GetCachedFrame(i);
					pFrame->SetToParent(Matrix4x4::Add(pFrame->GetToParent(), Matrix4x4::Scale(pAnimation->GetSRT(i), m_pAnimationTracks[j].GetWeight())));
					m_xmf4x4BoneTransforms[i] = pFrame->GetToParent();
				}
			}
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