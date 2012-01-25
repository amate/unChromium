/**
 *	@file	DLControlOption.cpp
 *	brief	donut�̃I�v�V���� : "�u���E�U"
 */

#include "stdafx.h"
#include "DLControlOption.h"
#include "../IniFile.h"
#include "MainOption.h" 				//+++ �����Ώ�


#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace MTL;


//+++ Donut.cpp���ړ�.
TCHAR	CDLControlOption::s_szUserAgent[MAX_PATH];					// UDT DGSTR
TCHAR	CDLControlOption::s_szUserAgent_cur[MAX_PATH];				//+++
bool	CDLControlOption::s_bUseDLManager = false;
int		CDLControlOption::s_nGPURenderStyle = CDLControlOption::GPURENDER_NONE;

DWORD CDLControlOption::s_dwDLControlFlags	   = DLCTL_DEFAULT/*DLCTL_BGSOUNDS | DLCTL_VIDEOS | DLCTL_DLIMAGES*/;	//+++ 0�ȊO�̒l�ɕύX.
DWORD CDLControlOption::s_dwExtendedStyleFlags = DVS_EX_FLATVIEW | DVS_EX_MOUSE_GESTURE ;							//+++ 0�ȊO�̒l�ɕύX.


//--------------------------------
/// �ݒ�𕜌�
void CDLControlOption::GetProfile()
{
	CIniFileI	pr( g_szIniFileName, _T("Browser") );

	if (pr.QueryValue( s_dwDLControlFlags, _T("DLControl_Flags") ) != ERROR_SUCCESS)
		s_dwDLControlFlags = DLCTL_DEFAULT;

  #if 1	//+++
	CString tmp = pr.GetStringUW(_T("UserAgent"));
	if (!tmp.IsEmpty())
		::lstrcpyn( s_szUserAgent, tmp, MAX_PATH );

	char	szDefUserAgent[MAX_PATH] = "\0";
	DWORD	size = MAX_PATH;
	::ObtainUserAgentString( 0 , szDefUserAgent , &size);
	::lstrcpyn(s_szUserAgent_cur, CString(szDefUserAgent), size);
  #else
	// UDT DGSTR
	DWORD		dwCount = MAX_PATH;
	if (pr.QueryValue(s_szUserAgent, _T("UserAgent"), &dwCount) != ERROR_SUCCESS)
		::lstrcpy( s_szUserAgent, _T("") );
	// ENDE
  #endif

	// UH
	pr.QueryValue( s_dwExtendedStyleFlags, _T("ViewStyle") );

	s_nGPURenderStyle = pr.GetValuei(_T("GPURenderStyle"), GPURENDER_NONE);

	pr.ChangeSectionName(_T("DownloadManager"));
	s_bUseDLManager = pr.GetValue(_T("UseDLManager")) != 0;
}

//---------------------------------
/// �ݒ��ۑ�
void CDLControlOption::WriteProfile()
{
	CIniFileO	pr( g_szIniFileName, _T("Browser") );
	pr.SetValue ( s_dwDLControlFlags, _T("DLControl_Flags") );
	pr.SetStringUW( s_szUserAgent, _T("UserAgent") ); // UDT DGSTR

	// UH
	pr.SetValue ( s_dwExtendedStyleFlags, _T("ViewStyle") );

	pr.SetValue( s_nGPURenderStyle, _T("GPURenderStyle") );

	pr.ChangeSectionName(_T("DownloadManager"));
	pr.SetValue(s_bUseDLManager, _T("UseDLManager"));
}



//--------------------------------------
/// ���[�U�[�G�[�W�F���g��ݒ�
void	CDLControlOption::SetUserAgent()
{
	std::vector<char>	userAgent;
	if (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_USER_AGENT_FLAG) {
		userAgent = Misc::tcs_to_sjis( s_szUserAgent );
	} else {
		userAgent = Misc::tcs_to_sjis( s_szUserAgent_cur );
	}
	::UrlMkSetSessionOption(URLMON_OPTION_USERAGENT , (void*)userAgent.data(), (int)userAgent.size(), 0);
}



