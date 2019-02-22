#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"
#include "Scene.h"
#include "Animation.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CGameObject::CGameObject()
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	serverPosition = XMFLOAT3(0, 0, 0);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	SetPrepareRotate(0.0f, 0.0f, 0.0f);
}

CGameObject::~CGameObject()
{
	ReleaseShaderVariables();

	if (m_pModel) m_pModel->Release();
}

void CGameObject::SetModel(CModel *pModel)
{
	if(m_pModel) m_pModel->Release();

	if (pModel)
	{
		m_pModel = pModel;
		m_pModel->AddRef();

		m_vxmAABB.empty();

		m_pModel->GetSkinnedMeshes(m_vSkinnedMeshes);

		int nStandardMeshes = pModel->GetMeshes();
		m_nSkinnedMeshes = pModel->GetSkinnedMeshes();
		m_nMeshes = nStandardMeshes + m_nSkinnedMeshes;

		for (int i = 0; i < m_nMeshes; i++) m_vxmAABB.emplace_back(BoundingBox());

		OnPrepareAnimate();
	}
}

void CGameObject::SetMesh(CMesh *pMesh, CCubeMesh *pCubeMesh)
{
	if (!m_pModel)
	{
		CModel *pModel = new CModel();
		pModel->SetMesh(pMesh, pCubeMesh, false);

		SetModel(pModel);
	}
	else
		m_pModel->SetMesh(pMesh, pCubeMesh, false);
}

void CGameObject::SetMaterial(CMaterial **ppMaterials, UINT nMaterials)
{
	if (!m_pModel) m_pModel = new CModel();

	m_pModel->SetMaterial(ppMaterials, nMaterials);
}

void CGameObject::SetWorldTransf(XMFLOAT4X4& xmf4x4World)
{ 
	m_xmf3Right = XMFLOAT3(xmf4x4World._11, xmf4x4World._12, xmf4x4World._13);
	m_xmf3Up = XMFLOAT3(xmf4x4World._21, xmf4x4World._22, xmf4x4World._23);
	m_xmf3Look = XMFLOAT3(xmf4x4World._31, xmf4x4World._32, xmf4x4World._33);
	m_xmf3Position = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	serverPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
}

XMFLOAT4X4 CGameObject::GetWorldTransf()
{
	XMFLOAT4X4 xmf4x4World;
	xmf4x4World = XMFLOAT4X4{
		m_xmf3Right.x,		m_xmf3Right.y,		m_xmf3Right.z, 0.0f,
		m_xmf3Up.x,			m_xmf3Up.y,			m_xmf3Up.z, 0.0f,
		m_xmf3Look.x,		m_xmf3Look.y,		m_xmf3Look.z, 0.0f,
		m_xmf3Position.x,	m_xmf3Position.y,	m_xmf3Position.z, 0.0f,
	};

	return xmf4x4World;
}

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수

	for (int i = 0; i < m_nMeshes; i++)
	{
		ID3D12Resource* pd3dcbGameObject = NULL;
		CB_GAMEOBJECT_INFO* pcbMappedGameObject = NULL;

		pd3dcbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
			D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

		pd3dcbGameObject->Map(0, NULL, (void **)&pcbMappedGameObject);

		m_vd3dcbGameObject.emplace_back(pd3dcbGameObject);
		m_vcbMappedGameObject.emplace_back(pcbMappedGameObject);
	}

	ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		ID3D12Resource* pd3dcbBoneTransforms = NULL;
		XMFLOAT4X4* pcbMappedBoneTransform = NULL;

		pd3dcbBoneTransforms = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
			D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

		pd3dcbBoneTransforms->Map(0, NULL, (void **)&pcbMappedBoneTransform);

		m_vd3dcbBoneTransforms.emplace_back(pd3dcbBoneTransforms);
		m_vcbxmf4x4BoneTransforms.emplace_back(pcbMappedBoneTransform);
	}
}

