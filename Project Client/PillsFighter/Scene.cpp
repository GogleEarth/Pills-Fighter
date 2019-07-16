#include "stdafx.h"
#include "Scene.h"
#include "Repository.h"
#include "Weapon.h"

ID3D12DescriptorHeap			*CScene::m_pd3dSrvUavDescriptorHeap = NULL;
ID3D12RootSignature				*CScene::m_pd3dGraphicsRootSignature = NULL;
ID3D12RootSignature				*CScene::m_pd3dComputeRootSignature = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvUavTextureCPUDescStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvUavTextureGPUDescStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvModelCPUDescStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvModelGPUDescStartHandle;

ID3D12DescriptorHeap			*CScene::m_pd3dRtvDescriptorHeap;
D3D12_CPU_DESCRIPTOR_HANDLE		CScene::m_d3dRtvCPUDesciptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE		CScene::m_d3dRtvGPUDesciptorStartHandle;

ID3D12DescriptorHeap			*CScene::m_pd3dDsvDescriptorHeap;
D3D12_CPU_DESCRIPTOR_HANDLE		CScene::m_d3dDsvCPUDesciptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE		CScene::m_d3dDsvGPUDesciptorStartHandle;

int								CScene::m_nPlayerRobotType = SELECT_CHARACTER_GM;

extern CFMODSound gFmodSound;

CScene::CScene()
{
	CONSOLE_CURSOR_INFO C;
	C.bVisible = 0;
	C.dwSize = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &C);
}

CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	BuildLightsAndMaterials();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pFontShader = new CFontShader();
	m_pFontShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	m_pPostProcessingShader = new CPostProcessingShader();
	m_pPostProcessingShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	m_pTestShader = new CTestShader();
	m_pTestShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	m_pComputeShader = new CComputeShader();
	m_pComputeShader->CreateShader(pd3dDevice, m_pd3dComputeRootSignature);
}

void CScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CreateOffScreenTextures(pd3dDevice);
}

void CScene::SetFont(ID3D12Device *pd3dDevice, CFont *pFont)
{
	pFont->SetSrv(pd3dDevice);
	m_pFont = pFont;
}

CTextObject* CScene::AddText(const wchar_t *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType)
{
	return (m_pFont->SetText(pstrText, xmf2Position, xmf2Scale, xmf2Padding, xmf4Color, nType));
}

void CScene::ChangeText(CTextObject *pTextObject, const wchar_t *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType)
{
	m_pFont->ChangeText(pTextObject, pstrText, xmf2Position, xmf2Scale, xmf2Padding, xmf4Color, nType);
}

void CScene::ReleaseObjects()
{
	if (m_pLights)
	{
		delete m_pLights;
		m_pLights = NULL;
	}

	if (m_pSkyBox)
	{
		delete m_pSkyBox;
		m_pSkyBox = NULL;
	}

	if (m_pTerrain)
	{
		delete m_pTerrain;
		m_pTerrain = NULL;
	}

	if (m_pFontShader)
	{
		delete m_pFontShader;
		m_pFontShader = NULL;
	}

	if (m_pWireShader)
	{
		delete m_pWireShader;
		m_pWireShader = NULL;
	}

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			if (m_ppShaders[i])
			{
				m_ppShaders[i]->ReleaseShaderVariables();
				m_ppShaders[i]->ReleaseObjects();
				delete m_ppShaders[i];
			}
		}
		delete[] m_ppShaders;
		m_ppShaders = NULL;
	}

	if (m_ppEffectShaders)
	{
		for (int i = 0; i < m_nEffectShaders; i++)
		{
			if (m_ppEffectShaders[i])
			{
				m_ppEffectShaders[i]->ReleaseShaderVariables();
				m_ppEffectShaders[i]->ReleaseObjects();
				delete m_ppEffectShaders[i];
			}
		}
		delete[] m_ppEffectShaders;
		m_ppEffectShaders = NULL;
	}

	if (m_pParticleShader)
	{
		m_pParticleShader->ReleaseShaderVariables();
		m_pParticleShader->ReleaseObjects();
		delete m_pParticleShader;
		m_pParticleShader = NULL;
	}

	if (m_pUserInterface)
	{
		m_pUserInterface->ReleaseShaderVariables();
		delete m_pUserInterface;
	}
	
	if(m_pFont)
		m_pFont->ClearTexts();

	if (m_pPostProcessingShader)
	{
		m_pPostProcessingShader->ReleaseShaderVariables();
		m_pPostProcessingShader->ReleaseObjects();
		delete m_pPostProcessingShader;
		m_pPostProcessingShader = NULL;
	}

	if (m_pTestShader)
	{
		m_pTestShader->ReleaseShaderVariables();
		m_pTestShader->ReleaseObjects();
		delete m_pTestShader;
		m_pTestShader = NULL;
	}

	if (m_pComputeShader)
	{
		delete m_pComputeShader;
		m_pComputeShader = NULL;
	}

	if (m_pd3dOffScreenTexture) m_pd3dOffScreenTexture->Release();
	if (m_pd3dGlowScreenTexture) m_pd3dGlowScreenTexture->Release();
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dMotionBlurScreenTexture) m_pd3dMotionBlurScreenTexture->Release();
	if (m_pd3dTempTexture) m_pd3dTempTexture->Release();
	if (m_pd3dMaskTexture) m_pd3dMaskTexture->Release();

	ReleaseShaderVariables();
}

void CScene::CreateOffScreenTextures(ID3D12Device *pd3dDevice)
{
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(d3dHeapProperties));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_HEAP_FLAGS d3dHeapFlags = D3D12_HEAP_FLAG_NONE;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(d3dResourceDesc));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = FRAME_BUFFER_WIDTH;
	d3dResourceDesc.Height = FRAME_BUFFER_HEIGHT;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_CLEAR_VALUE d3dClearValue;
	::ZeroMemory(&d3dClearValue, sizeof(d3dClearValue));
	d3dClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dClearValue.Color[0] = 0.0f;
	d3dClearValue.Color[1] = 0.0f;
	d3dClearValue.Color[2] = 0.0f;
	d3dClearValue.Color[3] = 1.0f;

	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dOffScreenTexture);
	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dGlowScreenTexture);
	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dTempTexture);
	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dMotionBlurScreenTexture);
	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dMaskTexture);

	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, d3dHeapFlags, &d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	CreateRtvDsvSrvUavOffScreens(pd3dDevice);
}

void CScene::CreateRtvDsvSrvUavOffScreens(ID3D12Device *pd3dDevice)
{
	CScene::CreateRenderTargetView(pd3dDevice, m_pd3dOffScreenTexture, D3D12_RTV_DIMENSION_TEXTURE2D, 1, &m_d3dRrvOffScreenCPUHandle);
	CScene::CreateDepthStencilView(pd3dDevice, m_pd3dDepthStencilBuffer, &m_d3dDsvOffScreenCPUHandle);
	m_d3dSrvOffScreenGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dOffScreenTexture, RESOURCE_TEXTURE2D);
	m_d3dUavOffScreenGPUHandle = CScene::CreateUnorderedAccessViews(pd3dDevice, m_pd3dOffScreenTexture);
	m_d3dSrvDepthStencilGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dDepthStencilBuffer, RESOURCE_TEXTURE2D, true);

	CScene::CreateRenderTargetView(pd3dDevice, m_pd3dGlowScreenTexture, D3D12_RTV_DIMENSION_TEXTURE2D, 1, &m_d3dRrvGlowScreenCPUHandle);
	m_d3dSrvGlowScreenGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dGlowScreenTexture, RESOURCE_TEXTURE2D);
	m_d3dUavGlowScreenGPUHandle = CScene::CreateUnorderedAccessViews(pd3dDevice, m_pd3dGlowScreenTexture);

	m_d3dSrvMotionBlurScreenGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dMotionBlurScreenTexture, RESOURCE_TEXTURE2D);
	m_d3dUavMotionBlurScreenGPUHandle = CScene::CreateUnorderedAccessViews(pd3dDevice, m_pd3dMotionBlurScreenTexture);

	m_d3dSrvTempTextureGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dTempTexture, RESOURCE_TEXTURE2D);
	m_d3dUavTempTextureGPUHandle = CScene::CreateUnorderedAccessViews(pd3dDevice, m_pd3dTempTexture);

	CScene::CreateRenderTargetView(pd3dDevice, m_pd3dMaskTexture, D3D12_RTV_DIMENSION_TEXTURE2D, 1, &m_d3dRrvMaskTextureCPUHandle);
	m_d3dSrvMaskTextureGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dMaskTexture, RESOURCE_TEXTURE2D);
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) if(m_ppShaders[i]) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nEffectShaders; i++) if(m_ppEffectShaders[i]) m_ppEffectShaders[i]->ReleaseUploadBuffers();
	if (m_pParticleShader) m_pParticleShader->ReleaseUploadBuffers();
	if (m_pUserInterface) m_pUserInterface->ReleaseUploadBuffers();
}

int CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return 0;
}  

void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_F1:
			//m_bActiveBloom = !m_bActiveBloom;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CScene::ProcessInput(UCHAR *pKeysBuffer, float fElapsedTime)
{
}

void CScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	if (m_pPlayer) m_pPlayer->Update(fTimeElapsed);

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->AnimateObjects(fTimeElapsed, pCamera);
	}

	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->AnimateObjects(fTimeElapsed, pCamera);
	}

	if (m_pParticleShader) m_pParticleShader->AnimateObjects(fTimeElapsed);

	if (m_pTerrain) m_pTerrain->Animate(fTimeElapsed, pCamera);

	if (m_pLights)
	{
		m_pLights->m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights->m_pLights->m_xmf3Direction = m_pPlayer->GetLook();
	}

	CheckCollision();

	m_fFPS = fTimeElapsed;
}

void CScene::PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->PrepareRender(pd3dCommandList);
	}

	if (m_pParticleShader) m_pParticleShader->PrepareRender(pd3dCommandList);
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dOffScreenTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	::TransitionResourceState(pd3dCommandList, m_pd3dGlowScreenTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	::TransitionResourceState(pd3dCommandList, m_pd3dMaskTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	pd3dCommandList->ClearRenderTargetView(m_d3dRrvOffScreenCPUHandle, Colors::Black, 0, NULL);
	pd3dCommandList->ClearRenderTargetView(m_d3dRrvGlowScreenCPUHandle, Colors::Black, 0, NULL);
	pd3dCommandList->ClearRenderTargetView(m_d3dRrvMaskTextureCPUHandle, Colors::Black, 0, NULL);
	pd3dCommandList->ClearDepthStencilView(m_d3dDsvOffScreenCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	if (m_bSelfIllumination)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCPUDescHandle[3] = { m_d3dRrvOffScreenCPUHandle , m_d3dRrvGlowScreenCPUHandle, m_d3dRrvMaskTextureCPUHandle };
		pd3dCommandList->OMSetRenderTargets(_countof(d3dCPUDescHandle), d3dCPUDescHandle, TRUE, &m_d3dDsvOffScreenCPUHandle);
	}
	else pd3dCommandList->OMSetRenderTargets(1, &m_d3dRrvOffScreenCPUHandle, TRUE, &m_d3dDsvOffScreenCPUHandle);

	if (pCamera) pCamera->OnPrepareRender(pd3dCommandList);

	if (m_pd3dcbLights)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_LIGHTS, d3dcbLightsGpuVirtualAddress);
	}

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);

	if (m_pTerrain)
	{
		m_pTerrain->Render(pd3dCommandList, pCamera);
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		if(m_ppShaders[i])
			m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	}
	
	m_nFPS = (m_nFPS + 1) % 5;

	if(pCamera) m_xmf4x4CurrViewProjection = pCamera->GetViewProjMatrix();
}

void CScene::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pWireShader) m_pWireShader->OnPrepareRender(pd3dCommandList);

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->RenderWire(pd3dCommandList, pCamera);
	}
}

void CScene::RenderEffects(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->Render(pd3dCommandList, pCamera);
	}

	if (m_pParticleShader) m_pParticleShader->Render(pd3dCommandList, pCamera);
}

void CScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pUserInterface) m_pUserInterface->Render(pd3dCommandList, NULL);

	if (m_pFontShader) m_pFontShader->Render(pd3dCommandList, NULL);

	if (m_pFont) m_pFont->Render(pd3dCommandList);
}

void CScene::RenderOffScreen(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dOffScreenTexture) pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, m_d3dSrvOffScreenGPUHandle);
	
	m_pPostProcessingShader->Render(pd3dCommandList, NULL);
}

void CScene::RenderTestTexture(ID3D12GraphicsCommandList *pd3dCommandList, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUHandle)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, d3dSrvGPUHandle);

	m_pTestShader->Render(pd3dCommandList, NULL);
}

void CScene::AfterRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dOffScreenTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::TransitionResourceState(pd3dCommandList, m_pd3dGlowScreenTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::TransitionResourceState(pd3dCommandList, m_pd3dMaskTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->AfterRender(pd3dCommandList);
	}

	if (m_pParticleShader) m_pParticleShader->AfterRender(pd3dCommandList);

	if(pCamera) m_xmf4x4PrevViewProjection = pCamera->GetViewProjMatrix();
	if(m_pPlayer) m_xmf3PrevPlayerPosition = m_pPlayer->GetPosition();
}

