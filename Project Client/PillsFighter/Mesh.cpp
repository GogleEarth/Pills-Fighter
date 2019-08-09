#include "stdafx.h"
#include "Mesh.h"
#include "Model.h"

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

CMesh::~CMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	m_pd3dPositionBuffer = NULL;
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	m_pd3dColorBuffer = NULL;
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	m_pd3dNormalBuffer = NULL;
	if (m_pd3dTangentBuffer) m_pd3dTangentBuffer->Release();
	m_pd3dTangentBuffer = NULL;
	if (m_pd3dBinormalBuffer) m_pd3dBinormalBuffer->Release();
	m_pd3dBinormalBuffer = NULL;
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	m_pd3dTextureCoord0Buffer = NULL;
	if (m_pd3dTextureCoord1Buffer) m_pd3dTextureCoord1Buffer->Release();
	m_pd3dTextureCoord1Buffer = NULL;

	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
	m_pxmf3Positions = NULL;
	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	m_pxmf4Colors = NULL;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	m_pxmf3Normals = NULL;
	if (m_pxmf3Tangents) delete[] m_pxmf3Tangents;
	m_pxmf3Tangents = NULL;
	if (m_pxmf3Binormals) delete[] m_pxmf3Binormals;
	m_pxmf3Binormals = NULL;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	m_pxmf2TextureCoords0 = NULL;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
	m_pxmf2TextureCoords1 = NULL;

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
			m_ppd3dSubSetIndexBuffers[i] = NULL;
			if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
			m_ppnSubSetIndices[i] = NULL;
		}
		if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		m_ppd3dSubSetIndexBuffers = NULL;
		if (m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;
		m_pd3dSubSetIndexBufferViews = NULL;

		if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
		m_pnSubSetIndices = NULL;
		if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
		m_ppnSubSetIndices = NULL;
	}
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
	
	if ((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
			m_ppd3dSubSetIndexUploadBuffers[i] = NULL;
		}
		if (m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
};

void CMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
}

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet, int nInstances)
{
	UpdateShaderVariables(pd3dCommandList);

	OnPreRender(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], nInstances, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstances, m_nOffset, 0);
	}
}

void CMesh::OnPreRenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
}

void CMesh::RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet, int nInstances)
{
	UpdateShaderVariables(pd3dCommandList);

	OnPreRenderToShadow(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], nInstances, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstances, m_nOffset, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CCubeMesh::CCubeMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT3 xmf3Center, XMFLOAT3 xmf3Extents) : CMesh()
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT*[m_nSubMeshes];

	m_ppd3dSubSetIndexBuffers = new ID3D12Resource*[m_nSubMeshes];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource*[m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

	m_pnSubSetIndices[0] = 36;
	m_ppnSubSetIndices[0] = new UINT[m_pnSubSetIndices[0]];

	m_ppnSubSetIndices[0][0] = 3; m_ppnSubSetIndices[0][1] = 1; m_ppnSubSetIndices[0][2] = 0;
	m_ppnSubSetIndices[0][3] = 2; m_ppnSubSetIndices[0][4] = 1; m_ppnSubSetIndices[0][5] = 3;
	m_ppnSubSetIndices[0][6] = 0; m_ppnSubSetIndices[0][7] = 5; m_ppnSubSetIndices[0][8] = 4;
	m_ppnSubSetIndices[0][9] = 1; m_ppnSubSetIndices[0][10] = 5; m_ppnSubSetIndices[0][11] = 0;
	m_ppnSubSetIndices[0][12] = 3; m_ppnSubSetIndices[0][13] = 4; m_ppnSubSetIndices[0][14] = 7;
	m_ppnSubSetIndices[0][15] = 0; m_ppnSubSetIndices[0][16] = 4; m_ppnSubSetIndices[0][17] = 3;
	m_ppnSubSetIndices[0][18] = 1; m_ppnSubSetIndices[0][19] = 6; m_ppnSubSetIndices[0][20] = 5;
	m_ppnSubSetIndices[0][21] = 2; m_ppnSubSetIndices[0][22] = 6; m_ppnSubSetIndices[0][23] = 1;
	m_ppnSubSetIndices[0][24] = 2; m_ppnSubSetIndices[0][25] = 7; m_ppnSubSetIndices[0][26] = 6;
	m_ppnSubSetIndices[0][27] = 3; m_ppnSubSetIndices[0][28] = 7; m_ppnSubSetIndices[0][29] = 2;
	m_ppnSubSetIndices[0][30] = 6; m_ppnSubSetIndices[0][31] = 4; m_ppnSubSetIndices[0][32] = 5;
	m_ppnSubSetIndices[0][33] = 7; m_ppnSubSetIndices[0][34] = 4; m_ppnSubSetIndices[0][35] = 6;

	m_ppd3dSubSetIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[0]);

	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];
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