void CGameObject::ReleaseShaderVariables()
{
	for (ID3D12Resource *pd3dcbGameObject : m_vd3dcbGameObject)
	{
		pd3dcbGameObject->Unmap(0, NULL);
		pd3dcbGameObject->Release();
	}

	for (ID3D12Resource *pd3dcbBoneTransforms : m_vd3dcbBoneTransforms)
	{
		pd3dcbBoneTransforms->Unmap(0, NULL);
		pd3dcbBoneTransforms->Release();
	}
}

void CGameObject::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;

	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_fPreparePitch), XMConvertToRadians(m_fPrepareYaw), XMConvertToRadians(m_fPrepareRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::UpdateWorldTransform()
{
	if(m_pModel) m_pModel->UpdateWorldTransform(&m_xmf4x4World);
}

bool CGameObject::CollisionCheck(CGameObject *pObject)
{
	std::vector<BoundingBox> vxmAABB = pObject->GetAABB();

	for(BoundingBox m_xmAABB : m_vxmAABB)
	{
		for (BoundingBox xmAABB : vxmAABB)
		{
			if (m_xmAABB.Intersects(xmAABB))
				return true;
		}
	}

	return false;
}

bool CGameObject::CollisionCheck(XMVECTOR *pxmf4Origin, XMVECTOR *pxmf4Look, float *pfDistance)
{
	bool bCollision = false;
	float fMinDistance = 1000.0f;
	float fDistance = 0.0f;

	for (BoundingBox m_xmAABB : m_vxmAABB)
	{
		if (m_xmAABB.Intersects(*pxmf4Origin, *pxmf4Look, fDistance))
		{
			if (fMinDistance > fDistance) fMinDistance = fDistance;
			bCollision = true;
		}
	}

	if (bCollision)
		*pfDistance = fMinDistance;

	return bCollision;
}

void CGameObject::MoveToCollision(CGameObject *pObject)
{
	std::vector<BoundingBox> vxmAABB = pObject->GetAABB();

	for (BoundingBox m_xmAABB : m_vxmAABB)
	{
		for (BoundingBox xmAABB : vxmAABB)
		{
			if (m_xmAABB.Intersects(xmAABB))
			{
				XMFLOAT3 xmf3Min = Vector3::Subtract(m_xmAABB.Center, m_xmAABB.Extents);
				XMFLOAT3 xmf3Max = Vector3::Add(m_xmAABB.Center, m_xmAABB.Extents);

				XMFLOAT3 xmf3ObjMin = Vector3::Subtract(xmAABB.Center, xmAABB.Extents);
				XMFLOAT3 xmf3ObjMax = Vector3::Add(xmAABB.Center, xmAABB.Extents);

				XMFLOAT3 xmf3Distance = XMFLOAT3(0.0f, 0.0f, 0.0f);
				XMFLOAT3 xmf3Temp = XMFLOAT3(0.0f, 0.0f, 0.0f);

				{
					if (xmf3Min.x < xmf3ObjMax.x) xmf3Distance.x = xmf3ObjMax.x - xmf3Min.x;
					if (xmf3Max.x > xmf3ObjMin.x)
					{
						xmf3Temp.x = xmf3ObjMin.x - xmf3Max.x;
						if (fabsf(xmf3Distance.x) > fabsf(xmf3Temp.x)) xmf3Distance.x = xmf3Temp.x;
					}

					if (xmf3Min.y < xmf3ObjMax.y) xmf3Distance.y = xmf3ObjMax.y - xmf3Min.y;
					if (xmf3Max.y > xmf3ObjMin.y)
					{
						xmf3Temp.y = xmf3ObjMin.y - xmf3Max.y;
						if (fabsf(xmf3Distance.y) > fabsf(xmf3Temp.y)) xmf3Distance.y = xmf3Temp.y;
					}

					if (xmf3Min.z < xmf3ObjMax.z) xmf3Distance.z = xmf3ObjMax.z - xmf3Min.z;
					if (xmf3Max.z > xmf3ObjMin.z)
					{
						xmf3Temp.z = xmf3ObjMin.z - xmf3Max.z;
						if (fabsf(xmf3Distance.z) > fabsf(xmf3Temp.z)) xmf3Distance.z = xmf3Temp.z;
					}
				}

				xmf3Temp = xmf3Distance;

				XMFLOAT3 xmf3Position = m_xmf3Position;

				if (!IsZero(xmf3Temp.x))
				{
					if (!IsZero(xmf3Temp.y))
					{
						if (fabsf(xmf3Temp.x) > fabsf(xmf3Temp.y))
						{
							xmf3Position.x = m_xmf3Position.x;
							xmf3Distance.x = 0.0f;

							if (!IsZero(xmf3Temp.z))
							{
								if (fabsf(xmf3Temp.y) > fabsf(xmf3Temp.z))
								{
									xmf3Position.y = m_xmf3Position.y;
									xmf3Distance.y = 0.0f;
								}
								else
								{
									xmf3Position.z = m_xmf3Position.z;
									xmf3Distance.z = 0.0f;
								}
							}
						}
						else
						{
							xmf3Position.y = m_xmf3Position.y;
							xmf3Distance.y = 0.0f;

							if (!IsZero(xmf3Temp.z))
							{
								if (fabsf(xmf3Temp.x) > fabsf(xmf3Temp.z))
								{
									xmf3Position.x = m_xmf3Position.x;
									xmf3Distance.x = 0.0f;
								}
								else
								{
									xmf3Position.z = m_xmf3Position.z;
									xmf3Distance.z = 0.0f;
								}
							}
						}
					}
					else if (!IsZero(xmf3Temp.z))
					{
						if (fabsf(xmf3Temp.x) > fabsf(xmf3Temp.z))
						{
							xmf3Position.x = m_xmf3Position.x;
							xmf3Distance.x = 0.0f;
						}
						else
						{
							xmf3Position.z = m_xmf3Position.z;
							xmf3Distance.z = 0.0f;
						}
					}
				}
				else if (!IsZero(xmf3Temp.y))
				{
					if (!IsZero(xmf3Temp.z))
					{
						if (fabsf(xmf3Temp.y) > fabsf(xmf3Temp.z))
						{
							xmf3Position.y = m_xmf3Position.y;
							xmf3Distance.y = 0.0f;
						}
						else
						{
							xmf3Position.z = m_xmf3Position.z;
							xmf3Distance.z = 0.0f;
						}
					}
				}

				SetPosition(Vector3::Add(xmf3Position, xmf3Distance));

				ProcessMoveToCollision(&m_xmAABB, &xmAABB);
			}
		}
	}
}

void CGameObject::Animate(float fTimeElapsed, CCamera *pCamera)
{
	if (m_pModel)
	{
		OnPrepareRender();

		if (m_pAnimationController) m_pAnimationController->AdvanceTime(fTimeElapsed);

		UpdateWorldTransform();

		int i = 0;
		m_pModel->UpdateCollisionBox(m_vxmAABB, &i);
	}
}

void CGameObject::SetSkinnedMeshBoneTransformConstantBuffer()
{
	for(int i = 0; i < m_vd3dcbBoneTransforms.size(); i++)
	{
		m_vSkinnedMeshes[i]->m_pd3dcbBoneTransforms = m_vd3dcbBoneTransforms[i];
		m_vSkinnedMeshes[i]->m_pcbxmf4x4BoneTransforms = m_vcbxmf4x4BoneTransforms[i];
	}
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_pAnimationController) m_pAnimationController->ApplyTransform();

	UpdateWorldTransform();

	UpdateShaderVariables(pd3dCommandList);

	int i = 0;
	if(m_pModel)
	{

		if (m_nSkinnedMeshes > 0) SetSkinnedMeshBoneTransformConstantBuffer();

		m_pModel->Render(pd3dCommandList, pCamera, m_vd3dcbGameObject, m_vcbMappedGameObject, &i);
	}
}

