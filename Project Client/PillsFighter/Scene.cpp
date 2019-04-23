#include "stdafx.h"
#include "Scene.h"
#include "Repository.h"

ID3D12DescriptorHeap			*CScene::m_pd3dDescriptorHeap = NULL;
ID3D12RootSignature				*CScene::m_pd3dGraphicsRootSignature = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE		CScene::m_d3dSrvGPUDescriptorStartHandle;

ID3D12DescriptorHeap			*CScene::m_pd3dRtvDescriptorHeap;
D3D12_CPU_DESCRIPTOR_HANDLE		CScene::m_d3dRtvCPUDesciptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE		CScene::m_d3dRtvGPUDesciptorStartHandle;

ID3D12DescriptorHeap			*CScene::m_pd3dDsvDescriptorHeap;
D3D12_CPU_DESCRIPTOR_HANDLE		CScene::m_d3dDsvCPUDesciptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE		CScene::m_d3dDsvGPUDesciptorStartHandle;

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
}

void CScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
}

void CScene::MoveCursor(float x, float y)
{
	if(m_pCursor) m_pCursor->MoveCursorPos(x, y);
}

void CScene::AddFont(ID3D12Device *pd3dDevice, CFont *pFont)
{
	pFont->SetSrv(pd3dDevice);
	m_vpFonts.emplace_back(pFont);
}

CTextObject* CScene::AddText(const char *pstrFont, const char *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color)
{
	CFont *pFont = NULL;

	for (const auto& Font : m_vpFonts)
	{
		if (!strcmp(Font->GetName(), pstrFont))
		{
			pFont = Font;
			break;
		}
	}

	return (pFont->SetText(pstrText, xmf2Position, xmf2Scale, xmf2Padding, xmf4Color));
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

	if (m_pCursor)
	{
		delete m_pCursor;
	}

	for (const auto& pFont : m_vpFonts)
	{
		pFont->ClearTexts();
	}

	ReleaseShaderVariables();
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

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void CScene::ProcessInput(UCHAR *pKeysBuffer, float fElapsedTime)
{
}

void CScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
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
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (pCamera)
	{
		pCamera->GenerateViewMatrix();
		pCamera->UpdateShaderVariables(pd3dCommandList);
	}

	UpdateShaderVariables(pd3dCommandList);

	if (m_pd3dcbLights)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_LIGHTS, d3dcbLightsGpuVirtualAddress);
	}
	
	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera, true);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera, true);

	for (int i = 0; i < m_nShaders; i++)
	{
		if(m_ppShaders[i])
			m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	}

	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->Render(pd3dCommandList, pCamera);
	}

	if (m_pParticleShader) m_pParticleShader->Render(pd3dCommandList, pCamera);

	if (m_pUserInterface) m_pUserInterface->Render(pd3dCommandList, pCamera);

	if (m_pFontShader) m_pFontShader->Render(pd3dCommandList, NULL);

	for (const auto& pFont : m_vpFonts)
	{
		pFont->Render(pd3dCommandList);
	}

	m_nFPS = (m_nFPS + 1) % 5;
}

void CScene::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_bRenderWire)
	{
		if (m_pWireShader) m_pWireShader->OnPrepareRender(pd3dCommandList);

		for (int i = 0; i < m_nShaders; i++)
		{
			if (m_ppShaders[i])
				m_ppShaders[i]->RenderWire(pd3dCommandList, pCamera);
		}
	}
}

void CScene::PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->PrepareRender(pd3dCommandList);
	}

	if (m_pParticleShader) m_pParticleShader->PrepareRender(pd3dCommandList);
}

void CScene::AfterRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (int i = 0; i < m_nEffectShaders; i++)
	{
		if (m_ppEffectShaders[i])
			m_ppEffectShaders[i]->AfterRender(pd3dCommandList);
	}

	if (m_pParticleShader) m_pParticleShader->AfterRender(pd3dCommandList);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
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
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[6];

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

	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3]; // t6: Texture Cube
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_CUBE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].Descriptor.ShaderRegister = 3; //HP
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

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

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

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

void CScene::CreateDescriptorHeaps(ID3D12Device *pd3dDevice, int nViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nViews;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDescriptorHeap);

	m_d3dSrvCPUDescriptorStartHandle = m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dSrvGPUDescriptorStartHandle = m_pd3dDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();

		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource *pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorStartHandle);
			m_d3dSrvCPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

			pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameterStartIndex + i) : nRootParameterStartIndex, m_d3dSrvGPUDescriptorStartHandle);
			m_d3dSrvGPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}

	return d3dSrvGPUDescriptorHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, UINT nSrvType)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;

	if (pd3dResource)
	{
		D3D12_RESOURCE_DESC d3dResourceDesc = pd3dResource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nSrvType);

		pd3dDevice->CreateShaderResourceView(pd3dResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorStartHandle);

		m_d3dSrvCPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		m_d3dSrvGPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}

	return d3dSrvGPUDescriptorHandle;
}

