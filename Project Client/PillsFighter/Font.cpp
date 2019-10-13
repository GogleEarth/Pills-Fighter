#include "stdafx.h"
#include "Font.h"
#include "Model.h"
#include "Scene.h"
#include "Shader.h"

CTextObject::CTextObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_nCharacter = 0;
	m_pCharacters = NULL;
	
	m_bUse = false;
	ZeroMemory(m_pText, sizeof(m_pText));

	m_pd3dcbFont = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(CFontVertex) * MAX_TEXT_LENGTH);

	m_pd3dcbFont->Map(0, NULL, (void**)&m_pcbMappedFont);

	m_d3dFontView.BufferLocation = m_pd3dcbFont->GetGPUVirtualAddress();
	m_d3dFontView.SizeInBytes = sizeof(CFontVertex) * MAX_TEXT_LENGTH;
	m_d3dFontView.StrideInBytes = sizeof(CFontVertex);

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf4Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_fRightAlignX = 0.0f;
}

CTextObject::~CTextObject()
{
	if (m_pd3dcbFont) m_pd3dcbFont->Release();

	ReleaseShaderVariables();
}

void CTextObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_FONT_INFO) + 255) & ~255);

	m_pd3dcbFontInfo = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes);

	m_pd3dcbFontInfo->Map(0, NULL, (void**)&m_pcbMappedFontInfo);
}

void CTextObject::UpdateVertexBuffer(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (int i = 0; i < m_nCharacter; i++)
	{
		memcpy(&m_pcbMappedFont[i], &m_pCharacters[i], sizeof(CFontVertex));
	}
}

void CTextObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMFLOAT3 xmf3Position = m_xmf3Position;
	xmf3Position.x += m_fRightAlignX;

	m_pcbMappedFontInfo->m_xmf3Position = xmf3Position;
	m_pcbMappedFontInfo->m_xmf4Color = m_xmf4Color;

	D3D12_GPU_VIRTUAL_ADDRESS d3dGPUAddress = m_pd3dcbFontInfo->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_FONT_INFO, d3dGPUAddress);
}

void CTextObject::ReleaseShaderVariables()
{
	if (m_pd3dcbFontInfo)
	{
		m_pd3dcbFontInfo->Unmap(0, NULL);
		m_pd3dcbFontInfo->Release();
	}
}

void CTextObject::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	UpdateVertexBuffer(pd3dCommandList);
	UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dFontView);

	pd3dCommandList->DrawInstanced(m_nCharacter, 1, 0, 0);
}

void CTextObject::SetText(const wchar_t *pstrText, CFontVertex *pFontVertex, int nLength)
{
	lstrcpynW(m_pText, pstrText, MAX_TEXT_LENGTH);
	m_nCharacter = nLength;
	m_pCharacters = pFontVertex;
	m_bUse = true;
}

void CTextObject::FreeText()
{
	m_nCharacter = 0;

	if (m_pCharacters)
	{
		delete[] m_pCharacters;
		m_pCharacters = NULL;
	}
}

void CTextObject::Release()
{
	m_bUse = false;

	FreeText();
}

///////////////////////////////////////////////////////////////////////////////////

CFont::CFont()
{
	::ZeroMemory(m_pstrName, sizeof(m_pstrName));
	m_nSize = 0;
	m_fTopPadding = 0.0f;
	m_fBottomPadding = 0.0f;
	m_fLeftPadding = 0.0f;
	m_fRightPadding = 0.0f;

	m_fLineHeight = 0.0f;
	m_nTextureWidth = 0;
	m_nTextureHeight = 0;

	m_nCharacters = 0;
	m_pCharacters = NULL;

	m_nKernings = 0;
	m_pKernings = NULL;

	m_pFontTexture = NULL;
}

CFont::~CFont()
{
}

