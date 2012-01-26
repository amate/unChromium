/**
 *	@file	CmbboxPrxy.cpp
 *	@brief	コンボボックス：プロキシ
 */

#include "stdafx.h"
#include "CmbboxPrxy.h"
#include <WinInet.h>
#include "DonutPFunc.h"
#include "MtlUser.h"
#include "option/ProxyDialog.h"

#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CComboBoxPrxyR::CComboBoxPrxyR()
{
	m_nIDEvent = 0;
	m_bUseIE   = UseIE();
}



LRESULT CComboBoxPrxyR::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	WORD wNotifyCode = HIWORD(wParam);
	if (wNotifyCode == CBN_SELCHANGE)
		OnSelectChange();

	return 1;
}



void CComboBoxPrxyR::OnSelectChange()
{
	int 	nIndex = GetCurSel();
	ChangeProxy(nIndex);
}

#if 1 //+++ UNICODE 対策. ひょっとすると、UrlMonはTCHAR未対応? かもで、代用品を用意.
struct SInternet_Proxy_Info {
    DWORD	dwAccessType;
    LPCSTR	lpszProxy;
    LPCSTR	lpszProxyBypass;
};
#endif


void CComboBoxPrxyR::ChangeProxy(int nIndex)
{
	if (m_bUseIE) return;

	CString 				str;
	MtlGetLBTextFixed(m_hWnd, nIndex, str);
#if 0
  #ifdef UNICODE	//+++	UNICODE対策.
	SInternet_Proxy_Info	proxyinfo;
	std::vector<char>		vec;
	std::vector<char>		bypass;
	if ( !str.IsEmpty() ) {
		vec						= Misc::tcs_to_sjis( str );
		bypass					= Misc::tcs_to_sjis( GetBypass() );
		proxyinfo.dwAccessType	= INTERNET_OPEN_TYPE_PROXY;
		proxyinfo.lpszProxy		= &vec[0];

		if ( bypass.empty() )
			proxyinfo.lpszProxyBypass = NULL;
		else
			proxyinfo.lpszProxyBypass = &bypass[0];
	} else {
		proxyinfo.dwAccessType	  = INTERNET_OPEN_TYPE_DIRECT;
		proxyinfo.lpszProxy 	  = NULL;
		proxyinfo.lpszProxyBypass = NULL;
	}
  #else
	CString 				strBypass;
	INTERNET_PROXY_INFO 	proxyinfo;

	if ( !str.IsEmpty() ) {
		proxyinfo.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
		proxyinfo.lpszProxy    = str;

		strBypass			   = GetBypass();

		if ( strBypass.IsEmpty() )
			proxyinfo.lpszProxyBypass = NULL;
		else
			proxyinfo.lpszProxyBypass = strBypass;
	} else {
		proxyinfo.dwAccessType	  = INTERNET_OPEN_TYPE_DIRECT;
		proxyinfo.lpszProxy 	  = NULL;
		proxyinfo.lpszProxyBypass = NULL;
	}
  #endif
	//UrlMkSetSessionOption(INTERNET_OPTION_PROXY, &proxyinfo, sizeof (proxyinfo), 0); 
#endif
#if 1
	INTERNET_PER_CONN_OPTION_LIST list = { sizeof(list) };
	list.pszConnection	= NULL;
    list.dwOptionCount = 3;
    list.pOptions = new INTERNET_PER_CONN_OPTION[3];
    // Set flags.
    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[0].Value.dwValue = str.IsEmpty() ? PROXY_TYPE_DIRECT : PROXY_TYPE_PROXY;

    // Set proxy name.
    list.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
	list.pOptions[1].Value.pszValue = str.IsEmpty() ? NULL : str.GetBuffer(0);

    // Set proxy override.
    list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
	CString strbypass = GetBypass();
	list.pOptions[2].Value.pszValue = strbypass.GetBuffer(0);

    // Set the options on the connection.
    InternetSetOption(NULL,
        INTERNET_OPTION_PER_CONNECTION_OPTION, &list, sizeof(list));

    // Free the allocated memory.
    delete [] list.pOptions;
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH , NULL, 0);
    //return bReturn;
#endif

	// プロキシファイルパス
	CString 	strFile = _GetFilePath( _T("Proxy.ini") );

	CIniFileO	pr( strFile, _T("PROXY") );
	pr.SetValue( (DWORD) nIndex, _T("TARGET") );
	pr.Close();

	// タイマー指定
	ResetTimer();
}



void CComboBoxPrxyR::OnTimer(UINT_PTR wTimerID, TIMERPROC)
{
	ATLTRACE2( atlTraceGeneral, 4, _T("CComboBoxPrxy::OnTimer\n") );

	if (wTimerID == m_nIDEvent) {
		int    nCount = GetCount() - 1;
		if (nCount == 0)
			return;

		time_t	nTime = 0;
		::time(&nTime);

		int    nRand  = int ( abs( ::rand() ) * abs( (long double) nTime ) );
		nRand = nRand % nCount;
		nRand = abs(nRand) + 1;

		SetCurSel(nRand);
		ChangeProxy(nRand);
	} else
		SetMsgHandled(FALSE);
}



