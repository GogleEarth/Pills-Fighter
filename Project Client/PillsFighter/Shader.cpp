#include "stdafx.h"
#include "Shader.h"
#include "DDSTextureLoader12.h"
#include "Repository.h"
#include "Scene.h"
#include "Animation.h"
#include "Weapon.h"

CShader::CShader()
{
}

CShader::~CShader()
{
	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();

	ReleaseShaderVariables();
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
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

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
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

D3D12_DEPTH_STENCIL_DESC CShader::CreateTransluDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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

D3D12_DEPTH_STENCIL_DESC CShader::CreateSODepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateAlwaysDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout() 
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob) 
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "VSTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "PSTextured", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(const WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob) 
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

void CShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
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

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);

	UpdateShaderVariables(pd3dCommandList);
}

void CShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRender(pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationShader::CSkinnedAnimationShader()
{
}

CSkinnedAnimationShader::~CSkinnedAnimationShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkinnedAnimationShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0,		DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0,		DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "BINORMAL", 0,		DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0,		DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "TEXCOORD", 0,		DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0,	DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0,	DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSkinnedAnimationShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", ppd3dShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

D3D12_RASTERIZER_DESC CWireShader::CreateRasterizerState() 
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

D3D12_INPUT_LAYOUT_DESC CWireShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CWireShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob) 
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "VSWire", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CWireShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "PSWire", "ps_5_1", ppd3dShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CObjectsShader::CObjectsShader()
{

}

CObjectsShader::~CObjectsShader()
{
	if (m_pvpObjects) delete[] m_pvpObjects;
}

void CObjectsShader::ReleaseObjects()
{
	for (int i = 0; i < m_nObjectGroup; i++)
	{
		if (m_pvpObjects[i].size())
		{
			for (auto& Object = m_pvpObjects[i].begin(); Object != m_pvpObjects[i].end();)
			{
				delete *Object;
				Object = m_pvpObjects[i].erase(Object);
			}
		}
	}
}

void CObjectsShader::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	CheckDeleteObjects();

	for (int i = 0; i < m_nObjectGroup; i++)
	{
		for (const auto& Object : m_pvpObjects[i]) Object->Animate(fTimeElapsed, pCamera);
	}
}

void CObjectsShader::CheckDeleteObjects()
{
	for (int i = 0; i < m_nObjectGroup; i++)
	{
		if (m_pvpObjects[i].size())
		{
			for (auto& Object = m_pvpObjects[i].begin(); Object != m_pvpObjects[i].end();)
			{
				if ((*Object)->IsDelete())
				{
					delete *Object;

					Object = m_pvpObjects[i].erase(Object);
				}
				else Object++;
			}
		}
	}
}

void CObjectsShader::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* pObject, int nGroup, bool bPrepareRotate, void *pContext)
{
	pObject->SetModel(m_vpModels[nGroup]);
	pObject->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	if(bPrepareRotate) pObject->AddPrepareRotate(0, 180, 0);

	m_pvpObjects[nGroup].emplace_back(pObject);
}

void CObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nObjectGroup; i++)
	{
		int nIndex = 0;

		for (auto& Object : m_pvpObjects[i])
		{
			if(nIndex == 0)
				Object->Render(pd3dCommandList, pCamera, true);
			else
				Object->Render(pd3dCommandList, pCamera, false);

			nIndex++;
		}
	}
}

void CObjectsShader::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	for (int i = 0; i < m_nObjectGroup; i++)
	{
		for (const auto& Object : m_pvpObjects[i])
		{
			Object->RenderWire(pd3dCommandList, pCamera);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CStandardObjectsShader::CStandardObjectsShader()
{

}

CStandardObjectsShader::~CStandardObjectsShader()
{

}

void CStandardObjectsShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_nObjectGroup = STANDARD_OBJECT_GROUP;
	m_pvpObjects = new std::vector<CGameObject*>[m_nObjectGroup];

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Bullet/Bullet.bin", NULL, NULL));
	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Bullet/BZK_Bullet.bin", NULL, NULL));
	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Bullet/Bullet.bin", NULL, NULL));
	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Item/Item_Repair.bin", NULL, NULL));

	RotateObject *pObject = new RotateObject();
	pObject->SetPosition(XMFLOAT3(0.0f, 20.0f, 0.0f));
	InsertObject(pd3dDevice, pd3dCommandList, pObject, STANDARD_OBJECT_INDEX_REPAIR_ITEM, true, pContext);

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Item/AMMO_BOX.bin", NULL, NULL));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CInstancingObjectsShader::CInstancingObjectsShader()
{

}

CInstancingObjectsShader::~CInstancingObjectsShader()
{

}

D3D12_SHADER_BYTECODE CInstancingObjectsShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "VSInsTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CInstancingObjectsShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CompileShaderFromFile(L"Shaders.hlsl", "PSInsTextured", "ps_5_1", ppd3dShaderBlob));
}

void CInstancingObjectsShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	ID3D12Resource* pd3dcbGameObject;
	VS_VB_INSTANCE* pcbMappedGameObject;

	for (int i = 0; i < m_nObjectGroup; i++)
	{
		pd3dcbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, static_cast<UINT>(sizeof(VS_VB_INSTANCE) * m_pvpObjects[i].size()), D3D12_HEAP_TYPE_UPLOAD,
			D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

		pd3dcbGameObject->Map(0, NULL, (void **)&pcbMappedGameObject);

		m_vpd3dcbGameObjects.emplace_back(pd3dcbGameObject);
		m_vpcbMappedGameObjects.emplace_back(pcbMappedGameObject);
	}
}

void CInstancingObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetGraphicsRootShaderResourceView(ROOT_PARAMETER_INDEX_INSTANCE, m_vpd3dcbGameObjects[nIndex]->GetGPUVirtualAddress());

	int nObjectIndex = 0;

	for (const auto& pObject : m_pvpObjects[nIndex])
	{
		pObject->UpdateInstanceShaderVariables(m_vpcbMappedGameObjects[nIndex], &nObjectIndex);
	}
}

void CInstancingObjectsShader::ReleaseShaderVariables()
{
	for (const auto& pd3dcb : m_vpd3dcbGameObjects)
	{
		pd3dcb->Unmap(0, NULL);
		pd3dcb->Release();

	}
}

void CInstancingObjectsShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
}

void CInstancingObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nObjectGroup; i++)
	{
		UpdateShaderVariables(pd3dCommandList, i);

		m_pvpObjects[i][0]->Render(pd3dCommandList, pCamera, true, static_cast<int>(m_pvpObjects[i].size()));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CObstacleShader::CObstacleShader()
{
}

CObstacleShader::~CObstacleShader()
{
}

void CObstacleShader::InsertObjectFromLoadInfFromBin(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, int nGroup)
{
	CGameObject *pObject = new CGameObject();

	FILE *pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	if (!pInFile) {
		std::cout << "lose bin file" << std::endl;
	}
	::rewind(pInFile);

	char pstrToken[64] = { '\0' };
	UINT nReads = 0;
	float loadedToken = 0;
	UINT cycle = 0;
	XMFLOAT3 posLoader = XMFLOAT3(0, 0, 0);
	XMFLOAT3 rotLoader = XMFLOAT3(0, 0, 0);

	while (feof(pInFile) == 0)
	{
		ReadPosrotFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "m_value"))
		{
			switch (cycle) {
			case 0:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				posLoader.x = loadedToken;
				++cycle;
				break;
			case 1:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				posLoader.y = loadedToken;
				++cycle;
				break;
			case 2:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				posLoader.z = loadedToken;
				++cycle;
				break;
			case 3:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				rotLoader.x = loadedToken;
				++cycle;
				break;
			case 4:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				rotLoader.y = loadedToken;
				++cycle;
				break;
			case 5:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				rotLoader.z = loadedToken;
				pObject = new CGameObject();
				pObject->SetPosition(posLoader);
				pObject->SetPrepareRotate(rotLoader.x, rotLoader.y, rotLoader.z);
				InsertObject(pd3dDevice, pd3dCommandList, pObject, nGroup, true, NULL);
				cycle = 0;
				break;
			}
		}
		else
		{
			std::cout << "bin file load error" << std::endl;
			break;
		}
	}

}

void CObstacleShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_nObjectGroup = INSTANCING_OBJECT_GROUP;
	m_pvpObjects = new std::vector<CGameObject*>[m_nObjectGroup];

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Hangar.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveHangarSelfData.bin", INSTANCING_OBJECT_INDEX_HANGAR);

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Double_Square.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveDoubleSquareSelfData.bin", INSTANCING_OBJECT_INDEX_DOUBLESQUARE);

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Octagon.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveOctagonSelfData.bin", INSTANCING_OBJECT_INDEX_OCTAGON);

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_OctagonLongTier.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveOctagonLongtierSelfData.bin", INSTANCING_OBJECT_INDEX_OCTAGONLONGTIER);

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Slope_top.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveSlopeTopSelfData.bin", INSTANCING_OBJECT_INDEX_SLOPETOP);

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Square.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveSquareSelfData.bin", INSTANCING_OBJECT_INDEX_SQUARE);

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Steeple_top.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveSteepleTopSelfData.bin", INSTANCING_OBJECT_INDEX_STEEPLETOP);

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Wall.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/WallSelfData.bin", INSTANCING_OBJECT_INDEX_WALL);

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Brick_Garage.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/B_BrickGarageSelfData.bin", INSTANCING_OBJECT_INDEX_BRICKGARAGE);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CSkinnedObjectsShader::CSkinnedObjectsShader()
{

}

CSkinnedObjectsShader::~CSkinnedObjectsShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkinnedObjectsShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0,		DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0,		DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "BINORMAL", 0,		DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0,		DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "TEXCOORD", 0,		DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0,	DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0,	DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSkinnedObjectsShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", ppd3dShaderBlob));
}

void CSkinnedObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	for (int i = 0; i < m_nObjectGroup; i++)
	{
		for (auto& Object : m_pvpObjects[i])
		{
			CShader::Render(pd3dCommandList, pCamera);

			Object->Render(pd3dCommandList, pCamera, true);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CRobotObjectsShader::CRobotObjectsShader()
{
}

CRobotObjectsShader::~CRobotObjectsShader()
{
}

void CRobotObjectsShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_nObjectGroup = SKINNED_OBJECT_GROUP;
	m_pvpObjects = new std::vector<CGameObject*>[m_nObjectGroup];

	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Robot/GM.bin", "./Resource/Animation/UpperBody.bin", "./Resource/Animation/UnderBody.bin"));
	m_vpModels.emplace_back(pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Robot/Gundam.bin", "./Resource/Animation/UpperBody.bin", "./Resource/Animation/UnderBody.bin"));

	m_pGimGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/GIM_GUN.bin", NULL, NULL);
	m_pBazooka = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/BZK.bin", NULL, NULL);
	m_pMachineGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/MACHINEGUN.bin", NULL, NULL);
	m_pSaber = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/Saber.bin", NULL, NULL);

	m_pd3dSceneRootSignature = (ID3D12RootSignature*)pContext;

#ifndef ON_NETWORKING
	CRobotObject *pObject = new CRobotObject();
	pObject->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	InsertObject(pd3dDevice, pd3dCommandList, pObject, SKINNED_OBJECT_INDEX_GUNDAM, true, pContext);

	//pObject = new CRobotObject();
	//pObject->SetPosition(XMFLOAT3(50.0f, 0.0f, 0.0f));

	//InsertObject(pd3dDevice, pd3dCommandList, pObject, SKINNED_OBJECT_INDEX_GM, true, pContext);

	pObject = new CRobotObject();
	pObject->SetPosition(XMFLOAT3(0.0f, 0.0f, 50.0f));

	InsertObject(pd3dDevice, pd3dCommandList, pObject, SKINNED_OBJECT_INDEX_GUNDAM, true, pContext);
#endif
}

void CRobotObjectsShader::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* pObject, int nGroup, bool bPrepareRotate, void *pContext)
{
	CSkinnedObjectsShader::InsertObject(pd3dDevice, pd3dCommandList, pObject, nGroup, bPrepareRotate, pContext);

	int nAnimationControllers = 2;
	CAnimationController **ppAnimationControllers = new CAnimationController*[nAnimationControllers];
	ppAnimationControllers[ANIMATION_UP] = new CAnimationController(1, m_vpModels[nGroup]->GetAnimationSet(ANIMATION_UP));
	ppAnimationControllers[ANIMATION_DOWN] = new CAnimationController(1, m_vpModels[nGroup]->GetAnimationSet(ANIMATION_DOWN));

	ppAnimationControllers[ANIMATION_UP]->SetTrackAnimation(0, ANIMATION_STATE_IDLE);
	ppAnimationControllers[ANIMATION_DOWN]->SetTrackAnimation(0, ANIMATION_STATE_IDLE);

	pObject->SetAnimationController(ppAnimationControllers[ANIMATION_UP], ANIMATION_UP);
	pObject->SetAnimationController(ppAnimationControllers[ANIMATION_DOWN], ANIMATION_DOWN);

	CRobotObject *pRobot = (CRobotObject*)pObject;

	CShader *pShader = new CShader();
	pShader->CreateShader(pd3dDevice, m_pd3dSceneRootSignature);

	pRobot->SetWeaponShader(pShader);
	pRobot->AddWeapon(pd3dDevice, pd3dCommandList, m_pGimGun, WEAPON_TYPE_OF_GUN | WEAPON_TYPE_OF_GIM_GUN);
	pRobot->AddWeapon(pd3dDevice, pd3dCommandList, m_pBazooka, WEAPON_TYPE_OF_GUN | WEAPON_TYPE_OF_BAZOOKA);
	pRobot->AddWeapon(pd3dDevice, pd3dCommandList, m_pMachineGun, WEAPON_TYPE_OF_GUN | WEAPON_TYPE_OF_MACHINEGUN);
	pRobot->AddWeapon(pd3dDevice, pd3dCommandList, m_pSaber, WEAPON_TYPE_OF_SABER);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEffectShader::CEffectShader()
{
}

CEffectShader::~CEffectShader()
{
	if (m_pd3dSOPipelineState) m_pd3dSOPipelineState->Release();
}

D3D12_STREAM_OUTPUT_DESC CEffectShader::CreateStreamOutput()
{
	D3D12_STREAM_OUTPUT_DESC d3dStreamOutputDesc;
	d3dStreamOutputDesc.pSODeclaration = NULL;
	d3dStreamOutputDesc.NumEntries = 0;
	d3dStreamOutputDesc.pBufferStrides = NULL;
	d3dStreamOutputDesc.NumStrides = 0;
	d3dStreamOutputDesc.RasterizedStream = 0;

	return(d3dStreamOutputDesc);
}

D3D12_SHADER_BYTECODE CEffectShader::CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderBytecode;
	d3dShaderBytecode.BytecodeLength = 0;
	d3dShaderBytecode.pShaderBytecode = NULL;

	return(d3dShaderBytecode);
}

D3D12_SHADER_BYTECODE CEffectShader::CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderBytecode;
	d3dShaderBytecode.BytecodeLength = 0;
	d3dShaderBytecode.pShaderBytecode = NULL;

	return(d3dShaderBytecode);
}

