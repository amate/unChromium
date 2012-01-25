/**
 *	@file	aboutdlg.cpp
 *	@brief	�o�[�W��������\������_�C�A���O
 */
#include "stdafx.h"
#include "aboutdlg.h"
#include "../MtlWin.h"
#include "../MtlMisc.h"
#include "../appconst.h"
#include "../DonutPFunc.h"

#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//�g�p���閼�O��Ԃ̒�`

using namespace WTL;


//�ȉ�����
CAboutDlg::CAboutDlg()
	: m_wndEdit(this, 1)			//�e�L�X�g�{�b�N�X�ւ̃��b�Z�[�W��ALT_MSG_MAP(1)�Ŏ󂯎��
	, m_nSubclassPos(0)
{	}

static BOOL	CALLBACK EnumChildProc(HWND hWndChild, LPARAM pfunc)
{
	function<void (HWND)>& func = *(function<void (HWND)>*)pfunc;
	func(hWndChild);
	return TRUE;
}


void	CAboutDlg::_subclassWindows(HWND hWndChild)
{
	CString strClass;
	GetClassName(hWndChild, strClass.GetBuffer(128), 128);
	if (strClass.CompareNoCase(_T("Static")) == 0) {
		if (::GetWindowTextLength(hWndChild) > 0) {
			ATLASSERT(m_nSubclassPos < _countof(m_static));
			m_static[m_nSubclassPos].SubclassWindow(hWndChild);
			++m_nSubclassPos;
		} else {
			if (CWindow(hWndChild).GetStyle() & SS_ICON)
				m_staticIcon.SubclassWindow(hWndChild);
		}
	} else if (::GetDlgCtrlID(hWndChild) == IDOK)
		m_btnOk.SubclassWindow(hWndChild);
}



LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
	SetMsgHandled(FALSE);

	CenterWindow( GetParent() );	//�_�C�A���O�͏�ɒ����\��

	//���x���ɏ���ݒ�
	CStatic label	   = GetDlgItem(IDC_STATIC_VERSION);
	CString strAppName = app::cnt_AppName;
	CString strAppVer  = app::cnt_AppVersion;
	MtlSetWindowText(label.m_hWnd, strAppName + _T(" ") + strAppVer);

	//�e�L�X�g�{�b�N�X�ɏ���ݒ�
	CEdit	edit	   = GetDlgItem(IDC_EDITINFO);
	MtlSetWindowText( edit.m_hWnd, GetEnvironInfo() );

	//�e�L�X�g�{�b�N�X���Ẵ��b�Z�[�W�������ł���悤�ɂ���
	m_wndEdit.SubclassWindow(edit.m_hWnd);

	WTL::CLogFont	lf;
	lf.SetMenuFont();
	m_wndEdit.SetFont(lf.CreateFontIndirect());

	if (IsCompositionEnabled()) {
		MARGINS m = {-1};
		CRect rcEdit;
		CRect rcWindow;
		m_wndEdit.GetWindowRect(&rcEdit);
		GetWindowRect(&rcWindow);
		m.cxLeftWidth	= rcEdit.left - rcWindow.left;
		m.cxRightWidth	= rcWindow.right - rcEdit.right;
		m.cyBottomHeight= rcWindow.bottom	- rcEdit.bottom;

		CPoint ptEdit;
		::MapWindowPoints(m_wndEdit, m_hWnd, &ptEdit, 1);
		m.cyTopHeight	= ptEdit.y;
		SetMargins(m);

		function<void (HWND)>	func = std::bind(&CAboutDlg::_subclassWindows, this, std::placeholders::_1);
		::EnumChildWindows(m_hWnd, EnumChildProc, (LPARAM)&func);
	}
	return TRUE;
}

/// �G�f�B�b�g�R���g���[���𔒔w�i��
HBRUSH	CAboutDlg::OnCtlColorStatic(CDCHandle dc, CStatic wndStatic)
{
	if (wndStatic.m_hWnd == m_wndEdit.m_hWnd)
		return (HBRUSH)GetStockObject(WHITE_BRUSH);

	SetMsgHandled(FALSE);
	return 0;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	//�_�C�A���O����
	EndDialog(wID);
	return 0;
}