void CGameObject::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	UpdateWorldTransform();

	UpdateShaderVariables(pd3dCommandList);

	int i = 0;
	if(m_pModel) m_pModel->RenderWire(pd3dCommandList, pCamera, m_vd3dcbGameObject, m_vcbMappedGameObject, &i);
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pModel) m_pModel->ReleaseUploadBuffers();
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	if (fPitch != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
			XMConvertToRadians(fPitch));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	}
	if (fYaw != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
			XMConvertToRadians(fYaw));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (fRoll != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look),
			XMConvertToRadians(fRoll));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf3Position.x = x;
	m_xmf3Position.y = y;
	m_xmf3Position.z = z;
}

void CGameObject::SetPosition(XMFLOAT3& xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

//게임 객체를 로컬 x-축 방향으로 이동한다. 
void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

//게임 객체를 로컬 y-축 방향으로 이동한다. 
void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

//게임 객체를 로컬 z-축 방향으로 이동한다. 
void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

//게임 객체를 정해진 방향으로 이동한다. 
void CGameObject::Move(XMFLOAT3 xmf3Direction, float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Direction, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

///////////////////////////////////////////////////////////////////////////////////

RandomMoveObject::RandomMoveObject() : CGameObject()
{
	InitRandomRotate();

	m_Time = 5.0f;
	m_MovingSpeed = 10.0f;
}

RandomMoveObject::~RandomMoveObject()
{
}

void RandomMoveObject::InitRandomRotate()
{
	m_RotateAngle = (rand() % 360) + 45.0f;
	m_RotateSpeed = m_RotateAngle / ((float)(rand() % 2) + 1.0f);

	m_Angle = 0.0f;
	m_ElapsedTime = 0.0f;
}

void RandomMoveObject::Animate(float fTimeElapsed, CCamera *pCamera)
{
	if (m_ElapsedTime >= m_Time)
	{
		InitRandomRotate();
	}
	else
	{
		if (m_Angle < m_RotateAngle)
		{
			m_Angle += m_RotateSpeed * fTimeElapsed;

			if (m_Angle > m_RotateAngle) m_Angle = m_RotateAngle;
			else Rotate(0.0f, m_RotateSpeed * fTimeElapsed, 0.0f);
		}
		else
			m_ElapsedTime += fTimeElapsed;
	}

	//MoveForward(m_MovingSpeed * fTimeElapsed);

	CGameObject::Animate(fTimeElapsed, pCamera);
}

///////////////////////////////////////////////////////////////////////////////////

RotateObject::RotateObject() : CGameObject()
{
	m_RotateSpeed = 20.0f;
}

RotateObject::~RotateObject()
{
}

void RotateObject::Animate(float fTimeElapsed, CCamera *pCamera)
{
//	Rotate(0.0f, m_RotateSpeed * fTimeElapsed, 0.0f);

	CGameObject::Animate(fTimeElapsed, pCamera);
}

///////////////////////////////////////////////////////////////////////////////////

Bullet::Bullet() : CGameObject()
{
	m_ElapsedTime = 0;
	m_DurationTime = 3.0f;
	m_MovingSpeed = 500.0f;
	m_RotationSpeed = 1440.0f;
}

Bullet::~Bullet()
{
}

void Bullet::Animate(float ElapsedTime, CCamera *pCamera)
{
	if (m_ElapsedTime >= m_DurationTime)
	{
#ifndef ON_NETWORKING
		Delete();
#endif
	}
	else
	{
		CGameObject::Rotate(0.0f, 0.0f, m_RotationSpeed * ElapsedTime);
#ifndef ON_NETWORKING
		MoveForward(m_MovingSpeed * ElapsedTime);
#endif
		m_ElapsedTime += ElapsedTime;
	}

	CGameObject::Animate(ElapsedTime);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : CGameObject()
{
	//m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Position = XMFLOAT3(-(nWidth *xmf3Scale.x/2), 0.0f, -(nLength *xmf3Scale.z / 2));

	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	CHeightMapGridMesh *pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(pHeightMapGridMesh, NULL);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pTerrainTexture = new CTexture(2, RESOURCE_TEXTURE2D_ARRAY, 0);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Base.dds", 0);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Tile.dds", 1);

	CShader *pShader = new CTerrainShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	CScene::CreateShaderResourceViews(pd3dDevice, pTerrainTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);
	
	CMaterial **ppTerrainMaterial = new CMaterial*[1];
	ppTerrainMaterial[0] = new CMaterial();
	ppTerrainMaterial[0]->SetTexture(pTerrainTexture);
	ppTerrainMaterial[0]->SetShader(pShader);

	SetMaterial(ppTerrainMaterial, 1);
}

CHeightMapTerrain::~CHeightMapTerrain()
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject()
{
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh, NULL);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0);
	//pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Skybox/SkyBox_0.dds", 0);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Skybox/SkyBox_1.dds", 0);

	CShader *pShader = new CSkyBoxShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	CScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, ROOT_PARAMETER_INDEX_TEXTURE_CUBE, false);

	CMaterial **ppSkyBoxMaterial = new CMaterial*[1];
	ppSkyBoxMaterial[0] = new CMaterial();
	ppSkyBoxMaterial[0]->SetTexture(pSkyBoxTexture);
	ppSkyBoxMaterial[0]->SetShader(pShader);

	SetMaterial(ppSkyBoxMaterial, 1);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	CGameObject::Render(pd3dCommandList, pCamera);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////

