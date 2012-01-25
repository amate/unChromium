/**
 *	@file	DialogHook.h
 *	@brief	�_�C�A���O�E�t�b�N
 */

#pragma once


#define PROMPT_CAPTION					_T("Explorer ���[�U�[ �v�����v�g")
#define STRING_PROMPT_TOOLTIP			_T( "���̃E�B���h�E���ǂ̃^�u�ɂ���ĊJ���ꂽ���̂�unDonut�͔c�����Ă���܂���B\n�ꍇ�ɂ���Ă̓A�N�e�B�u�ł͂Ȃ��^�u�ɊJ���ꂽ�A�����ȃT�C�g���J�����U���E�B���h�E�ł���\n�\��������܂��BOK�{�^���������ď��𑗐M����ۂ͂����ӂ��������B")
#define ACTIVEXDIALOG_WINDOWTEXT		_T("Microsoft Internet Explorer")
#define ACTIVEXDIALOG_WINDOWTEXT2		_T("Windows Internet Explorer")
#define ACTIVEXDIALOG_MESSAGE_PARTIAL	_T("���݂̃Z�L�����e�B�̐ݒ�ł́A���̃y�[�W��")
#define ACTIVEXDIALOG_MESSAGE_REASONS	_T("���̂����ꂩ�̗��R�ɂ�� 1 �܂��͂���ȏ�� ActiveX") 	//+++ 2ch�X��"unDonut Part16" 149 �𔽉f.



class CDialogHook {
private:
	static HHOOK	s_hHook;
	static HWND 	s_hWndMainFrame;


	enum {
		cnt_offsetX 	= 4,
		cnt_offsetY 	= 4
	};


public:
	static BOOL InstallHook(HWND hWndMainFrame)
	{
		if (s_hHook)
			return FALSE;

		s_hWndMainFrame 	= hWndMainFrame;
		DWORD	dwThreadID	= ::GetCurrentThreadId();
		s_hHook 			= ::SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, NULL, dwThreadID);
		return s_hHook != 0 /*? TRUE : FALSE*/;
	}


	static BOOL UninstallHook()
	{
		if (!s_hHook)
			return FALSE;

		BOOL bRet = ::UnhookWindowsHookEx(s_hHook) != 0;
		if (bRet)
			s_hHook = NULL;
		return bRet;
	}


private:
	static LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode < 0)
			return ::CallNextHookEx(s_hHook, nCode, wParam, lParam);

		CWPSTRUCT *pcwp = (CWPSTRUCT *) lParam;

		if (pcwp->message == WM_INITDIALOG) {
			//�_�C�A���O��������ߑ�
			LRESULT lRet	   = ::CallNextHookEx(s_hHook, nCode, wParam, lParam); //���������s

			//�_�C�A���O�̃E�B���h�E�n���h�����擾
			HWND	hWndFocus  = (HWND) pcwp->wParam;
			if ( !::IsWindow(hWndFocus) )
				return lRet;

			HWND	hDialog    = ::GetParent(hWndFocus);
			if ( !::IsWindow(hDialog) )
				return lRet;

			//�_�C�A���O�̐e�E�B���h�E���ŏ�ʃE�B���h�E�ł��邱�Ƃ��m�F
			if (::GetParent(hDialog) != s_hWndMainFrame)
				return lRet;

			//�L���v�V�������`�F�b�N
			CString strCaption = MtlGetWindowText(hDialog);

			if (strCaption == PROMPT_CAPTION) {
				//prompt�ɂ���Đ������ꂽ�_�C�A���O�Ɣ���
				PromptWindowProc(hDialog);
			} else if (strCaption == ACTIVEXDIALOG_WINDOWTEXT
					|| strCaption == ACTIVEXDIALOG_WINDOWTEXT2		//+++ for IE7.
			) {
				//IE�̃��b�Z�[�W�{�b�N�X
				HWND hWndStatic = ::GetDlgItem(hDialog, 0x0000FFFF);

				if ( ::IsWindow(hWndStatic) ) {
					CString strStaticText = MtlGetWindowText(hWndStatic);

					if (   strStaticText.Find(ACTIVEXDIALOG_MESSAGE_PARTIAL) != -1
						|| strStaticText.Find(ACTIVEXDIALOG_MESSAGE_REASONS) != -1		//+++ 2ch�X��"unDonut Part16" 149 �𔽉f.
					) {
						if ( _check_flag(MAIN_EX_KILLDIALOG, CMainOption::s_dwMainExtendedStyle) ) {
							//::SendMessage(hDialog,WM_CLOSE,0,0);
							::DestroyWindow(hDialog);
						}
					}
				}
			}

			return lRet;
		}

		return ::CallNextHookEx(s_hHook, nCode, wParam, lParam);
	}


	static BOOL PromptWindowProc(HWND hWnd)
	{
		HWND	 hOkButton = GetDlgItem(hWnd, 1);	//OK�{�^����ID��1�Ɖ��肵�ăn���h�����擾
		if (!hOkButton)
			return FALSE;

		HWND	 hToolTip  = ::CreateWindow(TOOLTIPS_CLASS,
											NULL,
											TTS_ALWAYSTIP,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											hWnd,
											NULL,
											_Module.GetModuleInstance(),
											NULL);
		if (!hToolTip)
			return FALSE;

		TOOLINFO	ti	= { sizeof (TOOLINFO) };
		ti.uFlags		= TTF_SUBCLASS | TTF_IDISHWND;
		ti.hwnd 		= hOkButton;
		ti.uId			= (UINT_PTR) hOkButton;

		RECT	 rc;
		::GetWindowRect(hOkButton, &rc);
		ti.hinst		= _Module.GetModuleInstance();
		ti.lpszText 	= STRING_PROMPT_TOOLTIP;

		//�c�[���`�b�v�̐ݒ�
		::SendMessage(hToolTip , TTM_ADDTOOL	   , 0, (LPARAM) &ti);						//�c�[���`�b�v�̒ǉ�
		::SendMessage(hToolTip , TTM_SETMAXTIPWIDTH, 0, (LPARAM) 500);						//�e�L�X�g�����s����悤�ɂ���
		::SendMessage( hToolTip, TTM_SETDELAYTIME  , TTDT_INITIAL, MAKELONG(1	 , 0) );	//�\���x�����ق�0��
		::SendMessage( hToolTip, TTM_SETDELAYTIME  , TTDT_AUTOPOP, MAKELONG(30000, 0) );	//�\�����Ԃ�30�b��

		ti.hwnd 	= hWnd;
		ti.uId		= (UINT_PTR) hWnd;
		::SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM) &ti);

		return TRUE;
	}
};