LRESULT CComboBoxPrxyR::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CCenterPropertySheet sheet( _T("プロキシ設定") );
	CProxyPropertyPage	 pageProxy;

	sheet.AddPage(pageProxy);
	sheet.DoModal();
	ResetProxyList();
	return 0;
}



void CComboBoxPrxyR::SetProxy()
{
	// プロキシファイルパス
	CString 	strFile  = _GetFilePath( _T("Proxy.ini") );

	// プロキシ番号
	DWORD		dwIndex = 0;
	CIniFileI	pr( strFile, _T("PROXY") );
	pr.QueryValue( dwIndex, _T("TARGET") );
	pr.Close();

	// 指定番号にする
	int 		nIndex	= SetCurSel( (int) dwIndex );
	ChangeProxy(nIndex);
}



void CComboBoxPrxyR::ResetProxyList()
{
	if (m_bUseIE) {
		EnableWindow(FALSE);
		return;
	}

	ResetContent();
	AddString(_T(""));

	// プロキシファイルパス
	CString 	strFile = _GetFilePath( _T("Proxy.ini") );

	CIniFileI	pr( strFile, _T("PROXY") );

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// プロキシ
	DWORD		dwLineCnt = 0;
	pr.QueryValue( dwLineCnt, _T("MAX") );

	for (int ii = 0; ii < (int) dwLineCnt; ii++) {
		CString 	strKey;
		strKey.Format(_T("%d"), ii);

		CString 	strProxy = pr.GetString( strKey );		//*+++ ここをGetStringUWにするかは後で.
		if ( strProxy.IsEmpty() )
			continue;
		AddString(strProxy);
	}

	pr.Close();
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// プロキシ設定
	SetProxy();
}



void CComboBoxPrxyR::ResetTimer()
{
	// プロキシファイルパス
	CString 	strFile = _GetFilePath( _T("Proxy.ini") );

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// ランダム
	CIniFileI	pr( strFile, _T("RAND") );

	DWORD		dwRandChk	  = 0;
	pr.QueryValue( dwRandChk, _T("Enable") );

	DWORD		dwRandTimeMin = 5;
	pr.QueryValue( dwRandTimeMin, _T("Min") );

	DWORD		dwRandTimeSec = 0;
	pr.QueryValue( dwRandTimeSec, _T("Sec") );

	pr.Close();
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// ランダム指定なし
	if (dwRandChk == 0) {
		if (m_nIDEvent != 0) {
			MTLVERIFY( ::KillTimer(m_hWnd, m_nIDEvent) );
			m_nIDEvent = 0;
		}

		return;
	}

	// タイマー設定
	int 		nAutoChgTime  = (dwRandTimeMin * 60 + dwRandTimeSec) * 1000;
	m_nIDEvent = ::SetTimer(m_hWnd, 1, nAutoChgTime, NULL);
}



/// IEのを使う
bool CComboBoxPrxyR::UseIE()
{
	// プロキシファイルパス
	CString 	strFile = _GetFilePath( _T("Proxy.ini") );

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// ローカル
	CIniFileI	pr( strFile, _T("USE_IE") );
	DWORD		dwUseIE = 1;
	pr.QueryValue( dwUseIE, _T("Enable") );
	pr.Close();
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	return dwUseIE != 0;
}



/// バイパスを得る
CString CComboBoxPrxyR::GetBypass()
{
	// プロキシファイルパス
	CString 		strFile = _GetFilePath( _T("Proxy.ini") );
	CString 		strBypass;

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// 非プロキシ
	{
		CIniFileI	pr( strFile, _T("NOPROXY") );
		DWORD		dwLineCnt = 0;
		pr.QueryValue( dwLineCnt, _T("MAX") );

		for (int ii = 0; ii < (int) dwLineCnt; ii++) {
			CString 	strKey;
			strKey.Format(_T("%d"), ii);
			CString 	strProxy = pr.GetString( strKey );		//*+++ ここをGetStringUWにするかは後で.
			if ( strProxy.IsEmpty() )
				continue;

			if (strBypass.IsEmpty() == FALSE)
				strBypass = strBypass + _T(";");

			strBypass = strBypass + strProxy;
		}

		//x pr.Close(); //+++
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// ローカル
	{
		CIniFileI	pr( strFile, _T("LOCAL") );
		DWORD		dwLocalChk = 0;
		pr.QueryValue( dwLocalChk, _T("Enable") );

		if (dwLocalChk == TRUE)
			strBypass = strBypass + _T(";<local>");

		//x pr.Close(); //+++
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	return strBypass;
}