LRESULT CAboutDlg::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
	//�e�L�X�g�S�I��
	CEdit  edit = GetDlgItem(IDC_EDITINFO);
	edit.SetSelAll();

	//���j���[��\��(�N���b�v�{�[�h�Ƀe�L�X�g���R�s�[���邩�ۂ�)
	CMenu  menu;
	menu.CreatePopupMenu();
	menu.AppendMenu( 0, 1, _T("�N���b�v�{�[�h�ɃR�s�[") );

	//���ڂ��I�����ꂽ��e�L�X�g���R�s�[
	CPoint pos;
	::GetCursorPos(&pos);
	int    nRet = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pos.x, pos.y, m_hWnd);

	if (nRet == 1)
		MtlSetClipboardText(MtlGetWindowText(edit.m_hWnd), m_hWnd);

	return 0;
}



//OS�����ʂł��Ȃ������ꍇ�̕�����
#define UNKNOWNCODE 		_T("OS:unknown")



CString CAboutDlg::GetEnvironInfo()
{
	//��ʂ�̊������擾
	CString strOSName	  = GetOSName(); //���莸�s����UNKNOWNCODE��Ԃ�
	CString strVersion	  = GetVersion();
	CString strOSInfo	  = GetOSInfo();
	CString strIEVersion  = GetIEVersion();
	CString strUpdateInfo = GetUpdateInfo();

	//���莸�s���̏���
	if (strOSName != UNKNOWNCODE)
		strOSName = _T("Windows ") + strOSName;

	//�����e�L�X�g�{�b�N�X�ɕ\��
	CString strInfo;
	strInfo.Format(_T("%s %s\r\nOS : %s (%s) %s\r\nIE : %s%s\r\n"),
				   app::cnt_AppName, app::cnt_AppVersion, strOSName, strVersion, strOSInfo, strIEVersion, strUpdateInfo);

	return strInfo;
}