void CFont::Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, const char *pstrFile)
{
	LoadDataFromFile(pstrFile);

	for (int i = 0; i < MAX_TEXT_SIZE; i++)
	{
		CTextObject *pTextObject = new CTextObject(pd3dDevice, pd3dCommandList);
		pTextObject->CreateShaderVariables(pd3dDevice, pd3dCommandList);

		m_qpTempTextObjects.push(pTextObject);
	}

	int nType;
	if (m_vTextureInfo.size() > 1) nType = RESOURCE_TEXTURE2D_ARRAY;
	else nType = RESOURCE_TEXTURE2D;

	m_pFontTexture = new CTexture((int)m_vTextureInfo.size(), nType, 0);

	for (int i = 0; i < m_vTextureInfo.size(); i++)
	{
		char pstrFileWithPath[64] = "./Resource/Font/";
		strcat_s(pstrFileWithPath, m_vTextureInfo[i].c_str());

		INT nLen = (int)(strlen(pstrFileWithPath)) + 1;
		WCHAR* pwstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
		MultiByteToWideChar(949, 0, pstrFileWithPath, -1, pwstrFileName, nLen);

		m_pFontTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrFileName, i);

		delete pwstrFileName;
	}
}

void CFont::SetSrv(ID3D12Device *pd3dDevice)
{
	CScene::CreateShaderResourceViews(pd3dDevice, m_pFontTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, false);
}

void CFont::ReleaseUploadBuffers()
{
	if (m_pFontTexture) m_pFontTexture->ReleaseUploadBuffers();
}

void CFont::ClearTexts()
{
	for (const auto& pTextObject : m_vpTextObjects)
	{
		pTextObject->Release();
	}
	for (const auto& pTextObject : m_vp3DTextObjects)
	{
		pTextObject->Release();
	}
}

void CFont::Destroy()
{
	while (!m_qpTempTextObjects.empty())
	{
		CTextObject* pTextObject = m_qpTempTextObjects.front();
		pTextObject->Release();

		delete pTextObject;

		m_qpTempTextObjects.pop();
	}

	for (const auto& pTextObject : m_vpTextObjects)
	{
		pTextObject->Release();

		delete pTextObject;
	}

	for (const auto& pTextObject : m_vp3DTextObjects)
	{
		pTextObject->Release();

		delete pTextObject;
	}

	if (m_pCharacters) delete[] m_pCharacters;
	if (m_pKernings) delete[] m_pKernings;
	if (m_pFontTexture) delete m_pFontTexture;
}

