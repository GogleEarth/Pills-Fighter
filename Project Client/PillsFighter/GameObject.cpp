#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"
#include "Scene.h"
#include "Weapon.h"
#include "Animation.h"
#include "Effect.h"

extern CFMODSound gFmodSound;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CGameObject::CGameObject()
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);

	serverPosition = XMFLOAT3(0, 0, 0);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_MovingSpeed = 0.0f;

	SetPrepareRotate(0.0f, 0.0f, 0.0f);
}

CGameObject::~CGameObject()
{
	ReleaseShaderVariables();

	if (m_pModel) m_pModel->Release();
	if (m_pShader)
	{
		m_pShader->ReleaseObjects();
		m_pShader->ReleaseShaderVariables();
		delete m_pShader;
		m_pShader = NULL;
	}
	if (m_ppAnimationControllers[ANIMATION_UP])
	{
		delete m_ppAnimationControllers[ANIMATION_UP];
		m_ppAnimationControllers[ANIMATION_UP] = NULL;
	}
	if (m_ppAnimationControllers[ANIMATION_DOWN])
	{
		delete m_ppAnimationControllers[ANIMATION_DOWN];
		m_ppAnimationControllers[ANIMATION_DOWN] = NULL;
	}
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

		m_nSkinnedMeshes = pModel->GetSkinnedMeshes();
		m_nMeshes = pModel->GetMeshes();

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

		pd3dcbGameObject = NULL;
	}
	m_vd3dcbGameObject.clear();

	for (ID3D12Resource *pd3dcbBoneTransforms : m_vd3dcbBoneTransforms)
	{
		pd3dcbBoneTransforms->Unmap(0, NULL);
		pd3dcbBoneTransforms->Release();

		pd3dcbBoneTransforms = NULL;
	}
	m_vd3dcbBoneTransforms.clear();
}

void CGameObject::SetHitPoint(int nHitPoint)
{
	m_nHitPoint = nHitPoint; 
	if (m_nHitPoint > m_nMaxHitPoint) 
		m_nHitPoint = m_nMaxHitPoint;
	if (m_nHitPoint < 0)
		m_nHitPoint = 0;
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

	if (m_ppAnimationControllers[ANIMATION_UP])
	{
		m_ppAnimationControllers[ANIMATION_UP]->ApplyTransform();
	}

	if (m_ppAnimationControllers[ANIMATION_DOWN])
	{
		m_ppAnimationControllers[ANIMATION_DOWN]->ApplyTransform();
	}

	UpdateWorldTransform();
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

				XMFLOAT3 worldObjectPos = pObject->GetPosition();

				XMFLOAT3 worldObjectAABBExtent = Vector3::Add(xmAABB.Center, xmAABB.Extents);
				XMFLOAT3 worldObjectAABBReverseExtent = Vector3::Subtract(xmAABB.Center, xmAABB.Extents);

				XMFLOAT3 newPosition = m_xmf3Position;

				XMFLOAT3 separateCenterMinus = XMFLOAT3(m_xmAABB.Center.x - SEPARATION, m_xmAABB.Center.y - SEPARATION, m_xmAABB.Center.z - SEPARATION);
				XMFLOAT3 separateCenterPlus = XMFLOAT3(m_xmAABB.Center.x + SEPARATION, 0, m_xmAABB.Center.z + SEPARATION);

				// 지붕에 올라서기
				if (m_xmf3Position.y > worldObjectAABBExtent.y - INVASION_ROOF && m_xmf3Position.y < worldObjectAABBExtent.y) {
					m_fVelocityY = 0.0f;
					m_nState |= OBJECT_STATE_ONGROUND;
					newPosition.y = xmAABB.Center.y + xmAABB.Extents.y ;
				}
				// x,z 충돌
				else {
					if (separateCenterMinus.x > worldObjectAABBExtent.x - INVASION && separateCenterMinus.x < worldObjectAABBExtent.x) {
						newPosition.x = worldObjectAABBExtent.x + SEPARATION;
					}
					if (separateCenterPlus.x < worldObjectAABBReverseExtent.x + INVASION && separateCenterPlus.x > worldObjectAABBReverseExtent.x) {
						newPosition.x = worldObjectAABBReverseExtent.x - SEPARATION;
					}
					if (separateCenterMinus.z > worldObjectAABBExtent.z - INVASION && separateCenterMinus.z < worldObjectAABBExtent.z) {
						newPosition.z = worldObjectAABBExtent.z + SEPARATION;
					}
					if (separateCenterPlus.z < worldObjectAABBReverseExtent.z + INVASION && separateCenterPlus.z > worldObjectAABBReverseExtent.z) {
						newPosition.z = worldObjectAABBReverseExtent.z - SEPARATION;
					}

					// 이미 건물내부로 들어와버린 경우 이전 프레임 위치 비교해서 들어온 곳으로 나가도록
					if (m_xmf3Position.x > worldObjectAABBReverseExtent.x + INVASION
						&& m_xmf3Position.x < worldObjectAABBExtent.x - INVASION
						&& m_xmf3Position.z > worldObjectAABBReverseExtent.z + INVASION
						&& m_xmf3Position.z < worldObjectAABBExtent.z - INVASION)
					{
						float result;
						float xMax = abs(m_xmf3PrevPosition.x - worldObjectAABBExtent.x);
						float xMin = abs(m_xmf3PrevPosition.x - worldObjectAABBReverseExtent.x);
						float zMax = abs(m_xmf3PrevPosition.z - worldObjectAABBExtent.z);
						float zMin = abs(m_xmf3PrevPosition.z - worldObjectAABBReverseExtent.z);

						result = xMax;
						if (result > xMin) { result = xMin; }
						if (result > zMax) { result = zMax; }
						if (result > zMin) { result = zMin; }

						if (result == xMax) { newPosition.x = worldObjectAABBExtent.x + SEPARATION; }
						else if (result == xMin) { newPosition.x = worldObjectAABBReverseExtent.x - SEPARATION; }
						else if (result == zMax) { newPosition.z = worldObjectAABBExtent.z + SEPARATION; }
						else if (result == zMin) { newPosition.z = worldObjectAABBReverseExtent.z - SEPARATION; }
					}
				}
				SetPosition(newPosition);
			}
		}
	}
}