void CScene::PreparePostProcessing(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_bMotionBlur || m_bSelfIllumination || m_bBloom)
		if (m_pd3dComputeRootSignature) pd3dCommandList->SetComputeRootSignature(m_pd3dComputeRootSignature);
}

void CScene::MotionBlur(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	if (m_bMotionBlur)
	{
		m_pComputeShader->SetMotionBlurPipelineState(pd3dCommandList);

		XMFLOAT3 xmf3PrevPosition = XMFLOAT3(m_xmf4x4PrevViewProjection._41, m_xmf4x4PrevViewProjection._42, m_xmf4x4PrevViewProjection._43);
		XMFLOAT3 xmf3Position = XMFLOAT3(m_xmf4x4CurrViewProjection._41, m_xmf4x4CurrViewProjection._42, m_xmf4x4CurrViewProjection._43);

		float rotVel = Vector3::Length(Vector3::Subtract(xmf3Position, xmf3PrevPosition));

		float moveVel = 0.0f;
		if(m_pPlayer)
			moveVel = Vector3::Length(Vector3::Subtract(m_pPlayer->GetPosition(), m_xmf3PrevPlayerPosition));

		if ((rotVel > 25.0f) || (moveVel > 2.5f))
		{
			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 16, &m_xmf4x4PrevViewProjection, 0);

			XMFLOAT4X4 xmf4x4Inverse = Matrix4x4::Inverse(m_xmf4x4CurrViewProjection);
			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 16, &xmf4x4Inverse, 16);
			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 1, &nWidth, 32);
			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 1, &nHeight, 33);
			pd3dCommandList->SetComputeRoot32BitConstants(COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO, 1, &m_fFPS, 34);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_DEPTH, m_d3dSrvDepthStencilGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_MASK, m_d3dSrvMaskTextureGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvOffScreenGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavMotionBlurScreenGPUHandle);
			pd3dCommandList->Dispatch(nWidth, nHeight, 1);

			m_bMotionBlurred = true;
		}
	}
}

void CScene::Bloom(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	UINT cxGroups = (UINT)ceilf(nWidth / 256.0f);
	UINT cyGroups = (UINT)ceilf(nHeight / 256.0f);

	if (m_bBloom)
	{
		m_pComputeShader->SetBrightFilterPipelineState(pd3dCommandList);
		pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvOffScreenGPUHandle);
		pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavGlowScreenGPUHandle);
		pd3dCommandList->Dispatch(nWidth, nHeight, 1);
	}

	if (m_bBloom || m_bSelfIllumination)
	{
		for (int i = 0; i < 1; i++)
		{
			m_pComputeShader->SetHorzPipelineState(pd3dCommandList);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvGlowScreenGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavTempTextureGPUHandle);
			pd3dCommandList->Dispatch(cxGroups, nHeight, 1);

			m_pComputeShader->SetVertPipelineState(pd3dCommandList);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvTempTextureGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavGlowScreenGPUHandle);
			pd3dCommandList->Dispatch(nWidth, cyGroups, 1);
		}
	}
}

void CScene::Blurring(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	UINT cxGroups = (UINT)ceilf(nWidth / 256.0f);
	UINT cyGroups = (UINT)ceilf(nHeight / 256.0f);

	if (m_bBloom || m_bSelfIllumination)
	{
		for (int i = 0; i < 1; i++)
		{
			m_pComputeShader->SetHorzPipelineState(pd3dCommandList);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvGlowScreenGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavTempTextureGPUHandle);
			pd3dCommandList->Dispatch(cxGroups, nHeight, 1);

			m_pComputeShader->SetVertPipelineState(pd3dCommandList);

			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvTempTextureGPUHandle);
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavGlowScreenGPUHandle);
			pd3dCommandList->Dispatch(nWidth, cyGroups, 1);
		}
	}
}

void CScene::Combine(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	if (m_bBloom || m_bSelfIllumination || m_bMotionBlurred)
	{
		m_pComputeShader->Set2AddPipelineState(pd3dCommandList);

		if ((m_bBloom || m_bSelfIllumination) && m_bMotionBlurred)
		{
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvMotionBlurScreenGPUHandle);
		}
		else
		{
			pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A, m_d3dSrvOffScreenGPUHandle);
		}

		pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B, m_d3dSrvGlowScreenGPUHandle);
		pd3dCommandList->SetComputeRootDescriptorTable(COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT, m_d3dUavOffScreenGPUHandle);
		pd3dCommandList->Dispatch(nWidth, nHeight, 1);

		m_bMotionBlurred = false;
	}
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType, DXGI_FORMAT dxFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = dxFormat;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = 0;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}

// statics

void CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[8];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 3;
	pd3dDescriptorRanges[0].BaseShaderRegister = 1; //t1 ~ t3: gtxtTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 4; //t4: Specular
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 5; //t5: Normal
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 6; //t6: Skybox
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 5;
	pd3dDescriptorRanges[4].BaseShaderRegister = 7; // t7~10: tile*4 + Setting
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 13; // t13: Envir
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 14; //t14: Emissive
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 15; //t15: Shadow map
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[MAX_ROOT_PARAMETER_INDEX];

	pd3dRootParameters[ROOT_PARAMETER_INDEX_OBJECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_OBJECT].Descriptor.ShaderRegister = 0; //Object
	pd3dRootParameters[ROOT_PARAMETER_INDEX_OBJECT].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_OBJECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_CAMERA].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CAMERA].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CAMERA].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CAMERA].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHTS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHTS].Descriptor.ShaderRegister = 2; //Lights
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHTS].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHTS].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; // t1: Diffuse Texture
	pd3dRootParameters[ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; // t4: Specular Texture
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2]; // t5: Normal Texture
	pd3dRootParameters[ROOT_PARAMETER_INDEX_NORMAL_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_EMISSIVE_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EMISSIVE_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EMISSIVE_TEXTURE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[6]; // t14: Emissive Texture
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EMISSIVE_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3]; // t6: Texture Cube
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].Descriptor.ShaderRegister = 3; //HP
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_OFFSETS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_OFFSETS].Descriptor.ShaderRegister = 5; //Offset
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_OFFSETS].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_OFFSETS].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_TRANSFORMS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_TRANSFORMS].Descriptor.ShaderRegister = 6; //Transforms
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_TRANSFORMS].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_BONE_TRANSFORMS].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_TILES].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TILES].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TILES].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4]; // t7~10
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TILES].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_EFFECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EFFECT].Descriptor.ShaderRegister = 7; // Effect
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EFFECT].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_EFFECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPRITE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPRITE].Descriptor.ShaderRegister = 4; //TextureSprite
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPRITE].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SPRITE].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_PARTICLE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PARTICLE].Descriptor.ShaderRegister = 8;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PARTICLE].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PARTICLE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_INSTANCE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_INSTANCE].Descriptor.ShaderRegister = 12;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_INSTANCE].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_INSTANCE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[5]; // t6: Texture Cube
	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRONMENTCUBE_CAMERA].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRONMENTCUBE_CAMERA].Descriptor.ShaderRegister = 9;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRONMENTCUBE_CAMERA].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_ENVIRONMENTCUBE_CAMERA].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].Constants.Num32BitValues = 2;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].Constants.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].Constants.ShaderRegister = 10;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_CURSOR_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCENE_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCENE_INFO].Constants.Num32BitValues = 5;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCENE_INFO].Constants.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCENE_INFO].Constants.ShaderRegister = 11;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SCENE_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_FONT_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FONT_INFO].Constants.Num32BitValues = 6;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FONT_INFO].Constants.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FONT_INFO].Constants.ShaderRegister = 12;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_FONT_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_SHADOW_MAP].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SHADOW_MAP].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SHADOW_MAP].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[7];
	pd3dRootParameters[ROOT_PARAMETER_INDEX_SHADOW_MAP].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHT_CAMERA_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHT_CAMERA_INFO].Descriptor.ShaderRegister = 13;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHT_CAMERA_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHT_CAMERA_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_ROBOT_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_ROBOT_INFO].Descriptor.ShaderRegister = 14;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_ROBOT_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_MINIMAP_ROBOT_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_PLAYER_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PLAYER_INFO].Descriptor.ShaderRegister = 15;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PLAYER_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_PLAYER_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[3];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0;
	pd3dSamplerDescs[2].MaxAnisotropy = 16;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
}

void CScene::CreateComputeRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[6];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1;
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 2;
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 3;
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 0;
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 4;
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[MAX_COMPUTE_ROOT_PARAMETER_INDEX];

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_A].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_B].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_C].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_C].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_C].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_INPUT_C].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_DEPTH].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_DEPTH].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_DEPTH].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_DEPTH].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MASK].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MASK].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MASK].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]); // SRV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MASK].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]); // UAV
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_OUTPUT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].Constants.Num32BitValues = 35;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].Constants.ShaderRegister = 0;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].Constants.RegisterSpace = 0;
	pd3dRootParameters[COMPUTE_ROOT_PARAMETER_INDEX_MOTION_BLUR_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&m_pd3dComputeRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
}

void CScene::CreateDescriptorHeaps(ID3D12Device *pd3dDevice, int nViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nViews;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dSrvUavDescriptorHeap);

	m_d3dSrvModelCPUDescStartHandle = m_pd3dSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dSrvModelGPUDescStartHandle = m_pd3dSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	m_d3dSrvUavTextureCPUDescStartHandle.ptr = m_pd3dSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + ::gnCbvSrvDescriptorIncrementSize * SCENE_MODEL_SRV_DESCRIPTOR_HEAP_COUNT;
	m_d3dSrvUavTextureGPUDescStartHandle.ptr = m_pd3dSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + ::gnCbvSrvDescriptorIncrementSize * SCENE_MODEL_SRV_DESCRIPTOR_HEAP_COUNT;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement, bool bIsModelTexture)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dGPUDescHandle;

	if (bIsModelTexture) d3dGPUDescHandle = m_d3dSrvModelGPUDescStartHandle;
	else d3dGPUDescHandle = m_d3dSrvUavTextureGPUDescStartHandle;

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();

		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource *pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType, d3dResourceDesc.Format);

			D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSrvCPUDescHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE *pd3dSrvGPUDescHandle;

			if (bIsModelTexture)
			{
				pd3dSrvCPUDescHandle = &m_d3dSrvModelCPUDescStartHandle;
				pd3dSrvGPUDescHandle = &m_d3dSrvModelGPUDescStartHandle;
			}
			else
			{
				pd3dSrvCPUDescHandle = &m_d3dSrvUavTextureCPUDescStartHandle;
				pd3dSrvGPUDescHandle = &m_d3dSrvUavTextureGPUDescStartHandle;
			}

			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, *pd3dSrvCPUDescHandle);

			pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameterStartIndex + i) : nRootParameterStartIndex, *pd3dSrvGPUDescHandle);

			(*pd3dSrvCPUDescHandle).ptr += ::gnCbvSrvDescriptorIncrementSize;
			(*pd3dSrvGPUDescHandle).ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}

	return d3dGPUDescHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, UINT nSrvType, bool bIsDS)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvUavTextureGPUDescStartHandle;

	if (pd3dResource)
	{
		D3D12_RESOURCE_DESC d3dResourceDesc = pd3dResource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
		
		if(bIsDS)
			d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nSrvType, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
		else
			d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nSrvType, d3dResourceDesc.Format);

		pd3dDevice->CreateShaderResourceView(pd3dResource, &d3dShaderResourceViewDesc, m_d3dSrvUavTextureCPUDescStartHandle);

		m_d3dSrvUavTextureCPUDescStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		m_d3dSrvUavTextureGPUDescStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}

	return d3dSrvGPUDescriptorHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateUnorderedAccessViews(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dUavGPUDescriptorHandle = m_d3dSrvUavTextureGPUDescStartHandle;

	if (pd3dResource)
	{
		pd3dDevice->CreateUnorderedAccessView(pd3dResource, NULL, NULL, m_d3dSrvUavTextureCPUDescStartHandle);

		m_d3dSrvUavTextureCPUDescStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		m_d3dSrvUavTextureGPUDescStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}

	return d3dUavGPUDescriptorHandle;
}

void CScene::ReleaseDescHeapsAndRootSignature()
{
	if (m_pd3dGraphicsRootSignature)
	{
		m_pd3dGraphicsRootSignature->Release();
		m_pd3dGraphicsRootSignature = NULL;
	}

	if (m_pd3dComputeRootSignature)
	{
		m_pd3dComputeRootSignature->Release();
		m_pd3dComputeRootSignature = NULL;
	}

	if (m_pd3dSrvUavDescriptorHeap)
	{
		m_pd3dSrvUavDescriptorHeap->Release();
		m_pd3dSrvUavDescriptorHeap = NULL;
	}

	if (m_pd3dRtvDescriptorHeap)
	{
		m_pd3dRtvDescriptorHeap->Release();
		m_pd3dRtvDescriptorHeap = NULL;
	}

	if (m_pd3dDsvDescriptorHeap)
	{
		m_pd3dDsvDescriptorHeap->Release();
		m_pd3dDsvDescriptorHeap = NULL;
	}
}

void CScene::SetDescHeapsAndGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature)	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dSrvUavDescriptorHeap)		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvUavDescriptorHeap);
}