void CFont::LoadDataFromFile(const char *pstrFileName)
{
	FILE *pFile;
	fopen_s(&pFile, pstrFileName, "r");

	char pstrToken[64];

	while (true)
	{
		fscanf_s(pFile, "%s", pstrToken, (UINT)sizeof(pstrToken));

		if (!strncmp(pstrToken, "info", 4)) // info
		{
			while (true)
			{
				fscanf_s(pFile, "%s", pstrToken, (UINT)sizeof(pstrToken));

				if (!strncmp(pstrToken, "fac", 3)) // face=
				{
					char *pstr = &pstrToken[6];

					memcpy(m_pstrName, pstr, strlen(pstr) - 1);
				}
				else if (!strncmp(pstrToken, "siz", 3)) // size=
				{
					char *pstr = &pstrToken[5];

					m_nSize = atoi(pstr);
				}
				else if (!strncmp(pstrToken, "bol", 3)) // bold=
				{
					// do nothing
				}
				else if (!strncmp(pstrToken, "ita", 3)) // italic=
				{
					// do nothing
				}
				else if (!strncmp(pstrToken, "cha", 3)) // charset=
				{
					// do nothing
				}
				else if (!strncmp(pstrToken, "uni", 3)) // unicode=
				{
					// do nothing
				}
				else if (!strncmp(pstrToken, "str", 3)) // stretchH=
				{
					// do nothing
				}
				else if (!strncmp(pstrToken, "smo", 3)) // smooth=
				{
					// do nothing
				}
				else if (!strncmp(pstrToken, "aa=", 3)) // aa=
				{
					// do nothing
				}
				else if (!strncmp(pstrToken, "pad", 3)) // padding=
				{
					char pstrValue = pstrToken[8];
					m_fTopPadding = (float)(atof(&pstrValue));

					pstrValue = pstrToken[10];
					m_fRightPadding = (float)(atof(&pstrValue));

					pstrValue = pstrToken[12];
					m_fBottomPadding = (float)(atof(&pstrValue));

					pstrValue = pstrToken[14];
					m_fLeftPadding = (float)(atof(&pstrValue));
				}
				else if (!strncmp(pstrToken, "spa", 3)) // spacing=
				{
					// do nothing
					break;
				}
			}
		}
		else if (!strncmp(pstrToken, "comm", 4)) // common
		{
			while (true)
			{
				fscanf_s(pFile, "%s", pstrToken, (UINT)sizeof(pstrToken));

				if (!strncmp(pstrToken, "lineH", 5)) // lineHeight=
				{
					char *pstr = &pstrToken[11];

					m_fLineHeight = (float)(atof(pstr));
				}
				else if (!strncmp(pstrToken, "base=", 5)) // base=
				{
				}
				else if (!strncmp(pstrToken, "scaleW", 6)) // scaleW=
				{
					char *pstr = &pstrToken[7];

					m_nTextureWidth = atoi(pstr);	
				}
				else if (!strncmp(pstrToken, "scaleH", 6)) // scaleH=
				{
					char *pstr = &pstrToken[7];

					m_nTextureHeight = atoi(pstr);
					
				}
				else if (!strncmp(pstrToken, "pages", 5)) // pages=
				{
					// do nothing
				}
				else if (!strncmp(pstrToken, "packe", 5)) // packed=
				{
					// do nothing
					break;
				}
			}
		}
		else if (!strncmp(pstrToken, "page", 4)) // page
		{
			while (true)
			{
				fscanf_s(pFile, "%s", pstrToken, (UINT)sizeof(pstrToken));

				if (!strncmp(pstrToken, "id=", 3)) // id=
				{
					// DO NOT ANYTHING
				}
				else if (!strncmp(pstrToken, "fil", 3)) // file="
				{
					char *pstr = &pstrToken[6];

					char pstrFile[10] = "";
					memcpy(pstrFile, pstr, strlen(pstr) - 1);

					m_vTextureInfo.emplace_back(pstrFile);
					break;
				}
			}
		}
		else if (!strncmp(pstrToken, "char", 4)) // chars
		{
			fscanf_s(pFile, "%s", pstrToken, (UINT)sizeof(pstrToken)); // count=

			char *pstrCount = &pstrToken[6];
			m_nCharacters = atoi(pstrCount);
			m_pCharacters = new CFontCharacter[m_nCharacters];
			::ZeroMemory(m_pCharacters, sizeof(CFontCharacter) * m_nCharacters);

			for (int i = 0; i < m_nCharacters; i++)
			{
				fscanf_s(pFile, "%s", pstrToken, (UINT)sizeof(pstrToken)); // char

				while (true)
				{
					fscanf_s(pFile, "%s", pstrToken, (UINT)sizeof(pstrToken));

					if (!strncmp(pstrToken, "id", 2)) // id=
					{
						char *pstr = &pstrToken[3];

						m_pCharacters[i].nId = atoi(pstr);
					}
					else if (!strncmp(pstrToken, "x=", 2)) // x=
					{
						char *pstr = &pstrToken[2];

						m_pCharacters[i].u = float(atof(pstr)) / float(m_nTextureWidth);
					}
					else if (!strncmp(pstrToken, "y=", 2)) // y=
					{
						char *pstr = &pstrToken[2];

						m_pCharacters[i].v = float(atof(pstr)) / float(m_nTextureHeight);
					}
					else if (!strncmp(pstrToken, "wi", 2)) // width=
					{
						char *pstr = &pstrToken[6];

						m_pCharacters[i].w = (float)(atof(pstr));
						m_pCharacters[i].tw = float(atof(pstr)) / float(m_nTextureWidth);
					}
					else if (!strncmp(pstrToken, "he", 2)) // height=
					{
						char *pstr = &pstrToken[7]; 

						m_pCharacters[i].h = (float)(atof(pstr));
						m_pCharacters[i].th = float(atof(pstr)) / float(m_nTextureHeight);
					}
					else if (!strncmp(pstrToken, "xo", 2)) // xoffset=
					{
						char *pstr = &pstrToken[8];

						m_pCharacters[i].xOffset = (float)(atof(pstr));
					}
					else if (!strncmp(pstrToken, "yo", 2)) // yoffset=
					{
						char *pstr = &pstrToken[8];

						m_pCharacters[i].yOffset = (float)(atof(pstr));
					}
					else if (!strncmp(pstrToken, "xa", 2)) // xadvance=
					{
						char *pstr = &pstrToken[9];

						m_pCharacters[i].xAdvance = (float)(atof(pstr));
					}
					else if (!strncmp(pstrToken, "pa", 2)) // page=
					{
						char *pstr = &pstrToken[5];

						m_pCharacters[i].nTexIndex = atoi(pstr);
					}
					else if (!strncmp(pstrToken, "ch", 2)) // chnl=
					{
						// do nothing
						break;
					}
				}
			}
		}
		else if (!strncmp(pstrToken, "kern", 4)) // kernings
		{
			fscanf_s(pFile, "%s", pstrToken, (UINT)sizeof(pstrToken)); // count=

			char *pstrCount = &pstrToken[6];
			m_nKernings = atoi(pstrCount);
			m_pKernings = new CFontKerning[m_nKernings];
			::ZeroMemory(m_pKernings, sizeof(CFontKerning) * m_nKernings);

			for (int i = 0; i < m_nKernings; i++)
			{
				while (true)
				{
					fscanf_s(pFile, "%s", pstrToken, (UINT)sizeof(pstrToken)); // kerning

					if (!strncmp(pstrToken, "fi", 2)) // first=
					{
						char *pstr = &pstrToken[6];

						m_pKernings[i].nFirstId = atoi(pstrCount);
					}
					else if (!strncmp(pstrToken, "se", 2)) // second=
					{
						char *pstr = &pstrToken[7];

						m_pKernings[i].nSecondId = atoi(pstrCount);
					}
					else if (!strncmp(pstrToken, "am", 2)) // amount=
					{
						char *pstr = &pstrToken[7];

						m_pKernings[i].fAmount = (float)(atoi(pstrCount));
						break;
					}
				}
			}
			break;
		}
	}
}

