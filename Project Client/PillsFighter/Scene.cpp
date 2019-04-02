#include "stdafx.h"
#include "Scene.h"
#include "Repository.h"

ID3D12DescriptorHeap *CScene::m_pd3dDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

extern CSound gSound;

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
	CreateDescriptorHeaps(pd3dDevice, pd3dCommandList, SCENE_DESCIPTOR_HEAP_COUNT);
}

void CScene::ReleaseObjects()
{
	CMaterial::ReleaseShaders();

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

	if (m_pParticleShader)
	{
		m_pParticleShader->ReleaseShaderVariables();
		m_pParticleShader->ReleaseObjects();
		delete m_pParticleShader;
		m_pParticleShader = NULL;
	}

	ReleaseShaderVariables();
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) if(m_ppShaders[i]) m_ppShaders[i]->ReleaseUploadBuffers();
	if (m_pParticleShader) m_pParticleShader->ReleaseUploadBuffers();
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i])
			m_ppShaders[i]->AnimateObjects(fTimeElapsed, pCamera);
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
	if (m_pd3dGraphicsRootSignature)	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dDescriptorHeap)			pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dDescriptorHeap);

	if (pCamera)
	{
		pCamera->SetViewportsAndScissorRects(pd3dCommandList);
		pCamera->UpdateShaderVariables(pd3dCommandList);
	}

	UpdateShaderVariables(pd3dCommandList);

	if (m_pd3dcbLights)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_LIGHTS, d3dcbLightsGpuVirtualAddress);
	}

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nShaders; i++)
	{
		if(m_ppShaders[i])
			m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	}

	if (m_pParticleShader) m_pParticleShader->Render(pd3dCommandList, pCamera);
}

void CScene::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if(m_pWireShader) m_pWireShader->OnPrepareRender(pd3dCommandList);

	for (int i = 0; i < m_nShaders; i++)
	{
		if(m_ppShaders[i])
			m_ppShaders[i]->RenderWire(pd3dCommandList, pCamera);
	}
}

void CScene::CreateDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nViews)
{
	if (!m_pd3dDescriptorHeap)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CColonyScene::CColonyScene() : CScene()
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
		m_pObjects[i] = NULL;

	m_pSound = new CColonySceneSound();
}

CColonyScene::~CColonyScene()
{
	if (m_pSound) delete m_pSound;
	m_pSound = NULL;
}

ID3D12RootSignature *CColonyScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[5];

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
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CColonyScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	CScene::BuildObjects(pd3dDevice, pd3dCommandList, pRepository);

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildLightsAndMaterials();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_nShaders = SHADER_INDEX;
	m_ppShaders = new CShader*[m_nShaders];

	CObstacleShader *pObstacleShader = new CObstacleShader();
	pObstacleShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pObstacleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_OBSTACLE] = pObstacleShader;

	CGundamShader *pGundamhader = new CGundamShader();
	pGundamhader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pGundamhader->Initialize(pd3dDevice, pd3dCommandList, pRepository, this);
	m_ppShaders[INDEX_SHADER_ENEMY] = pGundamhader;

	CBulletShader *pGGBulletShader = new CBulletShader();
	pGGBulletShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pGGBulletShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_GG_BULLET] = pGGBulletShader;

	CBZKBulletShader *pBZKBulletShader = new CBZKBulletShader();
	pBZKBulletShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pBZKBulletShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_BZK_BULLET] = pBZKBulletShader;

	CBulletShader *pMGBulletShader = new CBulletShader();
	pMGBulletShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pMGBulletShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_MG_BULLET] = pMGBulletShader;

	CHitSpriteShader *pHitSpriteShader = new CHitSpriteShader();
	pHitSpriteShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pHitSpriteShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppShaders[INDEX_SHADER_HIT_SPRITE] = pHitSpriteShader;

	CExpSpriteShader *pExpSpriteShader = new CExpSpriteShader();
	pExpSpriteShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pExpSpriteShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppShaders[INDEX_SHADER_EXP_SPRITE] = pExpSpriteShader;

	CRepairItemShader *pRepairItemShader = new CRepairItemShader();
	pRepairItemShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pRepairItemShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_REPAIR_ITEM] = pRepairItemShader;

	CFadeOutShader *pFadeOutShader = new CFadeOutShader();
	pFadeOutShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pFadeOutShader->Initialize(pd3dDevice, pd3dCommandList, NULL);
	m_ppShaders[INDEX_SHADER_EFFECT] = pFadeOutShader;

	CAmmoItemShader *pAmmoItemShader = new CAmmoItemShader();
	pAmmoItemShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pAmmoItemShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);
	m_ppShaders[INDEX_SHADER_AMMO_ITEM] = pAmmoItemShader;

	m_pWireShader = new CWireShader();
	m_pWireShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(4.0f, 1.0f, 4.0f);
	XMFLOAT4 xmf4Color(1.f, 1.f, 1.f, 1.0f);

	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("./Resource/Stage/HeightMap.raw"), 514, 514, 514, 514, xmf3Scale, xmf4Color);
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	m_pParticleShader = new CParticleShader();
	m_pParticleShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_pParticleShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);

	m_pGimGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/GIM_GUN.bin", false);
	m_pBazooka = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/BZK.bin", false);
	m_pMachineGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/MACHINEGUN.bin", false);

	m_pSound->PlayFMODSound(&(m_pSound->m_BGM));
}