D3D12_SHADER_BYTECODE CEffectShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderBytecode;
	d3dShaderBytecode.BytecodeLength = 0;
	d3dShaderBytecode.pShaderBytecode = NULL;

	return(d3dShaderBytecode);
}

D3D12_RASTERIZER_DESC CEffectShader::CreateRasterizerState()
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

D3D12_BLEND_DESC CEffectShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void CEffectShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;

	ID3DBlob *pd3dVertexBlob, *pd3dPixelBlob, *pd3dGeometryBlob;

	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelBlob);
	d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryBlob);
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.DepthStencilState = CreateTransluDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	d3dPipelineStateDesc.PS.BytecodeLength = 0;
	d3dPipelineStateDesc.PS.pShaderBytecode = 0;
	d3dPipelineStateDesc.VS = CreateSOVertexShader(&pd3dVertexBlob);
	d3dPipelineStateDesc.GS = CreateSOGeometryShader(&pd3dGeometryBlob);
	d3dPipelineStateDesc.StreamOutput = CreateStreamOutput();
	d3dPipelineStateDesc.DepthStencilState = CreateSODepthStencilState();

	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dSOPipelineState);
	
	if (pd3dVertexBlob) pd3dVertexBlob->Release();
	if (pd3dPixelBlob) pd3dPixelBlob->Release();
	if (pd3dGeometryBlob) pd3dGeometryBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CEffectShader::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nEffects; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
	}
}

void CEffectShader::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	if (m_ppEffects)
	{
		for (int i = 0; i < m_nEffects; i++)
		{
			if (m_ppEffects[i]) m_ppEffects[i]->Animate(fTimeElapsed);
		}
	}
}

void CEffectShader::ReleaseObjects()
{
	if (m_ppEffects)
	{
		for (int i = 0; i < m_nEffects; i++)
		{
			if (m_ppEffects[i]) delete m_ppEffects[i];
			if (m_ppTextures[i]) delete m_ppTextures[i];
		}

		delete[] m_ppEffects;
		delete[] m_ppTextures;
	}
}

void CEffectShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::OnPrepareRender(pd3dCommandList);

	if (m_ppEffects)
	{
		for (int i = 0; i < m_nEffects; i++)
		{
			if (m_ppTextures[i]) m_ppTextures[i]->UpdateShaderVariables(pd3dCommandList);
			if (m_ppEffects[i]) m_ppEffects[i]->Render(pd3dCommandList);
		}
	}
}

void CEffectShader::PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for(int i = 0; i < m_nEffects; i++) 
		if (m_ppEffects[i]) m_ppEffects[i]->ReadVertexCount(pd3dCommandList);

	if (m_pd3dSOPipelineState) pd3dCommandList->SetPipelineState(m_pd3dSOPipelineState);

	for (int i = 0; i < m_nEffects; i++) 
		if (m_ppEffects[i]) m_ppEffects[i]->SORender(pd3dCommandList);
}

void CEffectShader::AfterRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (int i = 0; i < m_nEffects; i++) 
		if (m_ppEffects[i]) m_ppEffects[i]->AfterRender(pd3dCommandList);
}

void CEffectShader::AddEffect(int nIndex, XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, int nEffectAniType, int nTextures)
{
	int nTextureIndex = 0;

	if (nTextures > 1)
		nTextureIndex = rand() % nTextures;

	if(m_ppEffects[nIndex]) m_ppEffects[nIndex]->AddVertex(xmf3Position, xmf2Size, nTextureIndex, nEffectAniType);
}

//////////////////////////////////////////////////////////////////////

CTimedEffectShader::CTimedEffectShader()
{
}

