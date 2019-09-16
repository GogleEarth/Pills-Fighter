// pch.cpp: 미리 컴파일된 헤더에 해당하는 소스 파일입니다. 성공하려면 컴파일이 필요합니다.

#include "pch.h"



// 일반적으로 이 파일을 무시하지만 미리 컴파일된 헤더를 사용하는 경우 유지합니다.


std::string get_file_name(const std::string strFileName)
{
	std::string strPathSeparator("\\");
	std::string strSuffixSeparator(".");

	int nPathpos = 1 + (int)(strFileName.find_last_of(strPathSeparator));
	if (nPathpos == 0)
	{
		strPathSeparator = "/";
		nPathpos = 1 + (int)(strFileName.find_last_of(strPathSeparator));
	}

	int nSuffixpos = (int)(strFileName.find_last_of(strSuffixSeparator));

	return strFileName.substr(nPathpos, nSuffixpos - nPathpos);
}

std::string GetFilePath(const std::string strFileName)
{
	std::string strPathSeparator("\\");

	int nPathpos = 1 + (int)(strFileName.find_last_of(strPathSeparator));
	if (nPathpos == 0)
	{
		strPathSeparator = "/";
		nPathpos = 1 + (int)(strFileName.find_last_of(strPathSeparator));
	}

	return strFileName.substr(0, nPathpos);
}

XMFLOAT2 CalculateCenter(float left, float right, float top, float bottom, bool isCenter)
{
	XMFLOAT2 xmf2Result;

	if (isCenter)
		xmf2Result = XMFLOAT2((left + right) * 0.5f, (top + bottom) * 0.5f);
	else
		xmf2Result = XMFLOAT2((left + right) * 0.5f, top);

	return xmf2Result;
}

XMFLOAT2 CalculateSize(float left, float right, float top, float bottom, bool isCenter)
{
	XMFLOAT2 xmf2Result;

	if (isCenter)
		xmf2Result = XMFLOAT2((right - left) * 0.5f, (top - bottom) * 0.5f);
	else
		xmf2Result = XMFLOAT2((right - left) * 0.5f, (top - bottom));

	return xmf2Result;
}