#include "stdafx.h"
#include "Mesh.h"

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

CMesh::~CMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dTangentBuffer) m_pd3dTangentBuffer->Release();
	if (m_pd3dBinormalBuffer) m_pd3dBinormalBuffer->Release();
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pd3dTextureCoord1Buffer) m_pd3dTextureCoord1Buffer->Release();
	if (m_pd3dMaterialIndexBuffer) m_pd3dMaterialIndexBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();

	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pxmf3Tangents) delete[] m_pxmf3Tangents;
	if (m_pxmf3Binormals) delete[] m_pxmf3Binormals;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
	if (m_pnMaterialIndices) delete[] m_pnMaterialIndices;
	if (m_pnIndices) delete[] m_pnIndices;
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;

	if (m_pd3dColorUploadBuffer) m_pd3dColorUploadBuffer->Release();
	m_pd3dColorUploadBuffer = NULL;

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;

	if (m_pd3dTangentUploadBuffer) m_pd3dTangentUploadBuffer->Release();
	m_pd3dTangentUploadBuffer = NULL;

	if (m_pd3dBinormalUploadBuffer) m_pd3dBinormalUploadBuffer->Release();
	m_pd3dBinormalUploadBuffer = NULL;

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;

	if (m_pd3dTextureCoord1UploadBuffer) m_pd3dTextureCoord1UploadBuffer->Release();
	m_pd3dTextureCoord1UploadBuffer = NULL;

	if (m_pd3dMaterialIndexUploadBuffer) m_pd3dMaterialIndexUploadBuffer->Release();
	m_pd3dMaterialIndexUploadBuffer = NULL;

	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dIndexUploadBuffer = NULL;
};

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);

		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CCubeMesh::CCubeMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT3 xmf3Center, float fWidth, float fHeight, float fDepth) : CMesh()
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth, fy = fHeight, fz = fDepth;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	m_pxmf3Positions[0] = XMFLOAT3(-fx + xmf3Center.x, +fy + xmf3Center.y, -fz + xmf3Center.z);
	m_pxmf3Positions[1] = XMFLOAT3(+fx + xmf3Center.x, +fy + xmf3Center.y, -fz + xmf3Center.z);
	m_pxmf3Positions[2] = XMFLOAT3(+fx + xmf3Center.x, +fy + xmf3Center.y, +fz + xmf3Center.z);
	m_pxmf3Positions[3] = XMFLOAT3(-fx + xmf3Center.x, +fy + xmf3Center.y, +fz + xmf3Center.z);
	m_pxmf3Positions[4] = XMFLOAT3(-fx + xmf3Center.x, -fy + xmf3Center.y, -fz + xmf3Center.z);
	m_pxmf3Positions[5] = XMFLOAT3(+fx + xmf3Center.x, -fy + xmf3Center.y, -fz + xmf3Center.z);
	m_pxmf3Positions[6] = XMFLOAT3(+fx + xmf3Center.x, -fy + xmf3Center.y, +fz + xmf3Center.z);
	m_pxmf3Positions[7] = XMFLOAT3(-fx + xmf3Center.x, -fy + xmf3Center.y, +fz + xmf3Center.z);

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

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

	m_pd3dIndexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}

CCubeMesh::~CCubeMesh()
{
}

void CCubeMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);

	CMesh::Render(pd3dCommandList);
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

CStandardMesh::CStandardMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CStandardMesh::~CStandardMesh()
{
}

