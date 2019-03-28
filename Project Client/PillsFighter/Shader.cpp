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

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
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
}

void CObjectsShader::ReleaseObjects()
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

void CObjectsShader::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	CheckDeleteObjects();

	for (const auto& Object : m_vObjects)
	{
		Object->Animate(fTimeElapsed, pCamera);
	}
}

void CObjectsShader::CheckDeleteObjects()
{
	if (m_vObjects.size())
	{
		for (auto& Object = m_vObjects.begin(); Object != m_vObjects.end();)
		{
			if ((*Object)->IsDelete())
			{
				delete *Object;

				Object = m_vObjects.erase(Object);
			}
			else
				Object++;
		}
	}
}

void CObjectsShader::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* pObject, bool bPrepareRotate, void *pContext)
{
	pObject->SetModel(m_pModel);
	pObject->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	if(bPrepareRotate) pObject->AddPrepareRotate(0, 180, 0);

	m_vObjects.emplace_back(pObject);
}

void CObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	for (const auto& Object : m_vObjects)
	{
		Object->Render(pd3dCommandList, pCamera);
	}
}

void CObjectsShader::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	for (const auto& Object : m_vObjects)
	{
		Object->RenderWire(pd3dCommandList, pCamera);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CSkinnedObjectsShader::CSkinnedObjectsShader()
{

}

CSkinnedObjectsShader::~CSkinnedObjectsShader()
{
}

void CSkinnedObjectsShader::ReleaseObjects()
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

void CSkinnedObjectsShader::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	CheckDeleteObjects();

	for (const auto& Object : m_vObjects)
	{
		Object->Animate(fTimeElapsed, pCamera);
	}
}

void CSkinnedObjectsShader::CheckDeleteObjects()
{
	if (m_vObjects.size())
	{
		for (auto& Object = m_vObjects.begin(); Object != m_vObjects.end();)
		{
			if ((*Object)->IsDelete())
			{
				delete *Object;

				Object = m_vObjects.erase(Object);
			}
			else
				Object++;
		}
	}
}

void CSkinnedObjectsShader::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* pObject, bool bPrepareRotate, void *pContext)
{
	pObject->SetModel(m_pModel);
	pObject->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	if(bPrepareRotate) pObject->AddPrepareRotate(0, 180, 0);

	m_vObjects.emplace_back(pObject);
}

void CSkinnedObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CSkinnedAnimationShader::Render(pd3dCommandList, pCamera);

	for (const auto& Object : m_vObjects)
	{
		Object->Render(pd3dCommandList, pCamera);
	}
}

void CSkinnedObjectsShader::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	for (const auto& Object : m_vObjects)
	{
		Object->RenderWire(pd3dCommandList, pCamera);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CGundamShader::CGundamShader()
{
}

CGundamShader::~CGundamShader()
{
}

void CGundamShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/GM/GM.bin", true);

	m_pGimGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/GIM_GUN.bin", false);
	m_pBazooka = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/BZK.bin", false);
	m_pMachineGun = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Weapon/MACHINEGUN.bin", false);

#ifndef ON_NETWORKING
	CRobotObject *pObject = new CRobotObject();
	pObject->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	InsertObject(pd3dDevice, pd3dCommandList, pObject, true, pContext);
#endif
}

void CGundamShader::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject* pObject, bool bPrepareRotate, void *pContext)
{
	CSkinnedObjectsShader::InsertObject(pd3dDevice, pd3dCommandList, pObject, bPrepareRotate, pContext);

	CAnimationController *pAnimationController = new CAnimationController(1, pObject->GetModel()->GetAnimationSet());
	pAnimationController->SetTrackAnimation(0, 0);
	pObject->SetAnimationController(pAnimationController);

	CRobotObject *pRobot = (CRobotObject*)pObject;

	pRobot->AddWeapon(pd3dDevice, pd3dCommandList, m_pGimGun, WEAPON_TYPE_OF_GUN | WEAPON_TYPE_OF_GIM_GUN);
	pRobot->AddWeapon(pd3dDevice, pd3dCommandList, m_pBazooka, WEAPON_TYPE_OF_GUN | WEAPON_TYPE_OF_BAZOOKA);
	pRobot->AddWeapon(pd3dDevice, pd3dCommandList, m_pMachineGun, WEAPON_TYPE_OF_GUN | WEAPON_TYPE_OF_MACHINEGUN);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CBulletShader::CBulletShader()
{
}

CBulletShader::~CBulletShader()
{
}

void CBulletShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Bullet/Bullet.bin", false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CBZKBulletShader::CBZKBulletShader()
{
}

CBZKBulletShader::~CBZKBulletShader()
{
}

void CBZKBulletShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Bullet/BZK_Bullet.bin", false);
}

////////////////////////////////////////////////////////////