void CGameObject::MoveToCollisionByRadius(CGameObject *pObject)
{
	float radius = pObject->GetCollisionRadius();
	XMFLOAT3 objectPos = pObject->GetPosition();
	float valX = m_xmf3Position.x - objectPos.x;
	float valY = m_xmf3Position.y - objectPos.y;
	float valZ = m_xmf3Position.z - objectPos.z;

	float distance = sqrtf(
		(valX) * (valX)
		+(valY) * (valY)
		+(valZ) * (valZ));

	if (radius > distance) {
		XMFLOAT3 direction;
		direction.x = valX / distance;
		direction.y = valY / distance;
		direction.z = valZ / distance;

		XMFLOAT3 newPosition;
		newPosition.x = objectPos.x + direction.x * radius;
		newPosition.y = objectPos.y + direction.y * radius;
		newPosition.z = objectPos.z + direction.z * radius;

		SetPosition(newPosition);
	}
}

void CGameObject::MoveToWorldRange()
{
	if (m_xmf3Position.x > WORLD_WIDTH) {
		m_xmf3Position.x = WORLD_WIDTH;
	}
	if (m_xmf3Position.y > WORLD_HEIGHT) {
		m_xmf3Position.y = WORLD_HEIGHT;
	}
	if (m_xmf3Position.z > WORLD_WIDTH) {
		m_xmf3Position.z = WORLD_WIDTH;
	}
	if (m_xmf3Position.x < -WORLD_WIDTH) {
		m_xmf3Position.x = -WORLD_WIDTH;
	}
	if (m_xmf3Position.y < -WORLD_HEIGHT) {
		m_xmf3Position.y = -WORLD_HEIGHT;
	}
	if (m_xmf3Position.z < -WORLD_WIDTH) {
		m_xmf3Position.z = -WORLD_WIDTH;
	}
}

void CGameObject::ApplyToParticle(CParticle *pParticle)
{
	if (pParticle)
	{
		pParticle->SetToFollowFramePosition();
	}
}

void CGameObject::ApplyToEffect(CFollowEffect *pEffect)
{
	if (pEffect)
	{
		pEffect->SetToFollowFramePosition();
	}
}

void CGameObject::AfterAdvanceAnimationController()
{
	if (m_ppAnimationControllers[ANIMATION_UP])
		m_ppAnimationControllers[ANIMATION_UP]->AfterAdvanceTime();

	if (m_ppAnimationControllers[ANIMATION_DOWN])
		m_ppAnimationControllers[ANIMATION_DOWN]->AfterAdvanceTime();
}