void CScene::ReleaseDescHeapsAndGraphicsRootSign()
{
	if (m_pd3dGraphicsRootSignature)
	{
		m_pd3dGraphicsRootSignature->Release();
		m_pd3dGraphicsRootSignature = NULL;
	}

	if (m_pd3dDescriptorHeap)
	{
		m_pd3dDescriptorHeap->Release();
		m_pd3dDescriptorHeap = NULL;
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
	if (m_pd3dDescriptorHeap)			pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dDescriptorHeap);
}

void CScene::CreateRtvAndDsvDescriptorHeaps(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = 6 + 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	m_d3dRtvCPUDesciptorStartHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dRtvGPUDesciptorStartHandle = m_pd3dRtvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	d3dDescriptorHeapDesc.NumDescriptors = 2;
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
	m_d3dSrvCPUDescriptorStartHandle = m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dSrvGPUDescriptorStartHandle = m_pd3dDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

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

void CLobbyScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);

	m_pLobbyShader = new CLobbyShader();
	m_pLobbyShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pLobbyShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);

	m_nTextures = 13;
	m_ppTextures = new CTexture*[m_nTextures];

	m_ppTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Base.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[0], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/GameStart.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[1], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[2]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/HLGameStart.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[2], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[11] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[11]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Ready.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[11], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[12] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[12]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/HLReady.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[12], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[3] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[3]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Left.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[3], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[4] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[4]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/HLLeft.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[4], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[5] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[5]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Right.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[5], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[6] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[6]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/HLRight.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[6], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);


	m_ppTextures[7] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[7]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Gim.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[7], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[8] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[8]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/HLGim.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[8], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[9] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[9]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Gundam.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[9], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[10] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[10]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/HLGundam.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[10], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);


	m_nUIRect = 6;
	m_ppUIRects = new CRect*[m_nUIRect];

	// Base UI
	XMFLOAT2 xmf2Center = CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
	XMFLOAT2 xmf2Size = CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
	m_ppUIRects[0] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = ::CalculateCenter(0.514688f, 0.902500f, -0.672778f, -0.816111f);
	xmf2Size = ::CalculateSize(0.514688f, 0.902500f, -0.672778f, -0.816111f);
	m_ppUIRects[1] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	// Left
	xmf2Center = ::CalculateCenter(0.514688f, 0.701564f, -0.430556f, -0.555556f);
	xmf2Size = ::CalculateSize(0.514688f, 0.701564f, -0.430556f, -0.555556f);
	m_ppUIRects[2] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	// Right
	xmf2Center = ::CalculateCenter(0.718750f, 0.902500f, -0.430556f, -0.555556f);
	xmf2Size = ::CalculateSize(0.718750f, 0.902500f, -0.430556f, -0.555556f);
	m_ppUIRects[3] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	// First : Gim
	xmf2Center = ::CalculateCenter(0.514688f, 0.632813f, -0.188889f, -0.322222f);
	xmf2Size = ::CalculateSize(0.514688f, 0.632813f, -0.188889f, -0.322222f);
	m_ppUIRects[4] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	// Second : Gundam
	xmf2Center = ::CalculateCenter(0.646875f, 0.770313f, -0.188889f, -0.322222f);
	xmf2Size = ::CalculateSize(0.646875f, 0.770313f, -0.188889f, -0.322222f);
	m_ppUIRects[5] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	// Third : ???
	//xmf2Center = ::CalculateCenter(0.784375f, 0.902500f, -0.188889f, -0.322222f);
	//xmf2Size = ::CalculateSize(0.784375f, 0.902500f, -0.188889f, -0.322222f);

	m_pCursor = new CCursor(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}

void CLobbyScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONUP:
		//MouseClick();
		break;
	default:
		break;
	}
}

void CLobbyScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	//JoinPlayer("First Player");
	//JoinPlayer("Second Player");
	//JoinPlayer("Third Player");
	//JoinPlayer("Fourth Player");
	//JoinPlayer("Fifth Player");
	//JoinPlayer("Sixth Player");
	//JoinPlayer("Seventh Player");
	//JoinPlayer("Ehighth Player");

	XMFLOAT2 xmf2Center = ::CalculateCenter(0.514688f, 0.902500f, -0.672778f, -0.816111f, true);
	XMFLOAT2 xmf2Size = ::CalculateSize(0.514688f, 0.902500f, -0.672778f, -0.816111f, true);

	m_StartButton.Center = XMFLOAT3(xmf2Center.x, xmf2Center.y, 1.0f);
	m_StartButton.Extents = XMFLOAT3(xmf2Size.x, xmf2Size.y, 1.0f);

	xmf2Center = ::CalculateCenter(0.514688f, 0.701564f, -0.430556f, -0.555556f, true);
	xmf2Size = ::CalculateSize(0.514688f, 0.701564f, -0.430556f, -0.555556f, true);

	m_SelectLeft.Center = XMFLOAT3(xmf2Center.x, xmf2Center.y, 1.0f);
	m_SelectLeft.Extents = XMFLOAT3(xmf2Size.x, xmf2Size.y, 1.0f);

	xmf2Center = ::CalculateCenter(0.718750f, 0.902500f, -0.430556f, -0.555556f, true);
	xmf2Size = ::CalculateSize(0.718750f, 0.902500f, -0.430556f, -0.555556f, true);

	m_SelectRight.Center = XMFLOAT3(xmf2Center.x, xmf2Center.y, 1.0f);
	m_SelectRight.Extents = XMFLOAT3(xmf2Size.x, xmf2Size.y, 1.0f);
}

