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

void CAnimation::LoadAnimationFromFile(FILE *pFile, int nFrames)
{
	char pstrToken[64] = { 0 };

	int nIndex;
	fscanf_s(pFile, "%d %s %f %d %d", &nIndex, m_pstrAnimationName, (int)sizeof(m_pstrAnimationName), &m_fAnimationLength, &m_nAnimationFPS, &m_nKeyFrameTransforms);

	m_pfKeyFrameTransformTimes = new float[m_nKeyFrameTransforms];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4*[m_nKeyFrameTransforms];

	for (int i = 0; i < m_nKeyFrameTransforms; i++)
	{

		fscanf_s(pFile, "%s", pstrToken, (int)sizeof(pstrToken)); // <Transforms>:

		fscanf_s(pFile, "%d %f", &nIndex, &m_pfKeyFrameTransformTimes[i]);

		m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nFrames];

		for (int j = 0; j < nFrames; j++)
		{
			fscanf_s(pFile, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
				&m_ppxmf4x4KeyFrameTransforms[i][j]._11, &m_ppxmf4x4KeyFrameTransforms[i][j]._12, &m_ppxmf4x4KeyFrameTransforms[i][j]._13, &m_ppxmf4x4KeyFrameTransforms[i][j]._14,
				&m_ppxmf4x4KeyFrameTransforms[i][j]._21, &m_ppxmf4x4KeyFrameTransforms[i][j]._22, &m_ppxmf4x4KeyFrameTransforms[i][j]._23, &m_ppxmf4x4KeyFrameTransforms[i][j]._24,
				&m_ppxmf4x4KeyFrameTransforms[i][j]._31, &m_ppxmf4x4KeyFrameTransforms[i][j]._32, &m_ppxmf4x4KeyFrameTransforms[i][j]._33, &m_ppxmf4x4KeyFrameTransforms[i][j]._34,
				&m_ppxmf4x4KeyFrameTransforms[i][j]._41, &m_ppxmf4x4KeyFrameTransforms[i][j]._42, &m_ppxmf4x4KeyFrameTransforms[i][j]._43, &m_ppxmf4x4KeyFrameTransforms[i][j]._44);
		}
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