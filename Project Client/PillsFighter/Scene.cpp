#include "stdafx.h"
#include "Scene.h"
#include "Repository.h"

CScene::CScene()
{
	CONSOLE_CURSOR_INFO C;
	C.bVisible = 0;
	C.dwSize = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &C);

	for (int i = 0; i < MAX_NUM_OBJECT; i++)
		m_pObjects[i] = NULL;
}


CScene::~CScene()
{
}

// 1 //
ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[4];

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
	pd3dRootParameters[ROOT_PARAMETER_INDEX_LIGHTS].Descriptor.ShaderRegister = 3; //Lights
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
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].Descriptor.ShaderRegister = 4; //HP
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_UI_INFO].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_SPRITE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_SPRITE].Constants.Num32BitValues = 4;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_SPRITE].Constants.ShaderRegister = 5; //TextureSprite
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_SPRITE].Constants.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_INDEX_TEXTURE_SPRITE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
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

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository)
{
	//그래픽 루트 시그너쳐를 생성한다. 
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	BuildLightsAndMaterials();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// [0] Building, [1] Bullet, [2] Enemy, [3] Effect
	m_nShaders = 4;
	m_ppShaders = new CObjectsShader*[m_nShaders];

	/////////////////////////////// Enemy Shader

	////// Building Shader
	CBuildingShader *pBuildingShader = new CBuildingShader();
	pBuildingShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pBuildingShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);

	m_ppShaders[INDEX_SHADER_OBSTACLE] = pBuildingShader;
	//m_ppShaders[INDEX_SHADER_OBSTACLE] = NULL;

	//// Enemy Shader
	CGundamShader *pGundamhader = new CGundamShader();
	pGundamhader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pGundamhader->Initialize(pd3dDevice, pd3dCommandList, pRepository);

	m_ppShaders[INDEX_SHADER_ENEMY] = pGundamhader;
	//m_ppShaders[INDEX_SHADER_ENEMY] = NULL;

	//// Bullet Shader
	CBulletShader *pBulletShader = new CBulletShader();
	pBulletShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pBulletShader->Initialize(pd3dDevice, pd3dCommandList, pRepository);

	m_ppShaders[INDEX_SHADER_BULLET] = pBulletShader;
	//m_ppShaders[INDEX_SHADER_BULLET] = NULL;

	// Effect Shader
	CEffectShader *pEffectShader = new CEffectShader();
	pEffectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pEffectShader->Initialize(pd3dDevice, pd3dCommandList, NULL);

	m_ppShaders[INDEX_SHADER_EFFECT] = pEffectShader;
	//m_ppShaders[INDEX_SHADER_EFFECT] = NULL;

	XMFLOAT3 xmf3Scale(4.0f, 1.0f, 4.0f);
	XMFLOAT4 xmf4Color(1.f, 1.f, 1.f, 1.0f);

	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("./Resource/HeightMap.raw"), 257, 257, 257, 257, xmf3Scale, xmf4Color);
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_pLights) delete m_pLights;
	if (m_pSkyBox) delete m_pSkyBox;
	if (m_pTerrain) delete m_pTerrain;

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
	}

	ReleaseShaderVariables();
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) if(m_ppShaders[i]) m_ppShaders[i]->ReleaseUploadBuffers();
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
	std::vector<CGameObject*> *vEnemys;
	std::vector<CGameObject*> *vBullets;

	vEnemys = m_ppShaders[INDEX_SHADER_ENEMY]->GetObjects();
	vBullets = m_ppShaders[INDEX_SHADER_BULLET]->GetObjects();

	for (const auto& Enemy : *vEnemys)
	{
		if(m_pPlayer->GetOOBB().Intersects(Enemy->GetOOBB()))
			std::cout << "Collision By Enemy\n" << std::endl;

		for (const auto& Bullet : *vBullets)
		{
			if (Bullet->GetOOBB().Intersects(Enemy->GetOOBB()))
				std::cout << "Collision Enemy By Bullet\n" << std::endl;
		}
	}
}

void CScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		if(m_ppShaders[i])
			m_ppShaders[i]->AnimateObjects(fTimeElapsed, pCamera);
	}

	CheckCollision();
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	SetGraphicsRootSignature(pd3dCommandList);

	if (pCamera)
	{
		pCamera->SetViewportsAndScissorRects(pd3dCommandList);
		pCamera->UpdateShaderVariables(pd3dCommandList);
	}

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_LIGHTS, d3dcbLightsGpuVirtualAddress);

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nShaders; i++)
	{
		if(m_ppShaders[i])
			m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	}
}

void CScene::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		if(m_ppShaders[i])
			m_ppShaders[i]->RenderWire(pd3dCommandList, pCamera);
	}
}

void CScene::SetPlayer(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
}

void CScene::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = false;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 100.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(130.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_bEnable = false;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 50.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
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
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255);
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

//////////////////////////////// for Networking

void CScene::InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT CreateObjectInfo)
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
		pGameObject = new CGameObject();
		pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);

		((CObjectsShader*)m_ppShaders[INDEX_SHADER_ENEMY])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject);
		break;
	case OBJECT_TYPE_BULLET:
		pGameObject = new Bullet();
		pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);

		((CObjectsShader*)m_ppShaders[INDEX_SHADER_BULLET])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject);
		break;
	case OBJECT_TYPE_OBSTACLE:
		((CObjectsShader*)m_ppShaders[INDEX_SHADER_OBSTACLE])->InsertObject(pd3dDevice, pd3dCommandList, pGameObject);
		break;
	}

	m_pObjects[CreateObjectInfo.Object_Index] = pGameObject;
}

void CScene::DeleteObject(PKT_DELETE_OBJECT DeleteObjectInfo)
{
	if (m_pObjects[DeleteObjectInfo.Object_Index])
	{
		m_pObjects[DeleteObjectInfo.Object_Index]->Delete();
		m_pObjects[DeleteObjectInfo.Object_Index] = NULL;
	}
}

void CScene::CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT CreateEffectInfo)
{
	CEffect *pEffect = new CEffect();
	pEffect->SetPosition(CreateEffectInfo.xmf3Position);
	EFFECT_TYPE efType = CreateEffectInfo.efType;

	((CObjectsShader*)m_ppShaders[INDEX_SHADER_EFFECT])->InsertObject(pd3dDevice, pd3dCommandList, pEffect, &efType);
}

void CScene::ApplyRecvInfo(PKT_ID pktID, LPVOID pktData)
{
	switch (pktID)
	{
	case PKT_ID_PLAYER_INFO:
		m_pObjects[((PKT_PLAYER_INFO*)pktData)->ID]->SetWorldTransf(((PKT_PLAYER_INFO*)pktData)->WorldMatrix);
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
		position.y += 10;
		m_pObjects[((PKT_UPDATE_OBJECT*)pktData)->Object_Index]->SetPosition(position);
		break;
	case PKT_ID_CREATE_EFFECT:
		break;
	}
}