CStandardMesh::CStandardMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CStandardMesh::~CStandardMesh()
{
}

void CStandardMesh::LoadMeshFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pfile)
{
	BYTE nstrLength;
	char pstrToken[64] = { 0 };

	m_nType |= TYPE_STANDARD_MESH;

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

			m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
				D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

			m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
			m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
			m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
		}
		else if (!strcmp(pstrToken, "<TextureCoords>:"))
		{
			m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

			fread_s(m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, sizeof(XMFLOAT2), m_nVertices, pfile);

			m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices,
				D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

			m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
			m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
			m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			m_pxmf3Normals = new XMFLOAT3[m_nVertices];

			fread_s(m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, sizeof(XMFLOAT3), m_nVertices, pfile);

			m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices,
				D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

			m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
			m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
			m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
		}
		else if (!strcmp(pstrToken, "<Binormals>:"))
		{
			m_pxmf3Binormals = new XMFLOAT3[m_nVertices];

			fread_s(m_pxmf3Binormals, sizeof(XMFLOAT3) * m_nVertices, sizeof(XMFLOAT3), m_nVertices, pfile);

			m_pd3dBinormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Binormals, sizeof(XMFLOAT3) * m_nVertices,
				D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBinormalUploadBuffer);

			m_d3dBinormalBufferView.BufferLocation = m_pd3dBinormalBuffer->GetGPUVirtualAddress();
			m_d3dBinormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
			m_d3dBinormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			m_pxmf3Tangents = new XMFLOAT3[m_nVertices];

			fread_s(m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices, sizeof(XMFLOAT3), m_nVertices, pfile);

			m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices,
				D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTangentUploadBuffer);

			m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
			m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
			m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			fread_s(&m_nSubMeshes, sizeof(int), sizeof(int), 1, pfile);

			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT*[m_nSubMeshes];

				m_ppd3dSubSetIndexBuffers = new ID3D12Resource*[m_nSubMeshes];
				m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource*[m_nSubMeshes];
				m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

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

							m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[i]);

							m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
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

void CStandardMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5] = { m_d3dPositionBufferView, m_d3dNormalBufferView, m_d3dBinormalBufferView, m_d3dTangentBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 5, pVertexBufferViews);
}

void CStandardMesh::OnPreRenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CSkinnedMesh::CSkinnedMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
}

CSkinnedMesh::~CSkinnedMesh()
{
	if (m_pxmu4BoneIndices)	delete[] m_pxmu4BoneIndices;
	m_pxmu4BoneIndices = NULL;
	if (m_pxmf4BoneWeights) delete[] m_pxmf4BoneWeights;
	m_pxmf4BoneWeights = NULL;

	if (m_pxmf4x4BindPoseBoneOffsets) delete[] m_pxmf4x4BindPoseBoneOffsets;
	m_pxmf4x4BindPoseBoneOffsets = NULL;
	if (m_ppstrSkinningBoneNames) delete[] m_ppstrSkinningBoneNames;
	m_ppstrSkinningBoneNames = NULL;

	if (m_pd3dBoneIndexBuffer) m_pd3dBoneIndexBuffer->Release();
	m_pd3dBoneIndexBuffer = NULL;
	if (m_pd3dBoneWeightBuffer) m_pd3dBoneWeightBuffer->Release();
	m_pd3dBoneWeightBuffer = NULL;

	ReleaseShaderVariables();
}

void CSkinnedMesh::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbBoneOffsets = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbBoneOffsets->Map(0, NULL, (void **)&m_pcbxmf4x4BoneOffsets);
}

