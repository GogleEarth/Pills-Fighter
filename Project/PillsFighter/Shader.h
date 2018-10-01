#pragma once

#include "GameObject.h"
#include "Camera.h"

// ½¦ÀÌ´õ ¸®¼Ò½º ºä¸¸ °¡Áü.
class CShader
{
public:
	CShader();
	virtual ~CShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayoutWire();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerStateWire();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateVertexShaderWire(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShaderWire(ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR *pszFileName, LPCSTR pszShaderName,
		LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature
		*pd3dGraphicsRootSignature);

	virtual void CreateSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nShaderResourceViews);
	virtual void CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {}
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {}
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World) {}
	virtual void ReleaseShaderVariables();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }

	virtual void ReleaseUploadBuffers() {}

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPrepareRenderWire(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pd3dSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	void SetStateRenderWire(BOOL bState) {	m_RenderWire = bState; }

protected:
	ID3D12PipelineState				*m_pd3dPipelineState = NULL;
	ID3D12PipelineState				*m_pd3dPipelineStateWire = NULL;

	BOOL							m_RenderWire = FALSE;
private:
	ID3D12DescriptorHeap			*m_pd3dSrvDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CTexturedShader : public CShader
{
public:
	CTexturedShader();
	virtual ~CTexturedShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define _WITH_BATCH_MATERIAL

class CPlayerShader : public CTexturedShader
{
public:
	CPlayerShader();
	virtual ~CPlayerShader();

private:
	CMaterial**	m_ppMaterials;
	UINT		m_nMaterials;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CObjectsShader : public CTexturedShader
{
public:
	CObjectsShader();
	virtual ~CObjectsShader();

	virtual void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, 
		wchar_t* TextureFileName, const char* MeshFileName, void *pContext = NULL );
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void ReleaseShaderVariables();
	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void InsertObject(CGameObject* Object);

	std::vector<CGameObject*>& GetObjects() { return m_vObjects; }

	void CheckDeleteObjects();

	void SetMaterial(CMaterial** ppMaterials, UINT nMaterials) {m_ppMaterials = ppMaterials; m_nMaterials = nMaterials;}

	void SetMesh(CMesh** ppMeshes, CCubeMesh** ppCubeMeshes, UINT nMeshes) { m_ppMeshes = ppMeshes; m_ppCubeMeshes = ppCubeMeshes; m_nMeshes = nMeshes; }

protected:
	UINT		m_nMeshes;
	CMesh**		m_ppMeshes = NULL;
	CCubeMesh**	m_ppCubeMeshes = NULL;

	UINT		m_nMaterials;
	CMaterial**	m_ppMaterials = NULL;

	std::vector<CGameObject*> m_vObjects;

};