/////////////////////////////////////////////////////////////////////////////
// CDLControlPropertyPage



// Constructor
CDLControlPropertyPage::CDLControlPropertyPage(HWND hMainWnd)
	: m_bInit(false)
{
	m_hMainWnd = hMainWnd;
	_SetData();
}



// Overrides
BOOL CDLControlPropertyPage::OnSetActive()
{
	SetModified(TRUE);
	if (m_bInit == false) {
		m_bInit = true;
		// UDT DGSTR
		if (m_edit.m_hWnd == NULL)
			m_edit.Attach( GetDlgItem(IDC_EDIT_USER_AGENT) );
		// ENDE
		GetDlgItem(IDC_EDIT_DEFAULT_USER_AGENT).SetWindowText(s_szUserAgent_cur);

		if (Misc::IsGpuRendering() == false) {
			GetDlgItem(IDC_RADIO_NONE).EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_CASH).EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_ALWAYS).EnableWindow(FALSE);
		}
		DoDataExchange(DDX_LOAD);
	}
	return TRUE;
}



BOOL CDLControlPropertyPage::OnKillActive()
{
	return TRUE;
}



BOOL CDLControlPropertyPage::OnApply()
{
	if ( DoDataExchange(DDX_SAVE) ) {
		_GetData();
		return TRUE;
	} else {
		return FALSE;
	}
}

//----------------------------------
/// �_�C�A���O����ݒ��ۑ�
void CDLControlPropertyPage::_GetData()
{
	// update dl control flags
	CDLControlOption::s_dwDLControlFlags = 0;

	if (m_nBGSounds /*== 1*/)		CDLControlOption::s_dwDLControlFlags |= DLCTL_BGSOUNDS;
	if (m_nVideos	/*== 1*/)		CDLControlOption::s_dwDLControlFlags |= DLCTL_VIDEOS;
	if (m_nDLImages /*== 1*/)		CDLControlOption::s_dwDLControlFlags |= DLCTL_DLIMAGES;
	if (m_nRunActiveXCtls == 0) 	CDLControlOption::s_dwDLControlFlags |= DLCTL_NO_RUNACTIVEXCTLS;
	if (m_nDLActiveXCtls  == 0) 	CDLControlOption::s_dwDLControlFlags |= DLCTL_NO_DLACTIVEXCTLS;
	if (m_nScripts		  == 0) 	CDLControlOption::s_dwDLControlFlags |= DLCTL_NO_SCRIPTS;
	if (m_nJava 		  == 0) 	CDLControlOption::s_dwDLControlFlags |= DLCTL_NO_JAVA;

	// UH
	CDLControlOption::s_dwExtendedStyleFlags = 0;
	if (m_nNaviLock    /* == 1*/)	CDLControlOption::s_dwExtendedStyleFlags |= DVS_EX_OPENNEWWIN;
	if (m_nFlatView    /* == 1*/)	CDLControlOption::s_dwExtendedStyleFlags |= DVS_EX_FLATVIEW;
	if (m_bMsgFilter   /* == 1*/)	CDLControlOption::s_dwExtendedStyleFlags |= DVS_EX_MESSAGE_FILTER;
	if (m_bMouseGesture/* == 1*/)	CDLControlOption::s_dwExtendedStyleFlags |= DVS_EX_MOUSE_GESTURE;
	if (m_bBlockMailto /* == 1*/)	CDLControlOption::s_dwExtendedStyleFlags |= DVS_EX_BLOCK_MAILTO;

  #if 1 //+++ �ʂ̃I�v�V�������Ǘ����Ă���ϐ����Ԏ؂肵�Ēǉ�.
	if (m_nNoCloseNL			)	CMainOption::s_dwMainExtendedStyle2 	 |= MAIN_EX2_NOCLOSE_NAVILOCK;		//+++ �ǉ�.
	else							CMainOption::s_dwMainExtendedStyle2 	 &= ~MAIN_EX2_NOCLOSE_NAVILOCK; 	//+++ �ǉ�.
	if (m_bUserAgent			)	CMainOption::s_dwMainExtendedStyle2 	 |= MAIN_EX2_USER_AGENT_FLAG;		//+++ �ǉ�.
	else							CMainOption::s_dwMainExtendedStyle2 	 &= ~MAIN_EX2_USER_AGENT_FLAG;	 	//+++ �ǉ�.
  #endif
	CDLControlOption::s_bUseDLManager = m_bUseDLManager != 0;


	//\\? SendMessage(m_hMainWnd, WM_COMMAND, ID_RESIZED, 0);
	m_edit.GetWindowText(CDLControlOption::s_szUserAgent, MAX_PATH);	// UDT DGSTR

	SetUserAgent();	// ���[�U�[�G�[�W�F���g�ύX
}