void CGameObject::Animate(float fTimeElapsed, CCamera *pCamera)
{
	if (m_bDie)
	{
		m_fRespawnTimeElapsed += fTimeElapsed;
	}

	if (m_pModel)
	{
		OnPrepareRender();
		
		if (m_ppAnimationControllers[ANIMATION_UP]) m_ppAnimationControllers[ANIMATION_UP]->AdvanceTime(fTimeElapsed);
		if (m_ppAnimationControllers[ANIMATION_DOWN]) m_ppAnimationControllers[ANIMATION_DOWN]->AdvanceTime(fTimeElapsed);

		AfterAdvanceAnimationController();

		UpdateWorldTransform();

		int i = 0;
		m_pModel->UpdateCollisionBox(m_vxmAABB, &i);
	}


	for (CParticle *pParticle : m_vpParticles)
	{
		ApplyToParticle(pParticle);
	}

	for (CFollowEffect *pEffect : m_vpFollowEffects)
	{
		ApplyToEffect(pEffect);
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

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, bool bSetShader, int nInstances)
{
	if (m_bDie) return;
	if (!m_bRender) return;

	if(nInstances > 1)
	{ 
		if (m_pModel)
		{
			m_pModel->Render(pd3dCommandList, pCamera, nInstances);
			return;
		}
	}

	OnPrepareRender();

	UpdateShaderVariables(pd3dCommandList);

	int i = 0;
	if (m_pModel)
	{
		if (m_pShader)
		{
			if(bSetShader) m_pShader->Render(pd3dCommandList, pCamera);
		}

		if (m_nSkinnedMeshes > 0) SetSkinnedMeshBoneTransformConstantBuffer();

		m_pModel->Render(pd3dCommandList, pCamera, m_vd3dcbGameObject, m_vcbMappedGameObject, &i, bSetTexture);
	}
}

void CGameObject::RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, bool bSetShader, int nInstances)
{
	if (m_bDie) return;
	if (!m_bRender) return;

	if (nInstances > 1)
	{
		if (m_pModel)
		{
			m_pModel->RenderToShadow(pd3dCommandList, pCamera, nInstances);
			return;
		}
	}

	OnPrepareRender();

	UpdateShaderVariables(pd3dCommandList);

	int i = 0;
	if (m_pModel)
	{
		if (m_pShader)
		{
			if (bSetShader) m_pShader->RenderToShadow(pd3dCommandList, pCamera);
		}

		if (m_nSkinnedMeshes > 0) SetSkinnedMeshBoneTransformConstantBuffer();

		m_pModel->RenderToShadow(pd3dCommandList, pCamera, m_vd3dcbGameObject, m_vcbMappedGameObject, &i, bSetTexture);
	}
}

void CGameObject::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, int nInstances)
{
	if (m_bDie) return;
	if (!m_bRender) return;

	OnPrepareRender();

	UpdateWorldTransform();

	UpdateShaderVariables(pd3dCommandList);

	int i = 0;
	if (m_pModel)
	{
		m_pModel->RenderWire(pd3dCommandList, pCamera, m_vd3dcbGameObject, m_vcbMappedGameObject, &i, nInstances);
	}
}

void CGameObject::UpdateInstanceShaderVariables(VS_VB_INSTANCE *pcbMappedGameObjects, int *pnIndex)
{
	if (m_pModel)
	{
		OnPrepareRender();

		UpdateWorldTransform();

		m_pModel->UpdateInstanceShaderVariables(pcbMappedGameObjects, pnIndex);
	}
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
	m_xmf3Position.x = m_xmf3Position.x;
	m_xmf3Position.y = m_xmf3Position.y;
	m_xmf3Position.z = m_xmf3Position.z;

	m_xmf3Position.x = x;
	m_xmf3Position.y = y;
	m_xmf3Position.z = z;
}

void CGameObject::SetPosition(XMFLOAT3& xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}
 