void CSkinnedMesh::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dcbBoneOffsets)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneOffsetsGpuVirtualAddress = m_pd3dcbBoneOffsets->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_BONE_OFFSETS, d3dcbBoneOffsetsGpuVirtualAddress); //Skinned Bone Offsets

		for (int i = 0; i < m_nSkinningBones; i++)
		{
			XMStoreFloat4x4(&m_pcbxmf4x4BoneOffsets[i], XMMatrixTranspose(XMLoadFloat4x4(&m_pxmf4x4BindPoseBoneOffsets[i])));
		}
	}

	if (m_pd3dcbBoneTransforms)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = m_pd3dcbBoneTransforms->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_BONE_TRANSFORMS, d3dcbBoneTransformsGpuVirtualAddress); //Skinned Bone Transforms

		for (int i = 0; i < m_nSkinningBones; i++)
		{
			XMStoreFloat4x4(&m_pcbxmf4x4BoneTransforms[i], XMMatrixTranspose(XMLoadFloat4x4(&m_ppSkinningBoneFrameCaches[i]->GetWorldTransf())));
			//memcpy(&m_pcbxmf4x4BoneTransforms[i], &m_ppSkinningBoneFrameCaches[i]->GetWorldTransf(), sizeof(XMFLOAT4X4));
		}
	}
}

void CSkinnedMesh::ReleaseShaderVariables()
{
	if (m_pd3dcbBoneOffsets) m_pd3dcbBoneOffsets->Release();
	m_pd3dcbBoneOffsets = NULL;
}

void CSkinnedMesh::ReleaseUploadBuffers()
{
	CStandardMesh::ReleaseUploadBuffers();

	if (m_pd3dBoneIndexUploadBuffer) m_pd3dBoneIndexUploadBuffer->Release();
	m_pd3dBoneIndexUploadBuffer = NULL;

	if (m_pd3dBoneWeightUploadBuffer) m_pd3dBoneWeightUploadBuffer->Release();
	m_pd3dBoneWeightUploadBuffer = NULL;
}

