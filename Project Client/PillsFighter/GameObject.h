#pragma once

#include "Camera.h"
#include "Model.h"
#include "Animation.h"
#include "Sound.h"

////////////////////////////////////////////////////////////////////////////////

class CShader;
class CAnimationController;
class CWeapon;
class CParticle;
class CFollowEffect;

struct MATERIAL
{
	XMFLOAT4		m_xmf4Ambient;
	XMFLOAT4		m_xmf4Diffuse;
	XMFLOAT4		m_xmf4Specular;
	XMFLOAT4		m_xmf4Emissive;
	float			m_fReflectionFactor;
};

#define WORLD_WIDTH 1000
#define WORLD_HEIGHT 470
#define SEPARATION 5
#define INVASION_ROOF 20
#define INVASION 6

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World;
	MATERIAL						m_Material;
	UINT							m_nTexturesMask;
};

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

protected:
	XMFLOAT4X4 m_xmf4x4World;

	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

protected:
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	// 렌더링 하기 전 오브젝트 회전 설정
	float m_fPreparePitch;
	float m_fPrepareYaw;
	float m_fPrepareRoll;

	// 이동 속력
	float m_MovingSpeed;
	
	// 중력가속도
	float				m_fVelocityY = 0.0f;

public:
	void SetModel(CModel *pModel);
	void SetMesh(CMesh *pMesh, CCubeMesh *pCubeMesh);
	void SetMaterial(CMaterial **ppMaterials, UINT nMaterials);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {};

	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture = true, bool bSetShader = true, int nInstances = 1);
	virtual void RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture = true, bool bSetShader = true, int nInstances = 1);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, int nInstances = 1);

	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseUploadBuffers();

	//게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다. 
	void SetLook(XMFLOAT3 xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetUp(XMFLOAT3 xmf3Up) { m_xmf3Up = xmf3Up; }
	void SetRight(XMFLOAT3 xmf3Right) { m_xmf3Right = xmf3Right; }
	void SetWorldTransf(XMFLOAT4X4& xmf4x4World);
	XMFLOAT3 GetPosition() { return m_xmf3Position; }
	XMFLOAT3 GetLook() { return m_xmf3Look; }
	XMFLOAT3 GetUp() { return m_xmf3Up; }
	XMFLOAT3 GetRight() { return m_xmf3Right; }
	float GetMovingSpeed() { return(m_MovingSpeed); }
	XMFLOAT4X4 GetWorldTransf();

	//게임 객체의 위치를 설정한다.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetMovingSpeed(float MovingSpeed) { m_MovingSpeed = MovingSpeed; }

	void SetPrepareRotate(float Pitch, float Yaw, float Roll) { m_fPreparePitch = Pitch; m_fPrepareYaw = Yaw; m_fPrepareRoll = Roll; }
	void AddPrepareRotate(float Pitch, float Yaw, float Roll) { m_fPreparePitch += Pitch; m_fPrepareYaw += Yaw; m_fPrepareRoll += Roll; }

	//게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3 xmf3Direction, float fDistance = 1.0f);

	//게임 객체를 회전(x-축, y-축, z-축)한다. 
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	
	void Delete() { m_Delete = TRUE; }
	bool IsDelete() { return m_Delete; }

protected:
	CModel							*m_pModel = NULL;

	bool							 m_Delete = FALSE;

	int								m_nHitPoint = 100;
	int								m_nMaxHitPoint = 100;
	XMFLOAT3						serverPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

public:
	int GetHitPoint() { return m_nHitPoint; }
	int GetMaxHitPoint() { return m_nMaxHitPoint; }
	void SetMaxHitPoint(int nMaxHitPoint) { m_nMaxHitPoint = nMaxHitPoint; }
	void SetHitPoint(int nHitPoint);
	virtual void OnPrepareAnimate() {};

public:
	int									m_nMeshes = 0;
	int									m_nSkinnedMeshes = 0;

	std::vector<BoundingBox>			m_vxmAABB;
	std::vector<ID3D12Resource*>		m_vd3dcbGameObject;
	std::vector<CB_GAMEOBJECT_INFO*>	m_vcbMappedGameObject;

	std::vector<CSkinnedMesh*>			m_vSkinnedMeshes;

	std::vector<ID3D12Resource*>		m_vd3dcbBoneTransforms;
	std::vector<XMFLOAT4X4*>			m_vcbxmf4x4BoneTransforms;

	CAnimationController				*m_ppAnimationControllers[2] = { NULL };

	void SetSkinnedMeshBoneTransformConstantBuffer();

	void UpdateWorldTransform();
	CModel *GetModel() { return m_pModel; }
	std::vector<BoundingBox>& GetAABB() { return m_vxmAABB; }
	int GetNumAABB() { return m_nMeshes; }
	bool CollisionCheck(CGameObject *pObject);
	bool CollisionCheck(XMVECTOR *pxmf4Origin, XMVECTOR *pxmf4Look, float *pfDistance);
	void MoveToCollision(CGameObject *pObject);
	void MoveToCollisionByRadius(CGameObject *pObject);
	void MoveToWorldRange();
	virtual void ProcessMoveToCollision(BoundingBox *pxmAABB, BoundingBox *pxmObjAABB) {}
	virtual void SetAnimationController(CAnimationController *pControllers, int nIndex) { m_ppAnimationControllers[nIndex] = pControllers; }
	virtual void SetAnimationTrackPosition(int nIndex, float fPosition) { m_ppAnimationControllers[nIndex]->SetTrackPosition(0, fPosition); }
	virtual float GetAnimationTrackPosition(int nIndex) { return m_ppAnimationControllers[nIndex]->GetTrackPosition(0); }

public:
	void AddParticle(CParticle *pParticle) { m_vpParticles.emplace_back(pParticle); };
	virtual void ApplyToParticle(CParticle *pParticle);
	std::vector<CParticle*>& GetParticles() { return m_vpParticles; }

	void AddEffect(CFollowEffect *pEffect) { m_vpFollowEffects.emplace_back(pEffect); };
	virtual void ApplyToEffect(CFollowEffect *pEffect);
	std::vector<CFollowEffect*>& GetEffects() { return m_vpFollowEffects; }

protected:
	std::vector<CParticle*>		m_vpParticles;
	std::vector<CFollowEffect*>	m_vpFollowEffects;


protected:
	int							m_nState = 0x00;
	int							m_nType = 0x00;

public:
	int GetState() { return m_nState; }
	void SetState(int nState) { m_nState = nState; }

	int GetType() { return m_nType; }

public:
	void UpdateInstanceShaderVariables(VS_VB_INSTANCE *pcbMappedGameObjects, int *pnIndex);

protected:
	CShader							*m_pShader = NULL;
	bool							m_bRender = true;

protected:
	std::unordered_set<CGameObject*> m_uspHitObjects;

public:
	bool IsRender() { return m_bRender; }
	void SetShader(CShader *pShader) { m_pShader = pShader; }
	void Show() { m_bRender = true; }
	void Hide() { m_bRender = false; }

	bool CheckDidHitObject(CGameObject *pObject) { return m_uspHitObjects.count(pObject) > 0; }
	void AddHitObject(CGameObject* pObject) { m_uspHitObjects.emplace(pObject); }

protected:
	bool m_bDie = false;
	float m_fRespawnTime = 0.0f;
	float m_fRespawnTimeElapsed = 0.0f;

public:
	virtual void ProcessDie(float fRespawnTime);
	virtual void ProcessRespawn(int nHP, XMFLOAT3 xmf3Position);
	bool IsDie() { return m_bDie; }
	float GetRespwanTime() { return m_fRespawnTime; }
	float GetRespwanElapsedTime() { return m_fRespawnTimeElapsed; }

public:
	virtual void AfterAdvanceAnimationController();
	virtual void SetCallBackKeys(CModel *pModel) {};

private:
	float collisionRadius = 0.0f;

public:
	void SetCollisionRadius(float radius) { collisionRadius = radius; }
	float GetCollisionRadius() { return collisionRadius; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RandomMoveObject : public CGameObject
{
public:
	RandomMoveObject();
	virtual ~RandomMoveObject();

	void InitRandomRotate();
	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);

private:
	float m_Time;
	float m_ElapsedTime;
	float m_Angle;

	float m_RotateSpeed; 
	float m_RotateAngle; 
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RotateObject : public CGameObject
{
public:
	RotateObject();
	virtual ~RotateObject();

	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);

private:
	float m_RotateSpeed; // 회전 속도 ( 초당 회전할 각도 Angle )
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Bullet : public CGameObject
{
public:
	Bullet();
	virtual ~Bullet();

	virtual void Animate(float ElapsedTime, CCamera *pCamera = NULL);

private:
	float m_RotationSpeed;
	float m_DurationTime; // 발사 후 생존?시간
	float m_ElapsedTime; // 행동한 시간
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage				*m_pHeightMapImage;

	int							m_nWidth;
	int							m_nLength;

	XMFLOAT3					m_xmf3Scale;

public:
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMapImage->GetHeight(x, z, bReverseQuad) * m_xmf3Scale.y); } //World
	XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture = true, bool bSetShader = true, int nInstances = 1);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CAnimationObject : public CGameObject
{
public:
	CAnimationObject();
	virtual ~CAnimationObject();

	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);