void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Move(XMFLOAT3 xmf3Direction, float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Direction, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::ProcessDie(float fRespawnTime)
{
	m_bDie = true;
	m_fRespawnTime = fRespawnTime;
	m_fRespawnTimeElapsed = 0.0f;

	m_nState = 0;
}

void CGameObject::ProcessRespawn(int nHP, XMFLOAT3 xmf3Position)
{
	m_bDie = false;
	m_nHitPoint = nHP;

	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_xmf3Position = xmf3Position;

	serverPosition = xmf3Position;
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

	MoveForward(m_MovingSpeed * fTimeElapsed);

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
	Rotate(0.0f, m_RotateSpeed * fTimeElapsed, 0.0f);

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

	CShader *pShader = new CTerrainShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);

	SetShader(pShader);

	CTexture *pTileTexture = new CTexture(5, RESOURCE_TEXTURE2D_ARRAY, 0);
	pTileTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Stage/2048CityMap.dds", 0);
	pTileTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Stage/Tile1.dds", 1);
	pTileTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Stage/Tile2.dds", 2);
	pTileTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Stage/Tile3.dds", 3);
	pTileTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Stage/Tile4.dds", 4);

	CScene::CreateShaderResourceViews(pd3dDevice, pTileTexture, ROOT_PARAMETER_INDEX_TILES, false, false);

	CMaterial **ppTileMaterial = new CMaterial*[1];
	ppTileMaterial[0] = new CMaterial();
	ppTileMaterial[0]->SetTexture(pTileTexture);

	SetMaterial(ppTileMaterial, 1);
}

CHeightMapTerrain::~CHeightMapTerrain()
{
	if (m_pHeightMapImage)
	{
		delete m_pHeightMapImage;
		m_pHeightMapImage = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject()
{
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh, NULL);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CShader *pShader = new CSkyBoxShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);

	CTexture *pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0);
	//pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Skybox/SkyBox_0.dds", 0);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Skybox/SkyBox_1.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, ROOT_PARAMETER_INDEX_TEXTURE_CUBE, false, false);

	CMaterial **ppSkyBoxMaterial = new CMaterial*[1];
	ppSkyBoxMaterial[0] = new CMaterial();
	ppSkyBoxMaterial[0]->SetTexture(pSkyBoxTexture);

	SetMaterial(ppSkyBoxMaterial, 1);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, bool bSetShader, int nInstances)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	CGameObject::Render(pd3dCommandList, pCamera, bSetTexture, bSetShader, nInstances);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////

CAnimationObject::CAnimationObject() : CGameObject()
{
}

CAnimationObject::~CAnimationObject()
{
}

void CAnimationObject::SetAnimationController(CAnimationController *pControllers, int nIndex)
{
	CGameObject::SetAnimationController(pControllers, nIndex);
}

void CAnimationObject::Animate(float ElapsedTime, CCamera *pCamera)
{
	CGameObject::Animate(ElapsedTime);
}

