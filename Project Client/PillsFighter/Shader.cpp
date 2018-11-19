#include "stdafx.h"
#include "Shader.h"
#include "DDSTextureLoader12.h"

CShader::CShader()
{
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = NULL;
}

CShader::~CShader()
{
	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
	if (m_pd3dPipelineStateWire) m_pd3dPipelineStateWire->Release();

	if (m_ppMaterials)
	{
		for (UINT i = 0; i < m_nMaterials; i++)
			if (m_ppMaterials[i]) delete m_ppMaterials[i];
		delete[] m_ppMaterials;
	}
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState() //래스터라이저 상태를 설정하기 위한 구조체를 반환한다. 
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerStateWire() //래스터라이저 상태를 설정하기 위한 구조체를 반환한다. 
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState() //깊이-스텐실 검사를 위한 상태를 설정하기 위한 구조체를 반환한다. 
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_BLEND_DESC CShader::CreateBlendState() //블렌딩 상태를 설정하기 위한 구조체를 반환한다. 
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout() //입력 조립기에게 정점 버퍼의 구조를 알려주기 위한 구조체를 반환한다.
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayoutWire() //입력 조립기에게 정점 버퍼의 구조를 알려주기 위한 구조체를 반환한다. 
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob) //정점 셰이더 바이트 코드를 생성(컴파일)한다. 
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "VSTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShaderWire(ID3DBlob **ppd3dShaderBlob) //정점 셰이더 바이트 코드를 생성(컴파일)한다. 
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "VSDiffused", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob) //픽셀 셰이더 바이트 코드를 생성(컴파일)한다. 
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "PSTextured", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShaderWire(ID3DBlob **ppd3dShaderBlob) //픽셀 셰이더 바이트 코드를 생성(컴파일)한다. 
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "PSDiffused", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(const WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob) 
//셰이더 소스 코드를 컴파일하여 바이트 코드 구조체를 반환한다. 
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *pd3dErrorBlob = NULL;
	::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, NULL);

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

void CShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature) //그래픽스 파이프라인 상태 객체를 생성한다. 
{
	ID3DBlob *pd3dVertexShaderBlob = NULL, *pd3dPixelShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	d3dPipelineStateDesc.RasterizerState = CreateRasterizerStateWire();
	d3dPipelineStateDesc.InputLayout = CreateInputLayoutWire();
	d3dPipelineStateDesc.VS = CreateVertexShaderWire(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShaderWire(&pd3dPixelShaderBlob);

	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineStateWire);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
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

void CShader::CreateSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nShaderResourceViews;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dSrvDescriptorHeap);

	m_d3dSrvCPUDescriptorStartHandle = m_pd3dSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dSrvGPUDescriptorStartHandle = m_pd3dSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

void CShader::CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement)
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

void CShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CShader::ReleaseUploadBuffers()
{
	if (m_ppMaterials)
	{
		for (UINT i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
		}
	}
}

void CShader::ReleaseShaderVariables()
{
	if (m_pd3dSrvDescriptorHeap) m_pd3dSrvDescriptorHeap->Release();

	if (m_ppMaterials)
	{
		for (UINT i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseShaderVariables();
		}
	}
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	// Pipeline Set, Texture Set
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvDescriptorHeap);

	UpdateShaderVariables(pd3dCommandList);
}

void CShader::OnPrepareRenderWire(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dPipelineStateWire) pd3dCommandList->SetPipelineState(m_pd3dPipelineStateWire);

	UpdateShaderVariables(pd3dCommandList);
}

void CShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRender(pd3dCommandList);
}

void CShader::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRenderWire(pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CObjectsShader::CObjectsShader()
{

}

CObjectsShader::~CObjectsShader()
{
	if (m_ppMeshes)
	{
		for (UINT j = 0; j < m_nMeshes; j++)
		{
			if (m_ppMeshes[j]) delete m_ppMeshes[j];
			if (m_ppCubeMeshes[j]) delete m_ppCubeMeshes[j];
		}

		delete[] m_ppMeshes;
		delete[] m_ppCubeMeshes;
	}
}

void CObjectsShader::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (UINT i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->ReleaseUploadBuffers();
			if (m_ppCubeMeshes[i]) m_ppCubeMeshes[i]->ReleaseUploadBuffers();
		}
	}

	CShader::ReleaseUploadBuffers();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CFixedObjectsShader::CFixedObjectsShader()
{
}

CFixedObjectsShader::~CFixedObjectsShader()
{
}

