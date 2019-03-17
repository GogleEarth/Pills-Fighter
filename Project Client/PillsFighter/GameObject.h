#pragma once

#include "Camera.h"
#include "Model.h"

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
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera);

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

protected:
	std::vector<CParticle*> m_vpParticles;
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
	// ������
	float m_Time; // �ð��Ǹ� ���� �ٲٴ� �ð� 
	float m_ElapsedTime; // �ൿ�� �ð�
	float m_Angle; // ȸ���� ����

	// ������
	float m_RotateSpeed; // ȸ�� �ӵ� ( �ʴ� ȸ���� ���� Angle )
	float m_RotateAngle; // ȸ���ؾ��� ����
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
	CModel *m_pNozzle = NULL;

public:
	CModel* GetNozzleFrame() { return m_pNozzle; }
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
	//UINT		m_nFactor;
};

struct CB_PARTICLE_INFO
{
	XMFLOAT3	gvPosition;
	float		gfElapsedTime;
	XMFLOAT4	gvRandom;
	XMFLOAT3	gvDirection;
	float		gfSpeed;
	float		gfDuration;
	//	float	gfEmitTime;
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
	float								m_fEmitTime;

	bool								m_nInit = false;
	int									m_nVertices;

	CGameObject							*m_pFollowObject = NULL;
	CModel								*m_pFollowFrame = NULL;

	ID3D12Resource						*m_pd3dcbParticle = NULL;
	CB_PARTICLE_INFO					*m_pcbMappedParticle;

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

	virtual void Initialize(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction, float fSpeed, float fDuration, float fEmitTime);

	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	void SetFollowObject(CGameObject *pObject, CModel *pModel);
	void SetToFollowFramePosition();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct CB_EFFECT_INFO
{
	float m_fElapsedTime;
	float m_fDuration;
};

struct CEffectVertex
{
	XMFLOAT3	m_xmf3Position;
	XMFLOAT2	m_xmf2Size;
	float		m_fAge;
};

class CEffect
{
public:
	CEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fDuration);
	virtual ~CEffect();


	// ��� ���ۿ� ���� �� [ ���� �ð�, ��� �ð�(������ �ð�) ]
	// ���� ���ۿ� �� ��. [ ��ġ, ������, ��� �ð� ]
protected:
#define	MAX_EFFECT_VERTEX_COUNT 1000
#define	MAX_EFFECT_INIT_VERTEX_COUNT 100

	ID3D12Resource						*m_pd3dInitVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW			m_d3dInitVertexBufferView;

	int									m_nInitVertices = 0;
	CEffectVertex						*m_pMappedInitVertices;

	ID3D12Resource						*m_pd3dVertexBuffer[2];
	D3D12_VERTEX_BUFFER_VIEW			m_d3dVertexBufferView[2];
	D3D12_STREAM_OUTPUT_BUFFER_VIEW		m_d3dSOBufferView[2];

	int									m_nDrawBufferIndex = 0;
	int									m_nSOBufferIndex = 1;
	ID3D12Resource						*m_pd3dBuffer;
	ID3D12Resource						*m_pd3dDummyBuffer;
	ID3D12Resource						*m_pd3dReadBackBuffer;

	int									m_nVertices = 0;

	float								m_fElapsedTime = 0.0f;
	float								m_fDuration = 0.0f;

	ID3D12Resource						*m_pd3dcbEffect = NULL;
	CB_EFFECT_INFO						*m_pcbMappedEffect = NULL;

public:
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReadVertexCount(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void SORender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void AfterRender(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void AddVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size);
	virtual void Animate(float fTimeElapsed);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CSprite : public CGameObject
{
public:
	CSprite(UINT nTextureIndex, float fSize);
	virtual ~CSprite();

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	void SpriteAnimate();

	void SetMaxSprite(int x, int y, int Max) { m_nMaxSpriteX = x; m_nMaxSpriteY = y; m_nMaxSprite = Max; m_xmf4Sprite.x = (float)1 / x; m_xmf4Sprite.y = (float)1 / y; }
	void SetSpritePos(int x, int y) { m_xmf4Sprite.z = (float)x; m_xmf4Sprite.w = (float)y; }
	EFFECT_TYPE GetSpriteType() { return m_nEffectType; }
	void SetSpriteType(EFFECT_TYPE nSpriteType) { m_nEffectType = nSpriteType; }

	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);

private:
	XMFLOAT4	m_xmf4Sprite;

	int			m_nSpritePosX;
	int			m_nSpritePosY;

	int			m_nMaxSpriteX;
	int			m_nMaxSpriteY;
	int			m_nMaxSprite;

	float		m_fSize;
	UINT		m_nTextureIndex;

	EFFECT_TYPE m_nEffectType;
};