CTimedEffectShader::~CTimedEffectShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTimedEffectShader::CreateInputLayout()
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TEXINDEX", 0, DXGI_FORMAT_R32_UINT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_STREAM_OUTPUT_DESC CTimedEffectShader::CreateStreamOutput()
{
	UINT nSODecls = 4;
	D3D12_SO_DECLARATION_ENTRY *pd3dStreamOutputDeclarations = new D3D12_SO_DECLARATION_ENTRY[nSODecls];

	pd3dStreamOutputDeclarations[0] = { 0, "POSITION", 0, 0, 3, 0 };
	pd3dStreamOutputDeclarations[1] = { 0, "AGE", 0, 0, 1, 0 };
	pd3dStreamOutputDeclarations[2] = { 0, "SIZE", 0, 0, 2, 0 };
	pd3dStreamOutputDeclarations[3] = { 0, "TEXINDEX", 0, 0, 1, 0 };

	UINT nStrides = 1;
	UINT *pnStride = new UINT[nStrides];
	pnStride[0] = sizeof(CFadeOutVertex);

	D3D12_STREAM_OUTPUT_DESC d3dStreamOutputDesc;
	d3dStreamOutputDesc.pSODeclaration = pd3dStreamOutputDeclarations;
	d3dStreamOutputDesc.NumEntries = nSODecls;
	d3dStreamOutputDesc.pBufferStrides = pnStride;
	d3dStreamOutputDesc.NumStrides = nStrides;
	d3dStreamOutputDesc.RasterizedStream = 0;

	return(d3dStreamOutputDesc);
}

D3D12_SHADER_BYTECODE CTimedEffectShader::CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSEffectStreamOut", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTimedEffectShader::CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSEffectStreamOut", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTimedEffectShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSEffectDraw", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTimedEffectShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSEffectDraw", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTimedEffectShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSEffectDraw", "ps_5_1", ppd3dShaderBlob));
}

void CTimedEffectShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	m_nEffects = TIMED_EFFECT_COUNT;
	m_ppTextures = new CTexture*[m_nEffects];
	m_ppEffects = new CEffect*[m_nEffects];

	m_ppTextures[TIMED_EFFECT_INDEX_MUZZLE_FIRE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[TIMED_EFFECT_INDEX_MUZZLE_FIRE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Effect/Muzzle1.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[TIMED_EFFECT_INDEX_MUZZLE_FIRE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppEffects[TIMED_EFFECT_INDEX_MUZZLE_FIRE] = new CFadeOut(pd3dDevice, pd3dCommandList, 0.1f);
	m_ppEffects[TIMED_EFFECT_INDEX_MUZZLE_FIRE]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////

CTextEffectShader::CTextEffectShader()
{

}

CTextEffectShader::~CTextEffectShader()
{
	if (m_pd3dTextPipelineState) m_pd3dTextPipelineState->Release();
}

void CTextEffectShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	CTimedEffectShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;

	ID3DBlob *pd3dVertexBlob, *pd3dPixelBlob, *pd3dGeometryBlob;

	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelBlob);
	d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryBlob);
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.DepthStencilState = CreateAlwaysDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dTextPipelineState);

	if (pd3dVertexBlob) pd3dVertexBlob->Release();
	if (pd3dPixelBlob) pd3dPixelBlob->Release();
	if (pd3dGeometryBlob) pd3dGeometryBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CTextEffectShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	m_nEffects = TEXT_EFFECT_COUNT;
	m_ppTextures = new CTexture*[m_nEffects];
	m_ppEffects = new CEffect*[m_nEffects];

	m_ppTextures[TEXT_EFFECT_INDEX_HIT_TEXT] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[TEXT_EFFECT_INDEX_HIT_TEXT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Effect/HIT.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[TEXT_EFFECT_INDEX_HIT_TEXT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppEffects[TEXT_EFFECT_INDEX_HIT_TEXT] = new CFadeOut(pd3dDevice, pd3dCommandList, 2.0f);
	m_ppEffects[TEXT_EFFECT_INDEX_HIT_TEXT]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CTextEffectShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dTextPipelineState);

	if (m_ppEffects)
	{
		for (int i = 0; i < m_nEffects; i++)
		{
			if (m_ppTextures[i]) m_ppTextures[i]->UpdateShaderVariables(pd3dCommandList);
			if (m_ppEffects[i]) m_ppEffects[i]->Render(pd3dCommandList);
		}
	}
}

//////////////////////////////////////////////////////////////////////

CSpriteShader::CSpriteShader()
{

}

CSpriteShader::~CSpriteShader()
{

}

D3D12_INPUT_LAYOUT_DESC CSpriteShader::CreateInputLayout()
{
	UINT nInputElementDescs = 6;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "SPRITEPOS", 0, DXGI_FORMAT_R32G32_UINT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "TEXINDEX", 0, DXGI_FORMAT_R32_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_STREAM_OUTPUT_DESC CSpriteShader::CreateStreamOutput()
{
	UINT nSODecls = 6;
	D3D12_SO_DECLARATION_ENTRY *pd3dStreamOutputDeclarations = new D3D12_SO_DECLARATION_ENTRY[nSODecls];

	pd3dStreamOutputDeclarations[0] = { 0, "POSITION", 0, 0, 3, 0 };
	pd3dStreamOutputDeclarations[1] = { 0, "SIZE", 0, 0, 2, 0 };
	pd3dStreamOutputDeclarations[2] = { 0, "SPRITEPOS", 0, 0, 2, 0 };
	pd3dStreamOutputDeclarations[3] = { 0, "AGE", 0, 0, 1, 0 };
	pd3dStreamOutputDeclarations[4] = { 0, "TEXINDEX", 0, 0, 1, 0 };
	pd3dStreamOutputDeclarations[5] = { 0, "TYPE", 0, 0, 1, 0 };

	UINT nStrides = 1;
	UINT *pnStride = new UINT[nStrides];
	pnStride[0] = sizeof(CSpriteVertex);

	D3D12_STREAM_OUTPUT_DESC d3dStreamOutputDesc;
	d3dStreamOutputDesc.pSODeclaration = pd3dStreamOutputDeclarations;
	d3dStreamOutputDesc.NumEntries = nSODecls;
	d3dStreamOutputDesc.pBufferStrides = pnStride;
	d3dStreamOutputDesc.NumStrides = nStrides;
	d3dStreamOutputDesc.RasterizedStream = 0;

	return(d3dStreamOutputDesc);
}

D3D12_SHADER_BYTECODE CSpriteShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpriteDraw", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSpriteShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSSpriteDraw", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSpriteShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSpriteDraw", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSpriteShader::CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpriteStreamOut", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSpriteShader::CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSSpriteStreamOut", "gs_5_1", ppd3dShaderBlob));
}

void CSpriteShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_nEffects = SPRITE_EFFECT_COUNT;
	m_ppTextures = new CTexture*[m_nEffects];
	m_ppEffects = new CEffect*[m_nEffects];;

	//
	m_ppTextures[SPRITE_EFFECT_INDEX_HIT] = new CTexture(2, RESOURCE_TEXTURE2D_ARRAY, 0);
	m_ppTextures[SPRITE_EFFECT_INDEX_HIT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Effect/Attack1.dds", 0);
	m_ppTextures[SPRITE_EFFECT_INDEX_HIT]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Effect/Attack2.dds", 1);

	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[SPRITE_EFFECT_INDEX_HIT], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppEffects[SPRITE_EFFECT_INDEX_HIT] = new CSprite(pd3dDevice, pd3dCommandList, 5, 2, 6, 0.2f);
	m_ppEffects[SPRITE_EFFECT_INDEX_HIT]->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//
	m_ppTextures[SPRITE_EFFECT_INDEX_EXPLOSION] = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0);
	m_ppTextures[SPRITE_EFFECT_INDEX_EXPLOSION]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Effect/Explosion.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[SPRITE_EFFECT_INDEX_EXPLOSION], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppEffects[SPRITE_EFFECT_INDEX_EXPLOSION] = new CSprite(pd3dDevice, pd3dCommandList, 5, 3, 12, 1.0f);
	m_ppEffects[SPRITE_EFFECT_INDEX_EXPLOSION]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CParticleShader::CParticleShader()
{

}

CParticleShader::~CParticleShader()
{
	if (m_pvpParticles)
	{
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			for (CParticle* pParticle : m_pvpParticles[i])
			{
				pParticle->ReleaseShaderVariables();
				delete pParticle;
			}
		}

		delete[] m_pvpParticles;
	}

	if (m_pvpTempParticles)
	{
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			while(!m_pvpTempParticles[i].empty())
			{
				CParticle *pParticle = m_pvpTempParticles[i].front();
				m_pvpTempParticles[i].pop();

				pParticle->ReleaseShaderVariables();
				delete pParticle;
			}
		}

		delete[] m_pvpTempParticles;
	}

	if (m_ppTextures)
	{
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			if (m_ppTextures[i]) delete m_ppTextures[i];
		}

		delete[] m_ppTextures;
	}

	if (m_pd3dSOPipelineState) m_pd3dSOPipelineState->Release();
}

D3D12_INPUT_LAYOUT_DESC CParticleShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	//pd3dInputElementDescs[5] = { "FACTOR", 0, DXGI_FORMAT_R32_UINT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_STREAM_OUTPUT_DESC CParticleShader::CreateStreamOutput()
{
	UINT nSODecls = 5;
	D3D12_SO_DECLARATION_ENTRY *pd3dStreamOutputDeclarations = new D3D12_SO_DECLARATION_ENTRY[nSODecls];

	pd3dStreamOutputDeclarations[0] = { 0, "POSITION", 0, 0, 3, 0 };
	pd3dStreamOutputDeclarations[1] = { 0, "VELOCITY", 0, 0, 3, 0 };
	pd3dStreamOutputDeclarations[2] = { 0, "SIZE", 0, 0, 2, 0 };
	pd3dStreamOutputDeclarations[3] = { 0, "TYPE", 0, 0, 1, 0 };
	pd3dStreamOutputDeclarations[4] = { 0, "AGE", 0, 0, 1, 0 };
	//pd3dStreamOutputDeclarations[5] = { 0, "FACTOR", 0, 0, 1, 0 };

	UINT nStrides = 1;
	UINT *pnStride = new UINT[nStrides];
	pnStride[0] = sizeof(CParticleVertex);

	D3D12_STREAM_OUTPUT_DESC d3dStreamOutputDesc;
	d3dStreamOutputDesc.pSODeclaration = pd3dStreamOutputDeclarations;
	d3dStreamOutputDesc.NumEntries = nSODecls;
	d3dStreamOutputDesc.pBufferStrides = pnStride;
	d3dStreamOutputDesc.NumStrides = nStrides;
	d3dStreamOutputDesc.RasterizedStream = 0;

	return(d3dStreamOutputDesc);
}

D3D12_SHADER_BYTECODE CParticleShader::CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSParticleStreamOut", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CParticleShader::CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSParticleStreamOut", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CParticleShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSParticleDraw", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CParticleShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSParticleDraw", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CParticleShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSParticleDraw", "ps_5_1", ppd3dShaderBlob));
}

D3D12_RASTERIZER_DESC CParticleShader::CreateRasterizerState()
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

void CParticleShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;

	ID3DBlob *pd3dVertexBlob, *pd3dPixelBlob, *pd3dGeometryBlob;

	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelBlob);
	d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryBlob);
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.DepthStencilState = CreateTransluDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	d3dPipelineStateDesc.PS.BytecodeLength = 0;
	d3dPipelineStateDesc.PS.pShaderBytecode = 0;
	d3dPipelineStateDesc.VS = CreateSOVertexShader(&pd3dVertexBlob);
	d3dPipelineStateDesc.GS = CreateSOGeometryShader(&pd3dGeometryBlob);
	d3dPipelineStateDesc.StreamOutput = CreateStreamOutput();
	d3dPipelineStateDesc.DepthStencilState = CreateSODepthStencilState();

	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dSOPipelineState);

	if (pd3dVertexBlob) pd3dVertexBlob->Release();
	if (pd3dPixelBlob) pd3dPixelBlob->Release();
	if (pd3dGeometryBlob) pd3dGeometryBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CParticleShader::ReleaseUploadBuffers()
{
	if (m_pvpTempParticles)
	{
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			for (int j = 0 ; j < m_pvpTempParticles[i].size(); j++)
			{
				CParticle *pParticle = m_pvpTempParticles[i].front();
				m_pvpTempParticles[i].pop();

				pParticle->ReleaseUploadBuffers();

				m_pvpTempParticles[i].push(pParticle);
			}
		}
	}

	if (m_ppTextures)
	{
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
		}
	}
}

void CParticleShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_pvpParticles = new std::vector<CParticle*>[PARTICLE_COUNT];
	m_pvpTempParticles = new std::queue<CParticle*>[PARTICLE_COUNT];

	CParticle *pParticle = NULL;

	for (int i = 0; i < 16; i++)
	{
		pParticle = new CParticle(pd3dDevice, pd3dCommandList, XMFLOAT2(4.0f, 4.0f));
		pParticle->Initialize(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 40.0f, 1.0f, 0.01f, false,
			XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 90.0f, 90.0f));
		pParticle->CreateShaderVariables(pd3dDevice, pd3dCommandList);

		m_pvpTempParticles[PARTICLE_INDEX_BOOSTER_FLARE].push(pParticle);

		pParticle = new CParticle(pd3dDevice, pd3dCommandList, XMFLOAT2(5.0f, 5.0f));
		pParticle->Initialize(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 10.0f, 4.0f, 0.1f, true,
			XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 90.0f, 90.0f));		
		pParticle->CreateShaderVariables(pd3dDevice, pd3dCommandList);

		m_pvpTempParticles[PARTICLE_INDEX_BOOSTER_FOG].push(pParticle);
	}

	m_ppTextures = new CTexture*[PARTICLE_COUNT];

	m_ppTextures[PARTICLE_INDEX_BOOSTER_FLARE] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[PARTICLE_INDEX_BOOSTER_FLARE]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Effect/Flare.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[PARTICLE_INDEX_BOOSTER_FLARE], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[PARTICLE_INDEX_BOOSTER_FOG] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[PARTICLE_INDEX_BOOSTER_FOG]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Effect/Fog1.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[PARTICLE_INDEX_BOOSTER_FOG], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);
}

void CParticleShader::SetFollowObject(CGameObject *pObject, CModel *pFrame)
{
	// Flare
	CParticle *pParticle = m_pvpTempParticles[PARTICLE_INDEX_BOOSTER_FLARE].front();
	m_pvpTempParticles[PARTICLE_INDEX_BOOSTER_FLARE].pop();

	pParticle->SetFollowObject(pObject, pFrame);
	m_pvpParticles[PARTICLE_INDEX_BOOSTER_FLARE].emplace_back(pParticle);

	// Fog
	pParticle = m_pvpTempParticles[PARTICLE_INDEX_BOOSTER_FOG].front();
	m_pvpTempParticles[PARTICLE_INDEX_BOOSTER_FOG].pop();

	pParticle->SetFollowObject(pObject, pFrame);
	m_pvpParticles[PARTICLE_INDEX_BOOSTER_FOG].emplace_back(pParticle);
}

void CParticleShader::CheckDeleteObjects()
{
}

void CParticleShader::AnimateObjects(float fTimeElapsed)
{
	CheckDeleteObjects();

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		for (const auto& pParticle : m_pvpParticles[i])
		{
			pParticle->Animate(fTimeElapsed);
		}
	}
}

void CParticleShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::OnPrepareRender(pd3dCommandList);

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		m_ppTextures[i]->UpdateShaderVariables(pd3dCommandList); 

		for (const auto& pParticle : m_pvpParticles[i])
		{
			pParticle->Render(pd3dCommandList);
		}
	}
}

void CParticleShader::PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		for (const auto& pParticle : m_pvpParticles[i])
		{
			pParticle->ReadVertexCount(pd3dCommandList);
		}
	}

	if (m_pd3dSOPipelineState) pd3dCommandList->SetPipelineState(m_pd3dSOPipelineState);

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		for (const auto& pParticle : m_pvpParticles[i])
		{
			pParticle->SORender(pd3dCommandList);
		}
	}
}

void CParticleShader::AfterRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		for (const auto& pParticle : m_pvpParticles[i])
		{
			pParticle->AfterRender(pd3dCommandList);
		}
	}
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
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkyBoxShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkyBox", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSkyBox", "ps_5_1", ppd3dShaderBlob));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CUserInterface::CUserInterface()
{

}

CUserInterface::~CUserInterface()
{
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

	if (m_pd3dPipelineStateBar) m_pd3dPipelineStateBar->Release();
	if (m_pd3dPipelineStateMinimap) m_pd3dPipelineStateMinimap->Release();
	if (m_pd3dPipelineStateBullet) m_pd3dPipelineStateBullet->Release();
}

D3D12_SHADER_BYTECODE CUserInterface::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VS_UI", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CUserInterface::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GS_UI", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CUserInterface::CreateGeometryShaderBar(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GS_UI_Bar", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CUserInterface::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS_UI", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CUserInterface::CreatePixelShaderBullet(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS_UI_BULLET", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CUserInterface::CreatePixelShaderMinimap(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS_UI_MINIMAP", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CUserInterface::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE",		0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

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

	d3dPipelineStateDesc.GS = CreateGeometryShaderBar(&pd3dGeometryShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);

	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineStateBar);

	d3dPipelineStateDesc.PS = CreatePixelShaderBullet(&pd3dPixelShaderBlob);

	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineStateBullet);

	d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShaderMinimap(&pd3dPixelShaderBlob);
	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineStateMinimap);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CUserInterface::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_PLAYER_VALUE) + 255) & ~255); //256의 배수

	m_pd3dcbPlayerHP = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbPlayerHP->Map(0, NULL, (void **)&m_pcbMappedPlayerHP);

	m_pd3dcbPlayerBooster = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbPlayerBooster->Map(0, NULL, (void **)&m_pcbMappedPlayerBooster);

	m_pd3dcbPlayerAmmo = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbPlayerAmmo->Map(0, NULL, (void **)&m_pcbMappedPlayerAmmo);

	ncbElementBytes = ((sizeof(CB_MINIMAP_PLAYER_POSITION) + 255) & ~255);
	m_MinimapPlayerRsc = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_MinimapPlayerRsc->Map(0, NULL, (void **)&m_cbMinimapPlayerInfo);
}

void CUserInterface::ReleaseShaderVariables()
{
	if (m_pd3dcbPlayerHP)
	{
		m_pd3dcbPlayerHP->Unmap(0, NULL);
		m_pd3dcbPlayerHP->Release();
	}

	if (m_pd3dcbPlayerBooster)
	{
		m_pd3dcbPlayerBooster->Unmap(0, NULL);
		m_pd3dcbPlayerBooster->Release();
	}

	if (m_pd3dcbPlayerAmmo)
	{
		m_pd3dcbPlayerAmmo->Unmap(0, NULL);
		m_pd3dcbPlayerAmmo->Release();
	}

	if (m_MinimapPlayerRsc)
	{
		m_MinimapPlayerRsc->Unmap(0, NULL);
		m_MinimapPlayerRsc->Release();
	}

	CShader::ReleaseShaderVariables();
}

void CUserInterface::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource *pd3dcb, CB_PLAYER_VALUE *pcbMapped, int nMaxValue, int nValue)
{
	pcbMapped->nMaxValue = nMaxValue;
	pcbMapped->nValue = nValue;

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dcb->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_UI_INFO, d3dGpuVirtualAddress);
}

