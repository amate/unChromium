/**
 *	@file	ProxyDialog.h
 *	@brief	donut�̃I�v�V���� : �v���L�V
 */

#include "stdafx.h"
#include "ProxyDialog.h"
#include "../IniFile.h"
#include "../DonutPFunc.h"


#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




// Constructor
CProxyPropertyPage::CProxyPropertyPage()
{
	m_nRandTimeMin = 5;
	m_nRandTimeSec = 0;
	m_nRandChk	   = 0;
	m_nLocalChk    = 0;
	m_nUseIE	   = 1;
}



// Overrides
BOOL CProxyPropertyPage::OnSetActive()
{
	SetModified(TRUE);

	if (m_editPrx.m_hWnd == NULL) {
		m_editPrx.Attach( GetDlgItem(IDC_EDIT1) );
	}

	if (m_editNoPrx.m_hWnd == NULL) {
		m_editNoPrx.Attach( GetDlgItem(IDC_EDIT2) );
	}

	if (m_editPrx.m_hWnd && m_editNoPrx.m_hWnd) {
		//+++ ������ r13test10�̂ق����������Ǝv����
	  #if 1 //+++ ����: r13test10�ł̏���.
		_SetData();
	  #else //+++ �����Fundonut+ �ł̏���.... OnApply�ł�_GetData()�ŁA_SetData���Ă�ӏ����Ȃ�...
		_GetData();
	  #endif
	}

	return DoDataExchange(FALSE);
}



BOOL CProxyPropertyPage::OnKillActive()
{
	return DoDataExchange(TRUE);
}



BOOL CProxyPropertyPage::OnApply()
{
	if ( DoDataExchange(TRUE) ) {
		_GetData();
		return TRUE;
	} else {
		return FALSE;
	}
}



