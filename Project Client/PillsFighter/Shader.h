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
	CModel						*m_pModel = NULL;

	std::vector<CGameObject*>	m_vObjects;

public:
	std::vector<CGameObject*>* GetObjects() { return &m_vObjects; }

	void CheckDeleteObjects();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL) {};
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* pObject, bool bPrepareRotate = false, void *pContext = NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CBulletShader : public CObjectsShader
{
public:
	CBulletShader();
	virtual ~CBulletShader();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CBZKBulletShader : public CObjectsShader
{
public:
	CBZKBulletShader();
	virtual ~CBZKBulletShader();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRepairItemShader : public CObjectsShader
{
public:
	CRepairItemShader();
	virtual ~CRepairItemShader();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAmmoItemShader : public CObjectsShader
{
public:
	CAmmoItemShader();
	virtual ~CAmmoItemShader();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CObstacleShader : public CObjectsShader
{
public:
	CObstacleShader();
	virtual ~CObstacleShader();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);
	void InsertObjectFromLoadInfFromBin(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName);

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

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CEffectShader : public CShader
{
public:
	CEffectShader();
	virtual ~CEffectShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOutput();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual D3D12_SHADER_BYTECODE CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_DEPTH_STENCIL_DESC CreateSODepthStencilState();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);

	void ReleaseUploadBuffers();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void InsertEffect(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size);

	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void ReleaseObjects();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	CEffect		*m_pEffect = NULL;
	CTexture	*m_pTexture = NULL;

	ID3D12PipelineState				*m_pd3dSOPipelineState = NULL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CSpriteShader : public CEffectShader
{
public:
	CSpriteShader();
	virtual ~CSpriteShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* pObject, bool bPrepareRotate = false, void *pContext = NULL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CParticleShader : public CShader
{
public:
	CParticleShader();
	virtual ~CParticleShader();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOutput();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual D3D12_SHADER_BYTECODE CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_DEPTH_STENCIL_DESC CreateSODepthStencilState();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual void ReleaseUploadBuffers();

	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);
	void SetFollowObject(CGameObject *pObject);

protected:
	ID3DBlob						*m_pd3dGeometryShaderBlob = NULL;

	ID3D12PipelineState				*m_pd3dSOPipelineState = NULL;

public:
	CTexture						*m_pTexture;

	std::vector<CParticle*>			m_vpParticles;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSkinnedObjectsShader : public CSkinnedAnimationShader
{
public:
	CSkinnedObjectsShader();
	virtual ~CSkinnedObjectsShader();

	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void ReleaseObjects();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	CModel						*m_pModel = NULL;

	std::vector<CGameObject*>	m_vObjects;

public:
	std::vector<CGameObject*>* GetObjects() { return &m_vObjects; }

	void CheckDeleteObjects();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL) {};
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* pObject, bool bPrepareRotate = false, void *pContext = NULL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CGundamShader : public CSkinnedObjectsShader
{
public:
	CGundamShader();
	virtual ~CGundamShader();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext = NULL);
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* pObject, bool bPrepareRotate = false, void *pContext = NULL);

protected:
	CModel		*m_pGimGun = NULL;
	CModel		*m_pBazooka = NULL;
	CModel		*m_pMachineGun = NULL;
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

class CUserInterface : public CShader
{
public:
	CUserInterface();
	virtual ~CUserInterface();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShaderBar(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource *pd3dcb, CB_PLAYER_VALUE *pcbMapped, int nMaxValue, int nValue);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	ID3D12PipelineState				*m_pd3dPipelineStateBar = NULL;

	CPlayer							*m_pPlayer = NULL;

	ID3D12Resource					*m_pd3dcbPlayerHP = NULL;
	CB_PLAYER_VALUE					*m_pcbMappedPlayerHP = NULL;

	ID3D12Resource					*m_pd3dcbPlayerBooster = NULL;
	CB_PLAYER_VALUE					*m_pcbMappedPlayerBooster = NULL;

	ID3D12Resource					*m_pd3dcbPlayerAmmo = NULL;
	CB_PLAYER_VALUE					*m_pcbMappedPlayerAmmo = NULL;

	int								m_nUIRect = 0;
	CRect							**m_ppUIRects = NULL;

	int								m_nTextures;
	CTexture						**m_ppTextures = NULL;

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
