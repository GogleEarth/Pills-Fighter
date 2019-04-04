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

struct MATERIAL
{
	XMFLOAT4		m_xmf4Ambient;
	XMFLOAT4		m_xmf4Diffuse;
	XMFLOAT4		m_xmf4Specular;
	XMFLOAT4		m_xmf4Emissive;
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

	// 렌더링 하기 전 오브젝트 회전 설정
	float m_fPreparePitch;
	float m_fPrepareYaw;
	float m_fPrepareRoll;

	// 이동 속력
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
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, int nInstances = 1);
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

	int								m_nHitPoint;
	int								m_nMaxHitPoint;
	XMFLOAT3						serverPosition;

public:
	int GetHitPoint() { return m_nHitPoint; }
	int GetMaxHitPoint() { return m_nMaxHitPoint; }
	void SetMaxHitPoint(int nMaxHitPoint) { m_nMaxHitPoint = nMaxHitPoint; }
	void SetHitPoint(int nHitPoint) { m_nHitPoint = nHitPoint; if (m_nMaxHitPoint < m_nHitPoint) m_nHitPoint = m_nMaxHitPoint; }
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

	CAnimationController				*m_pAnimationController = NULL;

	void SetSkinnedMeshBoneTransformConstantBuffer();

	void UpdateWorldTransform();
	CModel *GetModel() { return m_pModel; }
	std::vector<BoundingBox>& GetAABB() { return m_vxmAABB; }
	int GetNumAABB() { return m_nMeshes; }
	bool CollisionCheck(CGameObject *pObject);
	bool CollisionCheck(XMVECTOR *pxmf4Origin, XMVECTOR *pxmf4Look, float *pfDistance);
	void MoveToCollision(CGameObject *pObject);
	virtual void ProcessMoveToCollision(BoundingBox *pxmAABB, BoundingBox *pxmObjAABB) {}
	void SetAnimationController(CAnimationController *pController) { m_pAnimationController = pController; }

public:
	void AddParticle(CParticle *pParticle) { m_vpParticles.emplace_back(pParticle); };
	virtual void ApplyToParticle(CParticle *pParticle);

protected:
	std::vector<CParticle*> m_vpParticles;
	int					m_nState = 0x00;
	int					m_nType = 0x00;

public:
	int GetState() { return m_nState; }
	void SetState(int nState) { m_nState = nState; }

	int GetType() { return m_nType; }
	std::vector<CParticle*>& GetParticles() { return m_vpParticles; }

public:
	void UpdateInstanceShaderVariables(VS_VB_INSTANCE *pcbMappedGameObjects, int *pnIndex);
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
	// 고정값
	float m_Time; // 시간되면 방향 바꾸는 시간 
	float m_ElapsedTime; // 행동한 시간
	float m_Angle; // 회전한 각도

	// 랜덤값
	float m_RotateSpeed; // 회전 속도 ( 초당 회전할 각도 Angle )
	float m_RotateAngle; // 회전해야할 각도
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

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
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
	int		m_nAnimationState = ANIMATION_STATE_IDLE;
	BOOL	m_bAnimationChanged = FALSE;

public:
	void ChangeAnimation(int nState);
	int GetAnimationState() { return m_nAnimationState; }
	BOOL GetAnimationChanged() { return m_bAnimationChanged; }
	void SetAnimationChanged(BOOL bAnimationChagned) { m_bAnimationChanged = bAnimationChagned; }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CSoundCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSoundCallbackHandler() { }
	~CSoundCallbackHandler() { }

public:
	virtual void HandleCallback(CSound *pCallbackSound, FMOD::Sound *pfmodCallbackSound);
};

class CRobotObject : public CAnimationObject
{
public:
	CRobotObject();
	virtual ~CRobotObject();

	virtual void OnPrepareAnimate();

	virtual void EquipOnRightHand(CWeapon *pWeapon);
	virtual void EquipOnLeftHand(CWeapon *pWeapon);

	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

	virtual void ChangeWeapon(int nSlotIndex);
	virtual void ChangeWeaponByType(WEAPON_TYPE nType);
	virtual int GetWeaponIndex(int nType);
	void AddWeapon(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CModel *pWeaponModel, int nType);
	CWeapon* GetWeapon(int nIndex) { return m_vpWeapon[nIndex]; }

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

	CModel* GetLeftNozzleFrame() { return m_pLeftNozzle; }
	CModel* GetRightNozzleFrame() { return m_pRightNozzle; }

public:
	CRobotObjectSound *m_pSound = NULL;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct CB_EFFECT_INFO
{
	float m_fElapsedTime;
	float m_fDuration;
};

class CEffect
{
public:
	CEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nBytes, float fDuration);
	virtual ~CEffect();

protected:
#define	MAX_EFFECT_VERTEX_COUNT 1000
#define	MAX_EFFECT_INIT_VERTEX_COUNT 1000

	ID3D12Resource						*m_pd3dInitVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW			m_d3dInitVertexBufferView;

	int									m_nInitVertices = 0;
	void								*m_pMappedInitVertices;

	ID3D12Resource						*m_pd3dVertexBuffer[2];
	D3D12_VERTEX_BUFFER_VIEW			m_d3dVertexBufferView[2];
	D3D12_STREAM_OUTPUT_BUFFER_VIEW		m_d3dSOBufferView[2];

