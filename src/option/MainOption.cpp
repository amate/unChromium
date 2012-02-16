/**
 *	@file	MainOption.cpp
 *	@brief	donut�̃I�v�V���� : �S��, �S��2
 */

#include "stdafx.h"
#include "MainOption.h"
#include "../MtlWeb.h"
#include "../DonutPFunc.h"
#include "../ie_feature_control.h"
#include "../RecentClosedTabList.h"


////////////////////////////////////////////////////////////////////////////////
//CMainOption�̒�`
////////////////////////////////////////////////////////////////////////////////

DWORD		CMainOption::s_dwMainExtendedStyle		=
	  MAIN_EX_KILLDIALOG 
	| MAIN_EX_NOMDI 
	| MAIN_EX_INHERIT_OPTIONS 
	| MAIN_EX_EXTERNALNEWTAB 
	| MAIN_EX_EXTERNALNEWTABACTIVE;

DWORD		CMainOption::s_dwMainExtendedStyle2 	= 0;
DWORD		CMainOption::s_dwExplorerBarStyle		= 0;
DWORD		CMainOption::s_dwMaxWindowCount 		= 0;
DWORD		CMainOption::s_dwBackUpTime 			= 1;
DWORD		CMainOption::s_dwAutoRefreshTime		= 10;			// UDT DGSTR ( dai
bool		CMainOption::s_bTabMode 				= false;

volatile bool CMainOption::s_bAppClosing			= false;
volatile bool CMainOption::s_bIgnoreUpdateClipboard	= false;


DWORD	CMainOption::s_dwErrorBlock 			= 0;

CString	CMainOption::s_strExplorerUserDirectory;

bool	CMainOption::s_bTravelLogGroup			= false;
bool	CMainOption::s_bTravelLogClose			= false;

bool	CMainOption::s_bStretchImage			= false;

bool	CMainOption::s_bIgnore_blank			= false;
bool	CMainOption::s_bUseCustomFindBar		= false;
bool	CMainOption::s_bExternalNewTab			= true;
bool	CMainOption::s_bExternalNewTabActive	= true;

int		CMainOption::s_nMaxRecentClosedTabCount		= 16;
int		CMainOption::s_RecentClosedTabMenuType		= RECENTDOC_MENUTYPE_URL;

int		CMainOption::s_nAutoImageResizeType	= AUTO_IMAGE_RESIZE_LCLICK;

CString CMainOption::s_strCacheFolderPath;


CMainOption::CMainOption()
{
}



void CMainOption::GetProfile()
{
	CIniFileI	pr( g_szIniFileName, _T("Main") );
	{
		pr.QueryValue( s_dwMainExtendedStyle	, _T("Extended_Style")		);
		pr.QueryValue( s_dwMainExtendedStyle2	, _T("Extended_Style2") 	);
		pr.QueryValue( s_dwMaxWindowCount		, _T("Max_Window_Count")	);
		pr.QueryValue( s_dwBackUpTime			, _T("BackUp_Time") 		);
		pr.QueryValue( s_dwAutoRefreshTime		, _T("Auto_Refresh_Time")	);	// UDT DGSTR ( dai
		pr.QueryValue( s_dwExplorerBarStyle 	, _T("ExplorerBar_Style")	);	// UH
		pr.QueryValue( s_dwErrorBlock			, _T("ErrorBlock")			);	//minit
		s_bTravelLogGroup	= pr.GetValue(_T("TravelLogGroup"), s_bTravelLogGroup) != 0;
		s_bTravelLogClose	= pr.GetValue(_T("TravelLogClose"), s_bTravelLogClose) != 0;
		pr.QueryValue( s_nMaxRecentClosedTabCount, _T("MaxRecentClosedTabCount"));
		pr.QueryValue( s_RecentClosedTabMenuType , _T("RecentClosedTabMenuType"));
	}

	s_bTabMode			= (s_dwMainExtendedStyle & MAIN_EX_NOMDI) != 0;
	s_bIgnore_blank		= (s_dwMainExtendedStyle & MAIN_EX_IGNORE_BLANK) != 0;
	s_bUseCustomFindBar = (s_dwMainExtendedStyle & MAIN_EX_USECUSTOMFINDBER) != 0;
	s_bExternalNewTab	= (s_dwMainExtendedStyle & MAIN_EX_EXTERNALNEWTAB) != 0;
	s_bExternalNewTabActive = (s_dwMainExtendedStyle & MAIN_EX_EXTERNALNEWTABACTIVE) != 0;

	// NOTE. If all the Web Browser server on your desktop is unloaded, some OS automatically goes online.
	//		 And if all the Web Browser server on you application is unloaded, some OS automatically goes online.
	if ( _check_flag(MAIN_EX_LOADGLOBALOFFLINE, s_dwMainExtendedStyle) ) {
		MtlSetGlobalOffline( _check_flag(MAIN_EX_GLOBALOFFLINE, s_dwMainExtendedStyle) );
	}

	pr.QueryValue(s_nAutoImageResizeType, _T("AutoImageResizeType"));


	pr.ChangeSectionName( _T("Explorer_Bar") );
	s_strExplorerUserDirectory = pr.GetStringUW(_T("UserDirectory"));	

	s_strCacheFolderPath = pr.GetStringUW(_T("CacheFolderPath"));
}