void CColonyScene::SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pParticleShader) m_pParticleShader->SetFollowObject(m_pPlayer, m_pPlayer->GetRightNozzleFrame());
	if (m_pParticleShader) m_pParticleShader->SetFollowObject(m_pPlayer, m_pPlayer->GetLeftNozzleFrame());

	if (m_pPlayer)
	{
		m_pPlayer->SetGravity(m_fGravAcc);
		m_pPlayer->SetScene(this);
	}

	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, m_pGimGun, WEAPON_TYPE_OF_GIM_GUN, m_ppShaders[INDEX_SHADER_GG_BULLET]);
	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, m_pBazooka, WEAPON_TYPE_OF_BAZOOKA, m_ppShaders[INDEX_SHADER_BZK_BULLET]);
	m_pPlayer->AddWeapon(pd3dDevice, pd3dCommandList, m_pMachineGun, WEAPON_TYPE_OF_MACHINEGUN, m_ppShaders[INDEX_SHADER_MG_BULLET]);

	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_GIM_GUN, 50);
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_BAZOOKA, 20);
	m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_MACHINEGUN, 300);
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
	std::vector<CGameObject*> *vEnemys;
	std::vector<CGameObject*> *vBullets;
	std::vector<CGameObject*> *vBZKBullets;
	std::vector<CGameObject*> *vMGBullets;

	vEnemys = static_cast<CGundamShader*>(m_ppShaders[INDEX_SHADER_ENEMY])->GetObjects();
	vBullets = static_cast<CBulletShader*>(m_ppShaders[INDEX_SHADER_GG_BULLET])->GetObjects();
	vBZKBullets = static_cast<CBulletShader*>(m_ppShaders[INDEX_SHADER_BZK_BULLET])->GetObjects();
	vMGBullets = static_cast<CBulletShader*>(m_ppShaders[INDEX_SHADER_MG_BULLET])->GetObjects();

	for (const auto& Enemy : *vEnemys)
	{
		if (m_pPlayer->CollisionCheck(Enemy))
		{
			std::cout << "Collision Player By Enemy\n" << std::endl;
		}

		for (const auto& pBullet : *vBullets)
		{
			if (!pBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pBullet))
				{
					CRobotObjectSound *pSound = ((CRobotObject*)Enemy)->m_pSound;
					pSound->PlayFMODSound(&pSound->m_GGHit);

					((CFadeOutShader*)m_ppShaders[INDEX_SHADER_EFFECT])->InsertEffect(pBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f));

					((CSpriteShader*)m_ppShaders[INDEX_SHADER_HIT_SPRITE])->InsertEffect(pBullet->GetPosition(), XMFLOAT2(15.0f, 15.0f), EFFECT_ANIMATION_TYPE::EFFECT_ANIMATION_TYPE_ONE);

					pBullet->Delete();
					std::cout << "Collision Enemy By Bullet\n" << std::endl;
				}
			}
		}

		for (const auto& pBZKBullet : *vBZKBullets)
		{
			if (!pBZKBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pBZKBullet))
				{
					CRobotObjectSound *pSound = ((CRobotObject*)Enemy)->m_pSound;
					pSound->PlayFMODSound(&pSound->m_BZKHit);

					((CFadeOutShader*)m_ppShaders[INDEX_SHADER_EFFECT])->InsertEffect(pBZKBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f));

					((CSpriteShader*)m_ppShaders[INDEX_SHADER_EXP_SPRITE])->InsertEffect(pBZKBullet->GetPosition(), XMFLOAT2(25.0f, 25.0f), EFFECT_ANIMATION_TYPE::EFFECT_ANIMATION_TYPE_ONE);

					pBZKBullet->Delete();

					std::cout << "Collision Enemy By Bullet\n" << std::endl;
				}
			}
		}

		for (const auto& pMGBullet : *vMGBullets)
		{
			if (!pMGBullet->IsDelete())
			{
				if (Enemy->CollisionCheck(pMGBullet))
				{
					CRobotObjectSound *pSound = ((CRobotObject*)Enemy)->m_pSound;
					pSound->PlayFMODSound(&pSound->m_GGHit);

					((CFadeOutShader*)m_ppShaders[INDEX_SHADER_EFFECT])->InsertEffect(pMGBullet->GetPosition(), XMFLOAT2(0.04f, 0.02f));

					((CSpriteShader*)m_ppShaders[INDEX_SHADER_HIT_SPRITE])->InsertEffect(pMGBullet->GetPosition(), XMFLOAT2(25.0f, 25.0f), EFFECT_ANIMATION_TYPE::EFFECT_ANIMATION_TYPE_ONE);

					pMGBullet->Delete();

					std::cout << "Collision Enemy By Bullet\n" << std::endl;
				}
			}
		}
	}
