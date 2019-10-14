#pragma once

#pragma comment(lib, "ws2_32")
#include <iostream>
#include <WinSock2.h>
#include <sqlext.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

#define UNICODE


void show_error(SQLHANDLE, SQLSMALLINT, RETCODE);