void CMainOption::WriteProfile()
{
	if ( MtlIsGlobalOffline() )
		s_dwMainExtendedStyle |= MAIN_EX_GLOBALOFFLINE;
	else
		s_dwMainExtendedStyle &= ~MAIN_EX_GLOBALOFFLINE;

	CIniFileO	pr( g_szIniFileName, _T("Main") );
	{
		pr.SetValue( s_dwMainExtendedStyle	, _T("Extended_Style")		);
		pr.SetValue( s_dwMainExtendedStyle2 , _T("Extended_Style2") 	);
		pr.SetValue( s_dwExplorerBarStyle	, _T("ExplorerBar_Style")	);	// UDT DGSTR ( dai
		pr.SetValue( s_dwMaxWindowCount 	, _T("Max_Window_Count")	);
		pr.SetValue( s_dwBackUpTime 		, _T("BackUp_Time") 		);
		pr.SetValue( s_dwAutoRefreshTime	, _T("Auto_Refresh_Time")	);	// UDT DGSTR ( dai
		pr.SetValue( s_bTravelLogGroup		, _T("TravelLogGroup")		);
		pr.SetValue( s_bTravelLogClose		, _T("TravelLogClose")		);
		pr.SetValue( s_nMaxRecentClosedTabCount, _T("MaxRecentClosedTabCount"));
		pr.SetValue( s_RecentClosedTabMenuType , _T("RecentClosedTabMenuType"));
	}

	pr.SetValue(s_nAutoImageResizeType, _T("AutoImageResizeType"));


	pr.ChangeSectionName(_T("Explorer_Bar"));
	pr.SetStringUW( s_strExplorerUserDirectory, _T("UserDirectory") );

	pr.SetStringUW(s_strCacheFolderPath, _T("CacheFolderPath"));
}

void CMainOption::SetExplorerUserDirectory(const CString &strPath)
{
	s_strExplorerUserDirectory = strPath;

	CIniFileO pr(g_szIniFileName, _T("Explorer_Bar"));
	pr.SetStringUW( s_strExplorerUserDirectory, _T("UserDirectory") );
}

const CString& CMainOption::GetExplorerUserDirectory()
{
	return s_strExplorerUserDirectory;
}


bool CMainOption::IsQualify(int nWindowCount)
{
	if ( !(s_dwMainExtendedStyle & MAIN_EX_WINDOWLIMIT) )
		return true;

	if (nWindowCount < (int) s_dwMaxWindowCount)
		return true;
	else
		return false;
}



void CMainOption::OnMainExNewWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (s_dwMainExtendedStyle & MAIN_EX_NEWWINDOW)
		s_dwMainExtendedStyle &= ~MAIN_EX_NEWWINDOW;
	else
		s_dwMainExtendedStyle |= MAIN_EX_NEWWINDOW;
}



void CMainOption::OnMainExNoActivate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE)
		s_dwMainExtendedStyle &= ~MAIN_EX_NOACTIVATE;
	else
		s_dwMainExtendedStyle |= MAIN_EX_NOACTIVATE;
}



