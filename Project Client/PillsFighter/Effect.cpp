#include "stdafx.h"
#include "Effect.h"
#include "Model.h"
#include "GameObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEffect::CEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fDuration, UINT nBytes)
{
	m_nVertices = 0;
	m_nBytes = nBytes;
	m_fDuration = fDuration;

	m_pd3dInitVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, m_nBytes * MAX_EFFECT_INIT_VERTEX_COUNT);

	m_pd3dInitVertexBuffer->Map(0, NULL, (void**)&m_pMappedInitVertices);

	m_d3dInitVertexBufferView.BufferLocation = m_pd3dInitVertexBuffer->GetGPUVirtualAddress();
	m_d3dInitVertexBufferView.SizeInBytes = m_nBytes * MAX_EFFECT_INIT_VERTEX_COUNT;
	m_d3dInitVertexBufferView.StrideInBytes = m_nBytes;


	m_pd3dBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);

	m_pd3dDummyBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_SOURCE, NULL);

	m_pd3dReadBackBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64),
		D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);

	for (int i = 0; i < 2; i++)
	{
		m_pd3dVertexBuffer[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, m_nBytes * MAX_EFFECT_VERTEX_COUNT,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);

		m_d3dVertexBufferView[i].BufferLocation = m_pd3dVertexBuffer[i]->GetGPUVirtualAddress();
		m_d3dVertexBufferView[i].SizeInBytes = m_nBytes * MAX_EFFECT_VERTEX_COUNT;
		m_d3dVertexBufferView[i].StrideInBytes = m_nBytes;

		m_d3dSOBufferView[i].BufferFilledSizeLocation = m_pd3dBuffer->GetGPUVirtualAddress();
		m_d3dSOBufferView[i].BufferLocation = m_pd3dVertexBuffer[i]->GetGPUVirtualAddress();
		m_d3dSOBufferView[i].SizeInBytes = m_nBytes * MAX_EFFECT_VERTEX_COUNT;
	}
}

CEffect::~CEffect()
{
	if (m_pd3dBuffer)
	{
		m_pd3dBuffer->Release();
		m_pd3dBuffer = NULL;
	}

	if (m_pd3dDummyBuffer)
	{
		m_pd3dDummyBuffer->Release();
		m_pd3dDummyBuffer = NULL;
	}

	if (m_pd3dReadBackBuffer)
	{
		m_pd3dReadBackBuffer->Release();
		m_pd3dReadBackBuffer = NULL;
	}

	for (int i = 0; i < 2; i++)
	{
		if (m_pd3dVertexBuffer[i])
		{
			m_pd3dVertexBuffer[i]->Release();
			m_pd3dVertexBuffer[i] = NULL;
		}
	}

	if (m_pd3dInitVertexBuffer)
	{
		m_pd3dInitVertexBuffer->Unmap(0, NULL);
		m_pd3dInitVertexBuffer->Release();

		m_pd3dInitVertexBuffer = NULL;
	}

	ReleaseShaderVariables();
}

void CEffect::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255);

	m_pd3dcbEffect = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes);

	m_pd3dcbEffect->Map(0, NULL, (void**)&m_pcbMappedEffect);
}

void CEffect::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pcbMappedEffect->m_fElapsedTime = m_fElapsedTime;
	m_pcbMappedEffect->m_fDuration = m_fDuration;

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_EFFECT, m_pd3dcbEffect->GetGPUVirtualAddress());
}

void CEffect::ReleaseShaderVariables()
{
	if (m_pd3dcbEffect)
	{
		m_pd3dcbEffect->Unmap(0, NULL);
		m_pd3dcbEffect->Release();

		m_pd3dcbEffect = NULL;
	}
}

void CEffect::Animate(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CEffect::ReadVertexCount(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nDrawBufferIndex == 0)
	{
		m_nDrawBufferIndex = 1;
		m_nSOBufferIndex = 0;
	}
	else if (m_nDrawBufferIndex == 1)
	{
		m_nDrawBufferIndex = 0;
		m_nSOBufferIndex = 1;
	}

	D3D12_RANGE d3dRange = { 0, sizeof(UINT64) };
	UINT64 *nFilledSize = NULL;

	m_pd3dReadBackBuffer->Map(0, &d3dRange, (void**)&nFilledSize);

	m_nVertices = static_cast<int>((*nFilledSize) / m_nBytes);

	d3dRange = { 0, 0 };
	m_pd3dReadBackBuffer->Unmap(0, &d3dRange);
}