CString CAboutDlg::GetIEVersion()
{
	//���W�X�g������IE�̃o�[�W�������擾
	TCHAR	buf[MAX_PATH+2];
	DWORD	dwCount = MAX_PATH;
	Misc::CRegKey reg;

	memset(buf, 0, sizeof buf); //+++
	LRESULT rc = reg.Open( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Internet Explorer"), KEY_QUERY_VALUE );
	if (rc == ERROR_SUCCESS) {
		rc = reg.QueryStringValue( _T("Version"), buf, &dwCount);
		reg.Close();
	}

	if (rc != ERROR_SUCCESS) {
		rc = reg.Open( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Internet Explorer\\Version Vector"), KEY_QUERY_VALUE );
		if (rc == ERROR_SUCCESS) {
			rc = reg.QueryStringValue( _T("IE"), buf, &dwCount);
			//DWORD dw = 0;
			//LRESULT rc = reg.QueryDWORDValue( _T("IE"), dw);
			//sprintf(buf, _T("%d"), dw);
			reg.Close();
		}
	}


	return CString(buf);
}



CString CAboutDlg::GetUpdateInfo()
{
	//���W�X�g������Windows�̃A�b�v�f�[�g�����擾
	TCHAR	buf[MAX_PATH+2];
	DWORD	dwCount = MAX_PATH;
	Misc::CRegKey reg;

	memset(buf, 0, sizeof buf); //+++
	LRESULT rc = reg.Open( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_QUERY_VALUE );
	//+++ reg.Open( HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings") );
	if (rc == ERROR_SUCCESS) {
		reg.QueryStringValue( _T("MinorVersion"), buf, &dwCount);
		reg.Close();
	}

	return CString(buf);
}



CString CAboutDlg::GetOSInfo()
{
	//OS�̒ǉ������擾
	OSVERSIONINFO osvi = { sizeof (OSVERSIONINFO) };

	::GetVersionEx(&osvi);

	if (osvi.szCSDVersion[1] != 'A' && osvi.szCSDVersion[1] != 'B' && osvi.szCSDVersion[1] != 'C') {
		return CString(osvi.szCSDVersion);
	} else {
		return CString();
	}
}



CString CAboutDlg::GetVersion()
{
	//OS�̃o�[�W���������W���[�E�}�C�i�[�E�r���h�i���o�[�̏��ŕ\��
	OSVERSIONINFO osvi = { sizeof (OSVERSIONINFO) };

	::GetVersionEx(&osvi);

	CString 	  strVersion;
	strVersion.Format(_T("%d.%d.%d"), osvi.dwMajorVersion,
					  osvi.dwMinorVersion,
					  osvi.dwBuildNumber & 0xffff);
	return strVersion;
}



CString CAboutDlg::GetOSName()
{
	//OS�����擾����
	OSVERSIONINFO osvi = { sizeof (OSVERSIONINFO) };

	::GetVersionEx(&osvi);

	//�o�[�W�������ɏ����𕪂���
	if (osvi.dwMajorVersion == 3) {
		return GetOSName_Version3(osvi);
	} else if (osvi.dwMajorVersion == 4) {
		return GetOSName_Version4(osvi);
	} else if (osvi.dwMajorVersion == 5) {
		return GetOSName_Version5(osvi);
	} else if (osvi.dwMajorVersion == 6) {
		return GetOSName_Version6(osvi);
	} else {
		return UNKNOWNCODE;
	}
}



CString CAboutDlg::GetOSName_Version3(OSVERSIONINFO &osvi)	//NT3.51
{
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		if (osvi.dwMinorVersion == 51)
			return _T("NT 3.51");
		else
			return UNKNOWNCODE;
	} else {
		return UNKNOWNCODE;
	}
}



CString CAboutDlg::GetOSName_Version4(OSVERSIONINFO &osvi)	//95 98 Me NT4
{
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		if (osvi.dwMinorVersion >= 90) {
			//Me
			return _T("Me");
		} else if (osvi.dwMinorVersion >= 10) {
			//98
			if (osvi.szCSDVersion[1] == 'A') {
				return _T("98 SE");
			} else {
				return _T("98");
			}
		} else if (osvi.dwMinorVersion >= 0) {
			//95
			if (osvi.szCSDVersion[1] == 'B' || osvi.szCSDVersion[1] == 'C') {
				return _T("95 OSR2");
			} else {
				return _T("95");
			}
		} else {
			return UNKNOWNCODE;
		}
	} else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		//NT4
		if (osvi.dwMinorVersion == 0) {
			return _T("NT 4.0");
		} else {
			return UNKNOWNCODE;
		}
	} else {
		return UNKNOWNCODE;
	}
}



CString CAboutDlg::GetOSName_Version5(OSVERSIONINFO &osvi)	//2000 XP 2003Server
{
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
	  #if defined _WIN64
		if (osvi.dwMinorVersion >= 2) {
			return _T("Server 2003(x64)");
		} else if (osvi.dwMinorVersion == 1) {
			return _T("XP(x64)");
		} else {
			return _T("2000(x64)"); 						//�Ȃ�?
		}

	  #else
		if ( Misc::IsWow64() ) {
			if (osvi.dwMinorVersion >= 2) {
				return _T("Server 2003(x64)");
			} else if (osvi.dwMinorVersion == 1) {
				return _T("XP(x64)");
			} else {
				return _T("2000(x64)"); 					//�Ȃ�?
			}
		} else {
			if (osvi.dwMinorVersion >= 2) {
				return _T("Server 2003");
			} else if (osvi.dwMinorVersion == 1) {
				return _T("XP");
			} else {
				return _T("2000");
			}
		}
	  #endif

	} else {
		return UNKNOWNCODE;
	}
}



CString CAboutDlg::GetOSName_Version6(OSVERSIONINFO &osvi)	//Vista,Windows7
{
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		if (osvi.dwMinorVersion == 0) {
		  #if defined _WIN64
			return _T("Vista(x64)");
		  #else
			if ( Misc::IsWow64() )
				return _T("Vista(x64)");
			else
				return _T("Vista");
		  #endif
		} else if (osvi.dwMinorVersion == 1) {
		  #if defined _WIN64
			return _T("Windows7(x64)");
		  #else
			if ( Misc::IsWow64() )
				return _T("Windows7(x64)");
			else
				return _T("Windows7");
		  #endif
		} else {
			return UNKNOWNCODE;
		}
	} else {
		return UNKNOWNCODE;
	}
}