void CMainOption::OnMainExNoActivateNewWin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE_NEWWIN)
		s_dwMainExtendedStyle &= ~MAIN_EX_NOACTIVATE_NEWWIN;
	else
		s_dwMainExtendedStyle |= MAIN_EX_NOACTIVATE_NEWWIN;
}



////////////////////////////////////////////////////////////////////////////////
//CMainPropertyPage�̒�`
////////////////////////////////////////////////////////////////////////////////


void CMainPropertyPage::OnCheckExternalNewTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl)
{
	bool bCheck = CButton(GetDlgItem(IDC_CHECK_EXTERNALNEWTAB)).GetCheck() != 0;
	CButton btn = GetDlgItem(IDC_CHECK_EXTERNALNEWTABACTIVE);
	btn.EnableWindow(bCheck);
	if (bCheck == false)
		btn.SetCheck(FALSE);
}


void CMainPropertyPage::OnFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl)
{
	CFontDialog dlg(&m_lf);

	if (dlg.DoModal() == IDOK) {
		m_lf = dlg.m_lf;
	}
}



// Constructor
CMainPropertyPage::CMainPropertyPage(HWND hWnd)
	: m_wnd(hWnd)
	, m_bInit(false)
{
	_SetData();
}



// Overrides
BOOL CMainPropertyPage::OnSetActive()
{
	if (m_bInit == false) {
		m_bInit = true;
		DoDataExchange(DDX_LOAD);
		OnCheckExternalNewTab(0, 0, NULL);
	}
	SetModified(TRUE);
	return TRUE;
}



BOOL CMainPropertyPage::OnKillActive()
{
	return TRUE;//DoDataExchange(DDX_SAVE);
}



BOOL CMainPropertyPage::OnApply()
{
	if ( DoDataExchange(DDX_SAVE) ) {
		_GetData();
		return TRUE;
	} else {
		return FALSE;
	}
}



