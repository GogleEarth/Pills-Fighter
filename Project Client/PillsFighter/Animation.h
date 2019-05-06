#pragma once

#define ANIMATION_TYPE_ONCE			0
#define ANIMATION_TYPE_LOOP			1
#define ANIMATION_TYPE_PINGPONG		2

class CModel;
class CSound;

#define ANIMATION_CALLBACK_EPSILON	0.01f

struct CALLBACKKEY
{
	float  							m_fTime;
	void							*m_pData;
};

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() { }
	~CAnimationCallbackHandler() { }

public:
	virtual void HandleCallback(void *pData) { }
};

class CSoundCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSoundCallbackHandler() { }
	~CSoundCallbackHandler() { }

public:
	virtual void HandleCallback(void *pData);
};

struct SWITCH
{
	bool bCondition;
	bool *pbSwitch;
};

class CSwitchCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSwitchCallbackHandler() { }
	~CSwitchCallbackHandler() { }

public:
	virtual void HandleCallback(void *pData);
};

struct CONDITIONALSWITCH
{
	bool *pbCVariable;
	bool bCondition;
	bool *pbSwitch;
};

class CSwitchinConditionCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSwitchinConditionCallbackHandler() { }
	~CSwitchinConditionCallbackHandler() { }

public:
	virtual void HandleCallback(void *pData);
};


/////////////////////////////////////////////////////////////////////////////
///////

class CAnimation
{
public:
	CAnimation();
	virtual ~CAnimation();

protected:
	char							m_pstrAnimationName[64];

	float							m_fAnimationLength;

	int								m_nKeyFrameTransforms;

	float							*m_pfKeyFrameTransformTimes;
	XMFLOAT4X4						**m_ppxmf4x4KeyFrameTransforms;

	float							m_fAnimationTimePosition;
	int								m_nAnimationType;

public:
	int 							m_nCallbackKeys;
	CALLBACKKEY 					*m_pCallbackKeys;

	CAnimationCallbackHandler 		*m_pAnimationCallbackHandler;

public:
	void SetTimePosition(float fTrackTimePosition);

	XMFLOAT4X4 GetSRT(int nFrame);

	void LoadAnimationFromFile(FILE *pfile, int nFrames);

	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(CAnimationCallbackHandler *pCallbackHandler);
	void SetAnimationType(int nType) { m_nAnimationType = nType; }

	void* GetCallbackData();
	float GetLength() { return m_fAnimationLength; }
	int GetAnimationType() { return m_nAnimationType; }
	float GetPosition() { return m_fAnimationTimePosition; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CAnimationSet
{
public:
	CAnimationSet(int nAnimations);
	virtual ~CAnimationSet();

protected:
	int				m_nAnimationFrames;
	CModel			**m_ppAnimationFrameCaches;

	int				m_nAnimations;
	CAnimation		*m_pAnimations;

public:
	int GetAnimationCount() { return m_nAnimations; }
	CAnimation*	GetAnimation(int nIndex) { return &m_pAnimations[nIndex]; }
	int GetAnimationFrames() { return m_nAnimationFrames; }
	CModel* GetCachedFrame(int nIndex) { return m_ppAnimationFrameCaches[nIndex]; }
	void SetAnimationFrames(int nFrames) { m_nAnimationFrames = nFrames; }
	void SetAnimationFrameCaches(CModel **ppAnimationFrameCaches) { m_ppAnimationFrameCaches = ppAnimationFrameCaches; }

	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CAnimationTrack
{
public:
	CAnimationTrack();
	virtual ~CAnimationTrack();

protected:
	bool 			m_bEnable;
	float 			m_fSpeed;
	float 			m_fPosition;
	float 			m_fWeight;

	CAnimation 		*m_pAnimation;
	int				m_nAnimationState;

public:
	void SetAnimation(CAnimation *pAnimation) { m_pAnimation = pAnimation; }
	void SetAnimationState(int nState) { m_nAnimationState = nState; }
	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }
	void SetPosition(float fPosition) { m_fPosition = fPosition; }
	void SetAnimationType(int nType) { m_pAnimation->SetAnimationType(nType); }

	float GetPosition() { return m_fPosition; }
	float GetSpeed() { return m_fSpeed; }
	float GetWeight() {	return m_fWeight; }
	float GetLength() { return m_pAnimation->GetLength(); }
	CAnimation* GetAnimation() { return m_pAnimation; }
	int GetTrackAnimationState() { return m_nAnimationState; }
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
	float 							m_fTime;
	CAnimationSet					*m_pAnimationSet;

	int 							m_nAnimationTracks;
	CAnimationTrack 				*m_pAnimationTracks;

	XMFLOAT4X4						*m_xmf4x4BoneTransforms;

public:
	void SetAnimationSet(CAnimationSet *pAnimationSet);

	void SetTrackAnimation(int nAnimationTrackIndex, int nAnimationSetIndex);
	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);
	void SetTrackAnimationType(int nAnimationTrack, int nType);

	void AdvanceTime(float fElapsedTime);
	void AfterAdvanceTime();
	void ApplyTransform();

	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler);

	int GetTrackAnimationState(int nAnimationTrackIndex) { return m_pAnimationTracks[nAnimationTrackIndex].GetTrackAnimationState(); }
	CAnimationTrack* GetAnimationTrack(int nAnimationTrackIndex) { return &m_pAnimationTracks[nAnimationTrackIndex]; }
	float GetAnimationPosition(int nAnimationTrack) { return m_pAnimationTracks[nAnimationTrack].GetAnimation()->GetPosition(); }
	float GetTrackPosition(int nAnimationTrack) { return m_pAnimationTracks[nAnimationTrack].GetPosition(); }
	float GetTrackLength(int nAnimationTrack) { return m_pAnimationTracks[nAnimationTrack].GetLength(); }
};