void CProxyPropertyPage::_SetData()
{
	CString 	strFile = _GetFilePath( _T("Proxy.ini") );

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// �v���L�V
	{
		CIniFileI	pr( strFile, _T("PROXY") );
		DWORD		dwLineCnt = 0;
		pr.QueryValue( dwLineCnt, _T("MAX") );
		m_editPrx.SetSelAll();

		for (int ii = 0; ii < (int) dwLineCnt; ++ii) {
			CString strKey;
			strKey.Format(_T("%d"), ii);
			CString 	strProxy = pr.GetString( strKey );		//*+++ ������GetStringUW�ɂ��邩�͌��.
			if ( strProxy.IsEmpty() )
				continue;

			m_editPrx.ReplaceSel( LPCTSTR(strProxy) );
			m_editPrx.ReplaceSel( _T("\r\n") );
		}
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// �����_��
	{
		CIniFileI	pr( strFile, _T("RAND") );

		DWORD		dwRandChk	  = 0;
		pr.QueryValue( dwRandChk, _T("Enable") );
		m_nRandChk	   = dwRandChk;

		DWORD		dwRandTimeMin = 5;
		pr.QueryValue( dwRandTimeMin, _T("Min") );
		m_nRandTimeMin = dwRandTimeMin;

		DWORD		dwRandTimeSec = 0;
		pr.QueryValue( dwRandTimeSec, _T("Sec") );
		m_nRandTimeSec = dwRandTimeSec;
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// ��v���L�V
	{
		CIniFileI		pr( strFile, _T("NOPROXY") );
		DWORD			dwLineCnt	   = 0;
		pr.QueryValue( dwLineCnt, _T("MAX") );
		m_editNoPrx.SetSelAll();

		for (int ii = 0; ii < (int) dwLineCnt; ++ii) {
			CString 	strKey;
			strKey.Format(_T("%d"), ii);

			CString strProxy = pr.GetString( strKey );		//*+++ ������GetStringUW�ɂ��邩�͌��.
			if ( strProxy.IsEmpty() )
				continue;

			m_editNoPrx.ReplaceSel( LPCTSTR(strProxy) );
			m_editNoPrx.ReplaceSel( _T("\r\n") );
		}
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// ���[�J��
	{
		CIniFileI	pr( strFile, _T("LOCAL") );
		DWORD		dwLocalChk = 0;
		pr.QueryValue( dwLocalChk, _T("Enable") );
		m_nLocalChk = dwLocalChk;
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// ���[�J��
	{
		CIniFileI	pr( strFile, _T("USE_IE") );
		DWORD		dwUseIE    = 1;
		pr.QueryValue( dwUseIE, _T("Enable") );
		m_nUseIE	= dwUseIE;
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}



void CProxyPropertyPage::_GetData()
{
	CString 	strFile;

	strFile = _GetFilePath( _T("Proxy.ini") );

  #if 1 	//+++ ����: r13test10�ɑ΂� undonut+ �ő���������...
   #if 1	//+++ �O�̂��߃o�b�N�A�b�v�t�@�C���ɂ���.
	Misc::MoveToBackupFile(strFile);
   #else
	if (GetFileAttributes(strFile) != 0xFFFFFFFF)	//Proxy.ini�����݂��Ă���΍폜
		DeleteFile(strFile);
   #endif
  #endif

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// �v���L�V
	{
		CIniFileO	pr( strFile, _T("PROXY") );
		int 		nLineCnt = m_editPrx.GetLineCount();
		pr.SetValue( (DWORD) nLineCnt, _T("MAX") );

		for (int ii = 0; ii < nLineCnt; ++ii) {
			CString strKey;
			strKey.Format(_T("%d"), ii);

			TCHAR	cBuff[MAX_PATH];
			memset(cBuff, 0, MAX_PATH);
			int 	nTextSize = m_editPrx.GetLine(ii, cBuff, MAX_PATH);

		  #if 1 //+++ ����: r13test10�ɑ΂� undonut+ �ő���������.
			if (cBuff[0] == '\0')
				break;									//�����Ȃ�������I��
		  #endif

			cBuff[nTextSize] = '\0';

			CString strBuff(cBuff);
			pr.SetString(strBuff, strKey);
		}

		//x pr.Close(); 	//+++
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// �����_��
	{
		CIniFileO	pr( strFile, _T("RAND") );
		pr.SetValue( (DWORD) m_nRandChk, _T("Enable") );
		pr.SetValue( (DWORD) m_nRandTimeMin, _T("Min") );
		pr.SetValue( (DWORD) m_nRandTimeSec, _T("Sec") );
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// ��v���L�V
	{
		CIniFileO	pr( strFile, _T("NOPROXY") );
		int 		nLineCnt = m_editNoPrx.GetLineCount();
		pr.SetValue( (DWORD) nLineCnt, _T("MAX") );

		for (int ii = 0; ii < nLineCnt; ++ii) {
			CString strKey;
			strKey.Format(_T("%d"), ii);

			TCHAR	cBuff[MAX_PATH];
			memset(cBuff, 0, MAX_PATH);
			int 	nTextSize = m_editNoPrx.GetLine(ii, cBuff, MAX_PATH);

		  #if 1 //+++ ����: r13test10�ɑ΂� undonut+ �ő���������.
			if (cBuff[0] == _T('\0'))
				break;									//�����Ȃ�������I��
		  #endif

			cBuff[nTextSize] = _T('\0');

			CString strBuff(cBuff);
		  #if 1 //+++ ����: undonut+ �ŃR�����g�A�E�g���ꂽ����.
			//strBuff += _T("\n");
		  #endif
			pr.SetString(strBuff, strKey);
		}
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// ���[�J��
	{
		CIniFileO	pr( strFile, _T("LOCAL") );
		pr.SetValue( (DWORD) m_nLocalChk, _T("Enable") );
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// IE
	{
		CIniFileO	pr( strFile, _T("USE_IE") );
		pr.SetValue( (DWORD) m_nUseIE, _T("Enable") );
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}
