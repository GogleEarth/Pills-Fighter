#pragma once

#define MAX_CHAT_TEXT 50

class CTextSystem
{
public:
	CTextSystem();
	~CTextSystem();

	void Initialize(HWND hWnd);
	void Destroy(HWND hWnd);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	wchar_t* GetText() { return m_pwstrText; }
	void ResetText() { ::ZeroMemory(m_pwstrText, sizeof(m_pwstrText)); }
	void SetText(wchar_t *pwstr);

protected:
	HIMC		m_hIMC;
	wchar_t		m_pwstrText[MAX_CHAT_TEXT];
	int			m_nPos = 0;

	bool		m_bIme = false;

	std::vector<wchar_t*> m_vpwstrList;
};

