#pragma once

#include "GameObject.h"
#include "Camera.h"

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#define SHOT_COOLTIME			0.05f

class CShader;

struct CB_PLAYER_INFO
{
	XMFLOAT4X4					m_xmf4x4World;
};

class CPlayer : public CGameObject
{
protected:
	LPVOID m_pPlayerUpdatedContext = NULL;

	LPVOID m_pCameraUpdatedContext = NULL;

	CCamera *m_pCamera = NULL;

	ID3D12Resource					*m_pd3dcbPlayer = NULL;
	CB_PLAYER_INFO					*m_pcbMappedPlayer = NULL;

public:
	unsigned int Client_id;
	
	CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL, int nMeshes = 1, int nMaterials = 1);
	virtual ~CPlayer();

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }
	void SetShader(CShader* pShader) { m_pShader = pShader; }

	void Move(ULONG nDirection, float fDistance);
	void Move(const XMFLOAT3& xmf3Shift);
	
	//�÷��̾��� ��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ��̴�. 
	void Update(float fTimeElapsed);

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�. 
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual CCamera *SetCamera(float fTimeElapsed);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

private:
	CShader *m_pShader = NULL;

	CObjectsShader *m_pBulletShader = NULL;
	bool m_Shotable = TRUE;
	float m_ShotTime;

public:
	void SetBullet(CObjectsShader* Bullet) { m_pBulletShader = Bullet; }
	void Shot(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void CheckElapsedTime(float ElapsedTime); // �ð��� ������ ���� ���Ǿ��� ������ üũ�ϴ� �Լ�
};