void CMainPropertyPage::_GetData()
{
	// Update main style
	CMainOption::s_dwMainExtendedStyle = 0;

	if (m_nNewWindow			/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_NEWWINDOW;
	if (m_nNoActivate			/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_NOACTIVATE;
	if (m_nNoActivateNewWin 	/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_NOACTIVATE_NEWWIN;
	if (m_nOneInstance			/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_ONEINSTANCE;
	if (m_nLimit				/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_WINDOWLIMIT;
	if (m_nNoCloseDFG			/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_NOCLOSEDFG;
	if (m_nNoMDI				/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_NOMDI;
	if (m_nBackUp				/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_BACKUP;
	if (m_nAddFavoriteOldShell	/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_ADDFAVORITEOLDSHELL;
	if (m_nOrgFavoriteOldShell	/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_ORGFAVORITEOLDSHELL;

	if (m_nRegisterAsBrowser	/*== 1*/) {
		CMainOption::s_dwMainExtendedStyle |= MAIN_EX_REGISTER_AS_BROWSER;
		MtlSendOnCommand(m_wnd, ID_REGISTER_AS_BROWSER);
	} else {
		MtlSendOffCommand(m_wnd, ID_REGISTER_AS_BROWSER);
	}

	if (m_nLoadGlobalOffline	/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_LOADGLOBALOFFLINE;
	if (m_nKillDialog			/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_KILLDIALOG;
	if (m_nInheritOptions		/*== 1*/) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_INHERIT_OPTIONS;

	//+++ if (m_nNoCloseNL				) CMainOption::s_dwMainExtendedStyle |= MAIN_EX_NOCLOSE_NAVILOCK;			//+++ �ǉ�

#if 0	//* ���Ƃ�
	ie_coInternetSetFeatureEnabled(FEATURE_SECURITYBAND    , SET_FEATURE_ON_PROCESS, m_nKillDialog);	//+++
#endif

	if (s_bIgnore_blank)		CMainOption::s_dwMainExtendedStyle |= MAIN_EX_IGNORE_BLANK;
	if (s_bUseCustomFindBar)	CMainOption::s_dwMainExtendedStyle |= MAIN_EX_USECUSTOMFINDBER;
	if (s_bExternalNewTab) {
		CMainOption::s_dwMainExtendedStyle |= MAIN_EX_EXTERNALNEWTAB;
		if (s_bExternalNewTabActive) {
			CMainOption::s_dwMainExtendedStyle |= MAIN_EX_EXTERNALNEWTABACTIVE;
		}
	}

	// update max window count
	CMainOption::s_dwMaxWindowCount  = m_nMaxWindowCount;
	CMainOption::s_dwBackUpTime 	 = m_nBackUpTime;
	// UDT DGSTR ( dai
	m_nAutoRefreshTime				 = m_nAutoRefTimeMin * 60 + m_nAutoRefTimeSec;
	CMainOption::s_dwAutoRefreshTime = m_nAutoRefreshTime;
	// ENDE
	m_wnd.PostMessage(WM_COMMAND, ID_BACKUPOPTION_CHANGED);

	CIniFileO pr( g_szIniFileName, _T("Main") );
	m_lf.WriteProfile(pr);

	WriteProfile();
}



void CMainPropertyPage::_SetData()
{
	//+++ ����: MAIN_EX_FULLSCREEN, MAIN_EX_GLOBALOFFLINE, MAIN_EX2_NOCSTMMENU, MAIN_EX2_DEL_RECENTCLOSE �͂����Ŕ��f���Ȃ�.

	m_nNewWindow		   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_NEWWINDOW		  ) != 0;		//+++ ? 1 : 0;
	m_nNoActivate		   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE		  ) != 0;		//+++ ? 1 : 0;
	m_nNoActivateNewWin    = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE_NEWWIN  ) != 0;		//+++ ? 1 : 0;
	m_nOneInstance		   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_ONEINSTANCE		  ) != 0;		//+++ ? 1 : 0;
	m_nLimit			   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_WINDOWLIMIT		  ) != 0;		//+++ ? 1 : 0;
	m_nNoCloseDFG		   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOCLOSEDFG		  ) != 0;		//+++ ? 1 : 0;
	m_nNoMDI			   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOMDI			  ) != 0;		//+++ ? 1 : 0;
	m_nBackUp			   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_BACKUP			  ) != 0;		//+++ ? 1 : 0;
	m_nMaxWindowCount	   = CMainOption::s_dwMaxWindowCount;
	m_nBackUpTime		   = CMainOption::s_dwBackUpTime;
	// UDT DGSTR ( dai
	m_nAutoRefreshTime	   = CMainOption::s_dwAutoRefreshTime;
	m_nAutoRefTimeMin	   = CMainOption::s_dwAutoRefreshTime / 60;
	m_nAutoRefTimeSec	   = CMainOption::s_dwAutoRefreshTime % 60;
	// ENDE
	m_nAddFavoriteOldShell = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_ADDFAVORITEOLDSHELL) != 0;		//+++ ? 1 : 0;
	m_nOrgFavoriteOldShell = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_ORGFAVORITEOLDSHELL) != 0;		//+++ ? 1 : 0;
	m_nLoadGlobalOffline   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_LOADGLOBALOFFLINE  ) != 0;		//+++ ? 1 : 0;
	m_nKillDialog		   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_KILLDIALOG		  ) != 0;		//+++ ? 1 : 0;
	m_nRegisterAsBrowser   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_REGISTER_AS_BROWSER) != 0;		//+++ ? 1 : 0;
	m_nInheritOptions	   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_INHERIT_OPTIONS	  ) != 0;		//+++ ? 1 : 0;
	//+++ m_nNoCloseNL	   = (CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOCLOSE_NAVILOCK   ) != 0;		//+++ �ǉ�.

	// refresh our font
	WTL::CLogFont	lft;
	lft.SetMenuFont();
	m_lf = lft;

	CIniFileI	 pr( g_szIniFileName, _T("Main") );
	MTL::CLogFont	 lf;
	if ( lf.GetProfile(pr) )
		m_lf = lf;
}



////////////////////////////////////////////////////////////////////////////////
//CMainPropertyPage2�̒�`
////////////////////////////////////////////////////////////////////////////////

