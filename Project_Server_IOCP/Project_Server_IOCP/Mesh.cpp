#include "pch.h"
#include "Mesh.h"

CMesh::CMesh()
{
}

CMesh::~CMesh()
{
	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pxmf3Tangents) delete[] m_pxmf3Tangents;
	if (m_pxmf3Binormals) delete[] m_pxmf3Binormals;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
	if (m_pnMaterialIndices) delete[] m_pnMaterialIndices;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CCubeMesh::CCubeMesh(XMFLOAT3 xmf3Center, XMFLOAT3 xmf3Extents) : CMesh()
{
	m_nVertices = 8;

	float fx = xmf3Extents.x, fy = xmf3Extents.y, fz = xmf3Extents.z;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	m_pxmf3Positions[0] = XMFLOAT3(-fx + xmf3Center.x, +fy + xmf3Center.y, -fz + xmf3Center.z);
	m_pxmf3Positions[1] = XMFLOAT3(+fx + xmf3Center.x, +fy + xmf3Center.y, -fz + xmf3Center.z);
	m_pxmf3Positions[2] = XMFLOAT3(+fx + xmf3Center.x, +fy + xmf3Center.y, +fz + xmf3Center.z);
	m_pxmf3Positions[3] = XMFLOAT3(-fx + xmf3Center.x, +fy + xmf3Center.y, +fz + xmf3Center.z);
	m_pxmf3Positions[4] = XMFLOAT3(-fx + xmf3Center.x, -fy + xmf3Center.y, -fz + xmf3Center.z);
	m_pxmf3Positions[5] = XMFLOAT3(+fx + xmf3Center.x, -fy + xmf3Center.y, -fz + xmf3Center.z);
	m_pxmf3Positions[6] = XMFLOAT3(+fx + xmf3Center.x, -fy + xmf3Center.y, +fz + xmf3Center.z);
	m_pxmf3Positions[7] = XMFLOAT3(-fx + xmf3Center.x, -fy + xmf3Center.y, +fz + xmf3Center.z);
}

CCubeMesh::~CCubeMesh()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void FindXYZ(XMFLOAT3* pPositions, UINT nVertices, XMFLOAT3& Center, XMFLOAT3& Extents)
{
	XMFLOAT3 MinMaxVertex[2]; // [0] Min, [1] Max
	MinMaxVertex[0] = MinMaxVertex[1] = pPositions[0];

	for (UINT i = 1; i < nVertices; i++)
	{
		XMFLOAT3 Position = pPositions[i];

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

CStandardMesh::CStandardMesh() : CMesh()
{
}

CStandardMesh::~CStandardMesh()
{
}

void CStandardMesh::LoadMeshFromFile(FILE *pfile)
{
	BYTE nstrLength;
	char pstrToken[64] = { 0 };

	m_nType |= 0x01;

	fread_s(&m_nVertices, sizeof(UINT), sizeof(int), 1, pfile);

	fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
	fread_s(m_pstrName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

	while (true)
	{
		fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
		fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
		pstrToken[nstrLength] = '\0';

		if (!strcmp(pstrToken, "<Positions>:"))
		{
			m_pxmf3Positions = new XMFLOAT3[m_nVertices];
			fread_s(m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, sizeof(XMFLOAT3), m_nVertices, pfile);
		}
		else if (!strcmp(pstrToken, "<TextureCoords>:"))
		{
			m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
			fread_s(m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, sizeof(XMFLOAT2), m_nVertices, pfile);
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			m_pxmf3Normals = new XMFLOAT3[m_nVertices];
			fread_s(m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, sizeof(XMFLOAT3), m_nVertices, pfile);
		}
		else if (!strcmp(pstrToken, "<Binormals>:"))
		{
			m_pxmf3Binormals = new XMFLOAT3[m_nVertices];
			fread_s(m_pxmf3Binormals, sizeof(XMFLOAT3) * m_nVertices, sizeof(XMFLOAT3), m_nVertices, pfile);
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			m_pxmf3Tangents = new XMFLOAT3[m_nVertices];
			fread_s(m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices, sizeof(XMFLOAT3), m_nVertices, pfile);
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			fread_s(&m_nSubMeshes, sizeof(int), sizeof(int), 1, pfile);

			m_pnSubSetIndices = new int[m_nSubMeshes];
			m_ppnSubSetIndices = new UINT*[m_nSubMeshes];

			if (m_nSubMeshes > 0)
			{
				for (int i = 0; i < m_nSubMeshes; i++)
				{
					fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
					fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
					pstrToken[nstrLength] = '\0';

					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						fread_s(&m_pnSubSetIndices[i], sizeof(int), sizeof(int), 1, pfile);

						if (m_pnSubSetIndices[i] > 0)
						{
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];

							fread_s(m_ppnSubSetIndices[i], sizeof(int) * m_pnSubSetIndices[i], sizeof(int), m_pnSubSetIndices[i], pfile);
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}

	XMFLOAT3 xmf3Center;
	XMFLOAT3 xmf3Extents;
	FindXYZ(m_pxmf3Positions, m_nVertices, xmf3Center, xmf3Extents);
	SetAABB(xmf3Center, xmf3Extents);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
