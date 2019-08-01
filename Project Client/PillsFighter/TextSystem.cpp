#include "stdafx.h"
#include "TextSystem.h"


CTextSystem::CTextSystem()
{

}

CTextSystem::~CTextSystem()
{
}

void CTextSystem::Initialize(HWND hWnd)
{
	m_hIMC = ImmCreateContext();
	ImmAssociateContext(hWnd, m_hIMC);
	ResetText();
}

void CTextSystem::Destroy(HWND hWnd)
{
	ImmReleaseContext(hWnd, m_hIMC);
}

void CTextSystem::SetText(wchar_t *pwstr)
{ 
	::ZeroMemory(m_pwstrText, MAX_CHAT_TEXT); 
	lstrcpynW(m_pwstrText, pwstr, MAX_CHAT_TEXT);
	m_nPos = lstrlenW(pwstr); 
}

void CTextSystem::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_IME_STARTCOMPOSITION:
		m_pwstrText[m_nPos] = '\0';
		m_bIme = true;
		break;
	case WM_IME_ENDCOMPOSITION:
		m_pwstrText[m_nPos] = '\0';
		m_bIme = false;
		break;
	case WM_IME_COMPOSITION:
		if (MAX_CHAT_TEXT - 1 < m_nPos)
			break;

		if(lParam & GCS_RESULTSTR)
		{
			int len = ImmGetCompositionString(m_hIMC, GCS_RESULTSTR, NULL, 0);
			ImmGetCompositionString(m_hIMC, GCS_RESULTSTR, &m_pwstrText[m_nPos++], len);
		}

		if (lParam &GCS_COMPSTR)
		{
			int len = ImmGetCompositionString(m_hIMC, GCS_COMPSTR, NULL, 0);
			ImmGetCompositionString(m_hIMC, GCS_COMPSTR, &m_pwstrText[m_nPos], len);
		}

		break;
	case WM_CHAR:
		switch (wParam)
		{
		case VK_BACK:
			if (m_bIme) break;
			if (m_nPos == 0) break;

			m_pwstrText[--m_nPos] = '\0';
			break;
		default:
		{
			if (MAX_CHAT_TEXT - 1 < m_nPos)
				break;
			
			m_pwstrText[m_nPos++] = wParam;
			break;
		}
		}
		break;
	}
}