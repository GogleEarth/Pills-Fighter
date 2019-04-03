#include "stdafx.h"
#include "Font.h"

CFont::CFont()
{

}

CFont::~CFont()
{

}

void CFont::LoadDataFromFile(const char *pstrFileName)
{
	FILE *pFile;
	fopen_s(&pFile, pstrFileName, "r");

	char Temp[64];
	fscanf_s(pFile, "%s", Temp, sizeof(Temp)); // info
	fscanf_s(pFile, "%s", Temp, sizeof(Temp)); // face="Arial"
	fscanf_s(pFile, "%s", Temp, sizeof(Temp)); // face="Arial"


}