void CEffect::SORender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nVertices | m_nInitVertices)
	{
		pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		UpdateShaderVariables(pd3dCommandList);

		pd3dCommandList->SOSetTargets(0, 1, &m_d3dSOBufferView[m_nSOBufferIndex]);
	}

	if (m_nVertices > 0)
	{
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView[m_nDrawBufferIndex]);

		pd3dCommandList->DrawInstanced(m_nVertices, 1, 0, 0);
	}

	if (m_nInitVertices > 0)
	{
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dInitVertexBufferView);

		pd3dCommandList->DrawInstanced(m_nInitVertices, 1, 0, 0);
	}
}

void CEffect::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nVertices | m_nInitVertices)
	{
		pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		UpdateShaderVariables(pd3dCommandList);
	}

	if (m_nVertices > 0)
	{
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView[m_nDrawBufferIndex]);

		pd3dCommandList->DrawInstanced(m_nVertices, 1, 0, 0);
	}

	if (m_nInitVertices > 0)
	{
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dInitVertexBufferView);

		pd3dCommandList->DrawInstanced(m_nInitVertices, 1, 0, 0);
	}
}

void CEffect::AfterRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dBuffer, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
	pd3dCommandList->CopyResource(m_pd3dReadBackBuffer, m_pd3dBuffer);
	::TransitionResourceState(pd3dCommandList, m_pd3dBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	pd3dCommandList->CopyResource(m_pd3dBuffer, m_pd3dDummyBuffer);
	::TransitionResourceState(pd3dCommandList, m_pd3dBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);

	m_nInitVertices = 0;
}