void CUserInterface::UpdateShaderVariablesMinimapPlayer(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMFLOAT3 playerPos = m_pPlayer->GetPosition();
	XMFLOAT3 playerLook = m_pPlayer->GetLook();
	XMFLOAT3 playerRight = m_pPlayer->GetRight();

	m_cbMinimapPlayerInfo.playerPosition = XMFLOAT2(playerPos.x, playerPos.z);
	m_cbMinimapPlayerInfo.playerLook = XMFLOAT2(playerLook.x, playerLook.z);
	m_cbMinimapPlayerInfo.playerRight = XMFLOAT2(playerRight.x, playerRight.z);

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_PLAYER_INFO, m_MinimapPlayerRsc->GetGPUVirtualAddress());
}

void CUserInterface::ReleaseUploadBuffers()
{
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

	CShader::ReleaseUploadBuffers();
}

void CUserInterface::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_nTextures = 5;
	m_ppTextures = new CTexture*[m_nTextures];

	m_ppTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/UI/Base_UI.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[0], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/UI/UI_HP.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[1], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[2]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/UI/UI_Booster.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[2], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[3] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[3]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/UI/UI_Bullet.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[3], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[4] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[4]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/UI/UI_ScoreBoard.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[4], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_nUIRect = 6;
	m_ppUIRects = new CRect*[m_nUIRect];

	// Base UI
	XMFLOAT2 xmf2Center = ::CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
	XMFLOAT2 xmf2Size = ::CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
	m_ppUIRects[0] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = ::CalculateCenter(-0.375000f, -0.332812f, 0.257778f, -0.257778f);
	xmf2Size = ::CalculateSize(-0.375000f, -0.332812f, 0.257778f, -0.257778f);
	m_ppUIRects[1] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = ::CalculateCenter(0.375000f, 0.417287f, 0.257778f, -0.257778f);
	xmf2Size = ::CalculateSize(0.375000f, 0.417287f, 0.257778f, -0.257778f);
	m_ppUIRects[2] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = ::CalculateCenter(0.417187f, 0.456250f, 0.257778f, -0.257778f);
	xmf2Size = ::CalculateSize(0.417187f, 0.456250f, 0.257778f, -0.257778f);
	m_ppUIRects[3] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = ::CalculateCenter(0.63f, 0.93f, 0.93f, 0.4f);
	xmf2Size = ::CalculateSize(0.63f, 0.93f, 0.93f, 0.4f);
	m_ppUIRects[4] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = ::CalculateCenter(-0.2f, 0.2f, 0.9f, 0.75f);
	xmf2Size = ::CalculateSize(-0.2f, 0.2f, 0.9f, 0.75f);
	m_ppUIRects[5] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	m_pMinimap = (CTexture*)pContext;
}

void CUserInterface::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	// Draw HP BAR
	if (m_pd3dPipelineStateBar) pd3dCommandList->SetPipelineState(m_pd3dPipelineStateBar);
	UpdateShaderVariables(pd3dCommandList, m_pd3dcbPlayerHP, m_pcbMappedPlayerHP, m_pPlayer->GetMaxHitPoint(), m_pPlayer->GetHitPoint());
	if (m_ppTextures[1])
	{
		m_ppTextures[1]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[1]->Render(pd3dCommandList, 0);
	}

	// Draw Booster Gauge BAR
	UpdateShaderVariables(pd3dCommandList, m_pd3dcbPlayerBooster, m_pcbMappedPlayerBooster, 100, m_pPlayer->GetBoosterGauge());
	if (m_ppTextures[2])
	{
		m_ppTextures[2]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[2]->Render(pd3dCommandList, 0);
	}

	// Draw Ammo
	if (m_pd3dPipelineStateBullet) pd3dCommandList->SetPipelineState(m_pd3dPipelineStateBullet);
	CWeapon *pWeapon = m_pPlayer->GetRHWeapon();

	if (pWeapon->GetType() & WEAPON_TYPE_OF_GUN)
	{
		CGun *pRHGun = (CGun*)pWeapon;
		int nMaxReloadAmmo = pRHGun->GetMaxReloadAmmo();
		int nReloadedAmmo = pRHGun->GetReloadedAmmo();

		UpdateShaderVariables(pd3dCommandList, m_pd3dcbPlayerAmmo, m_pcbMappedPlayerAmmo, nMaxReloadAmmo, nReloadedAmmo);

		if (m_ppTextures[3])
		{
			m_ppTextures[3]->UpdateShaderVariables(pd3dCommandList);
			m_ppUIRects[3]->Render(pd3dCommandList, 0);
		}
	}

	UpdateShaderVariablesMinimapPlayer(pd3dCommandList);

	// Draw Minimap
	if (m_pd3dPipelineStateMinimap) pd3dCommandList->SetPipelineState(m_pd3dPipelineStateMinimap);
	if (m_pMinimap)
	{
		m_pMinimap->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[4]->Render(pd3dCommandList, 0);
	}

	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);

	// Draw Score Board
	if (m_ppTextures[5])
	{
		m_ppTextures[4]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[5]->Render(pd3dCommandList, 0);
	}

	// Draw Base UI
	if (m_ppTextures[0])
	{
		m_ppTextures[0]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[0]->Render(pd3dCommandList, 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

CFontShader::CFontShader()
{

}

CFontShader::~CFontShader()
{

}

D3D12_SHADER_BYTECODE CFontShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSFont", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CFontShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSFont", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CFontShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSFont", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CFontShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE",		0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "UVPOSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "UVSIZE",		0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CFontShader::CreateRasterizerState()
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

void CFontShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
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
	d3dPipelineStateDesc.DepthStencilState = CreateAlwaysDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLobbyShader::CLobbyShader()
{

}

CLobbyShader::~CLobbyShader()
{
}

D3D12_SHADER_BYTECODE CLobbyShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VS_UI", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CLobbyShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GS_UI", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CLobbyShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS_UI", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CLobbyShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE",		0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CLobbyShader::CreateRasterizerState()
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

D3D12_DEPTH_STENCIL_DESC CLobbyShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

void CLobbyShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
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

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

///////////////////////////////////////////////////////////////////////////////

CCursorShader::CCursorShader()
{

}

CCursorShader::~CCursorShader()
{

}

D3D12_SHADER_BYTECODE CCursorShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VS_UI", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CCursorShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSCursor", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CCursorShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS_UI", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CCursorShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE",		0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CCursorShader::CreateRasterizerState()
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

void CCursorShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
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
	d3dPipelineStateDesc.DepthStencilState = CreateAlwaysDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}




///////////////////////////////////////////////////////////////////////////////////////////////////

CMinimapRobotShader::CMinimapRobotShader()
{

}

CMinimapRobotShader::~CMinimapRobotShader()
{

}

D3D12_SHADER_BYTECODE CMinimapRobotShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VS_UI", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CMinimapRobotShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSMinimapEnemy", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CMinimapRobotShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS_UI", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CMinimapRobotShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE",		0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CMinimapRobotShader::CreateRasterizerState()
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

D3D12_DEPTH_STENCIL_DESC CMinimapRobotShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

void CMinimapRobotShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
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

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}




