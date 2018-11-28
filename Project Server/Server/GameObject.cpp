#include "pch.h"
#include "GameObject.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CGameObject::CGameObject()
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_ElapsedTime = 0;
	m_DurationTime = 3.0f;
	m_MovingSpeed = 1000.0f;
	m_RotationSpeed = 1440.0f;
}

CGameObject::~CGameObject()
{
	if (m_xmOOBB) delete[] m_xmOOBB;
}

void CGameObject::SetWorldTransf(XMFLOAT4X4& xmf4x4World)
{ 
	m_xmf3Right = XMFLOAT3(xmf4x4World._11, xmf4x4World._12, xmf4x4World._13);
	m_xmf3Up = XMFLOAT3(xmf4x4World._21, xmf4x4World._22, xmf4x4World._23);
	m_xmf3Look = XMFLOAT3(xmf4x4World._31, xmf4x4World._32, xmf4x4World._33);
	m_xmf3Position = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf4x4World = xmf4x4World;
}

void CGameObject::SetMesh(CMesh **ppMeshes, CCubeMesh **ppCubeMeshes, UINT nMeshes)
{
	m_ppMeshes = ppMeshes;
	m_ppCubeMeshes = ppCubeMeshes;
	m_nMeshes = nMeshes;

	if (m_xmOOBB) delete[] m_xmOOBB;
	m_xmOOBB = new BoundingOrientedBox[m_nMeshes];
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

	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_PPitch), XMConvertToRadians(m_PYaw), XMConvertToRadians(m_PRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Animate(float fTimeElapsed)
{
	if (m_Object_Type == OBJECT_TYPE_BULLET)
	{
		if (m_ElapsedTime >= m_DurationTime)
		{
			DeleteObject();
		}
		else
		{
			//CGameObject::Rotate(0.0f, 0.0f, m_RotationSpeed * fTimeElapsed);
			MoveForward(m_MovingSpeed * fTimeElapsed);
			//std::cout << index << " : " << m_xmf3Position.x << ", " << m_xmf3Position.y << ", " << m_xmf3Position.z << std::endl;

			m_ElapsedTime += fTimeElapsed;
		}
	}
	for (UINT i = 0; i < m_nMeshes; i++)
	{
		if (m_ppMeshes[i])
		{
			//XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_fPitch), XMConvertToRadians(m_fYaw), XMConvertToRadians(m_fRoll));
			//XMFLOAT4X4 xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

			OnPrepareRender();
			m_ppMeshes[i]->m_xmOOBB.Transform(m_xmOOBB[i], XMLoadFloat4x4(&m_xmf4x4World));
			XMStoreFloat4(&m_xmOOBB[i].Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB[i].Orientation)));
		}
	}
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

void RandomMoveObject::Animate(float ElapsedTime)
{
	if (m_ElapsedTime >= m_Time)
	{
		InitRandomRotate();
	}
	else
	{
		if (m_Angle < m_RotateAngle)
		{
			m_Angle += m_RotateSpeed * ElapsedTime;

			if (m_Angle > m_RotateAngle) m_Angle = m_RotateAngle;
			else Rotate(0.0f, m_RotateSpeed * ElapsedTime, 0.0f);
		}
		else
			m_ElapsedTime += ElapsedTime;
	}

	MoveForward(m_MovingSpeed * ElapsedTime);

	CGameObject::Animate(ElapsedTime);
}

///////////////////////////////////////////////////////////////////////////////////

Bullet::Bullet() : CGameObject()
{
	//m_ElapsedTime = 0;
	//m_DurationTime = 3.0f;
	//m_MovingSpeed = 1000.0f;
	//m_RotationSpeed = 1440.0f;
}

Bullet::~Bullet()
{
}

void Bullet::Animate(float ElapsedTime)
{
	/*if (m_ElapsedTime >= m_DurationTime)
	{
		DeleteObject();
	}
	else
	{
		CGameObject::Rotate(0.0f, 0.0f, m_RotationSpeed * ElapsedTime);
		MoveForward(m_MovingSpeed * ElapsedTime);

		m_ElapsedTime += ElapsedTime;
	}*/

	CGameObject::Animate(ElapsedTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////

void CreateRobotObjectMesh(CMesh**& ppMeshes, CCubeMesh**& ppCubeMeshes, UINT& nMeshes)
{
	ppMeshes = new CMesh*[7];
	::ZeroMemory(ppMeshes, sizeof(CMesh*) * 7);
	ppCubeMeshes = new CCubeMesh*[7];
	::ZeroMemory(ppCubeMeshes, sizeof(CCubeMesh*) * 7);	

	XMFLOAT3 Extents;
	XMFLOAT3 Center;

	ppMeshes[0] = new CMesh("./Resource/GM/Head/Head.FBX");
	Extents = ppMeshes[0]->GetExtents();
	Center = ppMeshes[0]->GetCenter();
	ppCubeMeshes[0] = new CCubeMesh(Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[1] = new CMesh("./Resource/GM/Body/UpperBody.FBX");
	Extents = ppMeshes[1]->GetExtents();
	Center = ppMeshes[1]->GetCenter();
	ppCubeMeshes[1] = new CCubeMesh(Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[2] = new CMesh("./Resource/GM/Body/LowerBody.FBX");
	Extents = ppMeshes[2]->GetExtents();
	Center = ppMeshes[2]->GetCenter();
	ppCubeMeshes[2] = new CCubeMesh(Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[3] = new CMesh("./Resource/GM/Arm/Arm-Left.FBX");
	Extents = ppMeshes[3]->GetExtents();
	Center = ppMeshes[3]->GetCenter();
	ppCubeMeshes[3] = new CCubeMesh(Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[4] = new CMesh("./Resource/GM/Arm/Arm-Right.FBX");
	Extents = ppMeshes[4]->GetExtents();
	Center = ppMeshes[4]->GetCenter();
	ppCubeMeshes[4] = new CCubeMesh(Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[5] = new CMesh("./Resource/GM/Leg/Leg-Left.FBX");
	Extents = ppMeshes[5]->GetExtents();
	Center = ppMeshes[5]->GetCenter();
	ppCubeMeshes[5] = new CCubeMesh(Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[6] = new CMesh("./Resource/GM/Leg/Leg-Right.FBX");
	Extents = ppMeshes[6]->GetExtents();
	Center = ppMeshes[6]->GetCenter();
	ppCubeMeshes[6] = new CCubeMesh(Center, Extents.x, Extents.y, Extents.z);

	nMeshes = 7;
}