void CEffect::AddVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, int nEffectAniType, int nAngle, XMFLOAT4 xmf4Color)
{
	((CEffectVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf3Position = xmf3Position;
	((CEffectVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf2Size = xmf2Size;
	((CEffectVertex*)m_pMappedInitVertices)[m_nInitVertices].m_nAngle = nAngle;
	((CEffectVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf4Color = xmf4Color;
	((CEffectVertex*)m_pMappedInitVertices)[m_nInitVertices++].m_fAge = 0.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLaserBeam::CLaserBeam(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fDuration) : CEffect(pd3dDevice, pd3dCommandList, fDuration, sizeof(CLaserVertex))
{
}

CLaserBeam::~CLaserBeam()
{
}

void CLaserBeam::AddVertexWithLookV(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, XMFLOAT3 xmf3Look, int nEffectAniType, XMFLOAT4 xmf4Color)
{
	((CLaserVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf3Position = xmf3Position;
	((CLaserVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf2Size = xmf2Size;
	((CLaserVertex*)m_pMappedInitVertices)[m_nInitVertices].m_fAge = 0.0f;
	((CLaserVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf4Color = xmf4Color;
	((CLaserVertex*)m_pMappedInitVertices)[m_nInitVertices++].m_xmf3Look = xmf3Look;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CFollowEffect::CFollowEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fDuration, UINT nBytes) : CEffect(pd3dDevice, pd3dCommandList, fDuration, nBytes)
{
}

CFollowEffect::~CFollowEffect()
{
}

void CFollowEffect::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_FOLLOW_EFFECT) + 255) & ~255);

	m_pd3dcbFollowEffect = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes);

	m_pd3dcbFollowEffect->Map(0, NULL, (void**)&m_pcbMappedFollowEffect);

	CEffect::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CFollowEffect::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pcbMappedFollowEffect->xmf3Position = m_xmf3Position;
	m_pcbMappedFollowEffect->m_fElapsedTime = m_fElapsedTime;
	m_pcbMappedFollowEffect->m_fDuration = m_fDuration;

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_FOLLOW_EFFECT_INFO, m_pd3dcbFollowEffect->GetGPUVirtualAddress());

	CEffect::UpdateShaderVariables(pd3dCommandList);
}

void CFollowEffect::ReleaseShaderVariables()
{
	if (m_pd3dcbFollowEffect)
	{
		m_pd3dcbFollowEffect->Unmap(0, NULL);
		m_pd3dcbFollowEffect->Release();

		m_pd3dcbFollowEffect = NULL;
	}

	CEffect::ReleaseShaderVariables();
}

void CFollowEffect::SetFollowObject(CGameObject *pObject, CModel *pModel)
{
	pObject->AddEffect(this);
	m_pFollowFrame = pModel;
}

void CFollowEffect::SetToFollowFramePosition()
{
	if (m_pFollowFrame)
	{
		m_xmf3Position = m_pFollowFrame->GetPosition();
	}
}

void CFollowEffect::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (!m_bShow) return;

	if (m_nVertices | m_nInitVertices)
	{
		pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		UpdateShaderVariables(pd3dCommandList);
	}

	if (m_nVertices > 0)
	{
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView[m_nDrawBufferIndex]);

		pd3dCommandList->DrawInstanced(m_nVertices, 1, 0, 0);
	}

	if (m_nInitVertices > 0)
	{
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dInitVertexBufferView);

		pd3dCommandList->DrawInstanced(m_nInitVertices, 1, 0, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSprite::CSprite(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nMaxX, UINT nMaxY, UINT nMax, float fDuration) : CEffect(pd3dDevice, pd3dCommandList, fDuration, sizeof(CSpriteVertex))
{
	m_xmf2SpriteSize = XMFLOAT2(1.0f / nMaxX, 1.0f / nMaxY);
	m_nMaxSpriteX = nMaxX;
	m_nMaxSpriteY = nMaxY;
	m_nMaxSprite = nMax;
	m_fDurationPerSprite = fDuration / (float)nMax;
}

CSprite::~CSprite()
{
	ReleaseShaderVariables();
}

void CSprite::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_SPRITE_INFO) + 255) & ~255);

	m_pd3dcbSprite = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes);

	m_pd3dcbSprite->Map(0, NULL, (void**)&m_pcbMappedSprite);

	CEffect::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CSprite::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pcbMappedSprite->m_xmf2SpriteSize = m_xmf2SpriteSize;
	m_pcbMappedSprite->m_nMaxSpriteX = m_nMaxSpriteX;
	m_pcbMappedSprite->m_nMaxSpriteY = m_nMaxSpriteY;
	m_pcbMappedSprite->m_nMaxSprite = m_nMaxSprite;
	m_pcbMappedSprite->m_fDurationPerSprite = m_fDurationPerSprite;

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_SPRITE, m_pd3dcbSprite->GetGPUVirtualAddress());

	CEffect::UpdateShaderVariables(pd3dCommandList);
}

void CSprite::ReleaseShaderVariables()
{
	if (m_pd3dcbSprite)
	{
		m_pd3dcbSprite->Unmap(0, NULL);
		m_pd3dcbSprite->Release();

		m_pd3dcbSprite = NULL;
	}

	CEffect::ReleaseShaderVariables();
}

void CSprite::AddVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, int nEffectAniType, int nAngle, XMFLOAT4 xmf4Color)
{
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf3Position = xmf3Position;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf2Size = xmf2Size;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmn2SpritePos = XMUINT2(0, 0);
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_fAge = 0.0f;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_nAngle = nAngle;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf4Color = xmf4Color;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices++].m_nType = nEffectAniType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CFollowSprite::CFollowSprite(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nMaxX, UINT nMaxY, UINT nMax, float fDuration) : CFollowEffect(pd3dDevice, pd3dCommandList, fDuration, sizeof(CSpriteVertex))
{
	m_xmf2SpriteSize = XMFLOAT2(1.0f / nMaxX, 1.0f / nMaxY);
	m_nMaxSpriteX = nMaxX;
	m_nMaxSpriteY = nMaxY;
	m_nMaxSprite = nMax;
	m_fDurationPerSprite = fDuration / (float)nMax;
}

CFollowSprite::~CFollowSprite()
{
	ReleaseShaderVariables();
}

void CFollowSprite::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_SPRITE_INFO) + 255) & ~255);

	m_pd3dcbSprite = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes);

	m_pd3dcbSprite->Map(0, NULL, (void**)&m_pcbMappedSprite);

	CFollowEffect::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CFollowSprite::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pcbMappedSprite->m_xmf2SpriteSize = m_xmf2SpriteSize;
	m_pcbMappedSprite->m_nMaxSpriteX = m_nMaxSpriteX;
	m_pcbMappedSprite->m_nMaxSpriteY = m_nMaxSpriteY;
	m_pcbMappedSprite->m_nMaxSprite = m_nMaxSprite;
	m_pcbMappedSprite->m_fDurationPerSprite = m_fDurationPerSprite;

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_SPRITE, m_pd3dcbSprite->GetGPUVirtualAddress());

	CFollowEffect::UpdateShaderVariables(pd3dCommandList);
}

