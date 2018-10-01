#pragma once

#include "Shader.h"

class CScene
{
public:
	CScene();
	~CScene();

	//씬에서 마우스와 키보드 메시지를 처리한다. 
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	//그래픽 루트 시그너쳐를 생성한다. 
	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	void SetGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList) { pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature); }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {};
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {};
	virtual void ReleaseShaderVariables() {};

	bool ProcessInput(UCHAR *pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	void ReleaseUploadBuffers();

protected:
	CPlayer						*m_pPlayer = NULL;

	CObjectsShader				**m_ppShaders = NULL;
	int							m_nShaders = 0;

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;

	int m_HitCount = 0;

public:
	// 충돌 체크를 검사한다.
	void CheckCollision();

	// 오브젝트가 삭제되어야하는지 검사한다.
	void CheckDeleteObject();

	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer;}
	CObjectsShader* GetShader(UINT index) { return m_ppShaders[index]; }

	void SetStateRenderWire(BOOL State);
};