__declspec(selectany) HHOOK CDialogHook::s_hHook			= NULL;
__declspec(selectany) HWND	CDialogHook::s_hWndMainFrame	= NULL;



#if 0	//+++	���łɖ��g�p�̂悤.

class CSearchBoxHook {
	static HINSTANCE	s_hInstDLL;

	typedef BOOL (WINAPI   * LPFINSTALLHOOK)(HWND);
	typedef BOOL (WINAPI   * LPFUNINSTALLHOOK)();


public:
	static BOOL InstallSearchHook(HWND hWndMainFrame)
	{
		LPFINSTALLHOOK		fpInstallHook;
		LPFUNINSTALLHOOK	fpUninstallHook;

		if (s_hInstDLL)
			return FALSE;

		s_hInstDLL		= ::LoadLibrary( _T("SearchHook.dll") );
		if (!s_hInstDLL)
			return FALSE;

		//�����ŁA�t�b�N����O�Ƀt�b�N�����֐������݂��邩�m�F����i�������������肷��ƔߎS�j
		fpUninstallHook = (LPFUNINSTALLHOOK) ::GetProcAddress( s_hInstDLL, _T("UninstallHook") );
		if (!fpUninstallHook)
			goto CLEAR;

		fpInstallHook	= (LPFINSTALLHOOK) ::GetProcAddress( s_hInstDLL, _T("InstallHook") );
		if (!fpInstallHook)
			goto CLEAR;

		if ( !fpInstallHook(hWndMainFrame) )
			goto CLEAR;

		return TRUE;

	  CLEAR:
		if (s_hInstDLL)
			::FreeLibrary(s_hInstDLL);

		s_hInstDLL		= NULL;
		return FALSE;
	}


	static BOOL UninstallSearchHook()
	{
		if (!s_hInstDLL)
			return FALSE;

		LPFUNINSTALLHOOK fpUninstallHook = (LPFUNINSTALLHOOK) ::GetProcAddress( s_hInstDLL, _T("UninstallHook") );

		if (!fpUninstallHook)
			return FALSE;

		//�����Ŗ����Ƃ������Ă���������
		BOOL	bRet	= fpUninstallHook();
		if (!bRet)
			return FALSE;

		//��������̂���������
		::FreeLibrary(s_hInstDLL);
		s_hInstDLL = NULL;
		return TRUE;
	}
};


__declspec(selectany) HINSTANCE CSearchBoxHook::s_hInstDLL = NULL;


#endif