CRepairItemShader::CRepairItemShader()
{
}

CRepairItemShader::~CRepairItemShader()
{
}

void CRepairItemShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Item/Item_Repair.bin", false);

	RotateObject *pObject = new RotateObject();
	pObject->SetPosition(XMFLOAT3(0.0f, 20.0f, 0.0f));
	
	InsertObject(pd3dDevice, pd3dCommandList, pObject, true, pContext);
}

////////////////////////////////////////////////////////////

CAmmoItemShader::CAmmoItemShader()
{
}

CAmmoItemShader::~CAmmoItemShader()
{
}

void CAmmoItemShader::Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Item/AMMO_BOX.bin", false);
}

////////////////////////////////////////////////////////////

CObstacleShader::CObstacleShader()
{
}

CObstacleShader::~CObstacleShader()
{
}

void CObstacleShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Hangar.bin", false);
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveHangarSelfData.bin");

	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Double_Square.bin", false);
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveDoubleSquareSelfData.bin");

	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Octagon.bin", false);
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveOctagonSelfData.bin");

	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_OctagonLongTier.bin", false);
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveOctagonLongtierSelfData.bin");

	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Slope_top.bin", false);
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveSlopeTopSelfData.bin");

	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Square.bin", false);
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveSquareSelfData.bin");

	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Building_Steeple_top.bin", false);
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/SaveSteepleTopSelfData.bin");

	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Buildings/Wall.bin", false);
	InsertObjectFromLoadInfFromBin(pd3dDevice, pd3dCommandList, "./Resource/Buildings/WallSelfData.bin");

}

void CObstacleShader::InsertObjectFromLoadInfFromBin(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName)
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
				InsertObject(pd3dDevice, pd3dCommandList, pObject, true);
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

//////////////////////////////////////////////////////////////////////

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
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
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

D3D12_DEPTH_STENCIL_DESC CEffectShader::CreateSODepthStencilState()
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

void CEffectShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_d3dPipelineStateDesc;

	ID3DBlob *pd3dVertexBlob, *pd3dPixelBlob, *pd3dGeometryBlob;

	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexBlob);
	m_d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelBlob);
	m_d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryBlob);
	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	m_d3dPipelineStateDesc.NumRenderTargets = 1;
	m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	m_d3dPipelineStateDesc.PS.BytecodeLength = 0;
	m_d3dPipelineStateDesc.PS.pShaderBytecode = 0;
	m_d3dPipelineStateDesc.VS = CreateSOVertexShader(&pd3dVertexBlob);
	m_d3dPipelineStateDesc.GS = CreateSOGeometryShader(&pd3dGeometryBlob);
	m_d3dPipelineStateDesc.StreamOutput = CreateStreamOutput();
	m_d3dPipelineStateDesc.DepthStencilState = CreateSODepthStencilState();

	hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dSOPipelineState);

	if (pd3dVertexBlob) pd3dVertexBlob->Release();
	if (pd3dPixelBlob) pd3dPixelBlob->Release();
	if (pd3dGeometryBlob) pd3dGeometryBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CEffectShader::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

void CEffectShader::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	if (m_pEffect) m_pEffect->Animate(fTimeElapsed);
}

void CEffectShader::ReleaseObjects()
{
	if (m_pEffect) delete m_pEffect;
	if (m_pTexture) delete m_pTexture;
}

void CEffectShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::OnPrepareRender(pd3dCommandList);

	if (m_pEffect) m_pEffect->ReadVertexCount(pd3dCommandList);

	if (m_pd3dSOPipelineState) pd3dCommandList->SetPipelineState(m_pd3dSOPipelineState);
	if (m_pEffect) m_pEffect->SORender(pd3dCommandList);

	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);

	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	if (m_pEffect) m_pEffect->Render(pd3dCommandList);

	if (m_pEffect) m_pEffect->AfterRender(pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////

CFadeOutShader::CFadeOutShader()
{
}

CFadeOutShader::~CFadeOutShader()
{
}

D3D12_INPUT_LAYOUT_DESC CFadeOutShader::CreateInputLayout()
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_STREAM_OUTPUT_DESC CFadeOutShader::CreateStreamOutput()
{
	UINT nSODecls = 3;
	D3D12_SO_DECLARATION_ENTRY *pd3dStreamOutputDeclarations = new D3D12_SO_DECLARATION_ENTRY[nSODecls];

	pd3dStreamOutputDeclarations[0] = { 0, "POSITION", 0, 0, 3, 0 };
	pd3dStreamOutputDeclarations[1] = { 0, "SIZE", 0, 0, 2, 0 };
	pd3dStreamOutputDeclarations[2] = { 0, "AGE", 0, 0, 1, 0 };

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

D3D12_SHADER_BYTECODE CFadeOutShader::CreateSOVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSEffectStreamOut", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CFadeOutShader::CreateSOGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSEffectStreamOut", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CFadeOutShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSEffectDraw", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CFadeOutShader::CreateGeometryShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSEffectDraw", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CFadeOutShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSEffectDraw", "ps_5_1", ppd3dShaderBlob));
}

void CFadeOutShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/HIT.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_pEffect = new CFadeOut(pd3dDevice, pd3dCommandList, 2.0f);
	m_pEffect->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CFadeOutShader::InsertEffect(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size)
{
	((CFadeOut*)m_pEffect)->AddVertex(xmf3Position, xmf2Size);
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CHitSpriteShader::CHitSpriteShader()
{

}

CHitSpriteShader::~CHitSpriteShader()
{

}

void CHitSpriteShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_pTexture = new CTexture(2, RESOURCE_TEXTURE2D_ARRAY, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Attack1.dds", 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Attack2.dds", 1);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_pEffect = new CSprite(pd3dDevice, pd3dCommandList, 5, 2, 6, 0.2f);
	m_pEffect->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CHitSpriteShader::InsertEffect(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, EFFECT_ANIMATION_TYPE nEffectAniType)
{
	((CSprite*)m_pEffect)->AddVertex(xmf3Position, xmf2Size, rand() % 2, nEffectAniType);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CExpSpriteShader::CExpSpriteShader()
{

}

CExpSpriteShader::~CExpSpriteShader()
{

}

void CExpSpriteShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Explosion.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);

	m_pEffect = new CSprite(pd3dDevice, pd3dCommandList, 5, 3, 12, 1.0f);
	m_pEffect->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CExpSpriteShader::InsertEffect(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, EFFECT_ANIMATION_TYPE nEffectAniType)
{
	((CSprite*)m_pEffect)->AddVertex(xmf3Position, xmf2Size, 0, nEffectAniType);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CParticleShader::CParticleShader()
{

}

CParticleShader::~CParticleShader()
{
	for (CParticle* pParticle : m_vpParticles)
	{
		pParticle->ReleaseShaderVariables();
		delete pParticle;
	}

	if (m_pTexture) delete m_pTexture;
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

D3D12_BLEND_DESC CParticleShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
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

D3D12_DEPTH_STENCIL_DESC CParticleShader::CreateSODepthStencilState()
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

void CParticleShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_d3dPipelineStateDesc;

	ID3DBlob *pd3dVertexBlob, *pd3dPixelBlob, *pd3dGeometryBlob;

	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexBlob);
	m_d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelBlob);
	m_d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryBlob);
	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	m_d3dPipelineStateDesc.NumRenderTargets = 1;
	m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	m_d3dPipelineStateDesc.PS.BytecodeLength = 0;
	m_d3dPipelineStateDesc.PS.pShaderBytecode = 0;
	m_d3dPipelineStateDesc.VS = CreateSOVertexShader(&pd3dVertexBlob);
	m_d3dPipelineStateDesc.GS = CreateSOGeometryShader(&pd3dGeometryBlob);
	m_d3dPipelineStateDesc.StreamOutput = CreateStreamOutput();
	m_d3dPipelineStateDesc.DepthStencilState = CreateSODepthStencilState();

	hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dSOPipelineState);

	if (pd3dVertexBlob) pd3dVertexBlob->Release();
	if (pd3dPixelBlob) pd3dPixelBlob->Release();
	if (m_pd3dGeometryShaderBlob) m_pd3dGeometryShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CParticleShader::ReleaseUploadBuffers()
{
	for (CParticle* pParticle : m_vpParticles)
	{
		pParticle->ReleaseUploadBuffers();
	}

	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

void CParticleShader::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository, void *pContext)
{
	CParticle *pParticle = NULL;

	for (int i = 0; i < 8 * 2; i++)
	{
		pParticle = new CParticle(pd3dDevice, pd3dCommandList);
		pParticle->Initialize(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 40.0f, 1.0f);
		pParticle->CreateShaderVariables(pd3dDevice, pd3dCommandList);

		m_vpParticles.emplace_back(pParticle);
	}

	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/flare.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);
}

void CParticleShader::SetFollowObject(CGameObject *pObject, CModel *pFrame)
{
	m_vpParticles[m_nParticleIndex++]->SetFollowObject(pObject, pFrame);
}

void CParticleShader::CheckDeleteObjects()
{
	if (m_vpParticles.size())
	{
		for (auto& Object = m_vpParticles.begin(); Object != m_vpParticles.end();)
		{
			if ((*Object)->IsDelete())
			{
				delete *Object;

				m_nParticleIndex--;
				Object = m_vpParticles.erase(Object);
			}
			else
				Object++;
		}
	}
}
void CParticleShader::AnimateObjects(float fTimeElapsed)
{
	CheckDeleteObjects();

	for(int i = 0; i < m_nParticleIndex; i++)
	{
		m_vpParticles[i]->Animate(fTimeElapsed);
	}
}

void CParticleShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	for (int i = 0; i < m_nParticleIndex; i++) m_vpParticles[i]->ReadVertexCount(pd3dCommandList);

	if (m_pd3dSOPipelineState) pd3dCommandList->SetPipelineState(m_pd3dSOPipelineState);
	for (int i = 0; i < m_nParticleIndex; i++) m_vpParticles[i]->SORender(pd3dCommandList);

	m_pTexture->UpdateShaderVariables(pd3dCommandList);
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	for (int i = 0; i < m_nParticleIndex; i++) m_vpParticles[i]->Render(pd3dCommandList);

	for (int i = 0; i < m_nParticleIndex; i++) m_vpParticles[i]->AfterRender(pd3dCommandList);
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

	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineStateBar);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
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

void CUserInterface::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_PLAYER_VALUE) + 255) & ~255); //256ÀÇ ¹è¼ö

	m_pd3dcbPlayerHP = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbPlayerHP->Map(0, NULL, (void **)&m_pcbMappedPlayerHP);


	m_pd3dcbPlayerBooster = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbPlayerBooster->Map(0, NULL, (void **)&m_pcbMappedPlayerBooster);


	m_pd3dcbPlayerAmmo = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbPlayerAmmo->Map(0, NULL, (void **)&m_pcbMappedPlayerAmmo);
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


	CShader::ReleaseShaderVariables();
}

void CUserInterface::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource *pd3dcb, CB_PLAYER_VALUE *pcbMapped, int nMaxValue, int nValue)
{
	pcbMapped->nMaxValue = nMaxValue;
	pcbMapped->nValue = nValue;

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dcb->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_UI_INFO, d3dGpuVirtualAddress);
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

	m_nTextures = 4;
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

	m_nUIRect = 4;
	m_ppUIRects = new CRect*[m_nUIRect];

	// Base UI
	XMFLOAT2 xmf2Center = CalculateCenter(-1.0f, 1.0f, 1.0f, -1.0f);
	XMFLOAT2 xmf2Size = CalculateSize(-1.0f, 1.0f, 1.0f, -1.0f);
	m_ppUIRects[0] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = CalculateCenter(-0.375000f, -0.332812f, 0.257778f, -0.257778f);
	xmf2Size = CalculateSize(-0.375000f, -0.332812f, 0.257778f, -0.257778f);
	m_ppUIRects[1] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = CalculateCenter(0.375000f, 0.417287f, 0.257778f, -0.257778f);
	xmf2Size = CalculateSize(0.375000f, 0.417287f, 0.257778f, -0.257778f);
	m_ppUIRects[2] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);

	xmf2Center = CalculateCenter(0.417187f, 0.456250f, 0.257778f, -0.257778f);
	xmf2Size = CalculateSize(0.417187f, 0.456250f, 0.257778f, -0.257778f);

	m_ppUIRects[3] = new CRect(pd3dDevice, pd3dCommandList, xmf2Center, xmf2Size);
}

void CUserInterface::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	// Draw HP BAR
	if (m_pd3dPipelineStateBar) pd3dCommandList->SetPipelineState(m_pd3dPipelineStateBar);
	UpdateShaderVariables(pd3dCommandList, m_pd3dcbPlayerHP, m_pcbMappedPlayerHP, m_pPlayer->GetMaxHitPoint(), m_pPlayer->GetHitPoint());
	if (m_ppTextures[1]) m_ppTextures[1]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[1]->Render(pd3dCommandList, 0);

	// Draw Booster Gauge BAR
	UpdateShaderVariables(pd3dCommandList, m_pd3dcbPlayerBooster, m_pcbMappedPlayerBooster, 100, m_pPlayer->GetBoosterGauge());
	if (m_ppTextures[2]) m_ppTextures[2]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[2]->Render(pd3dCommandList, 0);

	// Draw Ammo
	CGun *pRHGun = (CGun*)m_pPlayer->GetRHWeapon();
	int nMaxReloadAmmo = pRHGun->GetMaxReloadAmmo();
	int nReloadedAmmo = pRHGun->GetReloadedAmmo();

	UpdateShaderVariables(pd3dCommandList, m_pd3dcbPlayerAmmo, m_pcbMappedPlayerAmmo, nMaxReloadAmmo, nReloadedAmmo);
	if (m_ppTextures[3]) m_ppTextures[3]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[3]->Render(pd3dCommandList, 0);
	
	// Draw Base UI
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	if (m_ppTextures[0]) m_ppTextures[0]->UpdateShaderVariables(pd3dCommandList);
	m_ppUIRects[0]->Render(pd3dCommandList, 0);
}