D3D12_SHADER_BYTECODE CFixedObjectsShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "VSInstancingTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CFixedObjectsShader::CreateVertexShaderWire(ID3DBlob **ppd3dShaderBlob)
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "VSInstancingDiffused", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CFixedObjectsShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "PSInstancingTextured", "ps_5_1", ppd3dShaderBlob));
}

void CFixedObjectsShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(CB_GAMEOBJECT_INFO) * m_nObjects, 
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void **)&m_pcbMappedGameObjects);
}

void CFixedObjectsShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	CShader::ReleaseShaderVariables();
}

void CFixedObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootShaderResourceView(3, m_pd3dcbGameObjects->GetGPUVirtualAddress());

	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->OnPrepareRender();

		XMStoreFloat4x4(&m_pcbMappedGameObjects[j].m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->GetWorldTransf())));
		//::memcpy(&m_pcbMappedGameObjects[j].m_nMaterial, &m_ppObjects[j]->GetMaterial(0)->m_nReflection, sizeof(UINT));
		m_pcbMappedGameObjects[j].m_nMaterial = m_ppObjects[j]->GetMaterial(0)->m_nReflection;
	}
}

void CFixedObjectsShader::AnimateObjects(float fTimeElapsed)
{
	for (UINT i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i]->Animate(fTimeElapsed);
	}
}

void CFixedObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (UINT i = 0; i < m_nObjects; i++)
		{
			if (m_ppObjects[i]) delete m_ppObjects[i];
		}
		delete[] m_ppObjects;
	}
}

void CFixedObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::Render(pd3dCommandList, pCamera); // Pipeline Set

	if (m_ppObjects[0]) m_ppObjects[0]->Render(pd3dCommandList, pCamera, m_nObjects);
}

void CFixedObjectsShader::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::RenderWire(pd3dCommandList, pCamera); // Pipeline Set

	if (m_ppObjects[0]) m_ppObjects[0]->RenderWire(pd3dCommandList, pCamera, m_nObjects);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CBuildingShader::CBuildingShader() 
{
}

CBuildingShader::~CBuildingShader() 
{
}

void CBuildingShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CTexture* pTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTextures->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/hangar.dds", 0);

	CreateSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTextures, 2, false);

	m_nMaterials = 1;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	m_ppMaterials[0] = new CMaterial();
	m_ppMaterials[0]->SetTexture(pTextures);
	m_ppMaterials[0]->m_nReflection = 0;

	m_nMeshes = 1;
	m_ppMeshes = new CMesh*[m_nMeshes];
	m_ppCubeMeshes = new CCubeMesh*[m_nMeshes];

	m_ppMeshes[0] = new CMesh(pd3dDevice, pd3dCommandList, "./Resource/hangar.fbx");
	XMFLOAT3 Extents = m_ppMeshes[0]->GetExtents();
	XMFLOAT3 Center = m_ppMeshes[0]->GetCenter();
	m_ppCubeMeshes[0] = new CCubeMesh(pd3dDevice, pd3dCommandList, Center, Extents.x, Extents.y, Extents.z);

	m_nObjects = 3;
	m_ppObjects = new CGameObject*[m_nObjects];

	m_ppObjects[0] = new CGameObject();
	//m_ppObjects[0]->CreateShaderVariables(pd3dDevice, pd3dCommandList); 
	m_ppObjects[0]->SetPosition(XMFLOAT3(-200.0f, 0.0f, 100.0f));
	m_ppObjects[0]->SetPrepareRotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[0]->SetMesh(m_ppMeshes, m_ppCubeMeshes, m_nMeshes);
	m_ppObjects[0]->SetMaterial(m_ppMaterials, m_nMaterials);

	m_ppObjects[1] = new CGameObject();
	//m_ppObjects[1]->CreateShaderVariables(pd3dDevice, pd3dCommandList); 
	m_ppObjects[1]->SetPosition(XMFLOAT3(200.0f, 0.0f, 100.0f));
	m_ppObjects[1]->SetPrepareRotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[1]->SetMesh(m_ppMeshes, m_ppCubeMeshes, m_nMeshes);
	m_ppObjects[1]->SetMaterial(m_ppMaterials, m_nMaterials);

	m_ppObjects[2] = new CGameObject();
	//m_ppObjects[2]->CreateShaderVariables(pd3dDevice, pd3dCommandList); 
	m_ppObjects[2]->SetPosition(XMFLOAT3(0.0f, 0.0f, 100.0f));
	m_ppObjects[2]->SetPrepareRotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[2]->SetMesh(m_ppMeshes, m_ppCubeMeshes, m_nMeshes);
	m_ppObjects[2]->SetMaterial(m_ppMaterials, m_nMaterials);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CNonFixedObjectsShader::CNonFixedObjectsShader()
{
}

