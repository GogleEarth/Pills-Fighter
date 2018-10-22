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
	pd3dDescriptorRanges[0].BaseShaderRegister = 1; //t1: gtxtTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[4];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //World
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; // Texture
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[3].Descriptor.ShaderRegister = 0; //t0: gGameObjectInfo
	pd3dRootParameters[3].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[1];

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

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
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

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	//그래픽 루트 시그너쳐를 생성한다. 
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	// [0] Building, [1] Bullet, [2] Enemy
	//m_nShaders = 3;
	m_nShaders = 3;
	m_ppShaders = new CObjectsShader*[m_nShaders];

	/////////////////////////////// Enemy Shader

	////// Building Shader
	CBuildingShader *pBuildingShader = new CBuildingShader();
	pBuildingShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pBuildingShader->Initialize(pd3dDevice, pd3dCommandList, NULL);

	m_ppShaders[0] = pBuildingShader;

	////// Bullet Shader
	CBulletShader *pBulletShader = new CBulletShader();
	pBulletShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pBulletShader->Initialize(pd3dDevice, pd3dCommandList, NULL);

	m_ppShaders[1] = pBulletShader;

	CGundamShader *pGundamhader = new CGundamShader();
	pGundamhader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pGundamhader->Initialize(pd3dDevice, pd3dCommandList, NULL);

	m_ppShaders[2] = pGundamhader;

	// Terrain
	//m_pTerrain = new CRectTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	//m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);
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

	if (m_pTerrain)
	{
		m_pTerrain->ReleaseShaderVariables();
		delete m_pTerrain;
	}

	ReleaseShaderVariables();
}

void CScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();

	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
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

void CScene::CheckCollision()
{
	CGameObject	**ppBuildingObjects;
	UINT nBuildingObjects;

	std::vector<CGameObject*> vEnemyObjects;
	std::vector<CGameObject*> vBulletObjects;

	m_ppShaders[0]->GetObjects(NULL, &ppBuildingObjects, &nBuildingObjects);
	m_ppShaders[1]->GetObjects(&vBulletObjects, NULL, NULL);
	m_ppShaders[2]->GetObjects(&vEnemyObjects, NULL, NULL);

	for (const auto& Bullet : vBulletObjects)
	{
		for (UINT nIndexBuilding = 0; nIndexBuilding < nBuildingObjects; nIndexBuilding++)
		{
			for (UINT i = 0; i < ppBuildingObjects[nIndexBuilding]->GetNumMeshes(); i++)
			{
				for (UINT j = 0; j < Bullet->GetNumMeshes(); j++)
				{
					if (Bullet->GetOOBB(j).Intersects(ppBuildingObjects[nIndexBuilding]->GetOOBB(i)))
					{
						Bullet->DeleteObject();
					}
				}
			}
		}

		for (const auto& Enemy : vEnemyObjects)
		{
			for (UINT i = 0; i < Enemy->GetNumMeshes(); i++)
			{
				for (UINT j = 0; j < Bullet->GetNumMeshes(); j++)
				{
					if (Bullet->GetOOBB(j).Intersects(Enemy->GetOOBB(i)))
					{
						Bullet->DeleteObject();
					}
				}
			}
		}
	}

	

	for (const auto& Enemy : vEnemyObjects)
	{
		for (UINT nIndexBuilding = 0; nIndexBuilding < nBuildingObjects; nIndexBuilding++)
		{
			for (UINT i = 0; i < ppBuildingObjects[nIndexBuilding]->GetNumMeshes(); i++)
			{
				for (UINT j = 0; j < Enemy->GetNumMeshes(); j++)
				{
					BoundingOrientedBox EnemysOOBB = Enemy->GetOOBB(j);
					BoundingOrientedBox BuildingsOOBB = ppBuildingObjects[nIndexBuilding]->GetOOBB(i);

					if (EnemysOOBB.Intersects(BuildingsOOBB))
					{
						XMFLOAT3 BuildingMeshCenter = BuildingsOOBB.Center;
						XMFLOAT3 BuildingMeshExtents = BuildingsOOBB.Extents;

						XMFLOAT3 EnemyMeshCenter = EnemysOOBB.Center;
						XMFLOAT3 EnemyMeshExtents = EnemysOOBB.Extents;

						//if(EnemyPosition.x > BuildingMeshExtents.x + BuildingMeshCenter.x)
						// 충돌처리
					}
				}
			}
		}
	}
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	}

	CheckCollision();
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

	if(m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);
}

void CScene::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->RenderWire(pd3dCommandList, pCamera);
	}
}