#pragma once

#include "GameObject.h"
#include "Camera.h"

#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20

#define SHOT_COOLTIME 0.05f

class CObjectsShader;

class CPlayer : public CGameObject
{
protected:

	//플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdateCallback() 함수에서 사용하는 데이터이다. 
	LPVOID m_pPlayerUpdatedContext;

	//카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdateCallback() 함수에서 사용하는 데이터이다. 
	LPVOID m_pCameraUpdatedContext;

	//플레이어에 현재 설정된 카메라이다. 
	CCamera *m_pCamera = NULL;

public:
	CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CPlayer();

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	//플레이어를 이동하는 함수이다. 
	void Move(ULONG nDirection, float fDistance);
	void Move(const XMFLOAT3& xmf3Shift);

	//플레이어를 회전하는 함수이다. 
	void Rotate(float x, float y, float z);

	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다. 
	void Update(float fTimeElapsed);

	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	//카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다. 
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	//카메라를 변경하기 위하여 호출하는 함수이다. 
	virtual CCamera *SetCamera(float fTimeElapsed);

	//플레이어의 카메라가 3인칭 카메라일 때 플레이어(메쉬)를 렌더링한다. 
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

private:
	CObjectsShader *m_pBulletShader = NULL;
	bool m_Shotable = TRUE;
	float m_ShotTime;

public:
	void SetBullet(CObjectsShader* Bullet) { m_pBulletShader = Bullet; }
	void Shot();
	void CheckElapsedTime(float ElapsedTime); // 시간이 지남에 따라 사용되야할 변수를 체크하는 함수

};