protected:
	int		m_pnAnimationState[2] = { 0 };
	bool	m_pbAnimationChanged[2] = { 0 };

public:
	virtual void SetAnimationController(CAnimationController *pControllers, int nIndex);

	virtual bool ChangeAnimation(int nController, int nTrack, int nAnimation, bool bResetPosition = false);
	int GetAnimationState(int nController) { return m_pnAnimationState[nController]; }
	bool GetAnimationChanged(int nController) { return m_pbAnimationChanged[nController]; }
	void SetAnimationChanged(int nController, bool bAnimationChagned) { m_pbAnimationChanged[nController] = bAnimationChagned; }
	bool IsAnimationEnd(int nUpperUnder, int nTrack) { return m_ppAnimationControllers[nUpperUnder]->IsEndPosition(nTrack); }
	bool IsAnimationStart(int nUpperUnder, int nTrack) { return m_ppAnimationControllers[nUpperUnder]->IsStartPosition(nTrack); }
	bool AnimationIs(int nUpperUnder, int nAnimation) {	return m_pnAnimationState[nUpperUnder] == nAnimation; }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CRobotObject : public CAnimationObject
{
public:
	CRobotObject();
	virtual ~CRobotObject();

	virtual void OnPrepareAnimate();

	virtual void EquipOnRightHand(CWeapon *pWeapon);
	virtual void EquipOnLeftHand(CWeapon *pWeapon);

	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture = true, bool bSetShader = true, int nInstances = 1);
	virtual void RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture = true, bool bSetShader = true, int nInstances = 1);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nInstances = 1);

	virtual void ChangeWeapon(int nIndex);
	virtual void ChangeWeaponByType(WEAPON_TYPE nType);
	virtual int GetWeaponIndex(int nType);
	virtual void AddWeapon(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CWeapon *pWeapon);
	CWeapon* GetWeapon(int nIndex) { return m_vpWeapon[nIndex]; }
	virtual void SetCallBackKeys(CModel *pModel);

