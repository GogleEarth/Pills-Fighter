#pragma once

#include "GameObject.h"
#include "Camera.h"
#include "Player.h"

class CRepository;

class CShader
{
public:
	CShader();
	virtual ~CShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateTransluDepthStencilState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateSODepthStencilState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateAlwaysDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {}
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {};
	virtual void ReleaseShaderVariables() {};

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera) { }
	virtual void ReleaseObjects() { }

	virtual void ReleaseUploadBuffers() {};

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera) {};

protected:
	ID3D12PipelineState				*m_pd3dPipelineState = NULL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSkinnedAnimationShader : public CShader
{
public:
	CSkinnedAnimationShader();
	virtual ~CSkinnedAnimationShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWireShader : public CShader
{
public:
	CWireShader() {};
	virtual ~CWireShader() {};

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CObjectsShader : public CShader
{
public:
	CObjectsShader();
	virtual ~CObjectsShader();

	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void ReleaseObjects();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	int							m_nObjectGroup;
	std::vector<CModel*>		m_vpModels;
	std::vector<CGameObject*>	*m_pvpObjects = NULL;

public:
	std::vector<CGameObject*>& GetObjects(int nGroup) { return m_pvpObjects[nGroup]; }
	int GetGroups() { return m_nObjectGroup; }

	void CheckDeleteObjects();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, 
		CRepository *pRepository, void *pContext = NULL) {};
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, 
		CGameObject* pObject, int nGroup, bool bPrepareRotate, void *pContext);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CStandardObjectsShader : public CObjectsShader
{
public:
	CStandardObjectsShader();
	virtual ~CStandardObjectsShader();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, 
		CRepository *pRepository, void *pContext = NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VS_VB_INSTANCE
{
	XMFLOAT4X4						m_xmf4x4World;
	MATERIAL						m_Material;
	UINT							m_nTexturesMask;
};

class CInstancingObjectsShader : public CObjectsShader
{
public:
	CInstancingObjectsShader();
	virtual ~CInstancingObjectsShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex);
	virtual void ReleaseShaderVariables();

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	std::vector<ID3D12Resource*> m_vpd3dcbGameObjects;
	std::vector<VS_VB_INSTANCE*> m_vpcbMappedGameObjects;
};

////////////////////////////////////////

class CObstacleShader : public CInstancingObjectsShader
{
public:
	CObstacleShader();
	virtual ~CObstacleShader();

	void InsertObjectFromLoadInfFromBin(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, int nGroup);

	float ReadFloatFromFile(FILE *pInFile) {
		float fValue = 0;
		UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
		return(fValue);
	}
	BYTE LeftByteFromFile(FILE *pInFile, int byte) {
		UINT nReads = 0;
		char waste[64] = { '\0' };
		nReads = (UINT)::fread(waste, sizeof(char), byte, pInFile); 

		return(nReads);
	}
	BYTE ReadPosrotFromFile(FILE *pInFile, char *pstrToken) {
		BYTE nStrLength = 41;
		BYTE m_value = 7;
		UINT nReads = 0;
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), m_value, pInFile); 
		pstrToken[m_value] = '\0'; 

		return(nReads);
	}

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		CRepository *pRepository, void *pContext = NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSkinnedObjectsShader : public CObjectsShader
{
public:
	CSkinnedObjectsShader();
	virtual ~CSkinnedObjectsShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob); 
	
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRobotObjectsShader : public CSkinnedObjectsShader
{
public:
	CRobotObjectsShader();
	virtual ~CRobotObjectsShader();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		CRepository *pRepository, void *pContext = NULL);
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		CGameObject* pObject, int nGroup, bool bPrepareRotate, void *pContext);

protected:
	CModel		*m_pGimGun = NULL;
	CModel		*m_pBazooka = NULL;
	CModel		*m_pMachineGun = NULL;
	CModel		*m_pSaber = NULL;

	ID3D12RootSignature *m_pd3dSceneRootSignature = NULL;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CEffectShader : public CShader
{
public:
	CEffectShader();
	virtual ~CEffectShader();

	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOutput();

	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob);

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual void ReleaseUploadBuffers();

	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void AfterRender(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL) {}
	virtual void AddEffect(int nIndex, XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, int nEffectAniType, int nTextures = 1);

protected:
	// Effect Count = Texture Count
	int						m_nEffects = 0;
	CEffect					**m_ppEffects = NULL;
	CTexture				**m_ppTextures = NULL;

	ID3D12PipelineState		*m_pd3dSOPipelineState = NULL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define TIMED_EFFECT_COUNT 1

#define TIMED_EFFECT_INDEX_MUZZLE_FIRE 0

#define TIMED_EFFECT_INDEX_MUZZLE_FIRE_TEXTURES 1
class CTimedEffectShader : public CEffectShader
{
public:
	CTimedEffectShader();
	virtual ~CTimedEffectShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOutput();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual D3D12_SHADER_BYTECODE CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob);

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define TEXT_EFFECT_COUNT 1

#define TEXT_EFFECT_INDEX_HIT_TEXT 0

