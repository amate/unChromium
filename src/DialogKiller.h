/**
 *	@file	DialogKiller.h
 *	@brief	����̃_�C�A���O���o���Ȃ��悤�ɂ��鏈��.
 */
#pragma once


// for debug
#ifdef _DEBUG
	const bool _Mtl_DialogKiller_traceOn = false;
	#define dgkTRACE	if (_Mtl_DialogKiller_traceOn) ATLTRACE
#else
	#define dgkTRACE
#endif


class CDialogKiller2 {
public:

	static void KillDialog()
	{
		dgkTRACE( _T("KillDialog\n") );

		// I guess this is not so slow.
		MtlForEachTopLevelWindow( _T("#32770"), NULL, _Function_DialogFilter() );
		MtlForEachTopLevelWindow( _T("Internet Explorer_TridentDlgFrame"), NULL, _Function_TridentFrameDlgFilter() );
	}


private:
	struct _Function_DialogFilter {
		bool operator ()(HWND hWnd)
		{
			if ( MtlIsWindowCurrentProcess(hWnd) ) {
				CString 	strCaption = MtlGetWindowText(hWnd);
				if ( _Filter_ActiveXWarningDialog (hWnd, strCaption) )	return true;
				if ( _Filter_SecurityWarningDialog(hWnd, strCaption) )	return true;
				if ( _Filter_OfflineDialog(hWnd, strCaption) )			return true;
				if ( _Filter_ClientAttestationDialog(hWnd,strCaption) ) return true;
			}
			return true;
		}
	};


	struct _Function_TridentFrameDlgFilter {
		bool operator ()(HWND hWnd)
		{
			if ( MtlIsWindowCurrentProcess(hWnd) ) {
				CString strCaption = MtlGetWindowText(hWnd);
				_Filter_ScriptErrorTridentDlgFrame(hWnd, strCaption);
			}
			return true;
		}
	};


	static bool _Filter_ScriptErrorTridentDlgFrame(CWindow wnd, const CString &strCaption)
	{
		if ( (strCaption.Find( _T("�X�N���v�g") ) != -1 && strCaption.Find( _T("�G���[") ) != -1)
		   || (strCaption.Compare(_T("Internet Explorer �X�N���v�g �G���[")) == 0) )
		{
			// wnd.ModifyStyle(WS_VISIBLE, 0, SWP_NOACTIVATE);
			wnd.SendMessage(WM_CLOSE);
			// wnd.SendMessage(WM_KEYDOWN, VK_ESCAPE);
			return true;
		}

		return false;
	}


	static bool _Filter_OfflineDialog(CWindow wnd, const CString &strCaption)
	{
		if (strCaption.Find( _T("�I�t���C���ł�") ) != -1) {
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDCANCEL, 0) );
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDOK, 0) );
			// wnd.SendMessage(WM_KEYDOWN, VK_ESCAPE);
			return true;
		}

		return false;
	}


	static bool _Filter_ClientAttestationDialog(CWindow wnd, const CString &strCaption)
	{
		if (strCaption.Find( _T("�N���C�A���g�F��") ) != -1) {
			wnd.SendMessage(WM_CLOSE);
			return true;
		}

		return false;
	}


	static bool _Filter_SecurityWarningDialog(CWindow wnd, const CString &strCaption)
	{
		if ( strCaption != _T("�Z�L�����e�B���") )
			return false;

		CWindow 	wndInner = wnd.GetDlgItem(0x0000FFFF);
		if (wndInner.m_hWnd == NULL)
			return false;

		CString strOneStaticText = MtlGetWindowText(wndInner.m_hWnd);

		if (strOneStaticText.Find( _T("���̃y�[�W�ɂ�") ) != -1 && strOneStaticText.Find( _T("�ی�") ) != -1) {
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDCANCEL, 0) );
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDOK	, 0) );
			// wnd.SendMessage(WM_KEYDOWN, VK_ESCAPE);
			return true;
		}

		return false;
	}


	static bool _Filter_ActiveXWarningDialog(CWindow wnd, const CString &strCaption)
	{
		if (  strCaption != _T("Microsoft Internet Explorer")			// IE6�܂�
		   && strCaption != _T("Internet Explorer - Security Warning")	// IE7�A����
		   && strCaption != _T("Internet Explorer") 					// �X���ŕ񍐂����������
		   && strCaption != _T("Windows Internet Explorer") 			//+++ IE7�΍�. by 2ch�X��"unDonut Part16" 149��.
		   && strCaption != _T("Web �u���E�U")	//\\ IE8���ȁHflashplayer�̂������������
		   && strCaption != _T("Web �u���E�U�[") // IE9beta
		){
			return false;
		}

		CWindow 	wndInner = wnd.GetDlgItem(0x0000FFFF);
		if (wndInner.m_hWnd == NULL)
			return false;

		CString strOneStaticText = MtlGetWindowText(wndInner.m_hWnd);

		if (strOneStaticText.Find( _T("ActiveX") ) != -1 && strOneStaticText.Find( _T("���S") ) != -1) {
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDCANCEL, 0) );
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDOK, 0) );
			// wnd.SendMessage(WM_KEYDOWN, VK_ESCAPE);
			return true;
		} else if (strOneStaticText.Find( _T("ActiveX") ) != -1 && strOneStaticText.Find( _T("���s") ) != -1) {
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDCANCEL, 0) );
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDOK, 0) );
			// wnd.SendMessage(WM_KEYDOWN, VK_ESCAPE);
			return true;
		} else if (strOneStaticText.Find( _T("�J���܂���") ) != -1) {
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDCANCEL, 0) );
			wnd.SendMessage( WM_COMMAND, MAKEWPARAM(IDOK, 0) );
			// wnd.SendMessage(WM_KEYDOWN, VK_ESCAPE);
			return true;
		}

		return false;
	}
};