void CLobbyScene::SetPlayerIndex(int nIndex)
{
	char id[32];
	sprintf(id, "%d", nIndex);

	m_nPlayerIndex = nIndex;
	JoinPlayer(nIndex, id);
}

void CLobbyScene::ReleaseObjects()
{
	CScene::ReleaseObjects();
	
	for (int i = 0; i < m_nUIRect; i++)
	{
		if (m_ppUIRects[i])
			delete m_ppUIRects[i];
	}

	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i])
			delete m_ppTextures[i];
	}

	if (m_pLobbyShader) delete m_pLobbyShader;
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

int CLobbyScene::MouseClick()
{
	if (m_pCursor->CollisionCheck(m_StartButton))
	{
		if (m_nPlayerIndex == 0)
		{
			return MOUSE_CLICK_TYPE_START;
		}
		else
		{
			// Process Ready
		}
	}
	if (m_pCursor->CollisionCheck(m_SelectLeft))
	{
		m_nChoiceCharactor = (m_nChoiceCharactor - 1) > 0 ? m_nChoiceCharactor - 1 : 0;
	}
	if (m_pCursor->CollisionCheck(m_SelectRight))
	{
		m_nChoiceCharactor = (m_nChoiceCharactor + 1) < MAX_CHARACTERS ? m_nChoiceCharactor + 1 : MAX_CHARACTERS - 1;
	}

	return 0;
}

void CLobbyScene::CheckCollision()
{
	if (m_pCursor->CollisionCheck(m_StartButton)) m_bHLStartButton = true;
	else m_bHLStartButton = false;

	if (m_pCursor->CollisionCheck(m_SelectLeft)) m_bHLSelectLeft = true;
	else m_bHLSelectLeft = false;

	if (m_pCursor->CollisionCheck(m_SelectRight)) m_bHLSelectRight = true;
	else m_bHLSelectRight = false;
}

void CLobbyScene::StartScene()
{
	//gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundBGM, &(gFmodSound.m_pBGMChannel));
}

void CLobbyScene::JoinPlayer(int nIndex, const char *pstrPlayerName)
{
	XMFLOAT2 xmf2Pos;
	xmf2Pos.x = -0.862500f;
	xmf2Pos.y = 0.400000f;

	xmf2Pos.x += (nIndex / 4) * 0.7f;
	xmf2Pos.y += (nIndex % 4) * -0.375f;

	XMFLOAT4 xmf4Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	if (nIndex == m_nPlayerIndex)
	{
		xmf4Color.x = 1.0f;
		xmf4Color.z = 0.0f;
	}

	AddText("휴먼매직체", pstrPlayerName, xmf2Pos, XMFLOAT2(1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), xmf4Color);
}

void CLobbyScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pLobbyShader) m_pLobbyShader->Render(pd3dCommandList, pCamera);

	m_ppTextures[0]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[0]->Render(pd3dCommandList, 0);


	if (m_bHLStartButton)
	{
		if(m_nPlayerIndex == 0)
			m_ppTextures[2]->UpdateShaderVariables(pd3dCommandList);
		else
			m_ppTextures[12]->UpdateShaderVariables(pd3dCommandList);
	}
	else
	{
		if (m_nPlayerIndex == 0)
			m_ppTextures[1]->UpdateShaderVariables(pd3dCommandList);
		else
			m_ppTextures[11]->UpdateShaderVariables(pd3dCommandList);
	}
	m_ppUIRects[1]->Render(pd3dCommandList, 0);

	if (m_bHLSelectLeft)
		m_ppTextures[4]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[3]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[2]->Render(pd3dCommandList, 0);

	if (m_bHLSelectRight)
		m_ppTextures[6]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[5]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[3]->Render(pd3dCommandList, 0);

	if (m_nChoiceCharactor == 0)
		m_ppTextures[8]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[7]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[4]->Render(pd3dCommandList, 0);

	if (m_nChoiceCharactor == 1)
		m_ppTextures[10]->UpdateShaderVariables(pd3dCommandList);
	else
		m_ppTextures[9]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[5]->Render(pd3dCommandList, 0);

	CScene::Render(pd3dCommandList, pCamera);

	if (m_pCursor) m_pCursor->Render(pd3dCommandList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CColonyScene::CColonyScene() : CScene()
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
		m_pObjects[i] = NULL;

}

CColonyScene::~CColonyScene()
{
}

void CColonyScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		if (::GetCapture() == hWnd)
		{
			if (!m_LButtonDown) m_pPlayer->PrepareAttack(m_pPlayer->GetRHWeapon());

			m_LButtonDown = TRUE;
		}
		break;
	case WM_LBUTTONUP:
	{
		m_LButtonDown = FALSE;
		break;
	}
	default:
		break;
	}
}

void CColonyScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case VK_SPACE:
		m_pPlayer->DeactivationBooster();
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
		case 'B':
			m_bRenderWire = !m_bRenderWire;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CColonyScene::ProcessInput(UCHAR *pKeysBuffer, float fElapsedTime)
{
	ULONG dwDirection = 0;

	if (pKeysBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
	if (pKeysBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
	if (pKeysBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
	if (pKeysBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
	if (pKeysBuffer[VK_SPACE] & 0xF0) dwDirection |= DIR_UP;
	if (pKeysBuffer['R'] & 0xF0) m_pPlayer->Reload(m_pPlayer->GetRHWeapon());

	if (dwDirection) m_pPlayer->Move(dwDirection, m_pPlayer->GetMovingSpeed() * fElapsedTime);
	else m_pPlayer->ChangeAnimation(ANIMATION_STATE_IDLE);

	if (m_LButtonDown)
	{
		if (m_pPlayer) m_pPlayer->Attack(m_pPlayer->GetRHWeapon());
	}
}

void CColonyScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);
	
	m_nShaders = SHADER_INDEX;
	m_ppShaders = new CShader*[m_nShaders];
	ZeroMemory(m_ppShaders, sizeof(CShader*) * m_nShaders);

	/* 그룹 1 [ Standard Shader ] */
	CStandardObjectsShader *pStandardObjectsShader = new CStandardObjectsShader();
	pStandardObjectsShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pStandardObjectsShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS] = pStandardObjectsShader;

	/* 그룹 2 [ Instancing Shader ] */
	CObstacleShader *pInstancingObstacleShader = new CObstacleShader();
	pInstancingObstacleShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pInstancingObstacleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_INSTANCING_OBJECTS] = pInstancingObstacleShader;

	/* 그룹 3 [ Skinned Animation Shader ] */
	CRobotObjectsShader *pSkinnedRobotShader = new CRobotObjectsShader();
	pSkinnedRobotShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pSkinnedRobotShader->Initialize(pd3dDevice, pd3dCommandList, pRepository, m_pd3dGraphicsRootSignature);
	m_ppShaders[INDEX_SHADER_SKINND_OBJECTS] = pSkinnedRobotShader;

	m_nEffectShaders = EFFECT_SHADER_INDEX;
	m_ppEffectShaders = new CEffectShader*[m_nEffectShaders];

	// 그룹 4 [ Fade Out Shader ]
	CFadeOutShader *pFadeOutShader = new CFadeOutShader();
	pFadeOutShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pFadeOutShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_EFFECT_SHADER_EFFECT] = pFadeOutShader;

	// 그룹 5 [ Sprite Shader ]
	CHitSpriteShader *pHitSpriteShader = new CHitSpriteShader();
	pHitSpriteShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pHitSpriteShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_EFFECT_SHADER_HIT_SPRITE] = pHitSpriteShader;

	CExpSpriteShader *pExpSpriteShader = new CExpSpriteShader();
	pExpSpriteShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pExpSpriteShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppEffectShaders[INDEX_EFFECT_SHADER_EXP_SPRITE] = pExpSpriteShader;

	// 그룹 6 [ Wire Shader ]
	m_pWireShader = new CWireShader();
	m_pWireShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(4.0f, 1.0f, 4.0f);
	XMFLOAT4 xmf4Color(1.f, 1.f, 1.f, 1.0f);

	// 그룹 7 [ Terrain Shader ]
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("./Resource/Stage/HeightMap.raw"), 514, 514, 514, 514, xmf3Scale, xmf4Color);

	// 그룹 8 [ SkyBox Shader ]
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	// 그룹 9 [ Particle Shader ]
	m_pParticleShader = new CParticleShader();
	m_pParticleShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_pParticleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);

	m_pGimGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/GIM_GUN.bin", false);
	m_pBazooka = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/BZK.bin", false); 
	m_pMachineGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/MACHINEGUN.bin", false); 
}

void CColonyScene::ReleaseObjects()
{
	CScene::ReleaseObjects();

	if (m_pd3dEnvirCube) m_pd3dEnvirCube->Release();
	if (m_pd3dEnvirCubeDSBuffer) m_pd3dEnvirCubeDSBuffer->Release();

	for (int i = 0; i < 6; i++)
	{
		m_pCubeMapCamera[i]->ReleaseShaderVariables();
		delete m_pCubeMapCamera[i];
	}

	if (screenCaptureTexture) delete screenCaptureTexture;

	if (m_pMiniMapCamera) {
		m_pMiniMapCamera->ReleaseShaderVariables();
		delete m_pMiniMapCamera;
	}

	if (m_pd3dMinimapDepthStencilBuffer) m_pd3dMinimapDepthStencilBuffer->Release();
}

void CColonyScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CScene::SetAfterBuildObject(pd3dDevice, pd3dCommandList, pContext);

	CreateEnvironmentMap(pd3dDevice);
	CreateCubeMapCamera(pd3dDevice, pd3dCommandList);

	CreateMinimapMap(pd3dDevice);
	CreateMiniMapCamera(pd3dDevice, pd3dCommandList);

	if (m_pParticleShader) m_pParticleShader->SetFollowObject(m_pPlayer, m_pPlayer->GetRightNozzleFrame());
	if (m_pParticleShader) m_pParticleShader->SetFollowObject(m_pPlayer, m_pPlayer->GetLeftNozzleFrame());

	if (m_pPlayer)
	{
		m_pPlayer->SetGravity(m_fGravAcc);
		m_pPlayer->SetScene(this);
	}

	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, 
		m_pGimGun, WEAPON_TYPE_OF_GIM_GUN, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS], STANDARD_OBJECT_INDEX_GG_BULLET);
	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList,
		m_pBazooka, WEAPON_TYPE_OF_BAZOOKA, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS], STANDARD_OBJECT_INDEX_BZK_BULLET);
	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, 
		m_pMachineGun, WEAPON_TYPE_OF_MACHINEGUN, m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS], STANDARD_OBJECT_INDEX_MG_BULLET);

	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_GIM_GUN, 50);
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_BAZOOKA, 20);
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_MACHINEGUN, 300);

	CUserInterface *pUserInterface = new CUserInterface();
	pUserInterface->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pUserInterface->Initialize(pd3dDevice, pd3dCommandList, screenCaptureTexture);
	pUserInterface->SetPlayer(m_pPlayer);

	m_pUserInterface = pUserInterface;
}

void CColonyScene::StartScene()
{
	gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundBGM, &(gFmodSound.m_pBGMChannel));
}

void CColonyScene::CreateEnvironmentMap(ID3D12Device *pd3dDevice)
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

void CColonyScene::CreateRtvDsvSrvEnvironmentMap(ID3D12Device *pd3dDevice)
{
	CScene::CreateRenderTargetView(pd3dDevice, m_pd3dEnvirCube, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 6, m_d3dRrvEnvirCubeMapCPUHandle); 
	CScene::CreateDepthStencilView(pd3dDevice, m_pd3dEnvirCubeDSBuffer, &m_d3dDsvEnvirCubeMapCPUHandle);
	m_d3dSrvEnvirCubeMapGPUHandle = CScene::CreateShaderResourceViews(pd3dDevice, m_pd3dEnvirCube, RESOURCE_TEXTURE_CUBE);
}

void CColonyScene::CreateCubeMapCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
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

	m_d3dEMViewport = { 0.0f, 0.0f, float(CUBE_MAP_WIDTH), float(CUBE_MAP_HEIGHT), 0.0f, 1.0f };
	m_d3dEMScissorRect = { 0, 0, CUBE_MAP_WIDTH, CUBE_MAP_HEIGHT };

	for (int i = 0; i < 6; i++)
	{
		XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Looks[i], xmf3Ups[i], true);

		m_pCubeMapCamera[i] = new CCamera();

		m_pCubeMapCamera[i]->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCubeMapCamera[i]->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCubeMapCamera[i]->SetRight(xmf3Right);
		m_pCubeMapCamera[i]->SetUp(xmf3Ups[i]);
		m_pCubeMapCamera[i]->SetLook(xmf3Looks[i]);

		m_pCubeMapCamera[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

}

///////////////////////////////////////////////////////////////////////// 미니맵
void CColonyScene::CreateMinimapMap(ID3D12Device *pd3dDevice)
{
	// 렌더 타겟, 텍스처로 사용될 리소스를 생성.
	screenCaptureTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

	D3D12_CLEAR_VALUE d3dClear = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f } };

	m_pd3dMinimapRsc = screenCaptureTexture->CreateTexture(pd3dDevice, NULL, MINIMAP_BUFFER_WIDTH, MINIMAP_BUFFER_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClear, 0);

	// 렌더 타겟과 함께 Set해줘야하는 깊이 스텐실 버퍼를 생성. 
	// [ 기존에 프레임워크에 있었을 때는 후면 버퍼의 깊이 스텐실 버퍼 크기[w, h]와 미니맵 리소스의 크기가 같아서 미니맵용 깊이 스텐실 버퍼를 따로 만들지 않고 Clear 해주면서 재사용했었음. ]
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = MINIMAP_BUFFER_WIDTH;
	d3dResourceDesc.Height = MINIMAP_BUFFER_HEIGHT;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	d3dClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClear.DepthStencil.Depth = 1.0f;
	d3dClear.DepthStencil.Stencil = 0;

	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClear, __uuidof(ID3D12Resource), (void **)&m_pd3dMinimapDepthStencilBuffer);

	// 미니맵 리소스의 렌더 타겟 뷰, 깊이 스텐실 뷰, 쉐이더 리소스 뷰를 생성.
	CreateRtvDsvSrvMiniMap(pd3dDevice);
}