///////////////////////////////////////////////////////////////////////////////////////////////////

CMinimapShader::CMinimapShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

CMinimapShader::~CMinimapShader()
{
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

	for (int i = 0; i < m_nMinimapRobotRect; i++)
	{
		if (m_ppMinimapRobotRects[i])
			delete m_ppMinimapRobotRects[i];
	}

}

D3D12_SHADER_BYTECODE CMinimapShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VS_UI", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CMinimapShader::CreateVertexShaderMinimapRobot(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSMinimapEnemy", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CMinimapShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GS_UI", "gs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CMinimapShader::CreateGeometryShaderMinimapRobot(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSMinimapEnemy", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CMinimapShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS_UI", "ps_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CMinimapShader::CreatePixelShaderMinimapRobot(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSMinimapEnemy", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CMinimapShader::CreateInputLayoutMinimapRect()
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE",		0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "INDEX",		0, DXGI_FORMAT_R32_UINT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_INPUT_LAYOUT_DESC CMinimapShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE",		0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CMinimapShader::CreateRasterizerState()
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

void CMinimapShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
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
	d3dPipelineStateDesc.DepthStencilState = CreateAlwaysDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	d3dPipelineStateDesc.VS = CreateVertexShaderMinimapRobot(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.GS = CreateGeometryShaderMinimapRobot(&pd3dGeometryShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShaderMinimapRobot(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.InputLayout = CreateInputLayoutMinimapRect();
	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineStateMinimapRobot);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CMinimapShader::InsertMinimapRobot(CGameObject *object, int index)
{
	InsertMinimapRobotInfo(object->GetWorldTransf(), index);
}

void CMinimapShader::InsertMinimapRobotInfo(XMFLOAT4X4 objectWorld, int index)
{
	if (index % 2 == 1) {
		m_cbMinimapRobotInfo[index].enemyOrTeam = true; // 1: enemy
	}
	else {
		m_cbMinimapRobotInfo[index].enemyOrTeam = false; // 0: team
	}
	m_cbMinimapRobotInfo[index].robotPosition = XMFLOAT2(objectWorld._41, objectWorld._43);
}

void CMinimapShader::ReleaseUploadBuffers()
{
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

	if (m_ppMinimapRobotRects)
	{
		for (int i = 0; i < m_nUIRect; i++)
		{
			if (m_ppMinimapRobotRects[i]) m_ppMinimapRobotRects[i]->ReleaseUploadBuffers();
		}
	}

	CShader::ReleaseUploadBuffers();
}

void CMinimapShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = (((sizeof(CB_MINIMAP_ROBOT_POSITION) * ROBOTCOUNT) + 255) & ~255); //256의 배수

	m_MinimapRobotRsc = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_MinimapRobotRsc->Map(0, NULL, (void **)&m_cbMinimapRobotInfo);
}

void CMinimapShader::UpdateMinimapRobotInfo(CGameObject *object, BYTE id)
{
	// 인덱스 받아서 해당 인덱스의 미니맵인포를 바꾼다
	XMFLOAT4X4 world = object->GetWorldTransf();
	XMFLOAT2 position = XMFLOAT2(world._41, world._43);
	m_cbMinimapRobotInfo[(int)id].robotPosition = position;
}

void CMinimapShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbGpuVirtualAddress = m_MinimapRobotRsc->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_MINIMAP_ROBOT_INFO, d3dcbGpuVirtualAddress);
}

void CMinimapShader::ReleaseShaderVariables()
{
	if (m_MinimapRobotRsc)
	{
		m_MinimapRobotRsc->Unmap(0, NULL);
		m_MinimapRobotRsc->Release();
	}

	CShader::ReleaseShaderVariables();
}

void CMinimapShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_nTextures = 2;
	m_ppTextures = new CTexture*[m_nTextures];

	m_ppTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ppTextures[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/UI/Minimap/Radar_BG.dds", 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[0], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_ppTextures[1] = new CTexture(2, RESOURCE_TEXTURE2D_ARRAY, 0);
	m_ppTextures[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/UI/Minimap/Enemy_Icon.dds", 0);
	m_ppTextures[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/UI/Minimap/Team_Icon.dds", 1);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ppTextures[1], ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_nUIRect = 1;
	m_ppUIRects = new CRect*[m_nUIRect];

	// Minimap Terrain
	XMFLOAT2 xmf2Center = ::CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
	XMFLOAT2 xmf2Size = ::CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
	m_ppUIRects[0] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	m_nMinimapRobotRect = 8;
	m_ppMinimapRobotRects = new CMinimapRobotRect*[m_nMinimapRobotRect];

	xmf2Center = ::CalculateCenter(-0.1f, 0.1f, 0.1f, -0.1f);
	xmf2Size = ::CalculateSize(-0.1f, 0.1f, 0.1f, -0.1f);
	for (int i = 0; i < ROBOTCOUNT; i++) {
		m_ppMinimapRobotRects[i] = new CMinimapRobotRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
	}
}

void CMinimapShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	// Draw Minimap Terrain
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	if (m_ppTextures[0])
	{
		m_ppTextures[0]->UpdateShaderVariables(pd3dCommandList);
		m_ppUIRects[0]->Render(pd3dCommandList, 0);
	}

	// Draw Minimap Robots
	UpdateShaderVariables(pd3dCommandList);
	if (m_pd3dPipelineStateMinimapRobot) pd3dCommandList->SetPipelineState(m_pd3dPipelineStateMinimapRobot);
	m_ppTextures[1]->UpdateShaderVariables(pd3dCommandList);
	for (int i = 0; i < ROBOTCOUNT; i++) {
		if (m_ppTextures[i]) {
			m_ppMinimapRobotRects[i]->Render(pd3dCommandList, 0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