int CALLBACK CMainPropertyPage2::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED) {
		if (lpData)
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}



CString CMainPropertyPage2::BrowseForFolder(const CString& strTitle, const CString& strNowPath)
{
	TCHAR		szDisplayName[MAX_PATH] = _T("\0");

	BROWSEINFO	bi = {
		m_hWnd,
		NULL,
		szDisplayName,
		LPCTSTR(strTitle),				//+++ strTitle.GetBuffer(0),	//+++
		BIF_RETURNONLYFSDIRS,
		&BrowseCallbackProc,
		(LPARAM) LPCTSTR(strNowPath),	//+++ (LPARAM)strNowPath.GetBuffer(0),
		0
	};

	CItemIDList idl;

	idl.Attach( ::SHBrowseForFolder(&bi) );
	return idl.GetPath();
}



// Constructor
CMainPropertyPage2::CMainPropertyPage2(HWND hWnd, CRecentClosedTabList& rRecent) : 
	m_wnd(hWnd), 
	m_rRecentClosedTabList(rRecent),
	m_nMRUCountMin(CRecentClosedTabList::kMaxEntries_Min),
	m_nMRUCountMax(CRecentClosedTabList::kMaxEntries_Max)
{
	m_bInit 	     = FALSE;

	//+++ mainframe.cpp��InitStatusBar�ł̏����l�ɂ��킹��150��125�ɕύX.
	m_nSzPain1	     = 125;
	m_nSzPain2	     = 125;

	m_nChkSwapPain   = FALSE;

	m_nShowMenu      = FALSE;
	m_nShowToolBar   = FALSE;
	m_nShowAdress    = FALSE;
	m_nShowTab	     = FALSE;
	m_nShowLink      = FALSE;
	m_nShowSearch    = FALSE;
	m_nShowStatus    = FALSE;

	m_nMinBtn2Tray   = 0;		//+++

	_SetData();
}



// Overrides
BOOL CMainPropertyPage2::OnSetActive()
{
	if (!m_bInit) {
		m_bInit = TRUE;
		InitCtrls();
		DoDataExchange(DDX_LOAD);
	}

	SetModified(TRUE);
	return TRUE;
}



BOOL CMainPropertyPage2::OnKillActive()
{
	return TRUE;
}



BOOL CMainPropertyPage2::OnApply()
{
	if ( DoDataExchange(DDX_SAVE) ) {
		_GetData();
		return TRUE;
	} else {
		return FALSE;
	}
}



void CMainPropertyPage2::_GetData()
{
	CIniFileO pr( g_szIniFileName, _T("StatusBar") );
	pr.SetValue( MAKELONG(m_nSzPain1, m_nSzPain2), _T("SizePain") );
	pr.SetValue( m_nChkSwapPain 				 , _T("SwapPain") );
	//x pr.Close(); //+++

	//x CIniFileO pr( g_szIniFileName, _T("FullScreen") );
	pr.ChangeSectionName( _T("FullScreen")	);
	pr.SetValue( m_nShowMenu	, _T("ShowMenu")	);
	pr.SetValue( m_nShowToolBar , _T("ShowToolBar") );
	pr.SetValue( m_nShowAdress	, _T("ShowAdress")	);
	pr.SetValue( m_nShowTab 	, _T("ShowTab") 	);
	pr.SetValue( m_nShowLink	, _T("ShowLink")	);
	pr.SetValue( m_nShowSearch	, _T("ShowSearch")	);
	pr.SetValue( m_nShowStatus	, _T("ShowStatus")	);
	//pr.Close();

	CMainOption::s_bTravelLogGroup = m_nTravelLogGroup != 0;	//+++ ? true : false;
	CMainOption::s_bTravelLogClose = m_nTravelLogClose != 0;	//+++ ? true : false;

	m_rRecentClosedTabList.ResetMenu();
	m_rRecentClosedTabList.SetMaxEntries(s_nMaxRecentClosedTabCount);
	m_rRecentClosedTabList.SetMenuType(s_RecentClosedTabMenuType);
	m_rRecentClosedTabList.UpdateMenu();

  #if 0	//+++ ���s
	if (m_nTitleBarStrSwap)	CMainOption::s_dwMainExtendedStyle2 	 |=  MAIN_EX2_TITLEBAR_STR_SWAP;	//+++ �ǉ�.
	else					CMainOption::s_dwMainExtendedStyle2 	 &= ~MAIN_EX2_TITLEBAR_STR_SWAP;	//+++ �ǉ�.
  #endif
  #if 1 //+++ �ʂ̃I�v�V�������Ǘ����Ă���ϐ����Ԏ؂肵�Ēǉ�...���.. ����ς蕜��
	CMainOption::s_dwMainExtendedStyle2 &= ~(MAIN_EX2_CLOSEBTN2TRAY|MAIN_EX2_MINBTN2TRAY);
	if (m_nMinBtn2Tray == 2)
		CMainOption::s_dwMainExtendedStyle2 |= MAIN_EX2_CLOSEBTN2TRAY;
	else if (m_nMinBtn2Tray == 1)
		CMainOption::s_dwMainExtendedStyle2 |= MAIN_EX2_MINBTN2TRAY;
  #endif

	WriteProfile();
}



