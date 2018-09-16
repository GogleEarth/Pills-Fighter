#pragma once

#include "Shader.h"

class CScene
{
public:
	CScene();
	~CScene();

	//������ ���콺�� Ű���� �޽����� ó���Ѵ�. 
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	//�׷��� ��Ʈ �ñ׳��ĸ� �����Ѵ�. 
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
	// �浹 üũ�� �˻��Ѵ�.
	void CheckCollision();

	// ������Ʈ�� �����Ǿ���ϴ��� �˻��Ѵ�.
	void CheckDeleteObject();

	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer;}
	CObjectsShader* GetShader(UINT index) { return m_ppShaders[index]; }
};
