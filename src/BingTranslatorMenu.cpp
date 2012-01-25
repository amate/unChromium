// BingTranslatorMenu.cpp

#include "stdafx.h"
#include "BingTranslatorMenu.h"
#include <WinInet.h>
#include "MainFrame.h"
#include <boost/thread.hpp>
#include "MtlMisc.h"
#include "IniFile.h"

LPCTSTR appID = _T("6CB08565F99A903FB046716AA865A256A122E24C");


typedef CWinTraits<WS_THICKFRAME | WS_SYSMENU | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW | WS_EX_TOPMOST/*WS_EX_CLIENTEDGE*/> CDicWindowTraits;

///////////////////////////////////////////////////
// CDictionaryWindow

class CDictionaryWindow : public CWindowImpl<CDictionaryWindow, CWindow, CDicWindowTraits>
{
public:
	CDictionaryWindow()
	{
		CIniFileI	pr(g_szIniFileName, _T("DictionaryWindow"));
		m_Size.cx	= (int)pr.GetValue(_T("Width"), 250);
		m_Size.cy	= (int)pr.GetValue(_T("Height"), 270);
	}

	void	SetWord(const CString& strWord) 
	{
		if (m_strWord != strWord) {
			m_strWord = strWord;
			m_strTranslated.Empty();

			auto funcDictionary = [this] () {
				HINTERNET hInternet = NULL;
				HINTERNET hFile		= NULL;
				try {
					hInternet = ::InternetOpen(_T("Mozilla/5.0"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
					if (hInternet == NULL) 
						throw _T("InternetOpen�Ɏ��s");

					CString strUrl;
					strUrl.Format(_T("http://www.microsofttranslator.com/dictionary.ashx?from=en&to=ja&text=%s"), m_strWord);
					hFile = ::InternetOpenUrl(hInternet, strUrl, NULL, 0, 0, 0);
					if (hFile == NULL)
						throw _T("InternetOpenUrl�Ɏ��s");

					std::string strTempBuff;
					while (1) {
						enum { BuffSize = 512 };
						char Buff[BuffSize + 1] = "\0";
						DWORD ReadSize = 0;
						if (::InternetReadFile(hFile, (LPVOID)Buff, BuffSize, &ReadSize) && ReadSize == 0)
							break;

						Buff[ReadSize] = '\0';
						strTempBuff += Buff;
					}
					CString strBody = Misc::utf8_to_CString(strTempBuff.c_str());
					int nBegin = strBody.Find(_T('"'));
					int nEnd   = strBody.ReverseFind(_T('"'));
					if (nBegin == -1 || nEnd == -1 || nEnd == nBegin + 1)
						throw _T("�Y���Ȃ�");

					strBody = strBody.Mid(nBegin + 1, nEnd - nBegin - 1);
					CString strTranslated = Misc::urlstr_decodeJpn(strBody, 3);
					strTranslated.Replace(_T("<br />"), _T("\r\n"));
					strTranslated.Replace(_T("<hr />"), _T("�\�\�\�\�\�\�\�\�\�\�\�\�\\r\n"));
					strTranslated.Replace(_T("<span class=\"dictI\">"), _T(""));
					strTranslated.Replace(_T("</span>"), _T(""));
					DEBUGPUT(strTranslated);

					m_strTranslated = strTranslated;
					if (m_Edit.IsWindow()) {
						m_Edit.SetWindowText(_T(""));
						m_Edit.AppendText(strTranslated, TRUE);
						m_Edit.SetSelNone(TRUE);
					}
				}
				catch (LPCTSTR strError) {
					m_strTranslated = strError;
					if (m_Edit.IsWindow()) {
						m_Edit.SetWindowText(_T(""));
						m_Edit.AppendText(strError, TRUE);
					}
					DEBUGPUT(strError);
				}
				if (hFile)		::InternetCloseHandle(hFile);
				if (hInternet)	::InternetCloseHandle(hInternet);

			};
		

			boost::thread td(funcDictionary);
		}

	}
	void	KeepShow() 
	{
		MoveWindow(m_rcDefault);
		ShowWindow(TRUE);
	}

	CRect	ColcWindowPos(const CRect& rcMenu)
	{
		CPoint pt;
		::GetCursorPos(&pt);
		HMONITOR	hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		MONITORINFO moniInfo = { sizeof (MONITORINFO) };
		::GetMonitorInfo(hMonitor, &moniInfo);

		enum { cxMargin = 5, cyMargin = -20 };

		CRect rcWindow;
		rcWindow.top	= rcMenu.top + cyMargin;
		rcWindow.left	= rcMenu.right + cxMargin;
		rcWindow.right	= rcWindow.left + m_Size.cx;
		rcWindow.bottom	= rcWindow.top + m_Size.cy;
			
		if (rcWindow.right > moniInfo.rcWork.right) {	// ���j�^�[�̕��𒴂��Ă�
			rcWindow.left	= rcMenu.left - cxMargin - m_Size.cx;
			rcWindow.right	= rcWindow.left + m_Size.cx;
		}
		if (rcWindow.bottom	> moniInfo.rcWork.bottom) {	// ���j�^�[�̉����𒴂��Ă�
			int nButtomMargin = rcWindow.bottom	- moniInfo.rcWork.bottom;
			rcWindow.MoveToY(rcWindow.top - nButtomMargin);
		}
		m_rcDefault = rcWindow;
		return rcWindow;
	}

	BEGIN_MSG_MAP( x )
		MSG_WM_CREATE( OnCreate )
		MSG_WM_DESTROY( OnDestroy )
		MSG_WM_SIZE( OnSize )
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		SetWindowText(m_strWord);

		WTL::CLogFont	lf;
		lf.SetMenuFont();
		m_Edit.Create(m_hWnd, 0, NULL, WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL);
		m_Edit.SetFont(lf.CreateFontIndirect());
		//m_Edit.MoveWindow(0, 0, 250, 270);
		m_Edit.ShowWindow(TRUE);
		m_Edit.AppendText(m_strTranslated, TRUE);
		return 0;
	}

	void OnDestroy()
	{
		ModifyStyle(WS_BORDER, 0);

		if (m_Edit.IsWindow())
			m_Edit.DestroyWindow();
		m_Edit.m_hWnd = NULL;

		CIniFileO	pr(g_szIniFileName, _T("DictionaryWindow"));
		pr.SetValue(m_Size.cx, _T("Width"));
		pr.SetValue(m_Size.cy, _T("Height"));
	}

	void OnSize(UINT nType, CSize size)
	{
		if (size.cx != 0 && size.cy !=0) {
			CRect rc;
			GetWindowRect(&rc);
			//AdjustWindowRectEx(&rc, WS_THICKFRAME, FALSE, WS_EX_TOOLWINDOW);
			m_Size.SetSize(rc.Width(), rc.Height());
		}
		if (m_Edit.IsWindow())
			m_Edit.MoveWindow(0, 0, size.cx, size.cy);
	}


private:
	CEdit	m_Edit;
	CString m_strWord;
	CString m_strTranslated;
	CRect	m_rcDefault;
	CSize	m_Size;
};

/////////////////////////////////////////////////////////////////////
// CBingTranslatorMenu

// Constructor
CBingTranslatorMenu::CBingTranslatorMenu(CMainFrame* p)
	: m_pFrame(p)
	, m_pDicWindow(new CDictionaryWindow)
{
	m_menu.CreatePopupMenu();
	m_menu.AppendMenu(0, (UINT_PTR)0, _T("..."));
	//m_menu.AppendMenu(MF_SEPARATOR, (UINT_PTR)0);
}

// Destructor
CBingTranslatorMenu::~CBingTranslatorMenu()
{
	delete m_pDicWindow;
}

void CBingTranslatorMenu::OnInitMenuPopup(CMenuHandle menuPopup, UINT nIndex, BOOL bSysMenu)
{
	if (menuPopup.m_hMenu == m_menu.m_hMenu) {
		CString strSelected = m_pFrame->GetActiveSelectedText();
		if (m_strSelectedText == strSelected)
			return;

		m_strSelectedText = strSelected;

		while (m_menu.GetMenuItemCount() > 1) 
			m_menu.DeleteMenu(1, MF_BYPOSITION);
		m_menu.ModifyMenu(0, MF_BYPOSITION, (UINT_PTR)0, _T("..."));

		if (strSelected.IsEmpty())
			return;

		if (strSelected.GetLength() > 512) {
			m_menu.ModifyMenu(0, MF_BYPOSITION, (UINT_PTR)0, _T("���������������܂�"));
			return;
		}

		auto funcTranslate = [this, strSelected] () {
			HINTERNET hInternet = NULL;
			HINTERNET hFile		= NULL;
			try {
				hInternet = ::InternetOpen(_T("Mozilla/5.0"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
			if (hInternet == NULL) 
				throw _T("InternetOpen�Ɏ��s");

			CString strUrl;
			strUrl.Format(_T("http://api.microsofttranslator.com/v2/Http.svc/Translate?appId=%s&text=%s&from=en&to=ja"), appID, strSelected);
			hFile = ::InternetOpenUrl(hInternet, strUrl, NULL, 0, 0, 0);
			if (hFile == NULL)
				throw _T("InternetOpenUrl�Ɏ��s");

			std::string strTempBuff;
			while (1) {
				enum { BuffSize = 512 };
				char Buff[BuffSize] = "\0";
				DWORD ReadSize = 0;
				if (::InternetReadFile(hFile, (LPVOID)Buff, BuffSize, &ReadSize) && ReadSize == 0)
					break;

				Buff[ReadSize] = '\0';
				strTempBuff += Buff;
			}
			CString strBody = Misc::utf8_to_CString(strTempBuff.c_str());
			int nBegin = strBody.Find(_T('>'));
			int nEnd   = strBody.ReverseFind(_T('<'));
			if (nBegin == -1 || nEnd == -1 || nEnd < nBegin)
				throw _T("Empty");

			CDC	dc(::GetDC(NULL));
			WTL::CLogFont	lf;
			lf.SetMenuFont();
			WTL::CFont font;
			font.CreateFontIndirect(&lf);
			HFONT hOldFont = dc.SelectFont(font);

			std::vector<CString>	vecStr;
			CString str = m_strTranslated = strBody.Mid(nBegin + 1, nEnd - nBegin - 1);
			int nLength = str.GetLength();
			for (int i = 0; i < nLength; ++i) {
				CRect rc;
				const CString& strColc = str.Left(i);
				dc.DrawText(strColc, i, &rc, DT_CALCRECT);
				if (rc.right > 300) {
					vecStr.push_back(strColc);
					str = str.Mid(i);
					nLength -= i;
					i = 0;
				}
			}
			vecStr.push_back(str);
			dc.SelectFont(hOldFont);

			int nCount = (int)vecStr.size();
			for (int i = 0; i < nCount; ++i) {
				if (i == 0)
					m_menu.ModifyMenu(0, MF_BYPOSITION, ID_BINGTRANSLATE, vecStr[0]);
				else
					m_menu.AppendMenu(0, ID_BINGTRANSLATE, vecStr[i]);
			}
			
			m_menu.AppendMenu(MF_SEPARATOR, (UINT_PTR)0);

			CString strWord = strSelected;
			strWord.TrimLeft();
			strWord.TrimRight();
			int nSpace = strWord.Find(_T(' '));
			if (nSpace == -1) {
				m_menu.AppendMenu(0, ID_A_DICTIONARY, _T("����������"));
				m_menu.AppendMenu(MF_SEPARATOR, (UINT_PTR)0);
				m_pDicWindow->SetWord(strWord);
			}


			m_menu.AppendMenu(0, ID_COPYTRANSLATERESULT, _T("�|�󌋉ʂ��R�s�["));
			}
			catch (LPCTSTR strError) {
				strError;
				DEBUGPUT(strError);
			}

			if (hFile)		::InternetCloseHandle(hFile);
			if (hInternet)	::InternetCloseHandle(hInternet);	
		};
		boost::thread td(funcTranslate);
		//funcTranslate();
		return;
	}

	SetMsgHandled(FALSE);
}

void CBingTranslatorMenu::OnMenuSelect(UINT nItemID, UINT nFlags, CMenuHandle menu)
{
	if (menu.m_hMenu == m_menu.m_hMenu) {
		if (nItemID == ID_A_DICTIONARY) {		
			CRect rc;
			menu.GetMenuItemRect(NULL, 1, &rc);

			if (m_pDicWindow->IsWindow() == FALSE)
				m_pDicWindow->Create(NULL);
			m_pDicWindow->MoveWindow(m_pDicWindow->ColcWindowPos(rc));
			m_pDicWindow->ShowWindow(SW_SHOWNOACTIVATE);
		} else {
			if (m_pDicWindow->IsWindow())
				m_pDicWindow->ShowWindow(SW_HIDE);
		}
		return ;
	} else {
		if (m_pDicWindow->IsWindow())
			m_pDicWindow->ShowWindow(SW_HIDE);
	}
	if (menu.IsNull()) {
		if (m_pDicWindow->IsWindow())
			m_pDicWindow->DestroyWindow();
	}
	SetMsgHandled(FALSE);
}

// BingTranslator�Ŗ|�󂷂�
void CBingTranslatorMenu::OnBingTranslate(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_pFrame->OpenBingTranslator(m_strSelectedText);
}

// �|�󌋉ʂ��R�s�[
void CBingTranslatorMenu::OnCopyTranslateResult(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_strTranslated.IsEmpty() == FALSE)
		MtlSetClipboardText(m_strTranslated, NULL);
}

// �����E�B���h�E��\�������܂܂ɂ���
void CBingTranslatorMenu::OnShowDicWindow(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_pDicWindow->Create(NULL);
	m_pDicWindow->KeepShow();
}