bool CAnimationObject::ChangeAnimation(int nController, int nTrack, int nAnimation, bool bResetPosition)
{
	if (m_ppAnimationControllers)
	{
		if (nAnimation != m_pnAnimationState[nController])
		{
			m_pnAnimationState[nController] = nAnimation;
			m_ppAnimationControllers[nController]->SetTrackAnimation(nTrack, nAnimation);

			m_pbAnimationChanged[nController] = TRUE;

			if (bResetPosition)
			{
				m_ppAnimationControllers[nController]->SetTrackPosition(nTrack, 0.0f);
			}

			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////

CRobotObject::CRobotObject() : CAnimationObject()
{
	m_nType |= OBJECT_TYPE_ROBOT;
}

CRobotObject::~CRobotObject()
{
	//for (CWeapon *pWeapon : m_vpWeapon) delete pWeapon;
	gFmodSound.PauseFMODSound(m_pChannelBooster);
}

void CRobotObject::OnPrepareAnimate()
{
	m_pRightHand = m_pModel->FindFrame("Bip001_R_Hand");
	m_pLeftHand = m_pModel->FindFrame("Bip001_L_Hand");
	m_pLeftNozzle = m_pModel->FindFrame("Bone001");
	m_pRightNozzle = m_pModel->FindFrame("Bone002");
	m_pSpine = m_pModel->FindFrame("Bip001_Spine");
	m_pPelvis = m_pModel->FindFrame("Bip001_Pelvis");
	m_pLThigh = m_pModel->FindFrame("Bip001_L_Thigh");
	m_pRThigh = m_pModel->FindFrame("Bip001_R_Thigh");
	m_pBip = m_pModel->FindFrame("Bip001");
	m_pHead = m_pModel->FindFrame("Bip001_Head");
}

void CRobotObject::EquipOnRightHand(CWeapon *pWeapon)
{
	if (m_pRHWeapon) m_pRHWeapon->Hide();

	pWeapon->Show();
	m_pRHWeapon = pWeapon;
	m_pRHWeapon->SetOwner(this);
}

void CRobotObject::EquipOnLeftHand(CWeapon *pWeapon)
{
	if (m_pLHWeapon) m_pLHWeapon->Hide();

	m_pLHWeapon = pWeapon;
	m_pLHWeapon->SetOwner(this);

	if (m_pLHWeapon) m_pLHWeapon->Show();
}

void CRobotObject::AddWeapon(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CWeapon *pWeapon)
{
	pWeapon->Initialize();
	pWeapon->AddPrepareRotate(180.0f, 90.0f, -90.0f);

	if (!m_pRHWeapon) EquipOnRightHand(pWeapon);

	m_vpWeapon.emplace_back(pWeapon);
}

int CRobotObject::GetWeaponIndex(int nType)
{
	int i = 0;
	for (CWeapon *pWeapon : m_vpWeapon)
	{
		if (pWeapon->GetType() & nType)
			return i;

		i++;
	}

	return -1;
}

void CRobotObject::ChangeWeaponByType(WEAPON_TYPE nType)
{
	int nIndex;
	switch (nType)
	{
	case WEAPON_TYPE::WEAPON_TYPE_MACHINE_GUN:
		nIndex = GetWeaponIndex(WEAPON_TYPE_OF_MACHINEGUN);
		break;
	case WEAPON_TYPE::WEAPON_TYPE_BEAM_RIFLE:
		nIndex = GetWeaponIndex(WEAPON_TYPE_OF_BEAM_RIFLE);
		break;
	case WEAPON_TYPE::WEAPON_TYPE_GM_GUN:
		nIndex = GetWeaponIndex(WEAPON_TYPE_OF_GM_GUN);
		break;
	case WEAPON_TYPE::WEAPON_TYPE_BAZOOKA:
		nIndex = GetWeaponIndex(WEAPON_TYPE_OF_BAZOOKA);
		break;
	case WEAPON_TYPE::WEAPON_TYPE_SABER:
		nIndex = GetWeaponIndex(WEAPON_TYPE_OF_SABER);
		break;
	case WEAPON_TYPE::WEAPON_TYPE_TOMAHAWK:
		nIndex = GetWeaponIndex(WEAPON_TYPE_OF_TOMAHAWK);
		break;
	case WEAPON_TYPE::WEAPON_TYPE_BEAM_SNIPER:
		nIndex = GetWeaponIndex(WEAPON_TYPE_OF_BEAM_SNIPER);
		break;
	}

	if (nIndex != -1) ChangeWeapon(nIndex);
}

void CRobotObject::CRobotObject::ChangeWeapon(int nIndex)
{
	CWeapon *pWeapon = GetWeapon(nIndex);

	EquipOnRightHand(pWeapon);
}

void CRobotObject::ApplyToParticle(CParticle *pParticle)
{
	CGameObject::ApplyToParticle(pParticle);

	if (m_nState & OBJECT_STATE_BOOSTER) 
 		pParticle->SetEmit(true);
	else 
		pParticle->SetEmit(false);

	XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(135));

	XMFLOAT3 xmf3Direction = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
	pParticle->SetDirection(xmf3Direction);
}

void CRobotObject::ApplyToEffect(CFollowEffect *pEffect)
{
	CGameObject::ApplyToEffect(pEffect);

	if (m_nState & OBJECT_STATE_BOOSTER) 
		pEffect->Show();
	else 
		pEffect->Hide();
}

void CRobotObject::Animate(float fTimeElapsed, CCamera *pCamera)
{
	CGameObject::Animate(fTimeElapsed, pCamera);

	if (m_nState & OBJECT_STATE_BOOSTER)
	{
		gFmodSound.PlayFMODSoundLoop(gFmodSound.m_pSoundBooster, &m_pChannelBooster);
	}
	else
	{
		gFmodSound.PauseFMODSound(m_pChannelBooster);
	}

	if (m_pRHWeapon) m_pRHWeapon->SetOwnerTransform(m_pRightHand->GetWorldTransf());
}

void CRobotObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, bool bSetShader, int nInstances)
{
	CGameObject::Render(pd3dCommandList, pCamera, bSetTexture, bSetShader, nInstances);
}

void CRobotObject::RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, bool bSetShader, int nInstances)
{
	CGameObject::RenderToShadow(pd3dCommandList, pCamera, bSetTexture, bSetShader, nInstances);
}