#endif

	FindAimToTargetDistance();
}

void CColonyScene::CheckCollisionPlayer()
{
	std::vector<CGameObject*> *vObstacles;
	vObstacles = static_cast<CRepairItemShader*>(m_ppShaders[INDEX_SHADER_OBSTACLE])->GetObjects();

	for (const auto& Obstacle : *vObstacles)
	{
		m_pPlayer->MoveToCollision(Obstacle);
	}
}

void CColonyScene::FindAimToTargetDistance()
{
	std::vector<CGameObject*> *vEnemys = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_ENEMY])->GetObjects();
	std::vector<CGameObject*> *vObstacles = static_cast<CObjectsShader*>(m_ppShaders[INDEX_SHADER_OBSTACLE])->GetObjects();

	float fDistance = 1000.0f;
	float fTemp = 0.0f;
	CGameObject *pTarget = NULL;
	XMFLOAT3 xmf3CameraPos = m_pPlayer->GetCamera()->GetPosition();
	XMVECTOR xmvCameraPos = XMLoadFloat3(&xmf3CameraPos);
	XMVECTOR xmvLook = XMLoadFloat3(&(m_pPlayer->GetCamera()->GetLookVector()));

	XMFLOAT3 xmf3PlayerPos = m_pPlayer->GetPosition();

	for (const auto& Enemy : *vEnemys)
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

	for (const auto& Obstacle : *vObstacles)
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