	int									m_nDrawBufferIndex = 0;
	int									m_nSOBufferIndex = 1;
	ID3D12Resource						*m_pd3dBuffer;
	ID3D12Resource						*m_pd3dDummyBuffer;
	ID3D12Resource						*m_pd3dReadBackBuffer;

	int									m_nVertices = 0;
	UINT								m_nBytes = 0;

	float								m_fElapsedTime = 0.0f;
	float								m_fDuration = 0.0f;

	ID3D12Resource						*m_pd3dcbEffect = NULL;
	CB_EFFECT_INFO						*m_pcbMappedEffect = NULL;

public:
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void Animate(float fTimeElapsed);

	virtual void ReadVertexCount(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void SORender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void AfterRender(ID3D12GraphicsCommandList *pd3dCommandList);
};

/////////////////////////////////////////////////////////

struct CFadeOutVertex
{
	XMFLOAT3	m_xmf3Position;
	XMFLOAT2	m_xmf2Size;
	float		m_fAge;
};

class CFadeOut : public CEffect
{
public:
	CFadeOut(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fDuration);
	virtual ~CFadeOut();

	virtual void AddVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size);
};

/////////////////////////////////////////////////////////

struct CB_SPRITE_INFO
{
	XMFLOAT2 m_xmf2SpriteSize;
	UINT m_nMaxSpriteX;
	UINT m_nMaxSpriteY;
	UINT m_nMaxSprite;
	float m_fDurationPerSprite;
};

struct CSpriteVertex
{
	XMFLOAT3	m_xmf3Position;
	XMFLOAT2	m_xmf2Size;
	XMUINT2		m_xmn2SpritePos;
	float		m_fAge;
	UINT		m_nTextureIndex;
	UINT		m_nType;
};

class CSprite : public CEffect
{
public:
	CSprite(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nMaxX, UINT nMaxY, UINT nMax, float fDuration);
	virtual ~CSprite();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void AddVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, UINT nTextureIndex, EFFECT_ANIMATION_TYPE nEffectAniType);

protected:
	XMFLOAT2			m_xmf2SpriteSize = XMFLOAT2(0.0f, 0.0f);
	UINT				m_nMaxSpriteX = 0;
	UINT				m_nMaxSpriteY = 0;
	UINT				m_nMaxSprite = 0;

	float				m_fDurationPerSprite = 0.0f;

	ID3D12Resource		*m_pd3dcbSprite = NULL;
	CB_SPRITE_INFO		*m_pcbMappedSprite = NULL;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define PARTICLE_TYPE_EMITTER 0
#define MAX_PARTICLE_VERTEX_COUNT 1000

struct CParticleVertex
{
	XMFLOAT3	m_xmf3Position;
	XMFLOAT3	m_xmf3Velocity;
	XMFLOAT2	m_xmf2Size;
	UINT		m_nType;
	float		m_fAge;
};

struct CB_PARTICLE_INFO
{
	XMFLOAT3	m_vPosition;
	float		m_fElapsedTime;
	XMFLOAT4	m_vRandom;
	XMFLOAT3	m_vDirection;
	float		m_fSpeed;
	float		m_fDuration;
	bool		m_bEmit;
};

class CParticle
{
public:
	CParticle(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CParticle();

protected:
	ID3D12Resource						*m_pd3dInitVertexBuffer;
	ID3D12Resource						*m_pd3dInitVertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW			m_d3dInitVertexBufferView;

	ID3D12Resource						*m_pd3dVertexBuffer[2];
	D3D12_VERTEX_BUFFER_VIEW			m_d3dVertexBufferView[2];
	D3D12_STREAM_OUTPUT_BUFFER_VIEW		m_d3dSOBufferView[2];

	int									m_nDrawBufferIndex = 0;
	int									m_nSOBufferIndex = 1;
	ID3D12Resource						*m_pd3dBuffer;
	ID3D12Resource						*m_pd3dDummyBuffer;
	ID3D12Resource						*m_pd3dReadBackBuffer;

	XMFLOAT3							m_xmf3Position;
	XMFLOAT3							m_xmf3Direction;
	float								m_fSpeed;
	float								m_fElapsedTime;
	float								m_fDuration;
	bool								m_bEmit = true;

	bool								m_nInit = false;
	int									m_nVertices;

	CGameObject							*m_pFollowObject = NULL;
	CModel								*m_pFollowFrame = NULL;

	ID3D12Resource						*m_pd3dcbParticle = NULL;
	CB_PARTICLE_INFO					*m_pcbMappedParticle;

	bool								m_bDelete = false;

public:
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Animate(float fTimeElapsed);
	virtual void AfterRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void SORender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReadVertexCount(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void Initialize(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction, float fSpeed, float fDuration);

	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	void SetDirection(XMFLOAT3 xmf3Direction) { m_xmf3Direction = xmf3Direction; }
	void SetEmit(bool bEmit) { m_bEmit = bEmit; }

	void SetFollowObject(CGameObject *pObject, CModel *pModel);
	void SetToFollowFramePosition();
	void SetDirectionByFollowFrame();

	void Delete() { m_bDelete = true; }
	int IsDelete() { return m_bDelete; }
};
