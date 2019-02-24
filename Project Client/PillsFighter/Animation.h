#pragma once

#define ANIMATION_TYPE_ONCE			0
#define ANIMATION_TYPE_LOOP			1
#define ANIMATION_TYPE_PINGPONG		2

class CModel;

class CAnimation
{
public:
	CAnimation();
	virtual ~CAnimation();

protected:
	char			m_pstrAnimationName[64] = { 0 };

	float			m_fAnimationLength = 0.0f;

	int				m_nKeyFrameTransforms = 0; 

	float			*m_pfKeyFrameTransformTimes = NULL;
	XMFLOAT4X4		**m_ppxmf4x4KeyFrameTransforms = NULL;

	float			m_fAnimationTimePosition = 0.0f;
	int				m_nAnimationType = ANIMATION_TYPE_LOOP;

public:
	void SetTimePosition(float fTrackTimePosition);

	XMFLOAT4X4 GetSRT(int nFrame);

	void LoadAnimationFromFile(FILE *pfile, int nFrames);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CAnimationSet
{
public:
	CAnimationSet(int nAnimations);
	virtual ~CAnimationSet();

protected:
	int				m_nAnimationFrames = 0;
	CModel			**m_ppAnimationFrameCaches = NULL;

	int				m_nAnimations = 0;
	CAnimation		*m_pAnimations = NULL;

public:
	int GetAnimationCount() { return m_nAnimations; }
	CAnimation*	GetAnimation(int nIndex) { return &m_pAnimations[nIndex]; }
	int GetAnimationFrames() { return m_nAnimationFrames; }
	CModel* GetCachedFrame(int nIndex) { return m_ppAnimationFrameCaches[nIndex]; }
	void SetAnimationFrames(int nFrames) { m_nAnimationFrames = nFrames; }
	void SetAnimationFrameCaches(CModel **ppAnimationFrameCaches) { m_ppAnimationFrameCaches = ppAnimationFrameCaches; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CAnimationTrack
{
public:
	CAnimationTrack() {};
	virtual ~CAnimationTrack() {};

protected:
	BOOL 			m_bEnable = true;
	float 			m_fSpeed = 1.0f;
	float 			m_fPosition = 0.0f;
	float 			m_fWeight = 1.0f;

	CAnimation 		*m_pAnimation = NULL;

public:
	void SetAnimation(CAnimation *pAnimation) { m_pAnimation = pAnimation; }
	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }
	void SetPosition(float fPosition) { m_fPosition = fPosition; }

	float GetPosition() { return m_fPosition; }
	float GetSpeed() { return m_fSpeed; }
	float GetWeight() {	return m_fWeight; }
	CAnimation* GetAnimation() { return m_pAnimation; }
	bool IsEnable() { return m_bEnable; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CAnimationController
{
public:
	CAnimationController(int nAnimationTracks, CAnimationSet *pAnimationSet);
	virtual ~CAnimationController();

protected:
	float 							m_fTime = 0.0f;
	CAnimationSet					*m_pAnimationSet = NULL;

	int 							m_nAnimationTracks = 0;
	CAnimationTrack 				*m_pAnimationTracks = NULL;

	XMFLOAT4X4						*m_xmf4x4BoneTransforms = NULL;

public:
	void SetAnimationSet(CAnimationSet *pAnimationSet);

	void SetTrackAnimation(int nAnimationTrackIndex, int nAnimationSetIndex);
	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void AdvanceTime(float fElapsedTime);
	void ApplyTransform();
};