class CTextEffectShader : public CTimedEffectShader
{
public:
	CTextEffectShader();
	virtual ~CTextEffectShader();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	ID3D12PipelineState *m_pd3dTextPipelineState = NULL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define SPRITE_EFFECT_COUNT 2

#define SPRITE_EFFECT_INDEX_HIT 0
#define SPRITE_EFFECT_INDEX_EXPLOSION 1


#define SPRITE_EFFECT_INDEX_HIT_TEXTURES 2
#define SPRITE_EFFECT_INDEX_EXPLOSION_TEXTURES 1
class CSpriteShader : public CEffectShader
{
public:
	CSpriteShader();
	virtual ~CSpriteShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOutput();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual D3D12_SHADER_BYTECODE CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob);

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PARTICLE_COUNT 2

#define PARTICLE_INDEX_BOOSTER_FLARE 0
#define PARTICLE_INDEX_BOOSTER_FOG 1

#define PARTICLE_INDEX_BOOSTER_FLARE_TEXTURES 1
#define PARTICLE_INDEX_BOOSTER_FOG_TEXTURES 1
class CParticleShader : public CShader
{
public:
	CParticleShader();
	virtual ~CParticleShader();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOutput();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual D3D12_SHADER_BYTECODE CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob);

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual void ReleaseUploadBuffers();

	virtual void CheckDeleteObjects();
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void AfterRender(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);

	void SetFollowObject(CGameObject *pObject, CModel *pFrame);

protected:
	ID3D12PipelineState				*m_pd3dSOPipelineState = NULL;

	CTexture						**m_ppTextures = NULL;

	std::vector<CParticle*>			*m_pvpParticles = NULL;
	std::queue<CParticle*>			*m_pvpTempParticles = NULL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CTerrainShader : public CShader
{
public:
	CTerrainShader();
	virtual ~CTerrainShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CB_PLAYER_VALUE
{
	int nMaxValue;
	int nValue;
};

struct CB_MINIMAP_PLAYER_POSITION
{
	XMFLOAT2 playerPosition;
	XMFLOAT2 playerLook;
	XMFLOAT2 playerRight;
};

class CUserInterface : public CShader
{
public:
	CUserInterface();
	virtual ~CUserInterface();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShaderBar(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShaderBullet(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShaderMinimap(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource *pd3dcb, CB_PLAYER_VALUE *pcbMapped, int nMaxValue, int nValue);
	virtual void UpdateShaderVariablesMinimapPlayer(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	ID3D12PipelineState				*m_pd3dPipelineStateBar = NULL;
	ID3D12PipelineState				*m_pd3dPipelineStateBullet = NULL;

	CPlayer							*m_pPlayer = NULL;

	ID3D12Resource					*m_pd3dcbPlayerHP = NULL;
	CB_PLAYER_VALUE					*m_pcbMappedPlayerHP = NULL;

	ID3D12Resource					*m_pd3dcbPlayerBooster = NULL;
	CB_PLAYER_VALUE					*m_pcbMappedPlayerBooster = NULL;

	ID3D12Resource					*m_pd3dcbPlayerAmmo = NULL;
	CB_PLAYER_VALUE					*m_pcbMappedPlayerAmmo = NULL;

	ID3D12Resource					*m_MinimapPlayerRsc = NULL;
	CB_MINIMAP_PLAYER_POSITION		*m_cbMinimapPlayerInfo;

	int								m_nUIRect = 0;
	CRect							**m_ppUIRects = NULL;

	int								m_nTextures;
	CTexture						**m_ppTextures = NULL;

	ID3D12PipelineState				*m_pd3dPipelineStateMinimap = NULL;
	CTexture						*m_pMinimap = NULL;

public:
	void SetPlayer(CPlayer *pPlayer) { m_pPlayer = pPlayer; }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxShader : public CShader
{
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};

///////////////////////////////////////////////////////////////////////////////

class CFontShader : public CShader
{
public:
	CFontShader();
	virtual ~CFontShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);
};

///////////////////////////////////////////////////////////////////////////////

class CLobbyShader : public CShader
{
public:
	CLobbyShader();
	virtual ~CLobbyShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);
};

///////////////////////////////////////////////////////////////////////////////

class CCursorShader : public CShader
{
public:
	CCursorShader();
	virtual ~CCursorShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);
};




///////////////////////////////////////////////////////////////////////////////


class CMinimapRobotShader : public CShader
{
public:
	CMinimapRobotShader();
	virtual ~CMinimapRobotShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);
};

///////////////////////////////////////////////////////////////////////////////

#define ROBOTCOUNT 8

struct CB_MINIMAP_ROBOT_POSITION
{
	XMFLOAT2 robotPosition;
	BOOL enemyOrTeam;
};

class CMinimapShader : public CShader
{
public:
	CMinimapShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CMinimapShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateVertexShaderMinimapRobot(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShaderMinimapRobot(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShaderMinimapSight(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShaderMinimapRobot(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShaderMinimapBG(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShaderMinimapSight(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayoutMinimapRect();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	void UpdateMinimapRobotInfo(CGameObject *object, BYTE id);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	void InsertMinimapRobot(CGameObject *object, int index);
	void InsertMinimapRobotInfo(XMFLOAT4X4 objectWorld, int index);

protected:

	int								m_nUIRect = 0;
	CRect							**m_ppUIRects = NULL;

	int								m_nTextures;
	CTexture						**m_ppTextures = NULL;

	int								m_nMinimapRobotRect = 0;
	CMinimapRobotRect				**m_ppMinimapRobotRects = NULL;

	ID3D12PipelineState				*m_pd3dPipelineStateMinimapRobot = NULL;

	ID3D12Resource					*m_MinimapRobotRsc = NULL;
	CB_MINIMAP_ROBOT_POSITION		*m_cbMinimapRobotInfo = NULL;

	CTexture						*m_pMinimapRobotTexture;

	ID3D12PipelineState				*m_pd3dPipelineStateMinimapBG = NULL;
	ID3D12PipelineState				*m_pd3dPipelineStateMinimapSight = NULL;
};