CEffect::CEffect() : CGameObject()
{
	m_nMaxSpriteX = m_nMaxSpriteY = m_nSpritePosX = m_nSpritePosY = 0;
}

CEffect::~CEffect()
{
}

void CEffect::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	SetSpritePos(m_nSpritePosX, m_nSpritePosY);

	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_INDEX_TEXTURE_SPRITE, 4, &m_xmf4Sprite, 0);

	CGameObject::UpdateShaderVariables(pd3dCommandList);
}

void CEffect::Animate(float fTimeElapsed, CCamera *pCamera)
{
	SpriteAnimate();

	SetLookAt(pCamera->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));

	CGameObject::Animate(fTimeElapsed, pCamera);
}

void CEffect::SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3& xmf3Up)
{
	XMFLOAT3 xmf3Position(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z);

	XMFLOAT3 xmf3Look = Vector3::Normalize(Vector3::Subtract(xmf3Target, xmf3Position));
	XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);

	m_xmf3Right.x = xmf3Right.x; m_xmf3Right.y = xmf3Right.y; m_xmf3Right.z = xmf3Right.z;
	m_xmf3Up.x = xmf3Up.x; m_xmf3Up.y = xmf3Up.y; m_xmf3Up.z = xmf3Up.z;
	m_xmf3Look.x = xmf3Look.x; m_xmf3Look.y = xmf3Look.y; m_xmf3Look.z = xmf3Look.z;
}