// 미니맵 카메라 생성
void CColonyScene::CreateMiniMapCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_d3dMMViewport = { 0.0f, 0.0f, float(MINIMAP_BUFFER_WIDTH), float(MINIMAP_BUFFER_HEIGHT), 0.0f, 1.0f };
	m_d3dMMScissorRect = { 0, 0, MINIMAP_BUFFER_WIDTH, MINIMAP_BUFFER_HEIGHT };

	m_pMiniMapCamera = new CCamera();

	XMFLOAT3 xmf3Looks = XMFLOAT3(0.0f, -1.0f, 0.0f);
	XMFLOAT3 xmf3Ups = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Looks, xmf3Ups, true);

	m_pMiniMapCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_pMiniMapCamera->SetPlayer(m_pPlayer);
	m_pMiniMapCamera->GenerateProjectionMatrix(1.01f, 5000.0f, 1.0f, 60.0f);
	m_pMiniMapCamera->SetRight(xmf3Right);
	m_pMiniMapCamera->SetUp(xmf3Ups);
	m_pMiniMapCamera->SetLook(xmf3Looks);

	m_pMiniMapCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CColonyScene::CreateRtvDsvSrvMiniMap(ID3D12Device *pd3dDevice)
{
	CScene::CreateRenderTargetView(pd3dDevice, m_pd3dMinimapRsc, D3D12_RTV_DIMENSION_TEXTURE2D, 1, &m_d3dRtvMinimapCPUHandle);
	CScene::CreateDepthStencilView(pd3dDevice, m_pd3dMinimapDepthStencilBuffer, &m_d3dDsvMinimapCPUHandle);
	CScene::CreateShaderResourceViews(pd3dDevice, screenCaptureTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);
}

void CColonyScene::MinimapRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dMinimapRsc, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	if (m_pMiniMapCamera) {
		m_pMiniMapCamera->UpdateForMinimap(m_pMiniMapCamera->GetPlayer()->GetCamera()->GetLookVector());
		m_pMiniMapCamera->GenerateViewMatrix();
		m_pMiniMapCamera->UpdateShaderVariables(pd3dCommandList);
	}

	pd3dCommandList->ClearRenderTargetView(m_d3dRtvMinimapCPUHandle, Colors::Black, 0, NULL);
	pd3dCommandList->ClearDepthStencilView(m_d3dDsvMinimapCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	pd3dCommandList->OMSetRenderTargets(1, &m_d3dRtvMinimapCPUHandle, TRUE, &m_d3dDsvMinimapCPUHandle);

	pd3dCommandList->RSSetViewports(1, &m_d3dMMViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dMMScissorRect);

	UpdateShaderVariables(pd3dCommandList);

	//if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, m_pMiniMapCamera, true);

	//for (int i = 0; i < m_nShaders; i++)
	//{
	//	if (m_ppShaders[i])
	//		m_ppShaders[i]->Render(pd3dCommandList, m_pMiniMapCamera);
	//}

	if(m_ppShaders[2])
		m_ppShaders[2]->Render(pd3dCommandList, m_pMiniMapCamera);
		

	::TransitionResourceState(pd3dCommandList, m_pd3dMinimapRsc, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
}
/////////////////////////////////////////////////////////////////////////

void CColonyScene::RenderCubeMap(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dEnvirCube, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	for (int i = 0; i < 6; i++)
	{
		m_pCubeMapCamera[i]->SetPosition(m_pPlayer->GetPosition());
		m_pCubeMapCamera[i]->GenerateViewMatrix();
		m_pCubeMapCamera[i]->UpdateShaderVariables(pd3dCommandList);

		pd3dCommandList->ClearRenderTargetView(m_d3dRrvEnvirCubeMapCPUHandle[i], Colors::Black, 0, NULL);
		pd3dCommandList->ClearDepthStencilView(m_d3dDsvEnvirCubeMapCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		pd3dCommandList->OMSetRenderTargets(1, &m_d3dRrvEnvirCubeMapCPUHandle[i], TRUE, &m_d3dDsvEnvirCubeMapCPUHandle);

		pd3dCommandList->RSSetViewports(1, &m_d3dEMViewport);
		pd3dCommandList->RSSetScissorRects(1, &m_d3dEMScissorRect);

		UpdateShaderVariables(pd3dCommandList);

		if (m_pd3dcbLights)
		{
			D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
			pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_LIGHTS, d3dcbLightsGpuVirtualAddress);
		}

		if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, m_pCubeMapCamera[i], true);
		if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, m_pCubeMapCamera[i], true);

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

void CColonyScene::PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	CScene::PrepareRender(pd3dCommandList);

	if (m_nFPS % 5 == 0)
	{
		RenderCubeMap(pd3dCommandList);
	}

	MinimapRender(pd3dCommandList);
}

void CColonyScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dEnvirCube) pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_INDEX_ENVIRORMENTCUBE, m_d3dSrvEnvirCubeMapGPUHandle);

	CScene::Render(pd3dCommandList, pCamera);
}

void CColonyScene::ReleaseUploadBuffers()
{
	CScene::ReleaseUploadBuffers();

	if (m_pGimGun) m_pGimGun->ReleaseUploadBuffers();
	if (m_pBazooka) m_pBazooka->ReleaseUploadBuffers();
	if (m_pMachineGun) m_pMachineGun->ReleaseUploadBuffers();
}

