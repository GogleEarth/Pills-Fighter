#pragma once

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
	void SetTextLength(int nLength) { m_nTextLength = nLength; }

protected:
	HIMC		m_hIMC;
	wchar_t		m_pwstrText[256];
	int			m_nPos = 0;
	int			m_nTextLength = 256;

	bool		m_bIme = false;

	std::vector<wchar_t*> m_vpwstrList;
};