void CRobotObject::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nInstances)
{
	CGameObject::RenderWire(pd3dCommandList, pCamera, nInstances);
}

bool CRobotObject::ChangeAnimation(int nController, int nTrack, int nAnimation, bool bResetPosition)
{
	if (CAnimationObject::ChangeAnimation(nController, nTrack, nAnimation, bResetPosition))
	{
		if (nController == ANIMATION_UP)
		{
			m_uspHitObjects.clear();
		}

		return true;
	}

	return false;
}

void CRobotObject::AfterAdvanceAnimationController()
{
	if (m_pnAnimationState[ANIMATION_UP] != m_pnAnimationState[ANIMATION_DOWN])
	{
		//XMFLOAT4X4 xmf4x4Pelvis = m_pPelvis->GetToParent();
		//XMFLOAT4X4 xmf4x4Spine = m_pSpine->GetToParent();

		//XMFLOAT3 xmf3PelvisRight = XMFLOAT3(xmf4x4Pelvis._11, xmf4x4Pelvis._12, xmf4x4Pelvis._13);
		//XMFLOAT3 xmf3SpineRight = XMFLOAT3(xmf4x4Spine._11, xmf4x4Spine._12, xmf4x4Spine._13);

		//float fTheta = XMConvertToRadians(Vector3::Angle(xmf3PelvisRight, xmf3SpineRight));
		//if (!(fabsf(fTheta) < 0.001f))
		//{
		//	XMFLOAT3 xmf3Axis = Vector3::CrossProduct(xmf3PelvisRight, xmf3SpineRight, true);
		//	XMFLOAT4X4 xmf4x4Rotate;
		//	XMStoreFloat4x4(&xmf4x4Rotate, XMMatrixRotationAxis(XMLoadFloat3(&xmf3Axis), fTheta));

		//	xmf4x4Pelvis = Matrix4x4::Multiply(xmf4x4Pelvis, xmf4x4Rotate);
		//}

		//XMFLOAT3 xmf3PelvisUp = XMFLOAT3(xmf4x4Pelvis._21, xmf4x4Pelvis._22, xmf4x4Pelvis._23);
		//XMFLOAT3 xmf3SpineUp = XMFLOAT3(xmf4x4Spine._21, xmf4x4Spine._22, xmf4x4Spine._23);

		//fTheta = XMConvertToRadians(Vector3::Angle(xmf3PelvisUp, xmf3SpineUp));
		//if (!(fabsf(fTheta) < 0.001f))
		//{
		//	XMFLOAT3 xmf3Axis = Vector3::CrossProduct(xmf3PelvisUp, xmf3SpineUp, true);
		//	XMFLOAT4X4 xmf4x4Rotate;
		//	XMStoreFloat4x4(&xmf4x4Rotate, XMMatrixRotationAxis(XMLoadFloat3(&xmf3Axis), fTheta));

		//	xmf4x4Pelvis = Matrix4x4::Multiply(xmf4x4Pelvis, xmf4x4Rotate);
		//}

		//XMFLOAT3 xmf3PelvisLook = XMFLOAT3(xmf4x4Pelvis._31, xmf4x4Pelvis._32, xmf4x4Pelvis._33);
		//XMFLOAT3 xmf3SpineLook = XMFLOAT3(xmf4x4Spine._31, xmf4x4Spine._32, xmf4x4Spine._33);

		//fTheta = XMConvertToRadians(Vector3::Angle(xmf3PelvisLook, xmf3SpineLook));
		//if (!(fabsf(fTheta) < 0.001f))
		//{
		//	XMFLOAT3 xmf3Axis = Vector3::CrossProduct(xmf3PelvisLook, xmf3SpineLook, true);
		//	XMFLOAT4X4 xmf4x4Rotate;
		//	XMStoreFloat4x4(&xmf4x4Rotate, XMMatrixRotationAxis(XMLoadFloat3(&xmf3Axis), fTheta));

		//	xmf4x4Pelvis = Matrix4x4::Multiply(xmf4x4Pelvis, xmf4x4Rotate);
		//}

		//m_pPelvis->SetToParent(xmf4x4Pelvis);

		// pelvis의 회전 시키기 전 행렬을 저장.
		// pelvis의 행렬을 spine 행렬로 바꾸기. ( 포지션은 바꾸지 않음 )
		// pelvis의 바뀌기 전 행렬과 바뀐 후 행렬을 비교해서 각 축마다 몇 도씩 회전했는지 알아내기.
		// L Thigh과 R Thigh에 구한 회전각의 반대를 적용시키기.
	}

	CGameObject::AfterAdvanceAnimationController();
}