void CScene::CreateRtvAndDsvDescriptorHeaps(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = SCENE_RTV_DESCRIPTOR_HEAP_COUNT;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	m_d3dRtvCPUDesciptorStartHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dRtvGPUDesciptorStartHandle = m_pd3dRtvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	d3dDescriptorHeapDesc.NumDescriptors = SCENE_DSV_DESCRIPTOR_HEAP_COUNT;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	m_d3dDsvCPUDesciptorStartHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dDsvGPUDesciptorStartHandle = m_pd3dDsvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

void CScene::CreateRenderTargetView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_RTV_DIMENSION d3dRtvDimension, int nViews, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle)
{
	D3D12_RENDER_TARGET_VIEW_DESC d3dDesc;
	d3dDesc.ViewDimension = d3dRtvDimension;
	d3dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	for (int i = 0; i < nViews; i++)
	{
		switch (d3dRtvDimension)
		{
		case D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D:
			d3dDesc.Texture2D.MipSlice = 0;
			d3dDesc.Texture2D.PlaneSlice = 0;
			break;
		case D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
			d3dDesc.Texture2DArray.MipSlice = 0;
			d3dDesc.Texture2DArray.PlaneSlice = 0;
			d3dDesc.Texture2DArray.FirstArraySlice = i;
			d3dDesc.Texture2DArray.ArraySize = 1;
			break;
		}

		pd3dDevice->CreateRenderTargetView(pd3dResource, &d3dDesc, m_d3dRtvCPUDesciptorStartHandle);
		pd3dSaveCPUHandle[i] = m_d3dRtvCPUDesciptorStartHandle;

		m_d3dRtvCPUDesciptorStartHandle.ptr += ::gnRtvDescriptorIncrementSize;
		m_d3dRtvGPUDesciptorStartHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CScene::CreateDepthStencilView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	pd3dDevice->CreateDepthStencilView(pd3dResource, &d3dDepthStencilViewDesc, m_d3dDsvCPUDesciptorStartHandle);
	(*pd3dSaveCPUHandle) = m_d3dDsvCPUDesciptorStartHandle;

	m_d3dDsvCPUDesciptorStartHandle.ptr += ::gnDsvDescriptorIncrementSize;
	m_d3dDsvGPUDesciptorStartHandle.ptr += ::gnDsvDescriptorIncrementSize;
}

void CScene::ResetDescriptorHeapHandles()
{
	m_d3dSrvUavTextureCPUDescStartHandle.ptr = m_pd3dSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + ::gnCbvSrvDescriptorIncrementSize * SCENE_MODEL_SRV_DESCRIPTOR_HEAP_COUNT;
	m_d3dSrvUavTextureGPUDescStartHandle.ptr = m_pd3dSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + ::gnCbvSrvDescriptorIncrementSize * SCENE_MODEL_SRV_DESCRIPTOR_HEAP_COUNT;

	m_d3dRtvCPUDesciptorStartHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dRtvGPUDesciptorStartHandle = m_pd3dRtvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	m_d3dDsvCPUDesciptorStartHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dDsvGPUDesciptorStartHandle = m_pd3dDsvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLobbyScene::CLobbyScene()
{

}

CLobbyScene::~CLobbyScene()
{

}

int CLobbyScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONUP:
		return MouseClick();
		break;
	default:
		break;
	}

	return 0;
}

void CLobbyScene::MoveCursor(float x, float y)
{
	if (m_pCursor) m_pCursor->MoveCursorPos(x, y);
}

void CLobbyScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);

	if (!m_pLobbyShader)
	{
		m_pLobbyShader = new CLobbyShader();
		m_pLobbyShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
		m_pLobbyShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	}

	if(!m_pCursor) m_pCursor = new CCursor(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CScene::m_nPlayerRobotType = SELECT_CHARACTER_GM;

	m_d3dViewport = { 0.0f, 0.0f, float(FRAME_BUFFER_WIDTH), float(FRAME_BUFFER_HEIGHT), 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
}

void CLobbyScene::ReleaseObjects()
{
	CScene::ReleaseObjects();

	if (m_ppUIRects)
	{
		for (int i = 0; i < m_nUIRect; i++)
		{
			if (m_ppUIRects[i])
				delete m_ppUIRects[i];
		}
		delete[] m_ppUIRects;
		m_ppUIRects = NULL;
	}
	
	if (m_ppTextures)
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			if (m_ppTextures[i])
				delete m_ppTextures[i];
		}
		delete[] m_ppTextures;
		m_ppTextures = NULL;
	}
	
	if (m_pLobbyShader)
	{
		delete m_pLobbyShader;
		m_pLobbyShader = NULL;
	}

	if (m_pCursor)
	{
		delete m_pCursor;
		m_pCursor = NULL;
	}
}

void CLobbyScene::ReleaseUploadBuffers()
{
	CScene::ReleaseUploadBuffers();

	if (m_ppUIRects)
	{
		for (int i = 0; i < m_nUIRect; i++)
		{
			if (m_ppUIRects[i]) m_ppUIRects[i]->ReleaseUploadBuffers();
		}
	}

	if (m_ppTextures)
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
		}
	}

	if (m_pCursor) m_pCursor->ReleaseUploadBuffer();
}

void CLobbyScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);

	CScene::Render(pd3dCommandList, pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLobbyMainScene::CLobbyMainScene()
{

}

CLobbyMainScene::~CLobbyMainScene()
{

}

void CLobbyMainScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CLobbyScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);

	m_nTextures = LOBBY_MAIN_UI_TEXTURE_COUNT;
	m_ppTextures = new CTexture*[m_nTextures];

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_BASE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_BASE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/MainLobby.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_BASE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CREATE_ROOM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CREATE_ROOM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/CreateRoom.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CREATE_ROOM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CREATE_ROOM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CREATE_ROOM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/CreateRoomHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CREATE_ROOM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_UP] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_UP]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/Up.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_UP], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_UP] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_UP]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/UpHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_UP], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_DOWN] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_DOWN]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/Down.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_DOWN], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_DOWN] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_DOWN]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/DownHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_DOWN], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_ROOM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_ROOM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Main/RoomHL.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_ROOM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

	m_nUIRect = LOBBY_MAIN_UI_RECT_COUNT;
	m_ppUIRects = new CRect*[m_nUIRect];

	// Base UI
	XMFLOAT2 xmf2Center = CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
	XMFLOAT2 xmf2Size = CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_BASE] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	float width = 185.0f / FRAME_BUFFER_WIDTH;
	float height = 50.0f / FRAME_BUFFER_HEIGHT;
	float centerx = 0.796375f;
	float centery = -0.371222f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_CREATE_ROOM_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	width = 22.0f / FRAME_BUFFER_WIDTH;
	height = 12.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.0f;
	centery = 0.8f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_UP_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	centery = -0.173f;
	xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
	xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_DOWN_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	for (int i = 0; i < 8; i++)
	{
		width = 5.0f / FRAME_BUFFER_WIDTH;
		height = 38.0f / FRAME_BUFFER_HEIGHT;
		centerx = -0.952f;
		centery = 0.7225f - 0.1162f * i;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_MAIN_UI_RECT_ROOM_BUTTON_1 + i] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
	}

	::memset(m_bHLRoomButton, false, sizeof(m_bHLRoomButton));
}

void CLobbyMainScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CLobbyScene::SetAfterBuildObject(pd3dDevice, pd3dCommandList, pContext);

	float width = 185.0f / FRAME_BUFFER_WIDTH;
	float height = 50.0f / FRAME_BUFFER_HEIGHT;
	float centerx = 0.796375f;
	float centery = -0.371222f;
	m_CreateRoomButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_CreateRoomButton.Extents = XMFLOAT3(width, height, 1.0f);

	width = 1210.0f / FRAME_BUFFER_WIDTH;
	height = 12.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.0f;
	centery = 0.8f;
	m_UpButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_UpButton.Extents = XMFLOAT3(width, height, 1.0f);

	centery = -0.173f;
	m_DownButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_DownButton.Extents = XMFLOAT3(width, height, 1.0f);

	height = 38.0f / FRAME_BUFFER_HEIGHT;
	for (int i = 0; i < 8; i++)
	{
		centery = 0.7225f - 0.1162f * i;

		m_RoomButton[i].Center = XMFLOAT3(centerx, centery, 1.0f);
		m_RoomButton[i].Extents = XMFLOAT3(width, height, 1.0f);
	}

#ifndef ON_NETWORKING
	AddRoom(1);
	AddRoom(2);
	AddRoom(3);
	AddRoom(4);
	AddRoom(5);
	AddRoom(6);
	AddRoom(7);
	AddRoom(8);
	AddRoom(9);
#endif

}

void CLobbyMainScene::ReleaseObjects()
{
	CLobbyScene::ReleaseObjects();
}

void CLobbyMainScene::ReleaseUploadBuffers()
{
	CLobbyScene::ReleaseUploadBuffers();
}

