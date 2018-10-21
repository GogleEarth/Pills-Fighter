#pragma once

#include "Shader.h"
#include "Player.h"

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
	void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	void ReleaseUploadBuffers();

protected:
	CPlayer						*m_pPlayer = NULL;
<<<<<<< HEAD

	CShader						**m_ppShaders = NULL;
=======
	CPlayer						*m_pAnotherPlayer = NULL;
	CObjectsShader				**m_ppShaders = NULL;
>>>>>>> 697f565584b9a50f9fd662e5b71704e2eed30c0f
	int							m_nShaders = 0;

	CRectTerrain				*m_pTerrain = NULL;

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;

public:
	// �浹 üũ�� �˻��Ѵ�.
	void CheckCollision();

	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer;}
<<<<<<< HEAD
	CShader* GetBulletShader(UINT index) { return m_ppShaders[index]; }
=======
	void SetAnotherPlayer(CPlayer* pPlayer) { m_pAnotherPlayer = pPlayer; }
	CObjectsShader* GetShader(UINT index) { return m_ppShaders[index]; }
>>>>>>> 697f565584b9a50f9fd662e5b71704e2eed30c0f
};