void CFollowSprite::ReleaseShaderVariables()
{
	if (m_pd3dcbSprite)
	{
		m_pd3dcbSprite->Unmap(0, NULL);
		m_pd3dcbSprite->Release();

		m_pd3dcbSprite = NULL;
	}

	CFollowEffect::ReleaseShaderVariables();
}

void CFollowSprite::AddVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size, int nEffectAniType, int nAngle, XMFLOAT4 xmf4Color)
{
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf3Position = xmf3Position;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf2Size = xmf2Size;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmn2SpritePos = XMUINT2(0, 0);
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_fAge = 0.0f;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_nAngle = nAngle;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices].m_xmf4Color = xmf4Color;
	((CSpriteVertex*)m_pMappedInitVertices)[m_nInitVertices++].m_nType = nEffectAniType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CParticle::CParticle(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dMappedVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(CParticleVertex) * MAX_TEMP_PARTICLE_VERTEX_COUNT);

	m_d3MappedVertexBufferView.BufferLocation = m_pd3dMappedVertexBuffer->GetGPUVirtualAddress();
	m_d3MappedVertexBufferView.SizeInBytes = sizeof(CParticleVertex) * MAX_TEMP_PARTICLE_VERTEX_COUNT;
	m_d3MappedVertexBufferView.StrideInBytes = sizeof(CParticleVertex);

	m_pd3dMappedVertexBuffer->Map(0, NULL, (void**)&m_pMappedParticleVertices);

	m_pd3dBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);

	m_pd3dDummyBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_SOURCE, NULL);

	m_pd3dReadBackBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64),
		D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);

	for (int i = 0; i < 2; i++)
	{
		m_pd3dVertexBuffer[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(CParticleVertex) * MAX_PARTICLE_VERTEX_COUNT,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);

		m_d3dVertexBufferView[i].BufferLocation = m_pd3dVertexBuffer[i]->GetGPUVirtualAddress();
		m_d3dVertexBufferView[i].SizeInBytes = sizeof(CParticleVertex) * MAX_PARTICLE_VERTEX_COUNT;
		m_d3dVertexBufferView[i].StrideInBytes = sizeof(CParticleVertex);

		m_d3dSOBufferView[i].BufferFilledSizeLocation = m_pd3dBuffer->GetGPUVirtualAddress();
		m_d3dSOBufferView[i].BufferLocation = m_pd3dVertexBuffer[i]->GetGPUVirtualAddress();
		m_d3dSOBufferView[i].SizeInBytes = sizeof(CParticleVertex) * MAX_PARTICLE_VERTEX_COUNT;
	}
}

CParticle::~CParticle()
{
	if (m_pd3dBuffer)
	{
		m_pd3dBuffer->Release();
		m_pd3dBuffer = NULL;
	}
	if (m_pd3dDummyBuffer)
	{
		m_pd3dDummyBuffer->Release();
		m_pd3dDummyBuffer = NULL;
	}
	if (m_pd3dReadBackBuffer)
	{
		m_pd3dReadBackBuffer->Release();
		m_pd3dReadBackBuffer = NULL;
	}

	for (int i = 0; i < 2; i++)
	{
		if (m_pd3dVertexBuffer[i])
		{
			m_pd3dVertexBuffer[i]->Release();
			m_pd3dVertexBuffer[i] = NULL;
		}
	}

	if (m_pd3dMappedVertexBuffer)
	{
		m_pd3dMappedVertexBuffer->Unmap(0, NULL);
		m_pd3dMappedVertexBuffer->Release();

		m_pd3dMappedVertexBuffer = NULL;
	}

	ReleaseShaderVariables();
}

void CParticle::Initialize(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction, float fSpeed, float fDuration, bool bScaling, float fMass,
	XMFLOAT3 xmf3Right, XMFLOAT3 xmf3Up, XMFLOAT3 xmf3Look, XMFLOAT3 xmf3Angles)
{
	m_xmf3Position = xmf3Position;
	m_xmf3Direction = xmf3Direction;
	m_fSpeed = fSpeed;
	m_fDuration = fDuration;
	m_xmf3Angles = xmf3Angles;
	m_xmf3Right = xmf3Right;
	m_xmf3Up = xmf3Up;
	m_xmf3Look = xmf3Look;
	m_bScaling = bScaling;
	m_fMass = fMass;

	m_nDrawBufferIndex = 0;
	m_nSOBufferIndex = 1;
}