int CLobbyMainScene::MouseClick()
{
	if (m_pCursor->CollisionCheck(m_CreateRoomButton))
	{
		return LOBBY_MOUSE_CLICK_CREATE_ROOM;
	}

	if (m_RoomStart > 0)
	{
		if (m_pCursor->CollisionCheck(m_UpButton))
		{
			m_RoomStart--;
			
			for (auto& Room : m_Rooms)
			{
				Room.pRoom_map->MovePosition(XMFLOAT2(0.0f, -0.1162f));
				Room.pRoom_name->MovePosition(XMFLOAT2(0.0f, -0.1162f));
				Room.pRoom_num->MovePosition(XMFLOAT2(0.0f, -0.1162f));
				Room.pRoom_num_people->MovePosition(XMFLOAT2(0.0f, -0.1162f));
			}
		}
	}

	if (m_Rooms.size() - m_RoomStart > 8)
	{
		if (m_pCursor->CollisionCheck(m_DownButton))
		{
			m_RoomStart++;

			for (auto& Room : m_Rooms)
			{
				Room.pRoom_map->MovePosition(XMFLOAT2(0.0f, 0.1162f));
				Room.pRoom_name->MovePosition(XMFLOAT2(0.0f, 0.1162f));
				Room.pRoom_num->MovePosition(XMFLOAT2(0.0f, 0.1162f));
				Room.pRoom_num_people->MovePosition(XMFLOAT2(0.0f, 0.1162f));
			}
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (m_Rooms.size() - m_RoomStart <= i) continue;

		if (m_pCursor->CollisionCheck(m_RoomButton[i]))
		{
			m_nSelectRoom = m_RoomStart + i;
#ifndef ON_NETWORKING
			//DeleteRoom(m_Rooms[m_nSelectRoom].nRoom_num);
			ChangeRoomInfo(m_Rooms[m_nSelectRoom].nRoom_num, SCENE_TYPE_SPACE, 5);
#endif
			return LOBBY_MOUSE_CLICK_JOIN_ROOM;
		}
	}

	return 0;
}

void CLobbyMainScene::CheckCollision()
{
	if (m_pCursor->CollisionCheck(m_CreateRoomButton)) m_bHLCreateRoomButton = true;
	else m_bHLCreateRoomButton = false;

	if (m_pCursor->CollisionCheck(m_UpButton)) m_bHLUpButton = true;
	else m_bHLUpButton = false;

	if (m_pCursor->CollisionCheck(m_DownButton)) m_bHLDownButton = true;
	else m_bHLDownButton = false;

	for (int i = 0; i < 8; i++)
	{
		m_bHLRoomButton[i] = false;
		if (m_Rooms.size() - m_RoomStart <= i) continue;

		if (m_pCursor->CollisionCheck(m_RoomButton[i])) m_bHLRoomButton[i] = true;
	}
}

void CLobbyMainScene::StartScene()
{
}

void CLobbyMainScene::AddRoom(int n)
{
	ROOM_INFO_TEXT newRoom;

	wchar_t pstrNumber[6];
	wsprintfW(pstrNumber, L"%d", n);

	newRoom.nRoom_num = n;
	newRoom.pRoom_num = AddText(pstrNumber, XMFLOAT2(-0.943750f, 0.0f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);
	newRoom.pRoom_name = AddText(L"name", XMFLOAT2(-0.806250f, 0.0f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);
	newRoom.pRoom_map = AddText(L"콜로니", XMFLOAT2(0.389063f, 0.0f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);
	newRoom.pRoom_num_people = AddText(L"1/8", XMFLOAT2(0.776562f, 0.0f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);

	float yPos = (m_RoomStart + m_Rooms .size())* 0.1162f;
	XMFLOAT2 xmf2Position = XMFLOAT2(0.0f, 0.730222f - yPos);
	newRoom.pRoom_num->SetPosition(xmf2Position);
	newRoom.pRoom_name->SetPosition(xmf2Position);
	newRoom.pRoom_map->SetPosition(xmf2Position);
	newRoom.pRoom_num_people->SetPosition(xmf2Position);

	m_Rooms.emplace_back(newRoom);
}

void CLobbyMainScene::DeleteRoom(int n)
{
	int nStart = -1;

	for (int i = 0; i < m_Rooms.size(); i++)
	{
		if (m_Rooms[i].nRoom_num == n)
		{
			nStart = i;
			break;
		}
	}

	for (auto& Room = m_Rooms.begin(); Room != m_Rooms.end(); Room++)
	{
		if (Room->nRoom_num == n)
		{
			m_Rooms.erase(Room);
			break;
		}
	}

	for (int i = nStart; i < m_Rooms.size(); i++)
	{
		m_Rooms[i].pRoom_num->MovePosition(XMFLOAT2(0.0f, 0.1162f));
		m_Rooms[i].pRoom_name->MovePosition(XMFLOAT2(0.0f, 0.1162f));
		m_Rooms[i].pRoom_map->MovePosition(XMFLOAT2(0.0f, 0.1162f));
		m_Rooms[i].pRoom_num_people->MovePosition(XMFLOAT2(0.0f, 0.1162f));
	}
}

void CLobbyMainScene::ChangeRoomInfo(int index, int map, int people)
{
	wchar_t pstr[6];
	wsprintfW(pstr, L"%d/8", people);

	for (auto& room : m_Rooms)
	{
		if (room.nRoom_num == index)
		{
			if (map == SCENE_TYPE_COLONY)
				m_pFont->ChangeText(room.pRoom_map, L"콜로니", XMFLOAT2(0.389063f, 0.0f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);
			else if (map == SCENE_TYPE_SPACE)
				m_pFont->ChangeText(room.pRoom_map, L"스페이스", XMFLOAT2(0.389063f, 0.0f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);

			m_pFont->ChangeText(room.pRoom_num_people, pstr, XMFLOAT2(0.776562f, 0.0f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), LEFT_ALIGN);
		}
	}
}

void CLobbyMainScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);

	m_ppTextures[LOBBY_MAIN_UI_TEXTURE_BASE]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_MAIN_UI_TEXTURE_BASE]->Render(pd3dCommandList, 0);

	if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);

	if (m_bHLCreateRoomButton)
		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_CREATE_ROOM]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_MAIN_UI_TEXTURE_CREATE_ROOM]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_MAIN_UI_RECT_CREATE_ROOM_BUTTON]->Render(pd3dCommandList, 0);

	if (m_RoomStart > 0)
	{
		if (m_bHLUpButton)
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_UP]->UpdateShaderVariables(pd3dCommandList);
		else
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_UP]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[LOBBY_MAIN_UI_RECT_UP_BUTTON]->Render(pd3dCommandList, 0);
	}

	if (m_Rooms.size() - m_RoomStart > 8)
	{
		if (m_bHLDownButton)
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_DOWN]->UpdateShaderVariables(pd3dCommandList);
		else
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_DOWN]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[LOBBY_MAIN_UI_RECT_DOWN_BUTTON]->Render(pd3dCommandList, 0);
	}

	for (int i = 0; i < 8; i++)
	{
		if (m_bHLRoomButton[i])
		{
			m_ppTextures[LOBBY_MAIN_UI_TEXTURE_HL_ROOM]->UpdateShaderVariables(pd3dCommandList);
			m_ppUIRects[LOBBY_MAIN_UI_RECT_ROOM_BUTTON_1 + i]->Render(pd3dCommandList, 0);
		}
	}

	if (m_Rooms.size() > 0)
	{
		if (m_pFontShader) m_pFontShader->Render(pd3dCommandList, NULL);
		m_pFont->OnPrepareRender(pd3dCommandList);

		int nEnd = m_RoomStart + 8;
		if (nEnd > m_Rooms.size()) nEnd = m_Rooms.size();

		for (int i = m_RoomStart; i < nEnd; i++)
		{
			m_Rooms[i].pRoom_map->Render(pd3dCommandList);
			m_Rooms[i].pRoom_name->Render(pd3dCommandList);
			m_Rooms[i].pRoom_num->Render(pd3dCommandList);
			m_Rooms[i].pRoom_num_people->Render(pd3dCommandList);
		}
	}

	if (m_pCursor) m_pCursor->Render(pd3dCommandList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLobbyRoomScene::CLobbyRoomScene()
{

}

CLobbyRoomScene::~CLobbyRoomScene()
{

}

void CLobbyRoomScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CLobbyScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);

	{
		m_nTextures = LOBBY_ROOM_UI_TEXTURE_COUNT;
		m_ppTextures = new CTexture*[m_nTextures];

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MANAGER] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MANAGER]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/RoomLobbyManager.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MANAGER], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MEMBER] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MEMBER]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/RoomLobbyMember.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MEMBER], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Start.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/StartHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_READY] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_READY]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Ready.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_READY], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_READY_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_READY_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/ReadyHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_READY_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Leave.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/LeaveHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Colony.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/ColonyHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_SELECT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_SELECT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/ColonySelect.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_SELECT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Space.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);


		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/SpaceHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_SELECT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_SELECT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/SpaceSelect.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_SELECT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GM.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GMHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_SELECT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_SELECT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GMSelect.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_SELECT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/Gundam.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_HL] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_HL]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GundamHL.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_HL], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);

		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_SELECT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_SELECT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Room/GundamSelect.dds", 0);
		CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_SELECT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);
	}

	{
		m_nUIRect = LOBBY_ROOM_UI_RECT_COUNT;
		m_ppUIRects = new CRect*[m_nUIRect];

		// Base UI

		XMFLOAT2 xmf2Center = CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
		XMFLOAT2 xmf2Size = CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_BASE] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		float width = 185.0f / FRAME_BUFFER_WIDTH;
		float height = 50.0f / FRAME_BUFFER_HEIGHT;
		float centerx = 0.797187f;
		float centery = -0.375f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_START_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centerx = 0.797187f;
		centery = -0.60575f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_READY_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centerx = 0.797187f;
		centery = -0.8425f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_LEAVE_BUTTON] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);


		width = 90.0f / FRAME_BUFFER_WIDTH;
		height = 32.0f / FRAME_BUFFER_HEIGHT;
		centerx = 0.721875f;
		centery = 0.111111f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_COLONY] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centerx = 0.871875f;
		centery = 0.111111f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_SPACE] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);


		width = 58.0f / FRAME_BUFFER_WIDTH;
		height = 76.0f / FRAME_BUFFER_HEIGHT;
		centerx = 0.697313f;
		centery = -0.090444f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_GM] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centerx = 0.797313f;
		centery = -0.090444f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_GUNDAM] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		// left
		centerx = -0.270312f;
		centery = 0.705f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppPlayerRobotRects[0] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = 0.455;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppPlayerRobotRects[2] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = 0.205f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppPlayerRobotRects[4] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = -0.055f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppPlayerRobotRects[6] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		// right
		centerx = 0.516312f;
		centery = 0.705f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppPlayerRobotRects[1] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = 0.455;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppPlayerRobotRects[3] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = 0.205f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppPlayerRobotRects[5] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

		centery = -0.055f;
		xmf2Center = ::CalculateCenter(centerx - width, centerx + width, centery + height, centery - height);
		xmf2Size = ::CalculateSize(centerx - width, centerx + width, centery + height, centery - height);
		m_ppPlayerRobotRects[7] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
	}
}

void CLobbyRoomScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CLobbyScene::SetAfterBuildObject(pd3dDevice, pd3dCommandList, pContext);

#ifndef ON_NETWORKING
	JoinPlayer(0, L"1 : First Player", SELECT_CHARACTER_GM);
	JoinPlayer(1, L"2 : Second Player", SELECT_CHARACTER_GM);
	JoinPlayer(2, L"3 : Third Player", SELECT_CHARACTER_GM);
	JoinPlayer(3, L"4 : Fourth Player", SELECT_CHARACTER_GM);
	JoinPlayer(4, L"5 : Fifth Player", SELECT_CHARACTER_GM);
	JoinPlayer(5, L"6 : Sixth Player", SELECT_CHARACTER_GM);
	JoinPlayer(6, L"7 : Seventh Player", SELECT_CHARACTER_GM);
	JoinPlayer(7, L"8 : Ehighth Player", SELECT_CHARACTER_GM);
#endif

	float width = 185.0f / FRAME_BUFFER_WIDTH;
	float height = 50.0f / FRAME_BUFFER_HEIGHT;
	float centerx = 0.797187f;
	float centery = -0.375f;
	m_StartButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_StartButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.797187f;
	centery = -0.60575f;
	m_ReadyButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_ReadyButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.797187f;
	centery = -0.8425f;
	m_LeaveButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_LeaveButton.Extents = XMFLOAT3(width, height, 1.0f);


	width = 90.0f / FRAME_BUFFER_WIDTH;
	height = 32.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.721875f;
	centery = 0.111111f;
	m_ColonyButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_ColonyButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.871875f;
	centery = 0.111111f;
	m_SpaceButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_SpaceButton.Extents = XMFLOAT3(width, height, 1.0f);


	width = 58.0f / FRAME_BUFFER_WIDTH;
	height = 76.0f / FRAME_BUFFER_HEIGHT;
	centerx = 0.697313f;
	centery = -0.090444f;
	m_GMButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_GMButton.Extents = XMFLOAT3(width, height, 1.0f);

	centerx = 0.797313f;
	centery = -0.090444f;
	m_GundamButton.Center = XMFLOAT3(centerx, centery, 1.0f);
	m_GundamButton.Extents = XMFLOAT3(width, height, 1.0f);
}

void CLobbyRoomScene::SetPlayerIndex(int nServerIndex)
{
	wchar_t id[32];
	wsprintfW(id, L"%d", nServerIndex);

	m_nMyIndex = nServerIndex;
	JoinPlayer(nServerIndex, id, SELECT_CHARACTER_GM);
}

void CLobbyRoomScene::SetMap(int nMap)
{
	m_nCurrentMap = nMap;
}

void CLobbyRoomScene::ChangeSelectRobot(int nServerIndex, int nRobotType)
{
	m_umPlayerInfo[nServerIndex].nRobotType = nRobotType;
}

void CLobbyRoomScene::ReleaseObjects()
{
	CLobbyScene::ReleaseObjects();

	for (int i = 0; i < 8; i++)
	{
		if (m_ppPlayerRobotRects[i])
			delete m_ppPlayerRobotRects[i];
	}
}

void CLobbyRoomScene::ReleaseUploadBuffers()
{
	CLobbyScene::ReleaseUploadBuffers();

	for (int i = 0; i < 8; i++)
	{
		if (m_ppPlayerRobotRects[i])
			m_ppPlayerRobotRects[i]->ReleaseUploadBuffers();
	}
}

int CLobbyRoomScene::MouseClick()
{
	if (m_nMyIndex == 0)
	{
		if (m_pCursor->CollisionCheck(m_StartButton))
		{
			return LOBBY_MOUSE_CLICK_START;
		}
	}
	else
	{
		if (m_pCursor->CollisionCheck(m_ReadyButton))
		{
			return LOBBY_MOUSE_CLICK_READY;
		}
	}

	if (m_pCursor->CollisionCheck(m_LeaveButton))
	{
		return LOBBY_MOUSE_CLICK_LEAVE;
	}

	if (m_nMyIndex == 0)
	{
		if (m_pCursor->CollisionCheck(m_ColonyButton))
		{
			m_nCurrentMap = SCENE_TYPE_COLONY;

			return LOBBY_MOUSE_CLICK_SELECT_MAP;
		}

		if (m_pCursor->CollisionCheck(m_SpaceButton))
		{
			m_nCurrentMap = SCENE_TYPE_SPACE;

			return LOBBY_MOUSE_CLICK_SELECT_MAP;
		}
	}

	if (m_pCursor->CollisionCheck(m_GMButton))
	{
		CScene::m_nPlayerRobotType = SELECT_CHARACTER_GM;

		return LOBBY_MOUSE_CLICK_SELECT_ROBOT;
	}
	if (m_pCursor->CollisionCheck(m_GundamButton))
	{
		CScene::m_nPlayerRobotType = SELECT_CHARACTER_GUNDAM;

		return LOBBY_MOUSE_CLICK_SELECT_ROBOT;
	}

	return 0;
}

void CLobbyRoomScene::CheckCollision()
{
	if (m_pCursor->CollisionCheck(m_StartButton)) m_bHLStartButton = true;
	else m_bHLStartButton = false;

	if (m_pCursor->CollisionCheck(m_ReadyButton)) m_bHLReadyButton = true;
	else m_bHLReadyButton = false;

	if (m_pCursor->CollisionCheck(m_LeaveButton)) m_bHLLeaveButton = true;
	else m_bHLLeaveButton = false;

	if (m_nMyIndex == 0)
	{
		if (m_pCursor->CollisionCheck(m_ColonyButton)) m_bHLColonyButton = true;
		else m_bHLColonyButton = false;

		if (m_pCursor->CollisionCheck(m_SpaceButton)) m_bHLSpaceButton = true;
		else m_bHLSpaceButton = false;
	}

	if (m_pCursor->CollisionCheck(m_GMButton)) m_bHLGMButton = true;
	else m_bHLGMButton = false;

	if (m_pCursor->CollisionCheck(m_GundamButton)) m_bHLGundamButton = true;
	else m_bHLGundamButton = false;
}

void CLobbyRoomScene::StartScene()
{
	//gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundBGM, &(gFmodSound.m_pBGMChannel));
}