void CColonyScene::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT CreateObjectInfo)
{
	CGameObject* pGameObject = NULL;
	//pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);

	if (m_pObjects[CreateObjectInfo.Object_Index])
	{
		m_pObjects[CreateObjectInfo.Object_Index]->Delete();
	}

	switch (CreateObjectInfo.Object_Type)
	{
	case OBJECT_TYPE_PLAYER:
		pGameObject = new CRobotObject();
		pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);

		((CObjectsShader*)m_ppShaders[INDEX_SHADER_ENEMY])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);

		if (m_pParticleShader) m_pParticleShader->SetFollowObject(pGameObject, ((CRobotObject*)pGameObject)->GetRightNozzleFrame());
		if (m_pParticleShader) m_pParticleShader->SetFollowObject(pGameObject, ((CRobotObject*)pGameObject)->GetLeftNozzleFrame());

		break;
	case OBJECT_TYPE_MACHINE_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);

		((CObjectsShader*)m_ppShaders[INDEX_SHADER_MG_BULLET])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);
		break;
	case OBJECT_TYPE_OBSTACLE:
		((CObjectsShader*)m_ppShaders[INDEX_SHADER_OBSTACLE])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);
		break;
	case OBJECT_TYPE_ITEM_HEALING:
		pGameObject = new RotateObject();
		pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);

		((CObjectsShader*)m_ppShaders[INDEX_SHADER_REPAIR_ITEM])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);
		break;
	case OBJECT_TYPE_BZK_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);

		((CObjectsShader*)m_ppShaders[INDEX_SHADER_BZK_BULLET])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);
		break;
	case OBJECT_TYPE_BEAM_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);

		((CObjectsShader*)m_ppShaders[INDEX_SHADER_GG_BULLET])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);
		break;
	case OBJECT_TYPE_ITEM_AMMO:
		pGameObject = new RotateObject();
		pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);

		((CObjectsShader*)m_ppShaders[INDEX_SHADER_AMMO_ITEM])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject, true);
		break;
	}

	m_pObjects[CreateObjectInfo.Object_Index] = pGameObject;
}

void CColonyScene::DeleteObject(PKT_DELETE_OBJECT DeleteObjectInfo)
{
	if (m_pObjects[DeleteObjectInfo.Object_Index])
	{
		if (m_pObjects[DeleteObjectInfo.Object_Index]->GetType() & OBJECT_TYPE_ROBOT)
		{
			std::vector<CParticle*> vpParticles = ((CRobotObject*)m_pObjects[DeleteObjectInfo.Object_Index])->GetParticles();
			for (CParticle *pParticle : vpParticles) pParticle->Delete();
		}
		m_pObjects[DeleteObjectInfo.Object_Index]->Delete();
		m_pObjects[DeleteObjectInfo.Object_Index] = NULL;
	}
}

void CColonyScene::CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT CreateEffectInfo)
{
	EFFECT_TYPE nEffectType = CreateEffectInfo.nEffectType;
	EFFECT_ANIMATION_TYPE nEffectAniType = CreateEffectInfo.nEffectAniType;
	CEffect *pEffect = NULL;
	CSprite *pSprite = NULL;
	float fSize = (float)(rand() % 2000) / 100.0f;

	switch (nEffectType)
	{
	case EFFECT_TYPE::EFFECT_TYPE_WORD_HIT:
		((CFadeOutShader*)m_ppShaders[INDEX_SHADER_EFFECT])->InsertEffect(CreateEffectInfo.xmf3Position, XMFLOAT2(0.04f, 0.02f));
		break;
	case EFFECT_TYPE::EFFECT_TYPE_HIT:
		((CSpriteShader*)m_ppShaders[INDEX_SHADER_HIT_SPRITE])->InsertEffect(CreateEffectInfo.xmf3Position, XMFLOAT2(fSize, fSize), nEffectAniType);
		break;
	case EFFECT_TYPE::EFFECT_TYPE_EXPLOSION:
		((CSpriteShader*)m_ppShaders[INDEX_SHADER_EXP_SPRITE])->InsertEffect(CreateEffectInfo.xmf3Position, XMFLOAT2(fSize, fSize), nEffectAniType);
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
			pObject->ChangeWeaponByType(((PKT_PLAYER_INFO*)pktData)->Player_Weapon);
		}
		m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID]->SetState(((PKT_PLAYER_INFO*)pktData)->State);
		break;
	case PKT_ID_PLAYER_LIFE:
		m_pObjects[((PKT_PLAYER_LIFE*)pktData)->ID]->SetHitPoint(((PKT_PLAYER_LIFE*)pktData)->HP);
		break;
	case PKT_ID_CREATE_OBJECT:
		break;
	case PKT_ID_DELETE_OBJECT:
		DeleteObject(*((PKT_DELETE_OBJECT*)pktData));
		break;
	case PKT_ID_TIME_INFO:
		break;
	case PKT_ID_UPDATE_OBJECT:
		XMFLOAT3 position = ((PKT_UPDATE_OBJECT*)pktData)->Object_Position;
		m_pObjects[((PKT_UPDATE_OBJECT*)pktData)->Object_Index]->SetPosition(position);
		break;
	case PKT_ID_CREATE_EFFECT:
		break;
	}
}