void CParticle::AddVertex(CParticleVertex *pParticleVertices, int nVertices)
{
	memcpy(&m_pMappedParticleVertices[m_nMappedParticleVertices], pParticleVertices, sizeof(CParticleVertex) * nVertices);

	m_nMappedParticleVertices += nVertices;

	delete[] pParticleVertices;
}

void CParticle::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_PARTICLE_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö

	m_pd3dcbParticle = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes);
	m_pd3dcbParticle->Map(0, NULL, (void**)&m_pcbMappedParticle);
}

void CParticle::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pcbMappedParticle->m_vPosition = m_xmf3Position;
	m_pcbMappedParticle->m_fSpeed = m_fSpeed;
	m_pcbMappedParticle->m_vDirection = m_xmf3Direction;
	m_pcbMappedParticle->m_fDuration = m_fDuration;
	m_pcbMappedParticle->m_fElapsedTime = m_fElapsedTime;
	m_pcbMappedParticle->m_bEmit = m_bEmit;
	m_pcbMappedParticle->m_vRight = m_xmf3Right;
	m_pcbMappedParticle->m_vUp = m_xmf3Up;
	m_pcbMappedParticle->m_vLook = m_xmf3Look;
	m_pcbMappedParticle->m_vAngles = m_xmf3Angles;
	m_pcbMappedParticle->m_bScaling = m_bScaling;
	m_pcbMappedParticle->m_fMass = m_fMass;

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_PARTICLE, m_pd3dcbParticle->GetGPUVirtualAddress());
}

void CParticle::ReleaseShaderVariables()
{
	if (m_pd3dcbParticle)
	{
		m_pd3dcbParticle->Unmap(0, NULL);
		m_pd3dcbParticle->Release();

		m_pd3dcbParticle = NULL;
	}
}

void CParticle::SetFollowObject(CGameObject *pObject, CModel *pModel)
{
	pObject->AddParticle(this);
	m_pFollowFrame = pModel;
}

void CParticle::SetToFollowFramePosition()
{
	if (m_pFollowFrame)
	{
		m_xmf3Position = m_pFollowFrame->GetPosition();
		m_xmf3Right = m_pFollowFrame->GetRight();
		m_xmf3Up = m_pFollowFrame->GetUp();
		m_xmf3Look = m_pFollowFrame->GetLook();
	}
}

void CParticle::Animate(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CParticle::ReadVertexCount(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nVertices > 0)
	{
		if (m_nDrawBufferIndex == 0)
		{
			m_nDrawBufferIndex = 1;
			m_nSOBufferIndex = 0;
		}
		else if (m_nDrawBufferIndex == 1)
		{
			m_nDrawBufferIndex = 0;
			m_nSOBufferIndex = 1;
		}

		D3D12_RANGE d3dRange = { 0, sizeof(UINT64) };
		UINT64 *nFilledSize = NULL;

		m_pd3dReadBackBuffer->Map(0, &d3dRange, (void**)&nFilledSize);

		m_nVertices = static_cast<int>((*nFilledSize) / sizeof(CParticleVertex));

		d3dRange = { 0, 0 };
		m_pd3dReadBackBuffer->Unmap(0, &d3dRange);
	}
}

void CParticle::SORender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	pd3dCommandList->SOSetTargets(0, 1, &m_d3dSOBufferView[m_nSOBufferIndex]);

	if (m_nMappedParticleVertices > 0)
	{
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3MappedVertexBufferView);
		pd3dCommandList->DrawInstanced(m_nMappedParticleVertices, 1, 0, 0);
	}

	if (m_nVertices > 0)
	{
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView[m_nDrawBufferIndex]);
		pd3dCommandList->DrawInstanced(m_nVertices, 1, 0, 0);
	}
}

void CParticle::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	if (m_nVertices > 0)
	{
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView[m_nDrawBufferIndex]);
		pd3dCommandList->DrawInstanced(m_nVertices, 1, 0, 0);
	}
}

void CParticle::AfterRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::TransitionResourceState(pd3dCommandList, m_pd3dBuffer, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
	pd3dCommandList->CopyResource(m_pd3dReadBackBuffer, m_pd3dBuffer);
	::TransitionResourceState(pd3dCommandList, m_pd3dBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	pd3dCommandList->CopyResource(m_pd3dBuffer, m_pd3dDummyBuffer);
	::TransitionResourceState(pd3dCommandList, m_pd3dBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);

	m_nVertices += m_nMappedParticleVertices;
	m_nMappedParticleVertices = 0;
}
