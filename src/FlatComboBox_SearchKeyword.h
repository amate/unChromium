/**
 *	@file	FlatComboBox_SearchKeyword.h
 *	@brief	�t���b�g�E�R���{�E�{�b�N�X. �����L�[���[�h����̏ꍇ�ɁA�����G���W�������o�����߂̊g������������.
 */
#pragma once


// The Web Browser style flat combo box control (light-flat combo box)
//
// Note. I can't find the way to avoid the thumb flicker while resizing.
//
#include "MtlMisc.h"
#include "MtlWin.h"
#include "option/SkinOption.h"
#include "DonutPFunc.h"


typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL | CBS_HASSTRINGS, 0>
CComboBoxSearchKeywordTraits; 

///////////////////////////////////////////////////////////////////
// CFlatComboBox_SearchKeyword

class CFlatComboBox_SearchKeyword
	: public CWindowImpl<CFlatComboBox_SearchKeyword, CComboBox, CComboBoxSearchKeywordTraits>
	, public CTrackMouseLeave<CFlatComboBox_SearchKeyword>
{
public:
	DECLARE_WND_SUPERCLASS(_T("FlatComboBox_SearchKeyword"), CComboBox::GetWndClassName())

private:
	// Constants
	enum			 { s_kcxyBorder = 2 };
	enum _ComboState { _comboNormal, _comboHot, _comboPressed };
	enum			 { TIMER_ID = 1 };	//+++

	// Data members
	CString			m_strEmpty;			//+++

  #ifndef NO_FLATCOMBOBOX
	_ComboState 	m_state;
	int 			m_nDrawStyle;
  #endif
	int				m_nEditId;			//+++
	BYTE			m_bFocus;			//+++
	BYTE			m_bTitSw;			//+++

	BYTE/*BOOL*/	m_bExistManifest;

public:
	// Ctor
	CFlatComboBox_SearchKeyword()
		: m_nEditId(0)		//+++
		, m_bFocus(0)		//+++
		, m_bTitSw(0)		//+++
		, m_state(_comboNormal)
		, m_nDrawStyle(SKN_COMBO_STYLE_DEFAULT)
	{
		m_bExistManifest = IsExistManifestFile();
	}


	bool FlatComboBox_Install(HWND hWndCombo)
	{
		MTLVERIFY( SubclassWindow(hWndCombo) );
		return true;
	}


	void FlatComboBox_Uninstall()
	{
		if ( IsWindow() )
			UnsubclassWindow();
	}


	//+++ �L�[���[�h����̎��ɕ\�����镶�����ݒ�.
	void setEmptyStr(const CString& s, int nEditId, BOOL sw) {
		bool chg = (sw != m_bTitSw || m_nEditId != nEditId || m_strEmpty != s);
		if (chg) {
			m_strEmpty = s;
			m_nEditId  = nEditId;
			m_bTitSw   = sw;
			paint_EmptyStr();
		}
	}


	//+++
	void redrawEmptyStr()
	{
		if (IsFocus()) {
			m_bFocus = 1;
		} else {
			if (m_bFocus) {
				paint_EmptyStr();
			}
			m_bFocus = 0;
		}
	}


public:
	// Message map and handlers
	BEGIN_MSG_MAP(CFlatComboBox_SearchKeyword)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_SIZE , OnSize )
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		CHAIN_MSG_MAP(CTrackMouseLeave<CFlatComboBox_SearchKeyword>)
	END_MSG_MAP()


//\\private:

	void paint_EmptyStr()
	{
		if (m_hWnd) {
		  #if 1
			InvalidateRect(NULL, TRUE);
		  #else
			//SendMessage(WM_PAINT, 0, 0);
			//DrawEmptyStr();
		  #endif
		}
	}

