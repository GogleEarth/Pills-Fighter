#include "stdafx.h"
#include "Mesh.h"


CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, TCHAR *pstrFileName)
{
	std::wifstream InFile(pstrFileName);

	TCHAR pstrToken[64] = { '\0' };

	for (; ; )
	{
		InFile >> pstrToken;
		if (!InFile) break;

		if (!_tcscmp(pstrToken, _T("<Vertices>:")))
		{
			InFile >> m_nVertices;
			m_pVertices = new CDiffusedVertex[m_nVertices];

			float x, y, z;

			for (UINT i = 0; i < m_nVertices; i++)
			{
				InFile >> x >> y >> z;
				m_pVertices[i] = CDiffusedVertex(XMFLOAT3(x, y, z), RANDOM_COLOR);
			}

		}
		else if (!_tcscmp(pstrToken, _T("<Indices>:")))
		{
			InFile >> m_nIndices;
			m_pnIndices = new UINT[m_nIndices];
			for (UINT i = 0; i < m_nIndices; i++)
			{
				InFile >> m_pnIndices[i];
				m_pnIndices[i]--;
			}
		}
		else if (!_tcscmp(pstrToken, _T("<BoundingBox>")))
		{
			float x, y, z;
			InFile >> x >> y >> z;

			SetOOBB(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(x, y, z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
	}

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pVertices,
		sizeof(CDiffusedVertex) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(CDiffusedVertex);
	m_d3dVertexBufferView.SizeInBytes = sizeof(CDiffusedVertex) * m_nVertices;

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

	AddRef();
}

CMesh::~CMesh()
{
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();

	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
}

void CMesh::ReleaseUploadBuffers()
{
	//정점 버퍼를 위한 업로드 버퍼를 소멸시킨다. 
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	m_pd3dVertexUploadBuffer = NULL;

	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dIndexUploadBuffer = NULL;
};

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);

	if (m_pd3dIndexBuffer)
	{
		//인덱스 버퍼가 있으면 인덱스 버퍼를 파이프라인(IA: 입력 조립기)에 연결하고 인덱스를 사용하여 렌더링한다. 
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}