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

void CStandardMesh::LoadMeshFromFBX(FbxMesh *pfbxMesh)
{
	int nPolygon = pfbxMesh->GetPolygonCount();
	int nVertices = nPolygon * 3;

	m_nVertices = nVertices;
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	FbxGeometryElementNormal *pFbxGeoElemNorm = pfbxMesh->GetElementNormal();
	if (pFbxGeoElemNorm)
		m_pxmf3Normals = new XMFLOAT3[m_nVertices];

	FbxGeometryElementUV *pFbxGeoElemUV = pfbxMesh->GetElementUV();
	if (pFbxGeoElemUV)
	{
		m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
		m_pnMaterialIndices = new int[m_nVertices];
	}

	FbxGeometryElementBinormal *pFbxGeoElemBinormal = pfbxMesh->GetElementBinormal();
	if (pFbxGeoElemBinormal)
		m_pxmf3Binormals = new XMFLOAT3[m_nVertices];

	FbxGeometryElementTangent *pFbxGeoElemTangent = pfbxMesh->GetElementTangent();
	if (pFbxGeoElemTangent)
		m_pxmf3Tangents = new XMFLOAT3[m_nVertices];

	int nIndex = 0;

	for (int i = 0; i < nPolygon; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int nCtrlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
			int nTextureUVIndex = pfbxMesh->GetTextureUVIndex(i, j);
			int nIndexByCtrlPoint = pFbxGeoElemNorm->GetIndexArray().GetAt(nCtrlPointIndex);
			int nIndexByIndex = pFbxGeoElemNorm->GetIndexArray().GetAt(nIndex);

			FbxVector4 fv4CtrlPoint = pfbxMesh->GetControlPointAt(nCtrlPointIndex);
			m_pxmf3Positions[i * 3 + j] = XMFLOAT3(
				static_cast<float>(fv4CtrlPoint.mData[0]),
				static_cast<float>(fv4CtrlPoint.mData[1]),
				static_cast<float>(fv4CtrlPoint.mData[2]));

			auto fbxElemRefMode = pFbxGeoElemNorm->GetReferenceMode();

			switch (pFbxGeoElemNorm->GetMappingMode())
			{
			case FbxGeometryElement::eByControlPoint:
				switch (fbxElemRefMode)
				{
				case FbxGeometryElement::eDirect:
				{
					if (pFbxGeoElemNorm)
						m_pxmf3Normals[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nCtrlPointIndex).mData[0]),
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nCtrlPointIndex).mData[1]),
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nCtrlPointIndex).mData[2]));

					if (pFbxGeoElemBinormal)
						m_pxmf3Binormals[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nCtrlPointIndex).mData[0]),
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nCtrlPointIndex).mData[1]),
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nCtrlPointIndex).mData[2]));

					if (pFbxGeoElemTangent)
						m_pxmf3Tangents[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nCtrlPointIndex).mData[0]),
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nCtrlPointIndex).mData[1]),
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nCtrlPointIndex).mData[2]));

					if (pFbxGeoElemUV)
						m_pxmf2TextureCoords0[i * 3 + j] = XMFLOAT2(
							static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nCtrlPointIndex).mData[0]),
							1.0f - static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nCtrlPointIndex).mData[1]));
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					if (pFbxGeoElemNorm)
						m_pxmf3Normals[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0]),
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1]),
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[2]));

					if (pFbxGeoElemBinormal)
						m_pxmf3Binormals[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0]),
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1]),
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[2]));

					if (pFbxGeoElemTangent)
						m_pxmf3Tangents[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0]),
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1]),
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[2]));

					if (pFbxGeoElemUV)
						m_pxmf2TextureCoords0[i * 3 + j] = XMFLOAT2(
							static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[0]),
							1.0f - static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nIndexByCtrlPoint).mData[1]));
				}
				break;
				}
				break;
			case FbxGeometryElement::eByPolygonVertex:
				switch (fbxElemRefMode)
				{
				case FbxGeometryElement::eDirect:
				{
					if (pFbxGeoElemNorm)
						m_pxmf3Normals[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndex).mData[0]),
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndex).mData[1]),
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndex).mData[2]));

					if (pFbxGeoElemBinormal)
						m_pxmf3Binormals[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndex).mData[0]),
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndex).mData[1]),
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndex).mData[2]));

					if (pFbxGeoElemTangent)
						m_pxmf3Tangents[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndex).mData[0]),
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndex).mData[1]),
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndex).mData[2]));

					if (pFbxGeoElemUV)
						m_pxmf2TextureCoords0[i * 3 + j] = XMFLOAT2(
							static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[0]),
							1.0f - static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[1]));
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					if (pFbxGeoElemNorm)
						m_pxmf3Normals[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByIndex).mData[0]),
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByIndex).mData[1]),
							static_cast<float>(pFbxGeoElemNorm->GetDirectArray().GetAt(nIndexByIndex).mData[2]));

					if (pFbxGeoElemBinormal)
						m_pxmf3Binormals[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByIndex).mData[0]),
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByIndex).mData[1]),
							static_cast<float>(pFbxGeoElemBinormal->GetDirectArray().GetAt(nIndexByIndex).mData[2]));

					if (pFbxGeoElemTangent)
						m_pxmf3Tangents[i * 3 + j] = XMFLOAT3(
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByIndex).mData[0]),
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByIndex).mData[1]),
							static_cast<float>(pFbxGeoElemTangent->GetDirectArray().GetAt(nIndexByIndex).mData[2]));

					if (pFbxGeoElemUV)
						m_pxmf2TextureCoords0[i * 3 + j] = XMFLOAT2(
							static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[0]),
							1.0f - static_cast<float>(pFbxGeoElemUV->GetDirectArray().GetAt(nTextureUVIndex).mData[1]));
				}
				break;
				}
				break;
			}

			//if (!pfbxMesh->GetElementMaterial())
			//{
			//	if (!pfbxMesh->GetElementMaterial(1))
			//		printf("Not Found Material\n");
			//}

			//auto fbxMaterialIndex = &(pfbxMesh->GetElementMaterial()->GetIndexArray());
			//auto fbxMaterialMappingMode = pfbxMesh->GetElementMaterial()->GetMappingMode();

			//switch (fbxMaterialMappingMode)
			//{
			//case FbxGeometryElement::eByPolygon:
			//{
			//	m_pnMaterialIndices[i * 3 + j] = fbxMaterialIndex->GetAt(i);
			//}
			//break;
			//case FbxGeometryElement::eAllSame:
			//{
			//	m_pnMaterialIndices[i * 3 + j] = fbxMaterialIndex->GetAt(0);
			//}
			//break;
			//}

			nIndex++;
		}
	}

	XMFLOAT3 xmf3Center, xmf3Extents;
	FindXYZ(m_pxmf3Positions, m_nVertices, xmf3Center, xmf3Extents);

	SetAABB(xmf3Center, xmf3Extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE *pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y)*m_nWidth)] = pHeightMapPixels[x + (y*m_nWidth)];
		}
	}

	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z*m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z*m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1)*m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1)*m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

