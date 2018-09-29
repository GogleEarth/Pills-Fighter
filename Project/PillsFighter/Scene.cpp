#include "stdafx.h"
#include "Scene.h"


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

// 1 //
ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: gtxtTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //World
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	//그래픽 루트 시그너쳐를 생성한다. 
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	// [0] Enemy, [1] PlayerBullet
	m_nShaders = 2;
	m_ppShaders = new CObjectsShader*[m_nShaders];

	// Enemy Shader
	CObjectsShader *pObjectShader = new CObjectsShader();
	pObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pObjectShader->Initialize(pd3dDevice, pd3dCommandList, L"./Resource/GM/Head/Head.dds", "./Resource/GM/Head/Head.FBX", NULL);

	//CMesh** ppMeshes;
	//::CreateRobotObjectMesh(pd3dDevice, pd3dCommandList, ppMeshes, &pObjectShader);
	//pObjectShader->SetMesh(ppMeshes, 7);

	//CTexture** ppTextures;
	//::CreateRobotObjectTexture(pd3dDevice, pd3dCommandList, ppTextures);

	//::CreateRobotObjectShader(pd3dDevice, pd3dCommandList, ppTextures, pObjectShader);

	//CMaterial** ppMaterial = new CMaterial*[7];
	//for (int i = 0; i < 7; i++)
	//{
	//	ppMaterial[i] = new CMaterial();
	//	ppMaterial[i]->SetTexture(ppTextures[i]);
	//}
	//pObjectShader->SetMaterial(ppMaterial, 7);

	m_ppShaders[0] = pObjectShader;

	RandomMoveObject *pGameObject = NULL;

	//pGameObject = new RandomMoveObject(7, 7);
	pGameObject = new RandomMoveObject();
	pGameObject->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pGameObject->SetPrepareRotate(-90.0f, 0.0f, 0.0f);
	pGameObject->SetPosition(0.0f, 0.0f, 0.0f);
	m_ppShaders[0]->InsertObject(pGameObject);

	////// Bullet Shader
	pObjectShader = new CObjectsShader();
	pObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pObjectShader->Initialize(pd3dDevice, pd3dCommandList, L"./Resource/bullet.dds", "./Resource/bullet.fbx", NULL);

	m_ppShaders[1] = pObjectShader;
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			delete m_ppShaders[i];
		}
		delete[] m_ppShaders;
	}

	ReleaseShaderVariables();
}

void CScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
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

// 충돌체크 함수
void CScene::CheckCollision()
{
	for (const auto& Bullet : (m_ppShaders[1])->GetObjects())
	{
		for (const auto& Object : m_ppShaders[0]->GetObjects())
		{
			if (Object->GetOOBB().Intersects(Bullet->GetOOBB()))
			{
				Bullet->DeleteObject();
			}			
			//if (Object->GetAABB().Intersects(Bullet->GetAABB()))
			//{
			//	Bullet->DeleteObject();
			//}
		}
	}
}

void CScene::CheckDeleteObject()
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->CheckDeleteObjects();
	}
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	}

	CheckCollision();
	CheckDeleteObject();
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (pCamera) 
	{
		pCamera->SetViewportsAndScissorRects(pd3dCommandList);
		pCamera->UpdateShaderVariables(pd3dCommandList);
	}

	UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	}

	COORD pos;
	pos.X = 0;
	pos.Y = 0;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

	std::cout << m_HitCount << std::endl;
}