CNonFixedObjectsShader::~CNonFixedObjectsShader()
{
}

void CNonFixedObjectsShader::ReleaseShaderVariables()
{
	for (const auto& Object : m_vObjects)
	{
		Object->ReleaseShaderVariables();
	}

	CShader::ReleaseShaderVariables();
}

void CNonFixedObjectsShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void *pContext)
{
}

void CNonFixedObjectsShader::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* Object)
{
	Object->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	Object->SetMesh(m_ppMeshes, m_ppCubeMeshes, m_nMeshes);
	Object->SetMaterial(m_ppMaterials, m_nMaterials);

	m_vObjects.emplace_back(Object);
}

void CNonFixedObjectsShader::ReleaseObjects()
{
	if (m_vObjects.size())
	{
		for (auto& Object = m_vObjects.begin(); Object != m_vObjects.end();)
		{
			delete *Object;
			Object = m_vObjects.erase(Object);
		}
	}
}

void CNonFixedObjectsShader::AnimateObjects(float fTimeElapsed)
{
	CheckDeleteObjects();

	for (const auto& Object : m_vObjects)
	{
		Object->Animate(fTimeElapsed);
	}	
}

void CNonFixedObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	for (const auto& Object : m_vObjects)
	{
		Object->Render(pd3dCommandList, pCamera);
	}
}

void CNonFixedObjectsShader::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::RenderWire(pd3dCommandList, pCamera);

	for (const auto& Object : m_vObjects)
	{
		Object->RenderWire(pd3dCommandList, pCamera);
	}
}

void CNonFixedObjectsShader::CheckDeleteObjects()
{
	if (m_vObjects.size())
	{
		for (auto& Object = m_vObjects.begin(); Object != m_vObjects.end();)
		{
			if ((*Object)->IsDelete())
			{
				(*Object)->ReleaseShaderVariables();
				delete *Object;

				Object = m_vObjects.erase(Object);
			}
			else
				Object++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CBulletShader::CBulletShader()
{
}

CBulletShader::~CBulletShader()
{
}

void CBulletShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void *pContext)
{
	CTexture* pTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTextures->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Bullet/bullet.dds", 0);

	CreateSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTextures, 2, false);

	m_nMaterials = 1;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	m_ppMaterials[0] = new CMaterial();
	m_ppMaterials[0]->SetTexture(pTextures);
	m_ppMaterials[0]->m_nReflection = 0;

	m_nMeshes = 1;
	m_ppMeshes = new CMesh*[m_nMeshes];
	m_ppCubeMeshes = new CCubeMesh*[m_nMeshes];

	m_ppMeshes[0] = new CMesh(pd3dDevice, pd3dCommandList, "./Resource/Bullet/bullet.fbx");
	XMFLOAT3 Extents = m_ppMeshes[0]->GetExtents();
	XMFLOAT3 Center = m_ppMeshes[0]->GetCenter();
	m_ppCubeMeshes[0] = new CCubeMesh(pd3dDevice, pd3dCommandList, Center, Extents.x, Extents.y, Extents.z);
}

void CBulletShader::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* Object)
{
	Object->SetPrepareRotate(0.0f, 0.0f, 0.0f);

	CNonFixedObjectsShader::InsertObject(pd3dDevice, pd3dCommandList, Object);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CGundamShader::CGundamShader()
{
}

CGundamShader::~CGundamShader()
{
}

void CGundamShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void *pContext)
{
	UINT nTextures;
	CTexture **ppTexture;

	::CreateRobotObjectMesh(pd3dDevice, pd3dCommandList, m_ppMeshes, m_ppCubeMeshes, m_nMeshes);
	::CreateRobotObjectTexture(pd3dDevice, pd3dCommandList, ppTexture, nTextures);
	::CreateRobotObjectShader(pd3dDevice, pd3dCommandList, ppTexture, this);

	UINT nMaterials = nTextures;
	CMaterial **ppMaterials = new CMaterial*[nMaterials];

	for (UINT i = 0; i < nTextures; i++)
	{
		ppMaterials[i] = new CMaterial();
		ppMaterials[i]->SetTexture(ppTexture[i]);
		ppMaterials[i]->m_nReflection = 0;
	}

	SetMaterial(ppMaterials, nMaterials);

	RandomMoveObject *pObjects = new RandomMoveObject();
	pObjects->SetPosition(XMFLOAT3(0.0f, 0.0f, 10.0f));

	InsertObject(pd3dDevice, pd3dCommandList, pObjects);

	//pObjects = new RandomMoveObject();
	//pObjects->SetPosition(XMFLOAT3(0.0f, 0.0f, 10.0f));

	//InsertObject(pObjects);
}