void CMainPropertyPage2::_SetData()
{
	// UH
	CIniFileI	pr( g_szIniFileName, _T("StatusBar") );
	DWORD		dwVal = 0;
	if (pr.QueryValue( dwVal, _T("SizePain")	) == ERROR_SUCCESS) {
		m_nSzPain1 = LOWORD(dwVal);
		m_nSzPain2 = HIWORD(dwVal);
	}

	//+++ QueryValue(DWORD,)�́A���Ƃ�萬�������Ƃ��ɂ����l��ݒ肵�Ȃ��̂ŁA�G���[�`�F�b�N�͕s�v.
	pr.QueryValue( m_nChkSwapPain , _T("SwapPain")	 );

	//+++ QueryValue(DWORD,)�́A���Ƃ�萬�������Ƃ��ɂ����l��ݒ肵�Ȃ��̂ŁA�G���[�`�F�b�N�͕s�v.
	pr.ChangeSectionName( _T("FullScreen") );
	pr.QueryValue( m_nShowMenu	 , _T("ShowMenu")	 );
	pr.QueryValue( m_nShowToolBar, _T("ShowToolBar") );
	pr.QueryValue( m_nShowAdress , _T("ShowAdress")  );
	pr.QueryValue( m_nShowTab	 , _T("ShowTab")	 );
	pr.QueryValue( m_nShowLink	 , _T("ShowLink")	 );
	pr.QueryValue( m_nShowSearch , _T("ShowSearch")  );
	pr.QueryValue( m_nShowStatus , _T("ShowStatus")  );
	//pr.Close();

	m_nTravelLogGroup = CMainOption::s_bTravelLogGroup != 0;	//+++ ? 1 : 0;
	m_nTravelLogClose = CMainOption::s_bTravelLogClose != 0;	//+++ ? 1 : 0;

  #if 0	//+++ ���s
	m_nTitleBarStrSwap= (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_TITLEBAR_STR_SWAP) != 0;	//+++ �ǉ�.
  #endif

  #if 1 //+++ �ʂ̃I�v�V�������Ǘ����Ă���ϐ����Ԏ؂肵�Ēǉ�...���... ����ς�����
	if (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_CLOSEBTN2TRAY)
		m_nMinBtn2Tray = 2;
	else if (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_MINBTN2TRAY)
		m_nMinBtn2Tray = 1;
  #endif
}



void CMainPropertyPage2::InitCtrls()
{
	CString   strText;
	strText.Format(_T("(%d-%d)"), m_nMRUCountMin, m_nMRUCountMax);

	MtlSetWindowText(GetDlgItem(IDC_STATIC_MRU_MINMAX), strText);

	CComboBox cmbType( GetDlgItem(IDC_COMBO_MRU_MENUTYPE) );
	cmbType.AddString( _T("0 - URL") );
	cmbType.AddString( _T("1 - �^�C�g��") );
	cmbType.AddString( _T("2 - �^�C�g�� - URL") );
	cmbType.SetCurSel(0);
}
