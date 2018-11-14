// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
#pragma once

#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <iostream>
#include <stdlib.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>

#define SERVERPORT	9000
#define FLAG		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM
#define LANG		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
#define WS22		MAKEWORD(2,2)
#define MAX_CLIENT  2
#define BUFSIZE		76

enum PKT_ID
{
	PKT_ID_PLAYER_INFO = 1, // 플레이어 정보
	PKT_ID_PLAYER_LIFE = 2, // 플레이어 hp
	PKT_ID_CREATE_OBJECT = 3, // 오브젝트 생성
	PKT_ID_DELETE_OBJECT = 4 // 오브젝트 삭제
};

enum OBJECT_TYPE
{
	OBJECT_TYPE_BULLET, // 총알
	OBJECT_TYPE_OBSTACLE // 장애물
};

// 이하 패킷 구조체
struct PKT_PLAYER_INFO
{
	int Id;
	DirectX::XMFLOAT4X4 worldmatrix;
	int shooting;
};

struct PKT_PLAYER_LIFE
{
	int id;
	DWORD player_hp;
};

struct PKT_CREATE_OBJECT
{
	OBJECT_TYPE Object_Type;
	DirectX::XMFLOAT4X4 worldmatrix;
	BYTE Object_Index;
};

struct PKT_DELETE_OBJECT
{
	BYTE Object_Index;
};

////////////////////////