//-------------------------------------------
/// �_�C�A���O�ɃI�v�V������ݒ�
void CDLControlPropertyPage::_SetData()
{
	m_nBGSounds 	  = (CDLControlOption::s_dwDLControlFlags & DLCTL_BGSOUNDS) != 0;		//+++ ? 1 : 0;
	m_nVideos		  = (CDLControlOption::s_dwDLControlFlags & DLCTL_VIDEOS  ) != 0;		//+++ ? 1 : 0;
	m_nDLImages 	  = (CDLControlOption::s_dwDLControlFlags & DLCTL_DLIMAGES) != 0;		//+++ ? 1 : 0;

	m_nRunActiveXCtls = (CDLControlOption::s_dwDLControlFlags & DLCTL_NO_RUNACTIVEXCTLS ) == 0; 	//+++ ? 0 : 1;
	m_nDLActiveXCtls  = (CDLControlOption::s_dwDLControlFlags & DLCTL_NO_DLACTIVEXCTLS	) == 0; 	//+++ ? 0 : 1;
	m_nScripts		  = (CDLControlOption::s_dwDLControlFlags & DLCTL_NO_SCRIPTS		) == 0; 	//+++ ? 0 : 1;
	m_nJava 		  = (CDLControlOption::s_dwDLControlFlags & DLCTL_NO_JAVA			) == 0; 	//+++ ? 0 : 1;

	m_nNaviLock 	  = (CDLControlOption::s_dwExtendedStyleFlags & DVS_EX_OPENNEWWIN	 ) != 0;	//+++ ? 1 : 0;
	m_nFlatView 	  = (CDLControlOption::s_dwExtendedStyleFlags & DVS_EX_FLATVIEW 	 ) != 0;	//+++ ? 1 : 0;
	m_bMsgFilter	  = (CDLControlOption::s_dwExtendedStyleFlags & DVS_EX_MESSAGE_FILTER) != 0;	//+++ ? 1 : 0;
	m_bMouseGesture   = (CDLControlOption::s_dwExtendedStyleFlags & DVS_EX_MOUSE_GESTURE ) != 0;	//+++ ? 1 : 0;
	m_bBlockMailto	  = (CDLControlOption::s_dwExtendedStyleFlags & DVS_EX_BLOCK_MAILTO  ) != 0;	//+++ ? 1 : 0;

  #if 1 //+++ �ʂ̃I�v�V�������Ǘ����Ă���ϐ����Ԏ؂肵�Ēǉ�.
	m_nNoCloseNL	   = (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_NOCLOSE_NAVILOCK	  ) != 0;		//+++ �ǉ�.
	m_bUserAgent	   = (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_USER_AGENT_FLAG	  ) != 0;		//+++ �ǉ�.
  #endif
	m_bUseDLManager	   = CDLControlOption::s_bUseDLManager; 

	// UDT DGSTR
	if (lstrlen(CDLControlOption::s_szUserAgent) != 0) {
		m_strUserAgent = CDLControlOption::s_szUserAgent;
	} else {
		char	szDefUserAgent[MAX_PATH*2];
		szDefUserAgent[0]	= 0;	//+++
		DWORD	size = MAX_PATH;
		::ObtainUserAgentString( 0 , szDefUserAgent , &size);
		// NOTE: Here , the other way.
		m_strUserAgent = szDefUserAgent;
	}
	// ENDE
}