void CEffect::SpriteAnimate()
{
	if (m_nSpritePosX + m_nSpritePosY * m_nMaxSpriteX == m_nMaxSprite && m_efType == EFFECT_TYPE_ONE)
		Delete();
	else
	{
		m_nSpritePosX++;
		if (m_nSpritePosX == m_nMaxSpriteX)
		{
			m_nSpritePosX = 0;
			m_nSpritePosY++;
		}
		if (m_nSpritePosY == m_nMaxSpriteY)
			m_nSpritePosY = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////

CAnimationObject::CAnimationObject() : CGameObject()
{
}

CAnimationObject::~CAnimationObject()
{
}

void CAnimationObject::Animate(float ElapsedTime, CCamera *pCamera)
{
	CGameObject::Animate(ElapsedTime);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////

CEquipmentableObject::CEquipmentableObject() : CGameObject()
{

}

CEquipmentableObject::~CEquipmentableObject()
{
	m_pRightHand = NULL;
	m_pLeftHand = NULL;
}

void CEquipmentableObject::OnPrepareAnimate()
{
	//m_pRightHand = m_pModel->FindFrame("Bip001_R_Hand");
	//m_pLeftHand = m_pModel->FindFrame("Bip001_L_Hand");

	//m_nMeshes += 2;
}