void CColonyScene::CheckCollision()
{
#ifndef ON_NETWORKING
	std::vector<CGameObject*> vEnemys;
	std::vector<CGameObject*> vBullets;
	std::vector<CGameObject*> vBZKBullets;
	std::vector<CGameObject*> vMGBullets;

	vEnemys = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(SKINNED_OBJECT_INDEX_GM);
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

					((CFadeOutShader*)m_ppEffectShaders[INDEX_EFFECT_SHADER_EFFECT])->InsertEffect(pBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f));

					((CSpriteShader*)m_ppEffectShaders[INDEX_EFFECT_SHADER_HIT_SPRITE])->InsertEffect(pBullet->GetPosition(), XMFLOAT2(15.0f, 15.0f), EFFECT_ANIMATION_TYPE::EFFECT_ANIMATION_TYPE_ONE);

					pBullet->Delete();
					std::cout << "Collision Enemy By Bullet\n" << std::endl;
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

					((CFadeOutShader*)m_ppEffectShaders[INDEX_EFFECT_SHADER_EFFECT])->InsertEffect(pBZKBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f));

					((CSpriteShader*)m_ppEffectShaders[INDEX_EFFECT_SHADER_EXP_SPRITE])->InsertEffect(pBZKBullet->GetPosition(), XMFLOAT2(25.0f, 25.0f), EFFECT_ANIMATION_TYPE::EFFECT_ANIMATION_TYPE_ONE);

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

					((CFadeOutShader*)m_ppEffectShaders[INDEX_EFFECT_SHADER_EFFECT])->InsertEffect(pMGBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f));

					((CSpriteShader*)m_ppEffectShaders[INDEX_EFFECT_SHADER_HIT_SPRITE])->InsertEffect(pMGBullet->GetPosition(), XMFLOAT2(25.0f, 25.0f), EFFECT_ANIMATION_TYPE::EFFECT_ANIMATION_TYPE_ONE);

					pMGBullet->Delete();

					std::cout << "Collision Enemy By Bullet\n" << std::endl;
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

void CColonyScene::CheckCollisionPlayer()
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

void CColonyScene::FindAimToTargetDistance()
{
	std::vector<CGameObject*> vEnemys = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_SKINND_OBJECTS])->GetObjects(SKINNED_OBJECT_INDEX_GM);

	float fDistance = 1000.0f;
	float fTemp = 0.0f;
	CGameObject *pTarget = NULL;
	XMFLOAT3 xmf3CameraPos = m_pPlayer->GetCamera()->GetPosition();
	XMVECTOR xmvCameraPos = XMLoadFloat3(&xmf3CameraPos);
	XMVECTOR xmvLook = XMLoadFloat3(&(m_pPlayer->GetCamera()->GetLookVector()));

	XMFLOAT3 xmf3PlayerPos = m_pPlayer->GetPosition();

	for (const auto& Enemy : vEnemys)
	{
		// 카메라 이동 X 단 목표가 되지 않음.
		if (Enemy->CollisionCheck(&xmvCameraPos, &xmvLook, &fTemp))
		{
			float fDistBetweenCnP = Vector3::Length(Vector3::Subtract(xmf3PlayerPos, xmf3CameraPos));

			if (fDistBetweenCnP < fTemp)
			{
				if (fDistance > fTemp)
				{
					fDistance = fTemp;
					pTarget = Enemy;
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
					if (fDistance > fTemp)
					{
						fDistance = fTemp;
						pTarget = Obstacle;
					}
				}
			}
		}
	}
	
	if (m_pTerrain->CollisionCheck(&xmvCameraPos, &xmvLook, &fTemp))
	{
		if (fDistance > fTemp) fDistance = fTemp;
	}

	m_fCameraToTarget = fDistance;
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

//////////////////////////////// for Networking

void CColonyScene::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT *pCreateObjectInfo)
{
	CGameObject* pGameObject = NULL;
	//pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

	if (m_pObjects[pCreateObjectInfo->Object_Index])
	{
		m_pObjects[pCreateObjectInfo->Object_Index]->Delete();
	}

	CObjectsShader *pObjectsShader = NULL;

	switch (pCreateObjectInfo->Object_Type)
	{
	case OBJECT_TYPE_PLAYER:
		pGameObject = new CRobotObject();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_SKINND_OBJECTS];

		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, SKINNED_OBJECT_INDEX_GM, true, NULL);

		if (m_pParticleShader) m_pParticleShader->SetFollowObject(pGameObject, ((CRobotObject*)pGameObject)->GetRightNozzleFrame());
		if (m_pParticleShader) m_pParticleShader->SetFollowObject(pGameObject, ((CRobotObject*)pGameObject)->GetLeftNozzleFrame());

		break;
	case OBJECT_TYPE_OBSTACLE:
		printf("Do not Apply Insert Obstacle\n");
		//((CObjectsShader*)m_ppShaders[INDEX_SHADER_OBSTACLE])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);
		break;
	case OBJECT_TYPE_MACHINE_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGShot);
		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_MG_BULLET, true, NULL);
		break;
	case OBJECT_TYPE_ITEM_HEALING:
		pGameObject = new RotateObject();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_REPAIR_ITEM, true, NULL);
		break;
	case OBJECT_TYPE_BZK_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKShot);
		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_BZK_BULLET, true, NULL);
		break;
	case OBJECT_TYPE_BEAM_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGShot);
		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_GG_BULLET, true, NULL);
		break;
	case OBJECT_TYPE_ITEM_AMMO:
		pGameObject = new RotateObject();
		pGameObject->SetWorldTransf(pCreateObjectInfo->WorldMatrix);

		pObjectsShader = (CObjectsShader*)m_ppShaders[INDEX_SHADER_STANDARD_OBJECTS];
		pObjectsShader->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, STANDARD_OBJECT_INDEX_AMMO_ITEM, true, NULL);
		break;
	}

	m_pObjects[pCreateObjectInfo->Object_Index] = pGameObject;
}