void CSkinnedMesh::LoadSkinInfoFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pfile)
{
	BYTE nstrLength;
	char pstrToken[64] = { 0 };

	m_nType |= TYPE_SKINNED_MESH;

	fread_s(&m_nSkinningBones, sizeof(int), sizeof(int), 1, pfile);
	fread_s(&m_nVertices, sizeof(UINT), sizeof(int), 1, pfile);

	fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
	fread_s(m_pstrSkinnedMeshName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

	while(true)
	{
		fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
		fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
		pstrToken[nstrLength] = '\0';

		if (!strcmp(pstrToken, "<BoneNames>:"))
		{
			if (m_nSkinningBones > 0)
			{
				m_ppstrSkinningBoneNames = new char[m_nSkinningBones][64];
				m_ppSkinningBoneFrameCaches = new CModel*[m_nSkinningBones];

				for (int i = 0; i < m_nSkinningBones; i++)
				{
					fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
					fread_s(m_ppstrSkinningBoneNames[i], sizeof(char) * 64, sizeof(char), nstrLength, pfile);
					m_ppstrSkinningBoneNames[i][nstrLength] = '\0';

					m_ppSkinningBoneFrameCaches[i] = NULL;
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneOffsets>:"))
		{
			if (m_nSkinningBones > 0)
			{
				m_pxmf4x4BindPoseBoneOffsets = new XMFLOAT4X4[m_nSkinningBones];

				fread_s(m_pxmf4x4BindPoseBoneOffsets, sizeof(XMFLOAT4X4) * m_nSkinningBones, sizeof(XMFLOAT4X4), m_nSkinningBones, pfile);
			}
		}
		else if (!strcmp(pstrToken, "<BoneIndices>:"))
		{
			if (m_nVertices > 0)
			{
				m_pxmu4BoneIndices = new XMUINT4[m_nVertices];
				fread_s(m_pxmu4BoneIndices, sizeof(XMUINT4) * m_nVertices, sizeof(XMUINT4), m_nVertices, pfile);

				m_pd3dBoneIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmu4BoneIndices, sizeof(XMUINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneIndexUploadBuffer);

				m_d3dBoneIndexBufferView.BufferLocation = m_pd3dBoneIndexBuffer->GetGPUVirtualAddress();
				m_d3dBoneIndexBufferView.StrideInBytes = sizeof(XMUINT4);
				m_d3dBoneIndexBufferView.SizeInBytes = sizeof(XMUINT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BoneWeights>:"))
		{
			if (m_nVertices > 0)
			{
				m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];
				fread_s(m_pxmf4BoneWeights, sizeof(XMFLOAT4) * m_nVertices, sizeof(XMFLOAT4), m_nVertices, pfile);

				m_pd3dBoneWeightBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4BoneWeights, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneWeightUploadBuffer);

				m_d3dBoneWeightBufferView.BufferLocation = m_pd3dBoneWeightBuffer->GetGPUVirtualAddress();
				m_d3dBoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
				m_d3dBoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[7] = { m_d3dPositionBufferView, m_d3dNormalBufferView, m_d3dBinormalBufferView, m_d3dTangentBufferView, m_d3dTextureCoord0BufferView, m_d3dBoneIndexBufferView, m_d3dBoneWeightBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 7, pVertexBufferViews);
}

void CSkinnedMesh::OnPreRenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[4] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dBoneIndexBufferView, m_d3dBoneWeightBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 4, pVertexBufferViews);
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

///////
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

	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf3Normals = new XMFLOAT3[m_nVertices];
	m_pxmf4Colors = new XMFLOAT4[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	m_pxmf2TextureCoords1 = new XMFLOAT2[m_nVertices];

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = OnGetHeight(x, z, pHeightMapImage);
			//float fColor = OnGetHeight(x, z, pMapImage);

			m_pxmf3Positions[i] = XMFLOAT3((x*m_xmf3Scale.x), fHeight, (z*m_xmf3Scale.z));
			m_pxmf4Colors[i] = Vector4::Add(OnGetColor(x, z, pHeightMapImage), xmf4Color);
			m_pxmf3Normals[i] = pHeightMapImage->GetHeightMapNormal(x, z);
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

	m_pd3dNormalBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dColorBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColorUploadBuffer);

	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;

	m_pd3dTextureCoord0Buffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_pd3dTextureCoord1Buffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord1UploadBuffer);

	m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT*[m_nSubMeshes];

	m_ppd3dSubSetIndexBuffers = new ID3D12Resource*[m_nSubMeshes];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource*[m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

	m_pnSubSetIndices[0] = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	m_ppnSubSetIndices[0] = new UINT[m_pnSubSetIndices[0]];

	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	m_ppd3dSubSetIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[0]);

	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];

	XMFLOAT3 xmf3Center;
	XMFLOAT3 xmf3Extents;
	FindXYZ(m_pxmf3Positions, m_nVertices, xmf3Center, xmf3Extents);

	SetAABB(xmf3Center, xmf3Extents);
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

void CHeightMapGridMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5] = { m_d3dPositionBufferView, m_d3dNormalBufferView, m_d3dColorBufferView, m_d3dTextureCoord0BufferView, m_d3dTextureCoord1BufferView};
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 5, pVertexBufferViews);
}

void CHeightMapGridMesh::OnPreRenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList)
{
	//D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5] = { m_d3dPositionBufferView, m_d3dColorBufferView, m_d3dNormalBufferView, m_d3dTextureCoord0BufferView, m_d3dTextureCoord1BufferView};
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
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

void CSkyBoxMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CRect::CRect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT2 xmf2Center, XMFLOAT2 xmf2Size) : CMesh(pd3dDevice, pd3dCommandList)
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

CRect::~CRect()
{
	if (m_pd3dSizeBuffer) m_pd3dSizeBuffer->Release();
	m_pd3dSizeBuffer = NULL;

	if (m_pxmf2Positions) delete[] m_pxmf2Positions;
	m_pxmf2Positions = NULL;
	if (m_pxmf2Sizes) delete[] m_pxmf2Sizes;
	m_pxmf2Sizes = NULL;
}

void CRect::ReleaseUploadBuffers()
{
	if (m_pd3dSizeUploadBuffer) m_pd3dSizeUploadBuffer->Release();
	m_pd3dSizeUploadBuffer = NULL;

	CMesh::ReleaseUploadBuffers();
}

void CRect::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dSizeBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
}