void CRobotObject::SetCallBackKeys(CModel *pModel)
{
	int nAnimationControllers = 2;
	CAnimationController **ppAnimationControllers = new CAnimationController*[nAnimationControllers];
	ppAnimationControllers[ANIMATION_UP] = new CAnimationController(1, pModel->GetAnimationSet(ANIMATION_UP));
	ppAnimationControllers[ANIMATION_DOWN] = new CAnimationController(1, pModel->GetAnimationSet(ANIMATION_DOWN));

	ppAnimationControllers[ANIMATION_UP]->SetTrackAnimation(0, ANIMATION_STATE_IDLE);
	ppAnimationControllers[ANIMATION_DOWN]->SetTrackAnimation(0, ANIMATION_STATE_IDLE);

	CAnimationCallbackHandler *pAnimationCallbackHandler = new CAnimationCallbackHandler();

	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKeys(ANIMATION_STATE_WALK_FORWARD, 2);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_FORWARD, 0, 0.1f, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_TIME, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_FORWARD, 1, 0.6f, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_TIME, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetAnimationCallbackHandler(ANIMATION_STATE_WALK_FORWARD, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CAnimationCallbackHandler();
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKeys(ANIMATION_STATE_WALK_RIGHT, 2);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_RIGHT, 0, 0.2f, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_TIME, (void*)& CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_RIGHT, 1, 0.5f, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_TIME, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetAnimationCallbackHandler(ANIMATION_STATE_WALK_RIGHT, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CAnimationCallbackHandler();
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKeys(ANIMATION_STATE_WALK_LEFT, 2);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_LEFT, 0, 0.2f, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_TIME, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_LEFT, 1, 0.5f, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_TIME, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetAnimationCallbackHandler(ANIMATION_STATE_WALK_LEFT, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CAnimationCallbackHandler();
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKeys(ANIMATION_STATE_WALK_BACKWARD, 2);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_BACKWARD, 0, 0.13f, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_TIME, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_BACKWARD, 1, 0.5f, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_TIME, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetAnimationCallbackHandler(ANIMATION_STATE_WALK_BACKWARD, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CAnimationCallbackHandler();
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKeys(ANIMATION_STATE_BEAM_SABER_1_ONCE, 1);
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKey(ANIMATION_STATE_BEAM_SABER_1_ONCE, 0, FLT_MAX, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_MIDDLE, (void*)&CALLBACK_TYPE_SOUND_SABER_ATTACK);
	ppAnimationControllers[ANIMATION_UP]->SetAnimationCallbackHandler(ANIMATION_STATE_BEAM_SABER_1_ONCE, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CAnimationCallbackHandler();
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKeys(ANIMATION_STATE_BEAM_SABER_2_ONCE, 1);
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKey(ANIMATION_STATE_BEAM_SABER_2_ONCE, 0, FLT_MAX, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_MIDDLE, (void*)&CALLBACK_TYPE_SOUND_SABER_ATTACK);
	ppAnimationControllers[ANIMATION_UP]->SetAnimationCallbackHandler(ANIMATION_STATE_BEAM_SABER_2_ONCE, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CAnimationCallbackHandler();
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKeys(ANIMATION_STATE_BEAM_SABER_3_ONCE, 1);
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKey(ANIMATION_STATE_BEAM_SABER_3_ONCE, 0, FLT_MAX, CALLBACK_TYPE_SOUND, CALLBACK_POSITION_MIDDLE, (void*)&CALLBACK_TYPE_SOUND_SABER_ATTACK);
	ppAnimationControllers[ANIMATION_UP]->SetAnimationCallbackHandler(ANIMATION_STATE_BEAM_SABER_3_ONCE, pAnimationCallbackHandler);

	SetAnimationController(ppAnimationControllers[ANIMATION_UP], ANIMATION_UP);
	SetAnimationController(ppAnimationControllers[ANIMATION_DOWN], ANIMATION_DOWN);
}

void CRobotObject::ProcessDie(float fRespawnTime)
{
	CAnimationObject::ProcessDie(fRespawnTime);

	if (m_pRHWeapon) m_pRHWeapon->Hide();
}

void CRobotObject::ProcessRespawn(int nHP, XMFLOAT3 xmf3Position)
{
	CAnimationObject::ProcessRespawn(nHP, xmf3Position);

	if (m_pRHWeapon) m_pRHWeapon->Show();
}

/////////////////////////////////////////////////////////////////////////////////////////

CCursor::CCursor(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	m_xmf2CursorPos = XMFLOAT2(0.0f, 0.0f);

	float sizex = 84.0f / FRAME_BUFFER_WIDTH * 0.25f;
	float sizey = 128.0f / FRAME_BUFFER_HEIGHT * 0.25f;

	m_pMesh = new CRect(pd3dDevice, pd3dCommandList, XMFLOAT2(0.0f, 0.0f), XMFLOAT2(sizex, sizey));

	m_pShader = new CCursorShader();
	m_pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);

	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Lobby/Cursor.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);
}

CCursor::~CCursor()
{
	if (m_pMesh)
	{
		m_pMesh->ReleaseShaderVariables();
		delete m_pMesh;
		m_pMesh = NULL;
	}
	if (m_pTexture)
	{
		delete m_pTexture;
		m_pTexture = NULL;
	}
	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->ReleaseObjects();
		delete m_pShader;
		m_pTexture = NULL;
	}
}

void CCursor::ReleaseUploadBuffer()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

bool CCursor::CollisionCheck(BoundingBox& xmAABB)
{
	float sizex = 84.0f / FRAME_BUFFER_WIDTH * 0.25f;
	float sizey = 128.0f / FRAME_BUFFER_HEIGHT * 0.25f;
	
	return(xmAABB.Contains(Vector3::XMFloat3ToVector(XMFLOAT3(m_xmf2ScreenPos.x, m_xmf2ScreenPos.y, 1.0f))));
}

void CCursor::MoveCursorPos(float x, float y)
{
	x *= CURSOR_SENSITIVITY_X;
	y *= CURSOR_SENSITIVITY_Y;

	m_xmf2CursorPos.x = (m_xmf2CursorPos.x + x) > gnWndClientWidth ? gnWndClientWidth : (m_xmf2CursorPos.x + x) > 0 ? (m_xmf2CursorPos.x + x) : 0;
	m_xmf2CursorPos.y = (m_xmf2CursorPos.y + y) > gnWndClientHeight ? gnWndClientHeight : (m_xmf2CursorPos.y + y) > 0 ? (m_xmf2CursorPos.y + y) : 0;

	m_xmf2ScreenPos.x = ((2.0f  * m_xmf2CursorPos.x) / gnWndClientWidth) - 1.0f;
	m_xmf2ScreenPos.y = -((2.0f * m_xmf2CursorPos.y) / gnWndClientHeight) + 1.0f;
}

void CCursor::SetCursorPos(XMFLOAT2 xmf2Position)
{
	m_xmf2CursorPos = xmf2Position;
}

void CCursor::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_INDEX_CURSOR_INFO, 2, &m_xmf2ScreenPos, 0);
}

void CCursor::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	UpdateShaderVariable(pd3dCommandList);

	if (m_pShader) m_pShader->Render(pd3dCommandList, NULL);

	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh) m_pMesh->Render(pd3dCommandList, 0);
}

///////////////////////////////////////////////////////////////////////////////////

Meteor::Meteor() : CGameObject()
{
	m_MovingSpeed = 1.0f;
	m_fLifeTime = 10.0f;
	m_fElapsedTime = 0.0f;
	m_xmf3Look = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 0.0f, 1.0f);
}

Meteor::~Meteor()
{
}

void Meteor::Animate(float ElapsedTime, CCamera *pCamera)
{
	if (m_fElapsedTime >= m_fLifeTime)
	{
#ifndef ON_NETWORKING
		Delete();
#endif
	}
	else
	{
#ifndef ON_NETWORKING
		//MoveForward(m_MovingSpeed * ElapsedTime);
#endif
		MoveForward(m_MovingSpeed * ElapsedTime);
		m_fElapsedTime += ElapsedTime;
	}

	CGameObject::Animate(ElapsedTime);
}




