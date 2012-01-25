/**
 *	@file	DebugWindow.cpp
 *	@brief	�Ƃ肠�����I�ȓK���f�o�b�O�p�_�C�A���O�𐶐�����N���X
 */
#include "stdafx.h"
#include "DebugWindow.h"

#ifdef _DEBUG
static CDebugUtility	g_debugutil;
#endif

/////////////////////////////////////////////////
// CDebugUtility::Impl

class CDebugUtility::Impl
{
public:
	Impl();
	~Impl();

	void	Write(LPCTSTR strFormat, va_list argList);
	void	WriteIn(LPCTSTR strFormat, va_list argList);

private:
	void	_WriteConsole(LPCTSTR str);

	HANDLE m_hOut;
};


//------------------------------------
CDebugUtility::Impl::Impl()
{
	::AllocConsole();
	m_hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
}

//------------------------------------
CDebugUtility::Impl::~Impl()
{
	::FreeConsole();
}

//------------------------------------
void	CDebugUtility::Impl::Write(LPCTSTR strFormat, va_list argList)
{
	CString str;
	str.FormatV(strFormat, argList);
	_WriteConsole(str);
}


//------------------------------------
void	CDebugUtility::Impl::WriteIn(LPCTSTR strFormat, va_list argList)
{
	CString str;
	str.FormatV(strFormat, argList);
	str += _T("\n");
	_WriteConsole(str);
}

//------------------------------------
void	CDebugUtility::Impl::_WriteConsole(LPCTSTR str)
{
	DWORD dwWrite;
	::WriteConsole(m_hOut, str, lstrlen(str), &dwWrite, NULL);
}












/////////////////////////////////////////////////
// CDebugUtility

CDebugUtility::Impl* CDebugUtility::pImpl = NULL;

//------------------------------------
CDebugUtility::CDebugUtility()
{ 
	pImpl = new Impl;
}

//------------------------------------
CDebugUtility::~CDebugUtility()
{
	delete pImpl;
}

//------------------------------------
void CDebugUtility::Write(LPCTSTR pstrFormat, ...)
{
	va_list args;
	va_start(args, pstrFormat);	
	pImpl->Write(pstrFormat, args);
	va_end(args);
}

//------------------------------------
void CDebugUtility::WriteIn(LPCTSTR pstrFormat, ...)
{
	va_list args;
	va_start(args, pstrFormat);	
	pImpl->WriteIn(pstrFormat, args);
	va_end(args);
}




#if 0
#include "../resource.h"



#ifndef NDEBUG

CDebugWindow *		CDebugWindow::s_pThis = NULL;




void CDebugWindow::Create(HWND hWndParent)
{
	if (g_cnt_b_use_debug_window) {
		CWindow hWnd = baseclass::Create(hWndParent);
		hWnd.ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
		s_pThis = this;

		::DeleteFile(_GetFilePath(_T("log.txt")));
	}
}



void CDebugWindow::Destroy()
{
	if (g_cnt_b_use_debug_window) {
		if ( IsWindow() ) {
			baseclass::DestroyWindow();
		}
		s_pThis = NULL;
	}
}



void CDebugWindow::OutPutString(LPCTSTR pstrFormat, ...)
{
	if (!s_pThis)
		return;

	if ( !s_pThis->m_wndEdit.IsWindow() )
		return;

	if (!g_cnt_b_use_debug_window)
		return;

	CString strText;
	{
		va_list args;
		va_start(args, pstrFormat);
		
		strText.FormatV(pstrFormat, args);

		va_end(args);
	}

	FILE *fp = _tfopen(_GetFilePath(_T("log.txt")), _T("a"));
	if (fp) {
		_ftprintf(fp, _T("%s"), strText);
		fclose(fp);
	}
	strText.Replace(_T("\n"), _T("\r\n"));
//	if (bReturn) strText += _T("\r\n");

	s_pThis->m_wndEdit.AppendText(strText);
}

void CDebugWindow::OutPutMenu(CMenuHandle menu)
{
	OutPutString(_T("=======================\n"));
	for (int i = 0; i < menu.GetMenuItemCount(); ++i) {
		CString strText;
		menu.GetMenuString(i, strText, MF_BYPOSITION);
		UINT uCmdID = menu.GetMenuItemID(i);
		if (uCmdID == 0) { strText = _T("�\�\�\�\�\�\"); }
		if (uCmdID == -1) {	// �T�u���j���[�H
			OutPutString(_T("           : > %s\n"), strText);
		} else {
			OutPutString(_T("%05d : %s\n"), uCmdID, strText);
		}
	}
	OutPutString(_T("=======================\n\n"));

}


BOOL	CDebugWindow::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	SetMsgHandled(FALSE);

	//CRect rc(0, 170, 300, 600);
	CRect rc(-300, 0, 0, 500);
	MoveWindow(rc);

	m_wndEdit = GetDlgItem(IDC_EDIT1);
	//m_wndEdit.Create(m_hWnd, rc, NULL, ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL);

	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
//	pLoop->AddIdleHandler(this);

	SetWindowPos(HWND_TOPMOST, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	OnSize(0, CSize(300, 600));

	return TRUE;
}

void	CDebugWindow::OnDestroy()
{
	// ���b�Z�[�W���[�v���烁�b�Z�[�W�t�B���^�ƃA�C�h���n���h�����폜
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
	//pLoop->RemoveIdleHandler(this);
}


LRESULT CDebugWindow::OnSize(UINT, CSize size)
{
	SetMsgHandled(FALSE);

	if (m_wndEdit.IsWindow()) {
		CRect rc;
		GetClientRect(&rc);
		::InflateRect(&rc, -4, -4);
		m_wndEdit.MoveWindow(rc);
	}
	return 0;
}


void	CDebugWindow::OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	DestroyWindow();
}


#endif
#endif