void CStandardMesh::LoadMeshFromFBX(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FbxMesh *pfbxMesh)
{

	int nPolygon = pfbxMesh->GetPolygonCount();
	int nVertices = nPolygon * 3;

	m_nVertices = nVertices;
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	FbxGeometryElementNormal *pFbxGeoElemNorm = pfbxMesh->GetElementNormal();
	if(pFbxGeoElemNorm)
		m_pxmf3Normals = new XMFLOAT3[m_nVertices];

	FbxGeometryElementUV *pFbxGeoElemUV = pfbxMesh->GetElementUV();
	if (pFbxGeoElemUV)
	{
		m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
		m_pnMaterialIndices = new int[m_nVertices];
	}

	FbxGeometryElementBinormal *pFbxGeoElemBinormal = pfbxMesh->GetElementBinormal();
	if(pFbxGeoElemBinormal)
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

			auto fbxMaterialIndex = &(pfbxMesh->GetElementMaterial()->GetIndexArray());
			auto fbxMaterialMappingMode = pfbxMesh->GetElementMaterial()->GetMappingMode();

			switch (fbxMaterialMappingMode)
			{
			case FbxGeometryElement::eByPolygon:
			{
				m_pnMaterialIndices[i * 3 + j] = fbxMaterialIndex->GetAt(i);
			}
			break;
			case FbxGeometryElement::eAllSame:
			{
				m_pnMaterialIndices[i * 3 + j] = fbxMaterialIndex->GetAt(0);
			}
			break;
			}

			nIndex++;
		}
	}

	// 정점 버퍼 생성( 위치 )
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	// 정점 버퍼 생성( 법선 벡터 )
	if (pFbxGeoElemNorm)
	{
		m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

		m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
		m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
		m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
	}

	// 정점 버퍼 생성( 종법선 벡터 )
	if (pFbxGeoElemBinormal)
	{
		m_pd3dBinormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Binormals, sizeof(XMFLOAT3) * m_nVertices,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBinormalUploadBuffer);

		m_d3dBinormalBufferView.BufferLocation = m_pd3dBinormalBuffer->GetGPUVirtualAddress();
		m_d3dBinormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
		m_d3dBinormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
	}

	// 정점 버퍼 생성( 접선 벡터 )
	if (pFbxGeoElemTangent)
	{
		m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTangentUploadBuffer);

		m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
		m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
		m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
	}

	// 정점 버퍼 생성( 텍스쳐 좌표 )
	if (pFbxGeoElemUV)
	{
		m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

		m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
		m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
		m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
	}

	XMFLOAT3 Center, Extents;
	FindXYZ(m_pxmf3Positions, m_nVertices, Center, Extents);

	SetOOBB(Center, Extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

void CStandardMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { m_d3dPositionBufferView, m_d3dNormalBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);

	CMesh::Render(pd3dCommandList);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CUIRect::CUIRect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT2 xmf2Center, XMFLOAT2 xmf2Size) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

	m_nVertices = 1;
	m_pxmf2Positions = new XMFLOAT2[m_nVertices];
	m_pxmf2Sizes = new XMFLOAT2[m_nVertices];

	m_pxmf2Positions[0] = xmf2Center;
	m_pxmf2Sizes[0] = xmf2Size;

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2Positions, sizeof(XMFLOAT2), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT2);

	m_pd3dSizeBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2Sizes, sizeof(XMFLOAT2), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dSizeUploadBuffer);

	m_d3dSizeBufferView.BufferLocation = m_pd3dSizeBuffer->GetGPUVirtualAddress();
	m_d3dSizeBufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dSizeBufferView.SizeInBytes = sizeof(XMFLOAT2);
}

CUIRect::~CUIRect()
{
	if (m_pd3dSizeBuffer) m_pd3dSizeBuffer->Release();

	if (m_pxmf2Positions) delete[] m_pxmf2Positions;
	if (m_pxmf2Sizes) delete[] m_pxmf2Sizes;
}

void CUIRect::ReleaseUploadBuffers()
{
	if (m_pd3dSizeUploadBuffer) m_pd3dSizeUploadBuffer->Release();
	m_pd3dSizeUploadBuffer = NULL;

	CMesh::ReleaseUploadBuffers();
}

void CUIRect::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dSizeBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);

	CMesh::Render(pd3dCommandList);
}

/////////////////////////////////////////////////////////////////////////////////////////////////



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

CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void *pContext0, void *pContext1) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = nWidth * nLength;
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

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
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	m_pxmf2TextureCoords1 = new XMFLOAT2[m_nVertices];
	//m_pnTextureNumbers = new UINT[m_nVertices];

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = OnGetHeight(x, z, pHeightMapImage);
			//float fColor = OnGetHeight(x, z, pMapImage);

			m_pxmf3Positions[i] = XMFLOAT3((x*m_xmf3Scale.x), fHeight, (z*m_xmf3Scale.z));
			m_pxmf4Colors[i] = Vector4::Add(OnGetColor(x, z, pHeightMapImage), xmf4Color);
			//m_pxmf3Normals[i] = pHeightMapImage->GetHeightMapNormal(x, z);
			m_pxmf2TextureCoords0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			m_pxmf2TextureCoords1[i] = XMFLOAT2(float(x) / float(m_xmf3Scale.x*0.5f), float(z) / float(m_xmf3Scale.z*0.5f));
			//m_pnTextureNumbers[i] = (fColor > 250.0f) ? 1 : 2;

			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dColorBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColorUploadBuffer);

	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;

	//m_pd3dNormalBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

	//m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	//m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	//m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_pd3dTextureCoord1Buffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord1UploadBuffer);

	m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	//m_pd3TextureNumberBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pnTextureNumbers, sizeof(UINT) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureNumberUploadBuffer);

	//m_d3dTextureNumberBufferView.BufferLocation = m_pd3TextureNumberBuffer->GetGPUVirtualAddress();
	//m_d3dTextureNumberBufferView.StrideInBytes = sizeof(UINT);
	//m_d3dTextureNumberBufferView.SizeInBytes = sizeof(UINT) * m_nVertices;

	m_nIndices = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	m_pnIndices = new UINT[m_nIndices];

	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	m_pd3dIndexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
	if (m_pd3TextureNumberBuffer) m_pd3TextureNumberBuffer->Release();

	if (m_pnTextureNumbers) delete[] m_pnTextureNumbers;
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

XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void *pContext)
{
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);
	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);
	return(xmf4Color);
}

void CHeightMapGridMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dTextureNumberUploadBuffer) m_pd3dTextureNumberUploadBuffer->Release();
	m_pd3dTextureNumberUploadBuffer = NULL;
}

void CHeightMapGridMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	//D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5] = { m_d3dPositionBufferView, m_d3dColorBufferView, m_d3dNormalBufferView, m_d3dTextureCoord0BufferView, m_d3dTextureCoord1BufferView};
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[4] = { m_d3dPositionBufferView, m_d3dColorBufferView, m_d3dTextureCoord0BufferView, m_d3dTextureCoord1BufferView};
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 4, pVertexBufferViews);

	CMesh::Render(pd3dCommandList);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkyBoxMesh::CSkyBoxMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh()
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	// Front Quad (quads point inward)
	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[2] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, -fx, +fx);
	// Back Quad										
	m_pxmf3Positions[6] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[7] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[8] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[9] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[10] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[11] = XMFLOAT3(-fx, -fx, -fx);
	// Left Quad										
	m_pxmf3Positions[12] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[13] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[14] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[15] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[16] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[17] = XMFLOAT3(-fx, -fx, +fx);
	// Right Quad										
	m_pxmf3Positions[18] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[19] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[20] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[21] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[22] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[23] = XMFLOAT3(+fx, -fx, -fx);
	// Top Quad											
	m_pxmf3Positions[24] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[25] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[26] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[27] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[28] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[29] = XMFLOAT3(+fx, +fx, +fx);
	// Bottom Quad										
	m_pxmf3Positions[30] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[31] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[32] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[33] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[34] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[35] = XMFLOAT3(+fx, -fx, -fx);

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CSkyBoxMesh::~CSkyBoxMesh()
{
}

void CSkyBoxMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);

	CMesh::Render(pd3dCommandList);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRectMesh::CRectMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth, float fHeight) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 6;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f;

	m_pxmf3Positions[0] = XMFLOAT3(+fx, +fy, 0.0f);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, -fy, 0.0f);
	m_pxmf3Positions[2] = XMFLOAT3(-fx, -fy, 0.0f);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, -fy, 0.0f);
	m_pxmf3Positions[4] = XMFLOAT3(-fx, +fy, 0.0f);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, +fy, 0.0f);

	m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
	m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
	m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
}

CRectMesh::~CRectMesh()
{

}

void CRectMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}