CFontCharacter* CFont::GetChar(wchar_t c)
{
	for (int i = 0; i < m_nCharacters; i++)
	{
		if (c == m_pCharacters[i].nId)
			return &m_pCharacters[i];
	}
	
	return &m_pCharacters[0];
}

float CFont::GetKerning(wchar_t cFirst, wchar_t cSecond)
{
	for (int i = 0; i < m_nKernings; i++)
	{
		if ((cFirst == m_pKernings[i].nFirstId) && (cSecond == m_pKernings[i].nSecondId))
			return m_pKernings[i].fAmount;
	}

	return 0;
}

void CFont::CreateText(int nLength, CFontVertex* pFontVertices, const wchar_t *pstrText, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, int nType)
{
	float fPaddingW = (m_fLeftPadding + m_fRightPadding) * xmf2Padding.x;
	float fPaddingH = (m_fTopPadding + m_fBottomPadding) * xmf2Padding.y;

	wchar_t chPrev;
	
	XMFLOAT2 xmf2ChPosition = XMFLOAT2(0.0f, 0.0f);

	for (int i = 0; i < nLength; i++)
	{
		wchar_t ch;

		ch = pstrText[i];

		if (ch == '\0') break;
		else if (ch == '\n')
		{
			xmf2ChPosition.x = 0.0f;
			xmf2ChPosition.y += (m_fLineHeight / float(FRAME_BUFFER_WIDTH) + fPaddingH / float(FRAME_BUFFER_HEIGHT) ) * xmf2Scale.y;

			continue;
		}

		CFontCharacter *pFontchar = GetChar(ch);

		if (!pFontchar) continue;

		float fKerning = 0.0f;

		if (i != 0) fKerning = GetKerning(chPrev, ch);

		XMFLOAT2 xmf2Size;
		xmf2Size.x = pFontchar->w * xmf2Scale.x / float(FRAME_BUFFER_WIDTH);
		xmf2Size.y = pFontchar->h * xmf2Scale.y / float(FRAME_BUFFER_HEIGHT);

		XMFLOAT2 xmf2Pos;
		xmf2Pos.x = xmf2ChPosition.x + ((pFontchar->xOffset + fKerning) * xmf2Scale.x / float(FRAME_BUFFER_WIDTH));
		xmf2Pos.y = xmf2ChPosition.y - (pFontchar->yOffset * xmf2Scale.y / float(FRAME_BUFFER_HEIGHT));

		XMFLOAT2 xmf2UVSize;
		xmf2UVSize.x = pFontchar->tw;
		xmf2UVSize.y = pFontchar->th;

		XMFLOAT2 xmf2UVPos;
		xmf2UVPos.x = pFontchar->u;
		xmf2UVPos.y = pFontchar->v;

		pFontVertices[i].xmf2Pos = xmf2Pos;
		pFontVertices[i].xmf2Size = xmf2Size;
		pFontVertices[i].xmf2UVPos = xmf2UVPos;
		pFontVertices[i].xmf2UVSize = xmf2UVSize;
		pFontVertices[i].nTexIndex = pFontchar->nTexIndex;

		xmf2ChPosition.x += (pFontchar->xAdvance - fPaddingW) * xmf2Scale.x / float(FRAME_BUFFER_WIDTH);

		chPrev = ch;
	}
}