void CGundamShader::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* Object)
{
	Object->SetPrepareRotate(-90.0f, 0.0f, 0.0f);

	CNonFixedObjectsShader::InsertObject(pd3dDevice, pd3dCommandList, Object);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CTerrainShader::CTerrainShader()
{
}

CTerrainShader::~CTerrainShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTerrain", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTerrain", "ps_5_1", ppd3dShaderBlob));
}

void CTerrainShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature
	*pd3dGraphicsRootSignature)
{
	//m_nPipelineStates = 1;
	//m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

CUserInterface::CUserInterface()
{

}

CUserInterface::~CUserInterface()
{
	for (int i = 0; i < m_nUIRect; i++)
	{
		if (m_pUIRect[i])
			delete m_pUIRect[i];
	}
}

D3D12_SHADER_BYTECODE CUserInterface::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VS_UI", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CUserInterface::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GS_UI", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CUserInterface::CreateGeometryShaderHP(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GS_UI_HP", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CUserInterface::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS_UI", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CUserInterface::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE",		0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CUserInterface::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

void CUserInterface::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	ID3DBlob *pd3dVertexShaderBlob = NULL, *pd3dGeometryShaderBlob = NULL, *pd3dPixelShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	d3dPipelineStateDesc.GS = CreateGeometryShaderHP(&pd3dGeometryShaderBlob);

	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineStateHP);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CUserInterface::ReleaseUploadBuffers()
{
	if (m_pUIRect)
	{
		for (UINT i = 0; i < m_nUIRect; i++)
		{
			if (m_pUIRect[i]) m_pUIRect[i]->ReleaseUploadBuffers();
		}
	}

	CShader::ReleaseUploadBuffers();
}

void CUserInterface::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_PLAYER_HP) + 255) & ~255); //256의 배수

	m_pd3dcbPlayerHP = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbPlayerHP->Map(0, NULL, (void **)&m_pcbMappedPlayerHP);
}

void CUserInterface::ReleaseShaderVariables()
{
	if (m_pd3dcbPlayerHP)
	{
		m_pd3dcbPlayerHP->Unmap(0, NULL);
		m_pd3dcbPlayerHP->Release();
	}

	CShader::ReleaseShaderVariables();
}

void CUserInterface::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	//std::cout << *m_pPlayerMaxHP << std::endl;

	m_pcbMappedPlayerHP->MaxHP = *m_pPlayerMaxHP;
	m_pcbMappedPlayerHP->HP = *m_pPlayerHP;

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbPlayerHP->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(7, d3dGpuVirtualAddress);

}

XMFLOAT2 CalculateCenter(float left, float right, float top, float bottom)
{
	return XMFLOAT2((left + right) * 0.5f, bottom);
}

XMFLOAT2 CalculateSize(float left, float right, float top, float bottom)
{
	return XMFLOAT2((left - right) * 0.5f, (top - bottom));
}

void CUserInterface::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_nMaterials = 2;
	m_ppMaterials = new CMaterial*[m_nMaterials];

	CreateSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 2);

	CTexture* pTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTextures->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/HUD.dds", 0);

	m_ppMaterials[0] = new CMaterial();
	m_ppMaterials[0]->SetTexture(pTextures);

	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTextures, 2, false);

	pTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTextures->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/HP.dds", 0);

	m_ppMaterials[1] = new CMaterial();
	m_ppMaterials[1]->SetTexture(pTextures);

	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTextures, 2, false);

	m_nUIRect = 2;
	m_pUIRect = new CUIRect*[m_nUIRect];

	// Base UI
	XMFLOAT2 xmf2Center = CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
	XMFLOAT2 xmf2Size = CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
	m_pUIRect[0] = new CUIRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = CalculateCenter(-0.375000, -0.332812, 0.257778, -0.257778);
	xmf2Size = CalculateSize(-0.375000, -0.332812, 0.257778, -0.257778);
	m_pUIRect[1] = new CUIRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
}

void CUserInterface::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineStateHP);
	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvDescriptorHeap);

	UpdateShaderVariables(pd3dCommandList);

	// Draw HP BAR
	if (m_ppMaterials[1]) m_ppMaterials[1]->UpdateShaderVariables(pd3dCommandList); // Base UI
	m_pUIRect[1]->Render(pd3dCommandList);

	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);

	// Draw Base UI
	if (m_ppMaterials[0]) m_ppMaterials[0]->UpdateShaderVariables(pd3dCommandList); // Base UI
	m_pUIRect[0]->Render(pd3dCommandList);
}