void CColonyScene::DeleteObject(PKT_DELETE_OBJECT *pDeleteObjectInfo)
{
	if (m_pObjects[pDeleteObjectInfo->Object_Index])
	{
		if (m_pObjects[pDeleteObjectInfo->Object_Index]->GetType() & OBJECT_TYPE_ROBOT)
		{
			std::vector<CParticle*> vpParticles = ((CRobotObject*)m_pObjects[pDeleteObjectInfo->Object_Index])->GetParticles();
			for (CParticle *pParticle : vpParticles) pParticle->Delete();
		}
		m_pObjects[pDeleteObjectInfo->Object_Index]->Delete();
		m_pObjects[pDeleteObjectInfo->Object_Index] = NULL;
	}
}

void CColonyScene::CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT *pCreateEffectInfo)
{
	EFFECT_TYPE nEffectType = pCreateEffectInfo->efType;
	EFFECT_ANIMATION_TYPE nEffectAniType = pCreateEffectInfo->EftAnitType;
	CEffect *pEffect = NULL;
	CSprite *pSprite = NULL;
	float fSize = (float)(rand() % 2000) / 100.0f;

	switch (nEffectType)
	{
	case EFFECT_TYPE::EFFECT_TYPE_HIT_FONT:
		((CFadeOutShader*)m_ppEffectShaders[INDEX_EFFECT_SHADER_EFFECT])->InsertEffect(pCreateEffectInfo->xmf3Position, XMFLOAT2(0.04f, 0.02f));
		break;
	case EFFECT_TYPE::EFFECT_TYPE_HIT:
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGHit);
		((CSpriteShader*)m_ppEffectShaders[INDEX_EFFECT_SHADER_HIT_SPRITE])->InsertEffect(pCreateEffectInfo->xmf3Position, XMFLOAT2(fSize, fSize), nEffectAniType);
		break;
	case EFFECT_TYPE::EFFECT_TYPE_EXPLOSION:
		gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKHit);
		((CSpriteShader*)m_ppEffectShaders[INDEX_EFFECT_SHADER_EXP_SPRITE])->InsertEffect(pCreateEffectInfo->xmf3Position, XMFLOAT2(fSize, fSize), nEffectAniType);
		break;
	}
}

void CColonyScene::ApplyRecvInfo(PKT_ID pktID, LPVOID pktData)
{
	switch (pktID)
	{
	case PKT_ID_PLAYER_INFO:
		m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID]->SetWorldTransf(((PKT_PLAYER_INFO*)pktData)->WorldMatrix);
		m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID]->SetPrepareRotate(0.0f, 180.0f, 0.0f);
		if (((PKT_PLAYER_INFO*)pktData)->isChangeAnimation)
		{
			CAnimationObject *pObject = (CAnimationObject*)m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID];
			pObject->ChangeAnimation(((PKT_PLAYER_INFO*)pktData)->Player_Animation);
		}
		if (((PKT_PLAYER_INFO*)pktData)->isChangeWeapon)
		{
			CRobotObject *pObject = (CRobotObject*)m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID];
			pObject->ChangeWeaponByType((WEAPON_TYPE)((PKT_PLAYER_INFO*)pktData)->Player_Weapon);
		}
		m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID]->SetState(((PKT_PLAYER_INFO*)pktData)->State);
		break;
	case PKT_ID_PLAYER_LIFE:
		m_pObjects[((PKT_PLAYER_LIFE*)pktData)->ID]->SetHitPoint(((PKT_PLAYER_LIFE*)pktData)->HP);
		break;
	case PKT_ID_CREATE_OBJECT:
		break;
	case PKT_ID_DELETE_OBJECT:
		DeleteObject(((PKT_DELETE_OBJECT*)pktData));
		break;
	case PKT_ID_TIME_INFO:
		break;
	case PKT_ID_UPDATE_OBJECT:
		if (((PKT_UPDATE_OBJECT*)pktData)->Object_Index)
		{
			XMFLOAT3 position = ((PKT_UPDATE_OBJECT*)pktData)->Object_Position;
			m_pObjects[((PKT_UPDATE_OBJECT*)pktData)->Object_Index]->SetPosition(position);
		}
		break;
	case PKT_ID_CREATE_EFFECT:
		break;
	}
}