CTextObject* CFont::SetText(const wchar_t *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType)
{
	int nLength = (int)lstrlenW(pstrText);
	CFontVertex *pFontVertices = new CFontVertex[nLength];

	CreateText(nLength, pFontVertices, pstrText, xmf2Scale, xmf2Padding, nType);

	CTextObject* pTextObject = m_qpTempTextObjects.front();
	m_qpTempTextObjects.pop();

	pTextObject->SetText(pstrText, pFontVertices, nLength);
	pTextObject->SetColor(xmf4Color);
	pTextObject->SetPosition(xmf2Position);

	float x = 0.0f;
	if (nType == RIGHT_ALIGN)
	{
		x = -(pFontVertices[nLength - 1].xmf2Pos.x + pFontVertices[nLength - 1].xmf2Size.x);
	}
	pTextObject->SetRightAlign(x);


	m_vpTextObjects.emplace_back(pTextObject);

	return pTextObject;
}

XMINT2 CFont::Create3DText(int nLength, CFontVertex* pFontVertices, const wchar_t *pstrText, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding)
{
	float fPaddingW = (m_fLeftPadding + m_fRightPadding) * xmf2Padding.x;
	float fPaddingH = (m_fTopPadding + m_fBottomPadding) * xmf2Padding.y;

	wchar_t chPrev;

	std::wstring strReverse;

	XMFLOAT2 xmf2ChPosition = XMFLOAT2(0.0f, 0.0f);
	float fNextLineH = -(m_fLineHeight + fPaddingH) * xmf2Scale.y;

	int nLine = 1;

	XMFLOAT2 xmf2TotalSize = XMFLOAT2(0.0f, 0.0f);
	float y = 0.0f;

	for (int i = 0; i < nLength; i++)
	{
		wchar_t ch;

		ch = pstrText[i];

		if (ch == '\0') break;
		else if (ch == '\n')
		{
			xmf2ChPosition.x = 0.0f;
			xmf2ChPosition.y -= fNextLineH;

			nLine++;

			continue;
		}

		CFontCharacter *pFontchar = GetChar(ch);

		if (!pFontchar) continue;

		float fKerning = 0.0f;

		if (i != 0) fKerning = GetKerning(chPrev, ch);
		
		XMFLOAT2 xmf2Pos;
		xmf2Pos.x = xmf2ChPosition.x + ((pFontchar->xOffset + fKerning) * xmf2Scale.x);
		xmf2Pos.y = xmf2ChPosition.y - (pFontchar->yOffset * xmf2Scale.y);
		
		XMFLOAT2 xmf2Size;
		xmf2Size.x = pFontchar->w * xmf2Scale.x;
		xmf2TotalSize.x += xmf2Size.x;

		xmf2Size.y = pFontchar->h * xmf2Scale.y;
		if (xmf2TotalSize.y < xmf2Size.y) xmf2TotalSize.y = xmf2Size.y;

		float temp = xmf2Size.y + fabs(xmf2Pos.y);
		if (y < temp) y = temp;

		XMFLOAT2 xmf2UVSize;
		xmf2UVSize.x = pFontchar->tw;
		xmf2UVSize.y = pFontchar->th;

		XMFLOAT2 xmf2UVPos;
		xmf2UVPos.x = pFontchar->u;
		xmf2UVPos.y = pFontchar->v;

		pFontVertices[i].xmf2Pos = xmf2Pos;
		pFontVertices[i].xmf2Size = xmf2Size;
		pFontVertices[i].xmf2UVPos = xmf2UVPos;
		pFontVertices[i].xmf2UVSize = xmf2UVSize;
		pFontVertices[i].nTexIndex = pFontchar->nTexIndex;

		xmf2ChPosition.x += (pFontchar->xAdvance - fPaddingW) * xmf2Scale.x;

		chPrev = ch;
	}

	XMINT2 xmi2BufferSize;
	xmi2BufferSize.x = int(xmf2TotalSize.x);
	xmi2BufferSize.y = int(xmf2TotalSize.y);

	xmf2ChPosition.x *= 0.5f;

	for (int i = 0; i < nLength; i++)
	{
		pFontVertices[i].xmf2Pos.x /= xmf2ChPosition.x;
		pFontVertices[i].xmf2Pos.x -= 1.0f;
	}

	for (int i = 0; i < nLength; i++)
	{
		if (i == nLength - 1)
			pFontVertices[i].xmf2Size.x = fabs(xmf2ChPosition.x / xmf2ChPosition.x - pFontVertices[i].xmf2Pos.x);
		else
			pFontVertices[i].xmf2Size.x = fabs(pFontVertices[i + 1].xmf2Pos.x - pFontVertices[i].xmf2Pos.x);

		float t = pFontVertices[i].xmf2Size.y + fabs(pFontVertices[i].xmf2Pos.y);
		float r = (t / y) / t * 2.0f;
		pFontVertices[i].xmf2Size.y *= r;
		pFontVertices[i].xmf2Pos.y *= r ;

		pFontVertices[i].xmf2Pos.y += 1.0f;
	}

	return xmi2BufferSize;
}

