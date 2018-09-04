#pragma once

#include "Timer.h"
#include "Shader.h"
#include "Camera.h"
#include "GameObject.h"

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
	bool ProcessInput(UCHAR *pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	void ReleaseUploadBuffers();
	CObjectsShader* GetShader(UINT Index) { return &m_pShaders[Index]; }

	//그래픽 루트 시그너쳐를 생성한다. 
	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature();

	void CheckCollision();

protected:
	//씬은 게임 객체들의 집합이다. 게임 객체는 셰이더를 포함한다.
	ID3D12RootSignature *m_pd3dGraphicsRootSignature = NULL;
	CCamera *pCamera;

protected:
	//배치(Batch) 처리를 하기 위하여 씬을 셰이더들의 리스트로 표현한다. 
	CObjectsShader *m_pShaders = NULL;
	int m_nShaders = 0;
};