CHeightMapGridMesh::CHeightMapGridMesh(int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void *pContext0, void *pContext1) : CMesh()
{
	m_nVertices = nWidth * nLength;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext0;
	CHeightMapImage *pMapImage = (CHeightMapImage *)pContext1;

	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf4Colors = new XMFLOAT4[m_nVertices];
	//m_pxmf3Normals = new XMFLOAT3[m_nVertices];
	//m_pnTextureNumbers = new UINT[m_nVertices];

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = OnGetHeight(x, z, pHeightMapImage);
			//float fColor = OnGetHeight(x, z, pMapImage);

			m_pxmf3Positions[i] = XMFLOAT3((x*m_xmf3Scale.x), fHeight, (z*m_xmf3Scale.z));
			//m_pxmf3Normals[i] = pHeightMapImage->GetHeightMapNormal(x, z);
			m_pxmf2TextureCoords0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			m_pxmf2TextureCoords1[i] = XMFLOAT2(float(x) / float(m_xmf3Scale.x*0.5f), float(z) / float(m_xmf3Scale.z*0.5f));
			//m_pnTextureNumbers[i] = (fColor > 250.0f) ? 1 : 2;

			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	XMFLOAT3 xmf3Center;
	XMFLOAT3 xmf3Extents;
	FindXYZ(m_pxmf3Positions, m_nVertices, xmf3Center, xmf3Extents);

	SetAABB(xmf3Center, xmf3Extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void *pContext)
{
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	BYTE *pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z*nWidth)] * xmf3Scale.y;
	return(fHeight);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