CTextObject* CFont::Set3DText(int& nWidth, int& nHeight, const wchar_t *pstrText, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color)
{
	int nLength = (int)lstrlenW(pstrText);
	CFontVertex *pFontVertices = new CFontVertex[nLength];

	XMINT2 xmi2Size = Create3DText(nLength, pFontVertices, pstrText, xmf2Scale, xmf2Padding);
	nWidth = xmi2Size.x;
	nHeight = xmi2Size.y;

	CTextObject* pTextObject = m_qpTempTextObjects.front();
	m_qpTempTextObjects.pop();

	pTextObject->SetText(pstrText, pFontVertices, nLength);
	pTextObject->SetColor(xmf4Color);

	m_vp3DTextObjects.emplace_back(pTextObject);

	return pTextObject;
}

void CFont::ChangeText(CTextObject *pTextObject, const wchar_t *pstrText, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, int nType)
{
	pTextObject->FreeText();

	int nLength = (int)lstrlenW(pstrText);
	CFontVertex *pFontVertices = new CFontVertex[nLength];
	
	CreateText(nLength, pFontVertices, pstrText, xmf2Scale, xmf2Padding, nType);
	pTextObject->SetText(pstrText, pFontVertices, nLength);

	float x = 0.0f;
	if (nType == RIGHT_ALIGN)
	{
		x = -(pFontVertices[nLength - 1].xmf2Pos.x + pFontVertices[nLength - 1].xmf2Size.x);
	}
	pTextObject->SetRightAlign(x);
}

void CFont::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pFontTexture->UpdateShaderVariables(pd3dCommandList);

	for (const auto& pTextObject : m_vpTextObjects)
	{
		if (pTextObject->IsHide()) continue;

		pTextObject->Render(pd3dCommandList);
	}
}

void CFont::Render3DFont(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pFontTexture->UpdateShaderVariables(pd3dCommandList);

	for (const auto& pTextObject : m_vp3DTextObjects)
	{
		if (pTextObject->IsHide()) continue;

		pTextObject->Render(pd3dCommandList);
	}
}

void CFont::CheckUsingTexts()
{
	for (auto& pText = m_vpTextObjects.begin(); pText != m_vpTextObjects.end();)
	{
		if (!(*pText)->IsUsed())
		{
			m_qpTempTextObjects.push(*pText);
			pText = m_vpTextObjects.erase(pText);
		}
		else
			pText++;
	}

	for (auto& pText = m_vp3DTextObjects.begin(); pText != m_vp3DTextObjects.end();)
	{
		if (!(*pText)->IsUsed())
		{
			m_qpTempTextObjects.push(*pText);
			pText = m_vp3DTextObjects.erase(pText);
		}
		else
			pText++;
	}
}