private: //\\ �_���ȕ��@��������Ȃ�����
	//+++
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		LRESULT   lRet	   = DefWindowProc();
		paint_EmptyStr();
		return lRet;
	}


	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
	  #ifndef NO_FLATCOMBOBOX
		if (   m_nDrawStyle == SKN_COMBO_STYLE_DEFAULT
		   || (m_nDrawStyle == SKN_COMBO_STYLE_THEME && !m_bExistManifest) )
		{
		  #if 0
			CRect	  rc;
			GetUpdateRect(&rc, FALSE);

			ValidateRect(NULL); // validate the whole

			// invaldate only the internal rect
			CRect	  rcCombo  = _GetFlatComboInvalidateRect();
			CRect	  rcThumb  = _GetFlatThumbInvalidateRect();
			CRect	  rcUpdate = rc & rcThumb;
			InvalidateRect(&rcUpdate, TRUE);
			//rcUpdate = rc & rcCombo;
			//InvalidateRect(&rcUpdate, TRUE);

			// update!
			LRESULT   lRet	   = DefWindowProc();
			{
				CClientDC dc(m_hWnd);
				_DoComboPaint(dc.m_hDC);
			}
			DrawEmptyStr();	//+++
			return lRet;
		  #else
			CRect	  rc;
			GetUpdateRect(&rc, FALSE);

			ValidateRect(NULL); // validate the whole

			// invaldate only the internal rect
			CRect	  rcCombo  = _GetFlatComboInvalidateRect();
			CRect	  rcThumb  = _GetFlatThumbInvalidateRect();
			CRect	  rcUpdate = rc & rcThumb;
			InvalidateRect(&rcUpdate, FALSE);
			rcUpdate = rc & rcCombo;

			int ww = DrawEmptyStr();	// �\�Ȃ�A���͗���w�i�F�œh���āA���̏�ɃG���W������\��.
		   #if 1
			if (ww > 0) {	//+++
				//+++ ���͗���w�i�F�œh���āA���̏�ɃG���W������\�������ꍇ.
				//rcUpdate.left += ww;
			} else {	//+++ �G���W�����\�������Ă��Ȃ��ꍇ�́A�{���̏����ŁA���͗����ĕ`��.
						//+++ (OnPaint�𔲂������EDIT���ɂ���ē��͗����ĕ`�悳���)
				InvalidateRect(&rcUpdate, FALSE);
			}
		   #endif

			// update!
			LRESULT   lRet	   = DefWindowProc();
			{
				CClientDC dc(m_hWnd);
				_DoComboPaint(dc.m_hDC);	//+++ ����:�{�^������͗��̍ĕ`��...
			}
			//DrawEmptyStr();	//+++	���̃^�C�~���O�ŕ`�悵�������A���ۂɂ�OnPaint�𔲂������EDIT���ł̍ĕ`�悪�������ď㏑������Ă��܂��B
			return lRet;
		  #endif
		}
		else if (m_nDrawStyle == SKN_COMBO_STYLE_THEME && m_bExistManifest)	//+++ XP�X�^�C���̎��̃G���W�����\��...
	  #endif
		{
		  #if 1	//+++ �g�ɏ㏑�������̂ŁA�g�`���ɂȂ肻���ȃ^�C�~���O�ɖ������`��
			SetTimer(TIMER_ID,100 /*,NULL*/);	//+++ 0.1�b��ɃG���W�����`��... ���̊Ԃ�xp�X�^�C���̃R���{�{�b�N�X���`�悳��Ă���Ɗ���.
			bHandled = FALSE;
			return 0;
		  #else	// ���s
			LRESULT   lRet	   = DefWindowProc();
			DrawEmptyStr();	// �\�Ȃ�A���͗���w�i�F�œh���āA���̏�ɃG���W������\��.
			//bHandled = FALSE;
			return lRet;
		  #endif
		}
	  #ifndef NO_FLATCOMBOBOX
		else {
			bHandled = FALSE;
			return 0;
		}
	  #endif
	}

  #if 1	//+++ XP�X�^�C���̎��̃G���W�����\���𖳗����x�����čs�����߂̏���.
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if (wParam == TIMER_ID) {
			DrawEmptyStr();	//+++ �\�Ȃ�A���͗���w�i�F�œh���āA���̏�ɃG���W������\��.
			KillTimer(TIMER_ID);
			//bHandled = FALSE;
		}
		return 1;
	}
  #endif


  #ifndef NO_FLATCOMBOBOX
	void _DoComboPaint(CDCHandle dc)
	{
		_DrawFlatCombo(dc, m_state);
		_DrawFlatThumb(dc);
	}


	///+++ �R���{�{�b�N�X�̃{�^���̕`��
	void _DrawFlatThumb(CDCHandle &dc)
	{
		CRect	 rc 		= _GetOriginalThumbRect();

		dc.DrawEdge(rc, BDR_RAISEDINNER, BF_RECT);

		rc.DeflateRect(1, 1);
		COLORREF clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
		dc.Draw3dRect(rc, clrBtnFace, clrBtnFace);
	}



	CRect _GetFlatThumbInvalidateRect()
	{
		CRect rc = _GetOriginalThumbRect();

		rc.DeflateRect(2, 2);
		return rc;
	}


	CRect _GetFlatComboInvalidateRect()
	{
		CRect rc;

		GetClientRect(&rc);
		rc.DeflateRect(2, 2);
		rc.right -= ::GetSystemMetrics(SM_CXHTHUMB);
		return rc;
	}


	///+++ �L�[���[�h���͗��̕`��.
	void _DrawFlatCombo(CDCHandle &dc, _ComboState state)
	{
		COLORREF clrBtnFace = ::GetSysColor(COLOR_BTNFACE);

		CRect	 rc;
		GetClientRect(&rc);

		dc.Draw3dRect(rc, clrBtnFace, clrBtnFace);
		rc.DeflateRect(1, 1);

		switch (state) {
		case _comboNormal:
			dc.DrawEdge(rc, BDR_SUNKENOUTER, BF_RECT);
			break;

		case _comboHot:
			dc.DrawEdge(rc, BDR_RAISEDINNER, BF_RECT);
			break;

		case _comboPressed:
			break;
		}
	}
  #endif

	CRect _GetOriginalThumbRect()
	{
		CRect rc;

		GetClientRect(&rc);
		rc.right  -= s_kcxyBorder;
		rc.left    = rc.right - ::GetSystemMetrics(SM_CXHTHUMB);
		rc.top	  += s_kcxyBorder;
		rc.bottom -= s_kcxyBorder;
		return rc;
	}


	//+++ �L�[���[�h����̎��A��p�̕�����(�G���W����)��\������. (�w�i�F�����͑��͈͂�h��)
	int	DrawEmptyStr()
	{
		if (m_bTitSw && m_nEditId > 0 && m_strEmpty.IsEmpty() == 0) {
			HWND hWndEdit = GetDlgItem(m_nEditId);
			if (hWndEdit && ::GetWindowTextLength(hWndEdit) == 0 && hWndEdit != ::GetFocus()) {
				CRect		   rc;
				GetClientRect(&rc);
				CRect			rc2 = _GetOriginalThumbRect();
				rc.right -= rc2.Width();
			  #if 1
				rc.left   += 2;
				rc.right  -= 0;
				rc.top    += 2;
				rc.bottom -= 1;
			  #else
				rc.DeflateRect(2,2);
			  #endif
				int w = rc.right - rc.left;
				if (w > 0) {
					DWORD c   = GetSysColor(COLOR_WINDOWTEXT);
					DWORD k   = GetSysColor(COLOR_WINDOW);		//RGB(0xFF,0xFF,0xFF);
					DWORD col = GetSysColor(COLOR_GRAYTEXT);	// RGB(0x77,0x77,0x77);
				  #if 0
					{
						CClientDC dc0(hWndEdit);
						c = dc0.GetTextColor();
						k = dc0.GetBkColor();
					}
					if (c != k) {
						int r   = (BYTE(c >> 16) + BYTE(k >> 16)) >> 1;
						int g   = (BYTE(c >>  8) + BYTE(k >>  8)) >> 1;
						int b   = (BYTE(c >>  0) + BYTE(k >>  0)) >> 1;
						col     = RGB(r,g,b);
					}
				  #endif

					CClientDC dc(m_hWnd);
					// CFont	font;font.CreatePointFont(100, _T("Arial Black"));
					HFONT hFont    = (HFONT)::SendMessage(hWndEdit, WM_GETFONT, 0, 0);
					CFontHandle font( hFont );
					HFONT hOldFont = dc.SelectFont(font);

					dc.SetBkColor(k);
					dc.SetTextColor(col);

					//dc.SetBkMode(TRANSPARENT);
					dc.ExtTextOut(3,3,ETO_CLIPPED|ETO_OPAQUE,&rc,m_strEmpty);
					dc.SelectFont(hOldFont);
				} else {
					w = 0;
				}
				return w;
			}
		}
		return 0;
	}


	//+++ ���͗��Ƀt�H�[�J�X���Ă��邩�ۂ���Ԃ� (��win2k���Ƃ��܂������Ă��Ȃ�?)
	bool IsFocus()
	{
		if (m_hWnd && m_nEditId)
			return (::GetFocus() == GetDlgItem(m_nEditId));
		return false;
	}


	void _SetUseXPTheme(HWND hWnd, BOOL bUse)
	{
		HRESULT   (WINAPI * __SetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList) = NULL;

		HINSTANCE hInst = ::LoadLibrary( _T("UxTheme.dll") );

		if (hInst != NULL) {
			__SetWindowTheme = ( HRESULT (WINAPI *)(HWND, LPCWSTR, LPCWSTR) ) ::GetProcAddress( hInst, "SetWindowTheme" );

			if (__SetWindowTheme != NULL) {
				if (bUse)
					(__SetWindowTheme) ( hWnd, L" ", L"COMBOBOX");
				else
					(__SetWindowTheme) ( hWnd, (L" "), (L" ") );
			}

			::FreeLibrary( hInst );
		}
	}


public:
	void SetDrawStyle(int nStyle)
	{
	  #ifndef NO_FLATCOMBOBOX
		m_nDrawStyle = nStyle;
	  #endif
		m_bExistManifest = IsExistManifestFile();		//+++
		if (nStyle == SKN_COMBO_STYLE_THEME && m_bExistManifest) {
			_SetUseXPTheme(m_hWnd, TRUE);
		} else {
			_SetUseXPTheme(m_hWnd, FALSE);
		}
	}
};