XMFLOAT2 CLobbyRoomScene::GetPlayerTextPosition(int nServerIndex)
{
	XMFLOAT2 xmf2Pos;
	xmf2Pos.y = 0.705f;

	if(m_nCurrentSlotIndex % 2 == 0)
		xmf2Pos.x = -0.916875f;
	else
		xmf2Pos.x = -0.130937f;

	xmf2Pos.y += ((int)(m_nCurrentSlotIndex / 2)) * -0.255f;
	
	m_nCurrentSlotIndex++;

	return xmf2Pos;
}

void CLobbyRoomScene::JoinPlayer(int nServerIndex, const wchar_t *pstrPlayerName, int nRobotType)
{
	m_umPlayerInfo[nServerIndex].nSlotIndex = m_nCurrentSlotIndex;
	m_umPlayerInfo[nServerIndex].nRobotType = nRobotType;

	XMFLOAT2 xmf2Pos = GetPlayerTextPosition(m_nCurrentSlotIndex);

	XMFLOAT4 xmf4Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	if (nServerIndex == m_nMyIndex)
	{
		xmf4Color.x = 1.0f;
		xmf4Color.z = 0.0f;
	}

	m_ppTextObjects[nServerIndex] = AddText(pstrPlayerName, xmf2Pos, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), xmf4Color, LEFT_ALIGN);
}

void CLobbyRoomScene::LeavePlayer(int nServerIndex)
{
	int nSlotIndex = m_umPlayerInfo[nServerIndex].nSlotIndex;
	
	if (!m_ppTextObjects[nServerIndex])
		return;

	m_ppTextObjects[nServerIndex]->Release();
	m_ppTextObjects[nServerIndex] = NULL;
	m_nCurrentSlotIndex = 0;

	m_umPlayerInfo.erase(nServerIndex);

	for (auto& playerinfo : m_umPlayerInfo)
	{
		int nServerIndex = playerinfo.first;

		playerinfo.second.nSlotIndex = m_nCurrentSlotIndex;

		XMFLOAT2 xmf2Pos = GetPlayerTextPosition(m_nCurrentSlotIndex);

		XMFLOAT4 xmf4Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

		if (nServerIndex == m_nMyIndex)
		{
			xmf4Color.x = 1.0f;
			xmf4Color.z = 0.0f;
		}

		ChangeText(m_ppTextObjects[nServerIndex], m_ppTextObjects[nServerIndex]->GetText(), xmf2Pos, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), xmf4Color, LEFT_ALIGN);
	}
}

void CLobbyRoomScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);

	if (m_nMyIndex == 0)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MANAGER]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_BASE_MEMBER]->UpdateShaderVariables(pd3dCommandList);

	m_ppUIRects[LOBBY_ROOM_UI_RECT_BASE]->Render(pd3dCommandList, 0);

	CScene::RenderUI(pd3dCommandList);

	if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, NULL);

	if (m_nMyIndex == 0)
	{
		if (m_bHLStartButton)
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START_HL]->UpdateShaderVariables(pd3dCommandList);
		else
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_START]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_START_BUTTON]->Render(pd3dCommandList, 0);
	}
	else
	{
		if (m_bHLReadyButton)
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_READY_HL]->UpdateShaderVariables(pd3dCommandList);
		else
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_READY]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[LOBBY_ROOM_UI_RECT_READY_BUTTON]->Render(pd3dCommandList, 0);
	}

	if (m_bHLLeaveButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE_HL]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_LEAVE]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_LEAVE_BUTTON]->Render(pd3dCommandList, 0);

	if (m_bHLColonyButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_HL]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nCurrentMap == SCENE_TYPE_COLONY)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY_SELECT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_COLONY]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_COLONY]->Render(pd3dCommandList, 0);

	if (m_bHLSpaceButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_HL]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nCurrentMap == SCENE_TYPE_SPACE)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE_SELECT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_SPACE]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_SPACE]->Render(pd3dCommandList, 0);

	if (m_bHLGMButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_HL]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nPlayerRobotType == SELECT_CHARACTER_GM)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM_SELECT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_GM]->Render(pd3dCommandList, 0);

	if (m_bHLGundamButton)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_HL]->UpdateShaderVariables(pd3dCommandList);
	else if (m_nPlayerRobotType == SELECT_CHARACTER_GUNDAM)
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM_SELECT]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[LOBBY_ROOM_UI_RECT_GUNDAM]->Render(pd3dCommandList, 0);

	for (auto& playerinfo : m_umPlayerInfo)
	{
		int nServerIndex = playerinfo.first;
		int nSlotIndex = playerinfo.second.nSlotIndex;
		int nRobotType = playerinfo.second.nRobotType;

		if (!m_ppTextObjects[nServerIndex]) continue;

		if (nRobotType == SKINNED_OBJECT_INDEX_GM)
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GM]->UpdateShaderVariables(pd3dCommandList);
		else if (nRobotType == SKINNED_OBJECT_INDEX_GUNDAM)
			m_ppTextures[LOBBY_ROOM_UI_TEXTURE_GUNDAM]->UpdateShaderVariables(pd3dCommandList);

		m_ppPlayerRobotRects[nSlotIndex]->Render(pd3dCommandList, 0);
	}
	
	if (m_pCursor) m_pCursor->Render(pd3dCommandList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CBattleScene::CBattleScene() : CScene()
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
		m_pObjects[i] = NULL;

	m_xmf4x4PrevViewProjection = Matrix4x4::Identity();
}

CBattleScene::~CBattleScene()
{
}

int CBattleScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		if (::GetCapture() == hWnd)
		{
			if (!m_LButtonDown) m_pPlayer->PrepareAttack(m_pPlayer->GetRHWeapon());

			m_LButtonDown = TRUE;
			m_pPlayer->Attack(m_pPlayer->GetRHWeapon());
		}
		break;
	case WM_LBUTTONUP:
	{
		m_LButtonDown = FALSE;
		m_pPlayer->LButtonUp();
		break;
	}
	default:
		break;
	}

	return 0;
}

void CBattleScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CScene::OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_SHIFT:
			m_pPlayer->ShiftUp();
			break;
		case VK_SPACE:
			m_pPlayer->SpaceUp();
			break;
		default:
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
			m_pPlayer->ChangeWeapon(0);
			break;
		case '2':
			m_pPlayer->ChangeWeapon(1);
			break;
		case '3':
			m_pPlayer->ChangeWeapon(2);
			break;
		case '4':
			m_pPlayer->ChangeWeapon(3);
			break;
		case 'R': 
			m_pPlayer->Reload(m_pPlayer->GetRHWeapon());
			break;
		case VK_SPACE:
			m_pPlayer->ActivationBooster();
			break;
		case VK_SHIFT:
			m_pPlayer->ActivationDash();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CBattleScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);
	
	// Objects
	m_nShaders = SHADER_INDEX;
	m_ppShaders = new CShader*[m_nShaders];
	ZeroMemory(m_ppShaders, sizeof(CShader*) * m_nShaders);

	/* 그룹 1 [ Standard Shader ] */
	CStandardObjectsShader *pStandardObjectsShader = new CStandardObjectsShader();
	pStandardObjectsShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pStandardObjectsShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS] = pStandardObjectsShader;

	/* 그룹 2 [ Instancing Shader ] */
	BuildObstacleObjetcs(pd3dDevice, pd3dCommandList, pRepository);

	/* 그룹 3 [ Skinned Animation Shader ] */
	CRobotObjectsShader *pSkinnedRobotShader = new CRobotObjectsShader();
	pSkinnedRobotShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pSkinnedRobotShader->Initialize(pd3dDevice, pd3dCommandList, pRepository, m_pd3dGraphicsRootSignature);
	m_ppShaders[INDEX_SHADER_SKINND_OBJECTS] = pSkinnedRobotShader;

	// Effects
	m_nEffectShaders = EFFECT_SHADER_INDEX;
	m_ppEffectShaders = new CEffectShader*[m_nEffectShaders];

	// 그룹 1 [ Text Effect Shader ]
	CTextEffectShader *pTextEffectShader = new CTextEffectShader();
	pTextEffectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pTextEffectShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS] = pTextEffectShader;

	// 그룹 2 [ Timed Effect Shader ]
	CTimedEffectShader *pTimedEffectShader = new CTimedEffectShader();
	pTimedEffectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pTimedEffectShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS] = pTimedEffectShader;

	// 그룹 3 [ Sprite Shader ]
	CSpriteShader *pSpriteShader = new CSpriteShader();
	pSpriteShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pSpriteShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS] = pSpriteShader;
	   
	// Particle
	m_pParticleShader = new CParticleShader();
	m_pParticleShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pParticleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);

	////
	// Wire
	m_pWireShader = new CWireShader();
	m_pWireShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	// Terrain
	BuildTerrain(pd3dDevice, pd3dCommandList);

	// SkyBox
	BuildSkybox(pd3dDevice, pd3dCommandList);

	// Weapons
	m_pGimGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/GIM_GUN.bin", NULL, NULL);
	m_pBazooka = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/BZK.bin", NULL, NULL);
	m_pMachineGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/MACHINEGUN.bin", NULL, NULL);
	m_pSaber = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/Saber.bin", NULL, NULL);
}

void CBattleScene::ReleaseObjects()
{
	CScene::ReleaseObjects();

	if (m_pd3dEnvirCube) m_pd3dEnvirCube->Release();
	if (m_pd3dEnvirCubeDSBuffer) m_pd3dEnvirCubeDSBuffer->Release();
	if (m_pd3dShadowMap) m_pd3dShadowMap->Release();

	for (int i = 0; i < 6; i++)
	{
		if (m_pCubeMapCamera[i])
		{
			m_pCubeMapCamera[i]->ReleaseShaderVariables();
			delete m_pCubeMapCamera[i];
		}
	}

	if (m_pLightCamera)
	{
		m_pLightCamera->ReleaseShaderVariables();
		delete m_pLightCamera;
	}

	if (m_pMinimapShader)
	{
		m_pMinimapShader->ReleaseShaderVariables();
		delete m_pMinimapShader;
	}
}

void CBattleScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	if (m_pPlayer) m_pPlayer->ApplyGravity(m_fGravAcc, fTimeElapsed);

	CScene::AnimateObjects(fTimeElapsed, pCamera);
}

void CBattleScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CScene::SetAfterBuildObject(pd3dDevice, pd3dCommandList, pContext);

	CreateEnvironmentMap(pd3dDevice);
	CreateCubeMapCamera(pd3dDevice, pd3dCommandList);
	CreateShadowMap(pd3dDevice, pd3dCommandList, 4096 * 2, 4096 * 2);

	if (m_pParticleShader) m_pParticleShader->SetFollowObject(m_pPlayer, m_pPlayer->GetRightNozzleFrame());
	if (m_pParticleShader) m_pParticleShader->SetFollowObject(m_pPlayer, m_pPlayer->GetLeftNozzleFrame());

	if (m_pPlayer)
	{
		m_pPlayer->SetScene(this);
	}

	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, 
		m_pGimGun, WEAPON_TYPE_OF_GIM_GUN, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS], m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS], STANDARD_OBJECT_INDEX_GG_BULLET);
	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList,
		m_pBazooka, WEAPON_TYPE_OF_BAZOOKA, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS], m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS], STANDARD_OBJECT_INDEX_BZK_BULLET);
	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, 
		m_pMachineGun, WEAPON_TYPE_OF_MACHINEGUN, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS], m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS], STANDARD_OBJECT_INDEX_MG_BULLET);
	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList,
		m_pSaber, WEAPON_TYPE_OF_SABER, NULL, NULL, NULL);

#ifndef ON_NETWORKING
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_GIM_GUN, 50);
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_BAZOOKA, 20);
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_MACHINEGUN, 300);
#endif

	CUserInterface *pUserInterface = new CUserInterface();
	pUserInterface->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pUserInterface->Initialize(pd3dDevice, pd3dCommandList, NULL);
	pUserInterface->SetPlayer(m_pPlayer);

	m_pUserInterface = pUserInterface;

	//
	XMFLOAT2 xmf2Center = ::CalculateCenter(0.63f, 0.93f, 0.93f, 0.4f, true);
	XMFLOAT2 xmf2Size = ::CalculateSize(0.63f, 0.93f, 0.93f, 0.4f, true);

	CMinimapShader *pMinimapShader = new CMinimapShader(pd3dDevice, pd3dCommandList);
	pMinimapShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pMinimapShader->Initialize(pd3dDevice, pd3dCommandList, xmf2Center.x, xmf2Center.y, xmf2Size.x, xmf2Size.y);
	pMinimapShader->SetPlayer(m_pPlayer);

	m_pMinimapShader = pMinimapShader;
	//

	m_pRedScoreText = AddText(L"0", XMFLOAT2(-0.05f, 0.83f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.5f, 0.0f, 0.0f, 0.9f), RIGHT_ALIGN);
	m_pBlueScoreText = AddText(L"0", XMFLOAT2(0.02f, 0.83f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.5f, 0.9f), LEFT_ALIGN);
}

void CBattleScene::CreateEnvironmentMap(ID3D12Device *pd3dDevice)
{
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));

	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = CUBE_MAP_WIDTH;
	d3dResourceDesc.Height = CUBE_MAP_HEIGHT;
	d3dResourceDesc.DepthOrArraySize = 6;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE d3dClear = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f } };

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClear, __uuidof(ID3D12Resource), (void **)&m_pd3dEnvirCube);

	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	d3dClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClear.DepthStencil.Depth = 1.0f;
	d3dClear.DepthStencil.Stencil = 0;

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClear, __uuidof(ID3D12Resource), (void **)&m_pd3dEnvirCubeDSBuffer);

	CreateRtvDsvSrvEnvironmentMap(pd3dDevice);
}

