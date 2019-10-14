// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.

#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <mmsystem.h>

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <stdint.h>

#include <fstream>
#include <vector>
#include <random>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <queue>
#include <mutex>
#include <thread>

#include <assert.h>
#include <algorithm>
#include <memory.h>
#include <wrl.h>

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "winmm.lib")

#define FIXED_FRAME 1.0f / 60.0f
#define ITEM_HEALING_COOLTIME 60.f
#define ITEM_AMMO_COOLTIME 10.f
#define ITEM_ID 9
#define ITEM_AMMO1 10
#define ITEM_AMMO2 11

#define MAX_CLIENT 8
#define PLAYER_HP 100
#define MAX_SCORE 20

#define WITH_DATA_BASE

///////////////////////////////////////

#define EPSILON				1.0e-10f

extern std::string get_file_name(const std::string strFileName);
extern std::string GetFilePath(const std::string strFileName);
extern XMFLOAT2 CalculateCenter(float left, float right, float top, float bottom, bool isCenter = false);
extern XMFLOAT2 CalculateSize(float left, float right, float top, float bottom, bool isCenter = false);

inline bool IsZero(float fValue) { return((fabsf(fValue) < EPSILON)); }
inline bool IsEqual(float fA, float fB) { return(::IsZero(fA - fB)); }
inline float InverseSqrt(float fValue) { return 1.0f / sqrtf(fValue); }
inline void Swap(float *pfS, float *pfT) { float fTemp = *pfS; *pfS = *pfT; *pfT = fTemp; }

//3차원 벡터의 연산
namespace Vector3
{
	inline XMFLOAT3 XMVectorToFloat3(XMVECTOR& xmvVector)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, xmvVector);
		return(xmf3Result);
	}
	inline XMFLOAT3 ScalarProduct(XMFLOAT3& xmf3Vector, float fScalar, bool bNormalize =
		true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)) *
				fScalar);
		else
			XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector) * fScalar);
		return(xmf3Result);
	}
	inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) +
			XMLoadFloat3(&xmf3Vector2));
		return(xmf3Result);
	}
	inline XMFLOAT3 Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + (XMLoadFloat3(&xmf3Vector2)
			* fScalar));
		return(xmf3Result);
	}
	inline XMFLOAT3 Subtract(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) - XMLoadFloat3(&xmf3Vector2));
		return(xmf3Result);
	}
	inline float DotProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&xmf3Vector1),
			XMLoadFloat3(&xmf3Vector2)));
		return(xmf3Result.x);
	}
	inline XMFLOAT3 CrossProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, bool
		bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result,
				XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&xmf3Vector1),
					XMLoadFloat3(&xmf3Vector2))));
		else
			XMStoreFloat3(&xmf3Result, XMVector3Cross(XMLoadFloat3(&xmf3Vector1),
				XMLoadFloat3(&xmf3Vector2)));
		return(xmf3Result);
	}
	inline XMFLOAT3 Normalize(XMFLOAT3& xmf3Vector)
	{
		XMFLOAT3 m_xmf3Normal;
		XMStoreFloat3(&m_xmf3Normal, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)));
		return(m_xmf3Normal);
	}
	inline float Length(XMFLOAT3& xmf3Vector)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&xmf3Vector)));
		return(xmf3Result.x);
	}
	inline float Angle(XMVECTOR& xmvVector1, XMVECTOR& xmvVector2)
	{
		XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(xmvVector1, xmvVector2);
		return(XMConvertToDegrees(acosf(XMVectorGetX(xmvAngle))));
	}
	inline float Angle(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
	{
		return(Angle(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
	}
	inline XMFLOAT3 TransformNormal(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3TransformNormal(XMLoadFloat3(&xmf3Vector),
			xmmtxTransform));
		return(xmf3Result);
	}
	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3TransformCoord(XMLoadFloat3(&xmf3Vector),
			xmmtxTransform));
		return(xmf3Result);
	}
	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMFLOAT4X4& xmmtx4x4Matrix)
	{
		return(TransformCoord(xmf3Vector, XMLoadFloat4x4(&xmmtx4x4Matrix)));
	}
}
//4차원 벡터의 연산
namespace Vector4
{
	inline XMFLOAT4 Add(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) +
			XMLoadFloat4(&xmf4Vector2));
		return(xmf4Result);
	}
}
//행렬의 연산
namespace Matrix4x4
{
	inline XMFLOAT4X4 Identity()
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixIdentity());
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) *
			XMLoadFloat4x4(&xmmtx4x4Matrix2));
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMMATRIX& xmmtxMatrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * xmmtxMatrix2);
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 Multiply(XMMATRIX& xmmtxMatrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, xmmtxMatrix1 * XMLoadFloat4x4(&xmmtx4x4Matrix2));
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 Inverse(XMFLOAT4X4& xmmtx4x4Matrix)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixInverse(NULL,
			XMLoadFloat4x4(&xmmtx4x4Matrix)));
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 Transpose(XMFLOAT4X4& xmmtx4x4Matrix)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result,
			XMMatrixTranspose(XMLoadFloat4x4(&xmmtx4x4Matrix)));
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ,
		float FarZ)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio,
			NearZ, FarZ));
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 LookAtLH(XMFLOAT3& xmf3EyePosition, XMFLOAT3& xmf3LookAtPosition,
		XMFLOAT3& xmf3UpDirection)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookAtLH(XMLoadFloat3(&xmf3EyePosition),
			XMLoadFloat3(&xmf3LookAtPosition), XMLoadFloat3(&xmf3UpDirection)));
		return(xmmtx4x4Result);
	}
}
////////////////////////
