#include "pch.h"
#include "Mesh.h"
#include "FBXExporter.h"

void FindXYZ(CTexturedVertex* pVertices, UINT nVertices, XMFLOAT3& Center, XMFLOAT3& Extents)
{
	XMFLOAT3 MinMaxVertex[2]; // [0] Min, [1] Max
	MinMaxVertex[0] = MinMaxVertex[1] = pVertices[0].GetPosition();

	for (UINT i = 1; i < nVertices; i++)
	{
		XMFLOAT3 Position = pVertices[i].GetPosition();

		if (Position.x < MinMaxVertex[0].x) MinMaxVertex[0].x = Position.x;
		if (Position.y < MinMaxVertex[0].y) MinMaxVertex[0].y = Position.y;
		if (Position.z < MinMaxVertex[0].z) MinMaxVertex[0].z = Position.z;

		if (Position.x > MinMaxVertex[1].x) MinMaxVertex[1].x = Position.x;
		if (Position.y > MinMaxVertex[1].y) MinMaxVertex[1].y = Position.y;
		if (Position.z > MinMaxVertex[1].z) MinMaxVertex[1].z = Position.z;
	}

	Center = Vector3::Add(MinMaxVertex[0], MinMaxVertex[1]);
	Center.x /= 2; Center.y /= 2; Center.z /= 2;

	Extents = Vector3::Subtract(MinMaxVertex[1], MinMaxVertex[0]);
	Extents.x /= 2; Extents.y /= 2; Extents.z /= 2;
}

CMesh::CMesh(const char *pstrFileName)
{
	// FBXExporter 생성
	FBXExporter* myExporter = new FBXExporter();
	myExporter->Initialize();

	// FBX메쉬파일로드
	myExporter->LoadScene(pstrFileName);
	myExporter->ExportFBX(&m_nVertices, &m_nIndices); // 정점 및 인덱스 갯수 불러오기

	m_pVertices = new CTexturedVertex[m_nVertices];
	m_pnIndices = new UINT[m_nIndices];
	myExporter->WriteMeshToStream(m_pVertices, m_pnIndices);

	// 정점 버퍼 생성

	XMFLOAT3 Center, Extents;
	FindXYZ(m_pVertices, m_nVertices, Center, Extents);

	SetOOBB(Center, Extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CMesh::~CMesh()
{
	if (m_pVertices) delete[] m_pVertices;
	if (m_pnIndices) delete[] m_pnIndices;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CCubeMesh::CCubeMesh(XMFLOAT3 xmf3Center, float fWidth, float fHeight, float fDepth) : CMesh()
{
	m_nVertices = 8;

	float fx = fWidth, fy = fHeight, fz = fDepth;

	CDiffusedVertex pVertices[8];

	pVertices[0] = CDiffusedVertex(XMFLOAT3(-fx + xmf3Center.x, +fy + xmf3Center.y, -fz + xmf3Center.z), XMFLOAT4(255.0f, 0.0f, 0.0f, 255.0f));
	pVertices[1] = CDiffusedVertex(XMFLOAT3(+fx + xmf3Center.x, +fy + xmf3Center.y, -fz + xmf3Center.z), XMFLOAT4(255.0f, 0.0f, 0.0f, 255.0f));
	pVertices[2] = CDiffusedVertex(XMFLOAT3(+fx + xmf3Center.x, +fy + xmf3Center.y, +fz + xmf3Center.z), XMFLOAT4(255.0f, 0.0f, 0.0f, 255.0f));
	pVertices[3] = CDiffusedVertex(XMFLOAT3(-fx + xmf3Center.x, +fy + xmf3Center.y, +fz + xmf3Center.z), XMFLOAT4(255.0f, 0.0f, 0.0f, 255.0f));
	pVertices[4] = CDiffusedVertex(XMFLOAT3(-fx + xmf3Center.x, -fy + xmf3Center.y, -fz + xmf3Center.z), XMFLOAT4(255.0f, 0.0f, 0.0f, 255.0f));
	pVertices[5] = CDiffusedVertex(XMFLOAT3(+fx + xmf3Center.x, -fy + xmf3Center.y, -fz + xmf3Center.z), XMFLOAT4(255.0f, 0.0f, 0.0f, 255.0f));
	pVertices[6] = CDiffusedVertex(XMFLOAT3(+fx + xmf3Center.x, -fy + xmf3Center.y, +fz + xmf3Center.z), XMFLOAT4(255.0f, 0.0f, 0.0f, 255.0f));
	pVertices[7] = CDiffusedVertex(XMFLOAT3(-fx + xmf3Center.x, -fy + xmf3Center.y, +fz + xmf3Center.z), XMFLOAT4(255.0f, 0.0f, 0.0f, 255.0f));

	m_nIndices = 36;
	UINT pnIndices[36];

	pnIndices[0] = 3; pnIndices[1] = 1; pnIndices[2] = 0;
	pnIndices[3] = 2; pnIndices[4] = 1; pnIndices[5] = 3;
	pnIndices[6] = 0; pnIndices[7] = 5; pnIndices[8] = 4;
	pnIndices[9] = 1; pnIndices[10] = 5; pnIndices[11] = 0;
	pnIndices[12] = 3; pnIndices[13] = 4; pnIndices[14] = 7;
	pnIndices[15] = 0; pnIndices[16] = 4; pnIndices[17] = 3;
	pnIndices[18] = 1; pnIndices[19] = 6; pnIndices[20] = 5;
	pnIndices[21] = 2; pnIndices[22] = 6; pnIndices[23] = 1;
	pnIndices[24] = 2; pnIndices[25] = 7; pnIndices[26] = 6;
	pnIndices[27] = 3; pnIndices[28] = 7; pnIndices[29] = 2;
	pnIndices[30] = 6; pnIndices[31] = 4; pnIndices[32] = 5;
	pnIndices[33] = 7; pnIndices[34] = 4; pnIndices[35] = 6;
}

CCubeMesh::~CCubeMesh()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////