void CBattleScene::CreateRtvDsvSrvEnvironmentMap(ID3D12Device *pd3dDevice)
{
	CScene::CreateRenderTargetView(pd3dDevice, m_pd3dEnvirCube, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 6, m_d3dRrvEnvirCubeMapCPUHandle); 
	CScene::CreateDepthStencilView(pd3dDevice, m_pd3dEnvirCubeDSBuffer, &m_d3dDsvEnvirCubeMapCPUHandle);
	m_d3dSrvEnvirCubeMapGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dEnvirCube, RESOURCE_TEXTURE_CUBE);
}

void CBattleScene::CreateCubeMapCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMFLOAT3 xmf3Looks[6] = {
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f)
	};

	XMFLOAT3 xmf3Ups[6] = {
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f)
	};

	for (int i = 0; i < 6; i++)
	{
		XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Looks[i], xmf3Ups[i], true);

		m_pCubeMapCamera[i] = new CCamera();

		m_pCubeMapCamera[i]->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCubeMapCamera[i]->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCubeMapCamera[i]->SetRight(xmf3Right);
		m_pCubeMapCamera[i]->SetUp(xmf3Ups[i]);
		m_pCubeMapCamera[i]->SetLook(xmf3Looks[i]);
		m_pCubeMapCamera[i]->SetViewPort(0.0f, 0.0f, float(CUBE_MAP_WIDTH), float(CUBE_MAP_HEIGHT));
		m_pCubeMapCamera[i]->SetScissorRect(0, 0, CUBE_MAP_WIDTH, CUBE_MAP_HEIGHT);

		m_pCubeMapCamera[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}
}

void CBattleScene::CreateShadowMap(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));

	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = nWidth;
	d3dResourceDesc.Height = nHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE d3dClear;
	d3dClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClear.DepthStencil.Depth = 1.0f;
	d3dClear.DepthStencil.Stencil = 0;

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClear, __uuidof(ID3D12Resource), (void **)&m_pd3dShadowMap);

	CreateDsvSrvShadowMap(pd3dDevice);
	CreateLightCamera(pd3dDevice, pd3dCommandList, nWidth, nHeight);
}

void CBattleScene::CreateDsvSrvShadowMap(ID3D12Device *pd3dDevice)
{
	CScene::CreateDepthStencilView(pd3dDevice, m_pd3dShadowMap, &m_d3dDsvShadowMapCPUHandle);
	m_d3dSrvShadowMapGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dShadowMap, RESOURCE_TEXTURE2D, true);
}

void CBattleScene::RenderCubeMap(ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pMainObject)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dEnvirCube, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	for (int i = 0; i < 6; i++)
	{
		m_pCubeMapCamera[i]->SetPosition(pMainObject->GetPosition());
		m_pCubeMapCamera[i]->OnPrepareRender(pd3dCommandList);

		pd3dCommandList->ClearRenderTargetView(m_d3dRrvEnvirCubeMapCPUHandle[i], Colors::Black, 0, NULL);
		pd3dCommandList->ClearDepthStencilView(m_d3dDsvEnvirCubeMapCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		pd3dCommandList->OMSetRenderTargets(1, &m_d3dRrvEnvirCubeMapCPUHandle[i], TRUE, &m_d3dDsvEnvirCubeMapCPUHandle);
		
		if (m_pd3dcbLights)
		{
			D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
			pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_LIGHTS, d3dcbLightsGpuVirtualAddress);
		}

		if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, m_pCubeMapCamera[i]);
		if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, m_pCubeMapCamera[i]);

		for (int i = 0; i < m_nShaders; i++)
		{
			if (m_ppShaders[i])
				m_ppShaders[i]->Render(pd3dCommandList, m_pCubeMapCamera[i]);
		}

		for (int i = 0; i < m_nEffectShaders; i++)
		{
			if (m_ppEffectShaders[i])
				m_ppEffectShaders[i]->Render(pd3dCommandList, m_pCubeMapCamera[i]);
		}

		if (m_pParticleShader) m_pParticleShader->Render(pd3dCommandList, m_pCubeMapCamera[i]);
	}

	::TransitionResourceState(pd3dCommandList, m_pd3dEnvirCube, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void CBattleScene::PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	CScene::PrepareRender(pd3dCommandList);

	if (m_nFPS % 5 == 0)
	{
		RenderCubeMap(pd3dCommandList, m_pPlayer);
	}

	RenderShadowMap(pd3dCommandList);
}

void CBattleScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dEnvirCube) pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE, m_d3dSrvEnvirCubeMapGPUHandle);
	if (m_pd3dShadowMap) pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_SHADOW_MAP, m_d3dSrvShadowMapGPUHandle);

	CScene::Render(pd3dCommandList, pCamera);
}

void CBattleScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pMinimapShader) m_pMinimapShader->Render(pd3dCommandList, NULL);

	CScene::RenderUI(pd3dCommandList);
}

void CBattleScene::RenderOffScreen(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dOffScreenTexture) pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, m_d3dSrvOffScreenGPUHandle);
	
	m_pPostProcessingShader->Render(pd3dCommandList, NULL);
}

void CBattleScene::ReleaseUploadBuffers()
{
	CScene::ReleaseUploadBuffers();

	if (m_pGimGun) m_pGimGun->ReleaseUploadBuffers();
	if (m_pBazooka) m_pBazooka->ReleaseUploadBuffers();
	if (m_pMachineGun) m_pMachineGun->ReleaseUploadBuffers();
	if (m_pSaber) m_pSaber->ReleaseUploadBuffers();
	if (m_pMinimapShader) m_pMinimapShader->ReleaseUploadBuffers();
}

void CBattleScene::CheckCollision()
{
	std::vector<CGameObject*> vEnemys;

	for (int i = 0; i < SKINNED_OBJECT_GROUP; i++)
	{
		vEnemys = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(i);

		for (const auto& Enemy : vEnemys)
		{
			CRobotObject *enemy = (CRobotObject*)Enemy;

			if (!(Enemy->GetState() & OBJECT_STATE_SWORDING)) continue;
			CWeapon *pWeapon = ((CRobotObject*)Enemy)->GetWeapon(3);

			for (const auto& anotherE : vEnemys)
			{
				if (Enemy == anotherE) continue;
				if (!pWeapon->CollisionCheck(anotherE)) continue;


				XMFLOAT3 xmf3Pos = pWeapon->GetPosition();
				XMFLOAT3 xmf3EPos = Enemy->GetPosition();
				xmf3Pos.x = (xmf3Pos.x + xmf3EPos.x) * 0.5f;
				xmf3Pos.y = (xmf3Pos.y + xmf3EPos.y) * 0.5f;
				xmf3Pos.z = (xmf3Pos.z + xmf3EPos.z) * 0.5f;

				AddParticle(0, xmf3Pos, rand() % 10 + 10);

				if (enemy->PlayedSaberHitSound()) continue;
				enemy->PlaySaberHitSound();

				switch (rand() % 2)
				{
				case 0:
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit1);
					break;
				case 1:
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit2);
					break;
				}
			}

			if (m_pPlayer)
			{
				if (!pWeapon->CollisionCheck(m_pPlayer)) continue;

				XMFLOAT3 xmf3Pos = pWeapon->GetPosition();
				XMFLOAT3 xmf3EPos = m_pPlayer->GetPosition();
				xmf3Pos.x = (xmf3Pos.x + xmf3EPos.x) * 0.5f;
				xmf3Pos.y = (xmf3Pos.y + xmf3EPos.y) * 0.5f;
				xmf3Pos.z = (xmf3Pos.z + xmf3EPos.z) * 0.5f;

				AddParticle(0, xmf3Pos, rand() % 10 + 10);

				if (enemy->PlayedSaberHitSound()) continue;
				enemy->PlaySaberHitSound();

				switch (rand() % 2)
				{
				case 0:
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit1);
					break;
				case 1:
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit2);
					break;
				}
			}
		}

		if (m_pPlayer)
		{
			if (m_pPlayer->GetState() & OBJECT_STATE_SWORDING)
			{
				for (const auto& Enemy : vEnemys)
				{
					CWeapon *pWeapon = m_pPlayer->GetWeapon(3);
					if (!pWeapon->CollisionCheck(Enemy)) continue;

					XMFLOAT3 xmf3Pos = pWeapon->GetPosition();
					XMFLOAT3 xmf3EPos = Enemy->GetPosition();
					xmf3Pos.x = (xmf3Pos.x + xmf3EPos.x) * 0.5f;
					xmf3Pos.y = (xmf3Pos.y + xmf3EPos.y) * 0.5f;
					xmf3Pos.z = (xmf3Pos.z + xmf3EPos.z) * 0.5f;

					AddParticle(0, xmf3Pos, rand() % 10 + 10);

					if (m_pPlayer->PlayedSaberHitSound()) continue;
					m_pPlayer->PlaySaberHitSound();

					switch (rand() % 2)
					{
					case 0:
						gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit1);
						break;
					case 1:
						gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit2);
						break;
					}
				}
			}
		}
	}

#ifndef ON_NETWORKING
	//std::vector<CGameObject*> vEnemys;
	std::vector<CGameObject*> vBullets;
	std::vector<CGameObject*> vBZKBullets;
	std::vector<CGameObject*> vMGBullets;

	//vEnemys = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(SKINNED_OBJECT_INDEX_GUNDAM);
	vBullets = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_GG_BULLET);
	vBZKBullets = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_BZK_BULLET);
	vMGBullets = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_MG_BULLET);

	for (const auto& Enemy : vEnemys)
	{
		if (m_pPlayer->CollisionCheck(Enemy))
		{
			std::cout << "Collision Player By Enemy\n" << std::endl;
		}

		for (const auto& pBullet : vBullets)
		{
			if (!pBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pBullet))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f), 0);

					float fSize = (float)(rand() % 200) / 100.0f + 10.0f;
					m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS]->AddEffect(SPRITE_EFFECT_INDEX_HIT, pBullet->GetPosition(), XMFLOAT2(fSize, fSize), EFFECT_ANIMATION_TYPE_ONE, SPRITE_EFFECT_INDEX_HIT_TEXTURES);

					pBullet->Delete();
					std::cout << "Collision Enemy By Bullet\n" << std::endl;

					//AddParticle(0, pBullet->GetPosition());
				}
			}
		}

		for (const auto& pBZKBullet : vBZKBullets)
		{
			if (!pBZKBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pBZKBullet))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pBZKBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f), 0);

					float fSize = (float)(rand() % 200) / 100.0f + 10.0f;
					m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS]->AddEffect(SPRITE_EFFECT_INDEX_EXPLOSION, pBZKBullet->GetPosition(), XMFLOAT2(fSize * 2, fSize * 2), EFFECT_ANIMATION_TYPE_ONE, SPRITE_EFFECT_INDEX_EXPLOSION_TEXTURES);

					pBZKBullet->Delete();

					std::cout << "Collision Enemy By Bullet\n" << std::endl;
				}
			}
		}

		for (const auto& pMGBullet : vMGBullets)
		{
			if (!pMGBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pMGBullet))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pMGBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f), 0);

					float fSize = (float)(rand() % 200) / 100.0f + 10.0f;
					m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS]->AddEffect(SPRITE_EFFECT_INDEX_HIT, pMGBullet->GetPosition(), XMFLOAT2(fSize, fSize), EFFECT_ANIMATION_TYPE_ONE, SPRITE_EFFECT_INDEX_HIT_TEXTURES);

					pMGBullet->Delete();

					std::cout << "Collision Enemy By Bullet\n" << std::endl;

					//AddParticle(0, pMGBullet->GetPosition());
				}
			}
		}
	}
#endif
	
	std::vector<CGameObject*> vObstacles;
	CObjectsShader* pObjectsShader = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS]);

	int nGroups = pObjectsShader->GetGroups();
	for (int i = 0; i < nGroups; i++)
	{
		vObstacles = pObjectsShader->GetObjects(i);

		for (const auto& Obstacle : vObstacles)
		{
			// 카메라 이동 O
			if (m_pPlayer->CollisionCheck(Obstacle))
			{
				//std::cout << "Collision Player By Building\n" << std::endl;
				m_pPlayer->MoveToCollision(Obstacle);
			}
		}
	}

	FindAimToTargetDistance();
}

void CBattleScene::CheckCollisionPlayer()
{
	//std::vector<CGameObject*> *vObstacles;

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_HANGAR])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_DOUBLESQUARE])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_OCTAGON])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_OCTAGONLONGTIER])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SLOPETOP])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SQUARE])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STEEPLETOP])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}

	//vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_WALL])->GetObjects();
	//for (const auto& Obstacle : *vObstacles)
	//{
	//	m_pPlayer->MoveToCollision(Obstacle);
	//}
}

void CBattleScene::AddParticle(int nType, XMFLOAT3 xmf3Position, int nNum)
{
	m_pParticleShader->AddParticle(nType, xmf3Position, nNum);
};

