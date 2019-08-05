#pragma once

#define EFFECT_ANIMATION_TYPE_ONE 0 
#define EFFECT_ANIMATION_TYPE_LOOP 1

class CGameObject;
class CModel;

struct CB_EFFECT_INFO
{
	XMFLOAT4 m_xmf4Color;
	float m_fElapsedTime;
	float m_fDuration;
};

class CEffect
{
public:
	CEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nBytes, XMFLOAT4 xmf4Color, float fDuration);
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

	XMFLOAT4							m_xmf4Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
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

	virtual void AddVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, int nEffectAniType, int nAngle) {}
	virtual void AddVertexWithLookV(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, XMFLOAT3 xmf3Look, int nEffectAniType) {}
};

/////////////////////////////////////////////////////////

struct CFadeOutVertex
{
	XMFLOAT3	m_xmf3Position;
	float		m_fAge;
	XMFLOAT2	m_xmf2Size;
	int			m_nAngle;
};

class CFadeOut : public CEffect
{
public:
	CFadeOut(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4 xmf4Color, float fDuration);
	virtual ~CFadeOut();

	virtual void AddVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, int nEffectAniType, int nAngle);
};

/////////////////////////////////////////////////////////

struct CLaserVertex
{
	XMFLOAT3	m_xmf3Position;
	XMFLOAT2	m_xmf2Size;
	float		m_fAge;
	XMFLOAT3	m_xmf3Look;
};

class CLaserBeam : public CEffect
{
public:
	CLaserBeam(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4 xmf4Color, float fDuration);
	virtual ~CLaserBeam();

	virtual void AddVertexWithLookV(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, XMFLOAT3 xmf3Look, int nEffectAniType);
};

/////////////////////////////////////////////////////////

class CGlowEffect : public CEffect
{
public:
	CGlowEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4 xmf4Color, float fDuration);
	virtual ~CGlowEffect();

protected:

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
	UINT		m_nType;
	int			m_nAngle;
};

class CSprite : public CEffect
{
public:
	CSprite(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4 xmf4Color, UINT nMaxX, UINT nMaxY, UINT nMax, float fDuration);
	virtual ~CSprite();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void AddVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, int nEffectAniType, int nAngle);

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

#define PARTICLE_TYPE_COMMON 0
#define PARTICLE_TYPE_EMITTER 1
#define PARTICLE_TYPE_ONE_EMITTER 2

#define MAX_PARTICLE_VERTEX_COUNT 10000
#define MAX_TEMP_PARTICLE_VERTEX_COUNT 1000

struct CParticleVertex
{
	XMFLOAT3	m_xmf3Position;
	XMFLOAT3	m_xmf3Velocity;
	XMFLOAT2	m_xmf2Size;
	int			m_nType;
	float		m_fAge;
	int			m_nAngle;
};

struct CB_PARTICLE_INFO
{
	XMFLOAT3	m_vPosition;
	float		m_fSpeed;
	XMFLOAT3	m_vDirection;
	float		m_fDuration;
	XMFLOAT3	m_vRight;
	float		m_fElapsedTime;
	XMFLOAT3	m_vUp;
	float		m_fMass;
	XMFLOAT3	m_vLook;
	bool		m_bEmit;
	XMFLOAT3	m_vAngles;
	bool		m_bScaling;
	XMFLOAT4	m_xmf4Color;
};

class CParticle
{
public:
	CParticle(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CParticle();

protected:
	ID3D12Resource						*m_pd3dMappedVertexBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW			m_d3MappedVertexBufferView;
	CParticleVertex						*m_pMappedParticleVertices = NULL;
	int									m_nMappedParticleVertices = 0;

	ID3D12Resource						*m_pd3dVertexBuffer[2] = { NULL };
	D3D12_VERTEX_BUFFER_VIEW			m_d3dVertexBufferView[2];
	D3D12_STREAM_OUTPUT_BUFFER_VIEW		m_d3dSOBufferView[2];

	int									m_nDrawBufferIndex = 0;
	int									m_nSOBufferIndex = 1;
	ID3D12Resource						*m_pd3dBuffer = NULL;
	ID3D12Resource						*m_pd3dDummyBuffer = NULL;
	ID3D12Resource						*m_pd3dReadBackBuffer = NULL;

	XMFLOAT3							m_xmf3Position;
	XMFLOAT3							m_xmf3Direction;
	XMFLOAT3							m_xmf3Right;
	XMFLOAT3							m_xmf3Up;
	XMFLOAT3							m_xmf3Look;
	float								m_fSpeed;
	float								m_fElapsedTime;
	float								m_fDuration;
	bool								m_bEmit = true;
	XMFLOAT3							m_xmf3Angles;
	bool								m_bScaling = false;
	float								m_fMass = 0.0f;
	XMFLOAT4							m_xmf4Color;

	int									m_nVertices = 0;

	CGameObject							*m_pFollowObject = NULL;
	CModel								*m_pFollowFrame = NULL;

	ID3D12Resource						*m_pd3dcbParticle = NULL;
	CB_PARTICLE_INFO					*m_pcbMappedParticle = NULL;

	bool								m_bDelete = false;

public:
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void Animate(float fTimeElapsed);
	virtual void AfterRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void SORender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReadVertexCount(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void Initialize(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction, float fSpeed, float fDuration, bool bScaling, float fMass,
		XMFLOAT3 xmf3Right, XMFLOAT3 xmf3Up, XMFLOAT3 xmf3Look, XMFLOAT3 xmf3Angles, XMFLOAT4 xmf4Color);

	virtual void AddVertex(CParticleVertex *pParticleVertices, int nVertices);

	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	void SetDirection(XMFLOAT3 xmf3Direction) { m_xmf3Direction = xmf3Direction; }
	void SetRight(XMFLOAT3 xmf3Right) { m_xmf3Right = xmf3Right; }
	void SetUp(XMFLOAT3 xmf3Up) { m_xmf3Up = xmf3Up; }
	void SetLook(XMFLOAT3 xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetEmit(bool bEmit) { m_bEmit = bEmit; }

	void SetFollowObject(CGameObject *pObject, CModel *pModel);
	void SetToFollowFramePositions();

	void Delete() { m_bDelete = true; }
	int IsDelete() { return m_bDelete; }
};