protected:
	CModel *m_pLeftHand = NULL;
	CModel *m_pRightHand = NULL;

	CWeapon *m_pRHWeapon = NULL;
	CWeapon *m_pLHWeapon = NULL;

	std::vector<CWeapon*> m_vpWeapon;

public:
	CWeapon* GetRHWeapon() { return m_pRHWeapon; }
	CWeapon* GetLHWeapon() { return m_pLHWeapon; }

protected:
	CModel *m_pLeftNozzle = NULL;
	CModel *m_pRightNozzle = NULL;

public:
	virtual void ApplyToParticle(CParticle *pParticle);
	virtual void ApplyToEffect(CFollowEffect *pEffect);

	CModel* GetLeftNozzleFrame() { return m_pLeftNozzle; }
	CModel* GetRightNozzleFrame() { return m_pRightNozzle; }
	CModel* GetHeadFrame() { return m_pHead; }
	CModel* GetRightHandFrame() { return m_pRightHand; }

protected:
	FMOD::Channel *m_pChannelBooster = NULL;

protected:
	CModel *m_pSpine = NULL;
	CModel *m_pPelvis = NULL;
	CModel *m_pBip = NULL;
	CModel *m_pLThigh = NULL;
	CModel *m_pRThigh = NULL;
	CModel *m_pHead = NULL;
	
public:
	virtual void AfterAdvanceAnimationController();
	virtual bool ChangeAnimation(int nController, int nTrack, int nAnimation, bool bResetPosition = false);

	bool IsAnimationSwording() { return AnimationIs(ANIMATION_UP, ANIMATION_STATE_BEAM_SABER_1_ONCE) || AnimationIs(ANIMATION_UP, ANIMATION_STATE_BEAM_SABER_2_ONCE) || AnimationIs(ANIMATION_UP, ANIMATION_STATE_BEAM_SABER_3_ONCE); }

	virtual void ProcessDie(float fRespawnTime);
	virtual void ProcessRespawn(int nHP, XMFLOAT3 xmf3Position);
};

/////////////////////////////////////////////////////////////////////////////////////////

class CCursor
{
public:
	CCursor(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CCursor();

	void ReleaseUploadBuffer();
	
	void MoveCursorPos(float x, float y);
	void SetCursorPos(XMFLOAT2 xmf2Position);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	XMFLOAT2 GetPosition() { return m_xmf2CursorPos; }

	bool CollisionCheck(BoundingBox& xmAABB);

protected:

	CMesh *m_pMesh;
	CTexture *m_pTexture;
	CShader *m_pShader;

	XMFLOAT2 m_xmf2CursorPos;
	XMFLOAT2 m_xmf2ScreenPos;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Meteor : public CGameObject
{
public:
	Meteor();
	virtual ~Meteor();

	virtual void Animate(float ElapsedTime, CCamera *pCamera = NULL);

private:
	float		m_fLifeTime;		// 수명
	float		m_fElapsedTime;			// 경과 수명
};



