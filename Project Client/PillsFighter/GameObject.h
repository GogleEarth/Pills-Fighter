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

	// ������ �ϱ� �� ������Ʈ ȸ�� ����
	float m_fPreparePitch;
	float m_fPrepareYaw;
	float m_fPrepareRoll;

	// �̵� �ӷ�
	float m_MovingSpeed;
	
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

	//���� ��ü�� ���� ��ȯ ��Ŀ��� ��ġ ���Ϳ� ����(x-��, y-��, z-��) ���͸� ��ȯ�Ѵ�. 
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

	//���� ��ü�� ��ġ�� �����Ѵ�.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetMovingSpeed(float MovingSpeed) { m_MovingSpeed = MovingSpeed; }

	void SetPrepareRotate(float Pitch, float Yaw, float Roll) { m_fPreparePitch = Pitch; m_fPrepareYaw = Yaw; m_fPrepareRoll = Roll; }
	void AddPrepareRotate(float Pitch, float Yaw, float Roll) { m_fPreparePitch += Pitch; m_fPrepareYaw += Yaw; m_fPrepareRoll += Roll; }

	//���� ��ü�� ���� x-��, y-��, z-�� �������� �̵��Ѵ�.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3 xmf3Direction, float fDistance = 1.0f);

	//���� ��ü�� ȸ��(x-��, y-��, z-��)�Ѵ�. 
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
	void SetHitPoint(int nHitPoint) { m_nHitPoint = nHitPoint; if (m_nHitPoint > m_nMaxHitPoint) m_nHitPoint = m_nMaxHitPoint; }
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
	int							m_nState = 0x000;
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

public:
	void SetShader(CShader *pShader) { m_pShader = pShader; }
	void Show() { m_bRender = true; }
	void Hide() { m_bRender = false; }

public:
	virtual void AfterAdvanceAnimationController();
	virtual void SetCallBackKeys(CModel *pModel) {};

private:
	float collisionRadius;

public:
	void SetCollisionRadius(float radius) { collisionRadius = radius; }
	float GetCollisionRadius() { return collisionRadius; }

protected:
	std::unordered_set<CGameObject*> m_uspHitObjects;

public:
	bool CheckDidHitObject(CGameObject *pObject) { return m_uspHitObjects.count(pObject) > 0; }
	void AddHitObject(CGameObject* pObject) { m_uspHitObjects.emplace(pObject); }

protected:
	bool m_bDie = false;
	float m_fRespawnTime = 0.0f;

public:
	void ProcessDie(float fRespawnTime) { m_bDie = true; m_fRespawnTime = fRespawnTime; }
	bool IsDie() { return m_bDie; }
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
	float m_RotateSpeed; // ȸ�� �ӵ� ( �ʴ� ȸ���� ���� Angle )
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
	float m_DurationTime; // �߻� �� ����?�ð�
	float m_ElapsedTime; // �ൿ�� �ð�
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
	float		m_fLifeTime;		// ����
	float		m_fElapsedTime;			// ��� ����
};