void CBattleScene::FindAimToTargetDistance()
{
	std::vector<CGameObject*> vGMs = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(SKINNED_OBJECT_INDEX_GM);
	std::vector<CGameObject*> vGundams = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(SKINNED_OBJECT_INDEX_GUNDAM);

	float fDistance = 1000.0f;
	float fTemp = 0.0f;
	CGameObject *pTarget = NULL;
	XMFLOAT3 xmf3CameraPos = m_pPlayer->GetCamera()->GetPosition();
	XMVECTOR xmvCameraPos = XMLoadFloat3(&xmf3CameraPos);
	XMVECTOR xmvLook = XMLoadFloat3(&(m_pPlayer->GetCamera()->GetLookVector()));

	XMFLOAT3 xmf3PlayerPos = m_pPlayer->GetPosition();

	for (const auto& GM : vGMs)
	{
		// 카메라 이동 X 단 목표가 되지 않음.
		if (GM->CollisionCheck(&xmvCameraPos, &xmvLook, &fTemp))
		{
			float fDistBetweenCnP = Vector3::Length(Vector3::Subtract(xmf3PlayerPos, xmf3CameraPos));

			if (fDistBetweenCnP < fTemp)
			{
				if (fDistance > fTemp)
				{
					fDistance = fTemp;
					pTarget = GM;
				}
			}
		}
	}

	for (const auto& Gundam : vGundams)
	{
		// 카메라 이동 X 단 목표가 되지 않음.
		if (Gundam->CollisionCheck(&xmvCameraPos, &xmvLook, &fTemp))
		{
			float fDistBetweenCnP = Vector3::Length(Vector3::Subtract(xmf3PlayerPos, xmf3CameraPos));

			if (fDistBetweenCnP < fTemp)
			{
				if (fDistance > fTemp)
				{
					fDistance = fTemp;
					pTarget = Gundam;
				}
			}
		}
	}

	if (m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS])
	{
		std::vector<CGameObject*> vObstacles;
		CObjectsShader* pObjectsShader = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS]);

		int nGroups = pObjectsShader->GetGroups();

		for (int i = 0; i < nGroups; i++)
		{
			vObstacles = pObjectsShader->GetObjects(i);

			for (const auto& Obstacle : vObstacles)
			{
				// 카메라 이동 O
				if (Obstacle->CollisionCheck(&xmvCameraPos, &xmvLook, &fTemp))
				{
					float fDistBetweenCnP = Vector3::Length(Vector3::Subtract(xmf3PlayerPos, xmf3CameraPos));

					if (fDistBetweenCnP < fTemp)
					{
						if (fDistance > fTemp)
						{
							fDistance = fTemp;
							pTarget = Obstacle;
						}
					}
				}
			}
		}
	}
	
	if (m_pTerrain)
	{
		if (m_pTerrain->CollisionCheck(&xmvCameraPos, &xmvLook, &fTemp))
		{
			float fDistBetweenCnP = Vector3::Length(Vector3::Subtract(xmf3PlayerPos, xmf3CameraPos));

			if (fDistBetweenCnP < fTemp)
			{
				if (fDistance > fTemp) fDistance = fTemp;
			}
		}
	}

	m_fCameraToTarget = fDistance;
}

//////////////////////////////// for Networking

void CBattleScene::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT *pCreateObjectInfo)
{
	CGameObject* pGameObject = NULL;
	//pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

	if (m_pObjects[pCreateObjectInfo->Object_Index])
	{
		m_pObjects[pCreateObjectInfo->Object_Index]->Delete();
	}

	XMFLOAT3 xmf3Position = XMFLOAT3(pCreateObjectInfo->WorldMatrix._41, pCreateObjectInfo->WorldMatrix._42, pCreateObjectInfo->WorldMatrix._43);

	CObjectsShader *pObjectsShader = NULL;
	CEffectShader *pEffectShader = NULL;

	switch (pCreateObjectInfo->Object_Type)
	{
	case OBJECT_TYPE_PLAYER:
		pGameObject = new CRobotObject();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_SKINND_OBJECTS];

		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, pCreateObjectInfo->Robot_Type, true, NULL);

		if (m_pParticleShader) m_pParticleShader->SetFollowObject(pGameObject, ((CRobotObject*)pGameObject)->GetRightNozzleFrame());
		if (m_pParticleShader) m_pParticleShader->SetFollowObject(pGameObject, ((CRobotObject*)pGameObject)->GetLeftNozzleFrame());
		break;
	case OBJECT_TYPE_OBSTACLE:
		printf("Do not Apply Insert Obstacle\n");
		//((CObjectsShader*)m_ppShaders[INDEX_SHADER_OBSTACLE])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);
		break;
	case OBJECT_TYPE_ITEM_HEALING:
		pGameObject = new RotateObject();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_REPAIR_ITEM, true, NULL);
		break;
	case OBJECT_TYPE_MACHINE_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundMGShot);
		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_MG_BULLET, true, NULL);

		pEffectShader = (CEffectShader*)m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS];
		pEffectShader->AddEffect(TIMED_EFFECT_INDEX_MUZZLE_FIRE, xmf3Position, XMFLOAT2(2.5f, 2.5f), 0, TIMED_EFFECT_INDEX_MUZZLE_FIRE_TEXTURES);
		break;
	case OBJECT_TYPE_BZK_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKShot);
		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_BZK_BULLET, true, NULL);

		pEffectShader = (CEffectShader*)m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS];
		pEffectShader->AddEffect(TIMED_EFFECT_INDEX_MUZZLE_FIRE, xmf3Position, XMFLOAT2(2.5f, 2.5f), 0, TIMED_EFFECT_INDEX_MUZZLE_FIRE_TEXTURES);
		break;
	case OBJECT_TYPE_BEAM_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGShot);
		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_GG_BULLET, true, NULL);

		pEffectShader = (CEffectShader*)m_ppEffectShaders[INDEX_SHADER_TIMED_EEFECTS];
		pEffectShader->AddEffect(TIMED_EFFECT_INDEX_MUZZLE_FIRE, xmf3Position, XMFLOAT2(2.5f, 2.5f), 0, TIMED_EFFECT_INDEX_MUZZLE_FIRE_TEXTURES);
		break;
	case OBJECT_TYPE_ITEM_AMMO:
		pGameObject = new RotateObject();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_AMMO_ITEM, true, NULL);
		break;
	case OBJECT_TYPE_METEOR:
		pGameObject = new Meteor();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_METEOR, true, NULL);
		break;
	}

	m_pObjects[pCreateObjectInfo->Object_Index] = pGameObject;
}

void CBattleScene::DeleteObject(int nIndex)
{
	if (m_pObjects[nIndex])
	{
		if (m_pObjects[nIndex]->GetType() & OBJECT_TYPE_ROBOT)
		{
			std::vector<CParticle*> vpParticles = ((CRobotObject*)m_pObjects[nIndex])->GetParticles();
			for (CParticle *pParticle : vpParticles) pParticle->Delete();
		}
		m_pObjects[nIndex]->Delete();
		m_pObjects[nIndex] = NULL;
	}
}

void CBattleScene::CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT *pCreateEffectInfo)
{
	EFFECT_TYPE nEffectType = pCreateEffectInfo->efType;
	EFFECT_ANIMATION_TYPE nEffectAniType = pCreateEffectInfo->EftAnitType;
	CEffect *pEffect = NULL;
	CSprite *pSprite = NULL;
	float fSize = (float)(rand() % 200) / 100.0f + 10.0f;

	switch (nEffectType)
	{
	case EFFECT_TYPE::EFFECT_TYPE_HIT_FONT:
		m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pCreateEffectInfo->xmf3Position, XMFLOAT2(0.04f, 0.02f), 0);
		break;
	case EFFECT_TYPE::EFFECT_TYPE_HIT:
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);
		m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS]->AddEffect(SPRITE_EFFECT_INDEX_HIT, pCreateEffectInfo->xmf3Position, XMFLOAT2(fSize, fSize), nEffectAniType, SPRITE_EFFECT_INDEX_HIT_TEXTURES);
		break;
	case EFFECT_TYPE::EFFECT_TYPE_EXPLOSION:
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKHit);
		m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS]->AddEffect(SPRITE_EFFECT_INDEX_EXPLOSION, pCreateEffectInfo->xmf3Position, XMFLOAT2(fSize * 2, fSize * 2), nEffectAniType, SPRITE_EFFECT_INDEX_EXPLOSION_TEXTURES);
		break;
	}
}

void CBattleScene::ApplyRecvInfo(PKT_ID pktID, LPVOID pktData)
{
	switch (pktID)
	{
	case PKT_ID_PLAYER_INFO:
		if (!m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID]) break;

		m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID]->SetWorldTransf(((PKT_PLAYER_INFO*)pktData)->WorldMatrix);
		m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID]->SetPrepareRotate(0.0f, 180.0f, 0.0f);

		if (m_pMinimapShader) {
			m_pMinimapShader->UpdateMinimapRobotInfo(m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID], ((PKT_PLAYER_INFO*)pktData)->ID);
		}

		if (((PKT_PLAYER_INFO*)pktData)->isUpChangeAnimation)
		{
			CAnimationObject *pObject = (CAnimationObject*)m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID];
			pObject->ChangeAnimation(ANIMATION_UP, 0, ((PKT_PLAYER_INFO*)pktData)->Player_Up_Animation);
			pObject->SetAnimationTrackPosition(ANIMATION_UP, ((PKT_PLAYER_INFO*)pktData)->UpAnimationPosition);
		}

		if (((PKT_PLAYER_INFO*)pktData)->isDownChangeAnimation)
		{
			CAnimationObject *pObject = (CAnimationObject*)m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID];
			pObject->ChangeAnimation(ANIMATION_DOWN, 0, ((PKT_PLAYER_INFO*)pktData)->Player_Down_Animation);
			pObject->SetAnimationTrackPosition(ANIMATION_DOWN, ((PKT_PLAYER_INFO*)pktData)->DownAnimationPosition);
		}

		if (((PKT_PLAYER_INFO*)pktData)->isChangeWeapon)
		{
			CRobotObject *pObject = (CRobotObject*)m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID];
			pObject->ChangeWeaponByType((WEAPON_TYPE)((PKT_PLAYER_INFO*)pktData)->Player_Weapon);
		}
		m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID]->SetState(((PKT_PLAYER_INFO*)pktData)->State);
		break;
	case PKT_ID_PLAYER_LIFE:
		if (!m_pObjects[((PKT_PLAYER_LIFE*)pktData)->ID]) break;

		m_pObjects[((PKT_PLAYER_LIFE*)pktData)->ID]->SetHitPoint(((PKT_PLAYER_LIFE*)pktData)->HP);
		break;
	case PKT_ID_CREATE_OBJECT:
		break;
	case PKT_ID_DELETE_OBJECT:
		if (!m_pObjects[((PKT_DELETE_OBJECT*)pktData)->Object_Index]) break;

		DeleteObject(((PKT_DELETE_OBJECT*)pktData)->Object_Index);
		break;
	case PKT_ID_TIME_INFO:
		break;
	case PKT_ID_UPDATE_OBJECT:
		if (!m_pObjects[((PKT_UPDATE_OBJECT*)pktData)->Object_Index]) break;

		XMFLOAT3 position = ((PKT_UPDATE_OBJECT*)pktData)->Object_Position;
		m_pObjects[((PKT_UPDATE_OBJECT*)pktData)->Object_Index]->SetPosition(position);
		break;
	case PKT_ID_CREATE_EFFECT:
		break;
	case PKT_ID_SCORE:
	{
		PKT_SCORE *pktScore = (PKT_SCORE*)pktData;
		wchar_t pstrText[16];
		wsprintfW(pstrText, L"%d", pktScore->RedScore);
		ChangeText(m_pRedScoreText, pstrText, XMFLOAT2(-0.05f, 0.83f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.5f, 0.0f, 0.0f, 0.9f), RIGHT_ALIGN);
		wsprintfW(pstrText, L"%d", pktScore->BlueScore);
		ChangeText(m_pBlueScoreText, pstrText, XMFLOAT2(0.02f, 0.83f), XMFLOAT2(2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.5f, 0.9f), LEFT_ALIGN);
		break;
	}
	}
}

void CBattleScene::LeavePlayer(int nServerIndex)
{
	if (!m_pObjects[nServerIndex]) return;

	DeleteObject(nServerIndex);
}


//////////////////////////////////////////////////////////////////////////////////////////////////

CColonyScene::CColonyScene() : CBattleScene()
{
	m_fGravAcc = -9.8f;
}

CColonyScene::~CColonyScene()
{
}

void CColonyScene::BuildObstacleObjetcs(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CObstacleShader *pInstancingObstacleShader = new CObstacleShader();
	pInstancingObstacleShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pInstancingObstacleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS] = pInstancingObstacleShader;
}

void CColonyScene::BuildTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMFLOAT3 xmf3Scale(4.0f, 1.0f, 4.0f);
	XMFLOAT4 xmf4Color(1.f, 1.f, 1.f, 1.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("./Resource/Stage/HeightMap.raw"), 514, 514, 514, 514, xmf3Scale, xmf4Color);
}

void CColonyScene::BuildSkybox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}

void CColonyScene::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = false;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 100.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(130.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_bEnable = false;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 50.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
}

void CColonyScene::CreateLightCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	XMFLOAT3 xmf3Look = XMFLOAT3(1.0f, -1.0f, 0.0f);

	XMFLOAT3 xmf3Right = XMFLOAT3(0.0f, 0.0f, -1.0f);

	XMFLOAT3 xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);

	m_pLightCamera = new CLightCamera();

	m_pLightCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_pLightCamera->GenerateOrthogonalMatrix(nWidth / 4, nHeight / 4, 0.0f, 5000.0f);
	m_pLightCamera->SetRight(xmf3Right);
	m_pLightCamera->SetUp(xmf3Up);
	m_pLightCamera->SetLook(xmf3Look);
	m_pLightCamera->SetPosition(XMFLOAT3(-1000.0f, 1300.0f, 0.0f));
	m_pLightCamera->SetViewPort(0.0f, 0.0f, float(nWidth), float(nHeight));
	m_pLightCamera->SetScissorRect(0, 0, nWidth, nHeight);

	m_pLightCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CColonyScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255);
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CColonyScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));

	XMFLOAT4 xmf4Random = XMFLOAT4(dist1(mt) * 0.7f, dist1(mt) * 0.6f, dist1(mt) * 0.5f, dist1(mt) * 0.4f);
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_INDEX_SCENE_INFO, 4, &xmf4Random, 0);
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_INDEX_SCENE_INFO, 1, &m_fGravAcc, 4);
}

void CColonyScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
		m_pd3dcbLights = NULL;
	}
}

void CColonyScene::RenderShadowMap(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dShadowMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	pd3dCommandList->ClearDepthStencilView(m_d3dDsvShadowMapCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	pd3dCommandList->OMSetRenderTargets(0, NULL, FALSE, &m_d3dDsvShadowMapCPUHandle);

	if (m_pLightCamera)
	{
		m_pLightCamera->OnPrepareRender(pd3dCommandList);
	}

	if (m_pTerrain)
		m_pTerrain->RenderToShadow(pd3dCommandList, m_pLightCamera);

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->RenderToShadow(pd3dCommandList, m_pLightCamera);
	}

	if(m_pPlayer)
		m_pPlayer->RenderToShadow(pd3dCommandList, m_pLightCamera);
	
	::TransitionResourceState(pd3dCommandList, m_pd3dShadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void CColonyScene::StartScene()
{
	gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundBGM, &(gFmodSound.m_pBGMChannel));
}

void CColonyScene::EndScene()
{
	gFmodSound.PauseFMODSound(gFmodSound.m_pBGMChannel);
}

void CColonyScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	CBattleScene::RenderUI(pd3dCommandList);

	//RenderTestTexture(pd3dCommandList, m_d3dSrvShadowMapGPUHandle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

CSpaceScene::CSpaceScene() : CBattleScene()
{
	m_fGravAcc = 0.0f;
}

CSpaceScene::~CSpaceScene()
{
}

void CSpaceScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CBattleScene::OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case 'V':
			m_pPlayer->VUp();
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'V':
			m_pPlayer->ActivationBooster();
			m_pPlayer->VDown();
			break;
		}
		break;
	default:
		break;
	}
}

void CSpaceScene::BuildObstacleObjetcs(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CSpaceObstacleShader *pInstancingObstacleShader = new CSpaceObstacleShader();
	pInstancingObstacleShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pInstancingObstacleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS] = pInstancingObstacleShader;
}

void CSpaceScene::BuildSkybox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}

void CSpaceScene::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = false;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 100.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(130.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_bEnable = false;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 50.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
}

void CSpaceScene::CreateLightCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight)
{
	XMFLOAT3 xmf3Look = XMFLOAT3(1.0f, -1.0f, 0.0f);

	XMFLOAT3 xmf3Right = XMFLOAT3(0.0f, 0.0f, -1.0f);

	XMFLOAT3 xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);

	m_pLightCamera = new CLightCamera();

	m_pLightCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_pLightCamera->GenerateOrthogonalMatrix(nWidth*0.5, nHeight*0.5, 0.0f, 5000.0f);
	m_pLightCamera->SetRight(xmf3Right);
	m_pLightCamera->SetUp(xmf3Up);
	m_pLightCamera->SetLook(xmf3Look);
	m_pLightCamera->SetPosition(XMFLOAT3(-1000.0f, 1000.0f, 200.0f));
	m_pLightCamera->SetViewPort(0.0f, 0.0f, float(nWidth), float(nHeight));
	m_pLightCamera->SetScissorRect(0, 0, nWidth, nHeight);

	m_pLightCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CSpaceScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255);
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CSpaceScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));

	XMFLOAT4 xmf4Random = XMFLOAT4(dist1(mt) * 0.7f, dist1(mt) * 0.6f, dist1(mt) * 0.5f, dist1(mt) * 0.4f);
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_INDEX_SCENE_INFO, 4, &xmf4Random, 0);
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_INDEX_SCENE_INFO, 1, &m_fGravAcc, 4);
}

void CSpaceScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
		m_pd3dcbLights = NULL;
	}
}

void CSpaceScene::RenderShadowMap(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dShadowMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	pd3dCommandList->ClearDepthStencilView(m_d3dDsvShadowMapCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	pd3dCommandList->OMSetRenderTargets(0, NULL, FALSE, &m_d3dDsvShadowMapCPUHandle);

	if (m_pLightCamera)
	{
		m_pLightCamera->OnPrepareRender(pd3dCommandList);
	}

	if (m_pTerrain)
		m_pTerrain->RenderToShadow(pd3dCommandList, m_pLightCamera);

	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->RenderToShadow(pd3dCommandList, m_pLightCamera);
	}

	if (m_pPlayer)
		m_pPlayer->RenderToShadow(pd3dCommandList, m_pLightCamera);

	::TransitionResourceState(pd3dCommandList, m_pd3dShadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void CSpaceScene::StartScene()
{
	gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundBGM, &(gFmodSound.m_pBGMChannel));
}

void CSpaceScene::EndScene()
{
	gFmodSound.PauseFMODSound(gFmodSound.m_pBGMChannel);
}

void CSpaceScene::RenderUI(ID3D12GraphicsCommandList *pd3dCommandList)
{
	CBattleScene::RenderUI(pd3dCommandList);

	//RenderTestTexture(pd3dCommandList, m_d3dSrvShadowMapGPUHandle);
}

void CSpaceScene::CheckCollision()
{
	std::vector<CGameObject*> vEnemys;

	for (int i = 0; i < SKINNED_OBJECT_GROUP; i++)
	{
		vEnemys = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(i);

		for (const auto& Enemy : vEnemys)
		{
			CRobotObject *enemy = (CRobotObject*)Enemy;

			if (!(Enemy->GetState() & OBJECT_STATE_SWORDING)) continue;
			CWeapon *pWeapon = ((CRobotObject*)Enemy)->GetWeapon(3);

			for (const auto& anotherE : vEnemys)
			{
				if (Enemy == anotherE) continue;
				if (!pWeapon->CollisionCheck(anotherE)) continue;


				XMFLOAT3 xmf3Pos = pWeapon->GetPosition();
				XMFLOAT3 xmf3EPos = Enemy->GetPosition();
				xmf3Pos.x = (xmf3Pos.x + xmf3EPos.x) * 0.5f;
				xmf3Pos.y = (xmf3Pos.y + xmf3EPos.y) * 0.5f;
				xmf3Pos.z = (xmf3Pos.z + xmf3EPos.z) * 0.5f;

				AddParticle(0, xmf3Pos, rand() % 10 + 10);

				if (enemy->PlayedSaberHitSound()) continue;
				enemy->PlaySaberHitSound();

				switch (rand() % 2)
				{
				case 0:
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit1);
					break;
				case 1:
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit2);
					break;
				}
			}

			if (m_pPlayer)
			{
				if (!pWeapon->CollisionCheck(m_pPlayer)) continue;

				XMFLOAT3 xmf3Pos = pWeapon->GetPosition();
				XMFLOAT3 xmf3EPos = m_pPlayer->GetPosition();
				xmf3Pos.x = (xmf3Pos.x + xmf3EPos.x) * 0.5f;
				xmf3Pos.y = (xmf3Pos.y + xmf3EPos.y) * 0.5f;
				xmf3Pos.z = (xmf3Pos.z + xmf3EPos.z) * 0.5f;

				AddParticle(0, xmf3Pos, rand() % 10 + 10);

				if (enemy->PlayedSaberHitSound()) continue;
				enemy->PlaySaberHitSound();

				switch (rand() % 2)
				{
				case 0:
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit1);
					break;
				case 1:
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit2);
					break;
				}
			}
		}

		if (m_pPlayer)
		{
			if (m_pPlayer->GetState() & OBJECT_STATE_SWORDING)
			{
				for (const auto& Enemy : vEnemys)
				{
					CWeapon *pWeapon = m_pPlayer->GetWeapon(3);
					if (!pWeapon->CollisionCheck(Enemy)) continue;

					XMFLOAT3 xmf3Pos = pWeapon->GetPosition();
					XMFLOAT3 xmf3EPos = Enemy->GetPosition();
					xmf3Pos.x = (xmf3Pos.x + xmf3EPos.x) * 0.5f;
					xmf3Pos.y = (xmf3Pos.y + xmf3EPos.y) * 0.5f;
					xmf3Pos.z = (xmf3Pos.z + xmf3EPos.z) * 0.5f;

					AddParticle(0, xmf3Pos, rand() % 10 + 10);

					if (m_pPlayer->PlayedSaberHitSound()) continue;
					m_pPlayer->PlaySaberHitSound();

					switch (rand() % 2)
					{
					case 0:
						gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit1);
						break;
					case 1:
						gFmodSound.PlayFMODSound(gFmodSound.m_pSoundSaberHit2);
						break;
					}
				}
			}
		}
	}

#ifndef ON_NETWORKING
	//std::vector<CGameObject*> vEnemys;
	std::vector<CGameObject*> vBullets;
	std::vector<CGameObject*> vBZKBullets;
	std::vector<CGameObject*> vMGBullets;

	//vEnemys = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(SKINNED_OBJECT_INDEX_GUNDAM);
	vBullets = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_GG_BULLET);
	vBZKBullets = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_BZK_BULLET);
	vMGBullets = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS])->GetObjects(STANDARD_OBJECT_INDEX_MG_BULLET);

	for (const auto& Enemy : vEnemys)
	{
		if (m_pPlayer->CollisionCheck(Enemy))
		{
			std::cout << "Collision Player By Enemy\n" << std::endl;
		}

		for (const auto& pBullet : vBullets)
		{
			if (!pBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pBullet))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f), 0);

					float fSize = (float)(rand() % 200) / 100.0f + 10.0f;
					m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS]->AddEffect(SPRITE_EFFECT_INDEX_HIT, pBullet->GetPosition(), XMFLOAT2(fSize, fSize), EFFECT_ANIMATION_TYPE_ONE, SPRITE_EFFECT_INDEX_HIT_TEXTURES);

					pBullet->Delete();
					std::cout << "Collision Enemy By Bullet\n" << std::endl;

					//AddParticle(0, pBullet->GetPosition());
				}
			}
		}

		for (const auto& pBZKBullet : vBZKBullets)
		{
			if (!pBZKBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pBZKBullet))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pBZKBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f), 0);

					float fSize = (float)(rand() % 200) / 100.0f + 10.0f;
					m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS]->AddEffect(SPRITE_EFFECT_INDEX_EXPLOSION, pBZKBullet->GetPosition(), XMFLOAT2(fSize * 2, fSize * 2), EFFECT_ANIMATION_TYPE_ONE, SPRITE_EFFECT_INDEX_EXPLOSION_TEXTURES);

					pBZKBullet->Delete();

					std::cout << "Collision Enemy By Bullet\n" << std::endl;
				}
			}
		}

		for (const auto& pMGBullet : vMGBullets)
		{
			if (!pMGBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pMGBullet))
				{
					gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);

					m_ppEffectShaders[INDEX_SHADER_TEXT_EEFECTS]->AddEffect(TEXT_EFFECT_INDEX_HIT_TEXT, pMGBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f), 0);

					float fSize = (float)(rand() % 200) / 100.0f + 10.0f;
					m_ppEffectShaders[INDEX_SHADER_SPRITE_EFFECTS]->AddEffect(SPRITE_EFFECT_INDEX_HIT, pMGBullet->GetPosition(), XMFLOAT2(fSize, fSize), EFFECT_ANIMATION_TYPE_ONE, SPRITE_EFFECT_INDEX_HIT_TEXTURES);

					pMGBullet->Delete();

					std::cout << "Collision Enemy By Bullet\n" << std::endl;

					//AddParticle(0, pMGBullet->GetPosition());
				}
			}
		}
	}
#endif

	std::vector<CGameObject*> vObstacles;
	CObjectsShader* pObjectsShader = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS]);

	int nGroups = pObjectsShader->GetGroups();
	for (int i = 0; i < nGroups; i++)
	{
		vObstacles = pObjectsShader->GetObjects(i);

		for (const auto& Obstacle : vObstacles)
		{
			// 카메라 이동 O
			if (m_pPlayer->CollisionCheck(Obstacle))
			{
				m_pPlayer->MoveToCollisionByRadius(Obstacle);
			}
		}
	}

	FindAimToTargetDistance();
}
