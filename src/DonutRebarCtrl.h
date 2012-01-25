/**
 *	@file	DonutRebarCtrl.h
 *	@brief	ReBar(�Œ肵�Ă��Ȃ������̃c�[���o�[���Ĕz�u�����肷��)����
 */
#pragma once

#include <atlctrls.h>


//////////////////////////////////////////////////////////////////
// CReBarCtrlImpl

template <class T, class TBase = CReBarCtrl, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CReBarCtrlImpl : public CWindowImpl< T, TBase, TWinTraits >
{
public:
	// Message map
	BEGIN_MSG_MAP(CReBarCtrlImpl)
		//MESSAGE_HANDLER(WM_PAINT, OnPaint)
		//MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackGround)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()


private:
	// Overrides
	void OnRefreshBackGround() { }


	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		bHandled = FALSE;
		T *pT = static_cast<T *>(this);
		pT->OnRefreshBackGround();
		return 0;
	}

};



///////////////////////////////////////////////////////////////////////
// CDonutReBarCtrl

class CDonutReBarCtrl : public CReBarCtrlImpl<CDonutReBarCtrl>
{
	CBitmap 	m_bmpBack;
	CBitmap 	m_bmpStretch;
	int 		m_nBGStyle;

public:
	CDonutReBarCtrl() : m_nBGStyle(0)
	{
	}


	// Overrides
	void OnRefreshBackGround()
	{
		if (m_nBGStyle == SKN_REBAR_STYLE_STRETCH) {
			CRect	rc;
			GetClientRect(&rc);

			CClientDC	hDesktopDC(NULL);	//HDC 	hDesktopDC	= ::GetDC(NULL);	//+++ WTL�ȃN���X�Ɍ�Еt���C����.
			CDC 	dcSrc, dcDest;
			dcDest.CreateCompatibleDC(hDesktopDC);
			dcSrc.CreateCompatibleDC(hDesktopDC);

			if (m_bmpStretch.m_hBitmap)
				m_bmpStretch.DeleteObject();

			m_bmpStretch.CreateCompatibleBitmap( hDesktopDC, rc.Width(), rc.Height() );

			HBITMAP hOldbmpSrc	= dcSrc.SelectBitmap(m_bmpBack.m_hBitmap);
			HBITMAP hOldbmpDest = dcDest.SelectBitmap(m_bmpStretch.m_hBitmap);

			SIZE	size;
			m_bmpBack.GetSize(size);
			::StretchBlt(dcDest, 0, 0, rc.Width(), rc.Height(), dcSrc , 0, 0, size.cx, size.cy, SRCCOPY);

			dcSrc.SelectBitmap(hOldbmpSrc);
			dcDest.SelectBitmap(hOldbmpDest);

			_SetBackGround(m_bmpStretch);

			//::ReleaseDC(NULL, hDesktopDC);		//+++ �����ƁA�K�v.	//+++ WTL�ȃN���X�Ɍ�Еt���C����.

			InvalidateRect(NULL, TRUE);
		}
	}


	void RefreshSkinState()
	{
		//�w�i�`����@�̐ݒ�
		_SetDrawStyle(CSkinOption::s_nRebarBGStyle);
		CString strBmpPath = _GetSkinDir() + _T("rebar.bmp");

		//�w�i�̃r�b�g�}�b�v�ǂݍ���
		m_bmpBack.Attach( AtlLoadBitmapImage(strBmpPath.GetBuffer(0), LR_LOADFROMFILE) );

		if (m_nBGStyle == SKN_REBAR_STYLE_TILE)
			_SetBackGround(m_bmpBack);

		OnRefreshBackGround();

		//���E���̐ݒ�
		_SetBoader( !CSkinOption::s_nRebarNoBoader );

		//�ĕ`��
		InvalidateRect(NULL, TRUE);
	}


private:
	//Function
	void _SetDrawStyle(int nStyle)
	{
		m_nBGStyle = nStyle;

	  #if 1	//+++ uxtheme.dll �̊֐��̌Ăяo������ύX.
		if (nStyle == SKN_REBAR_STYLE_THEME)
			UxTheme_Wrap::SetWindowTheme(m_hWnd, NULL, L"REBAR");
		else
			UxTheme_Wrap::SetWindowTheme(m_hWnd, L" ", L" ");
	  #else
		CTheme	theme;
		if (nStyle == SKN_REBAR_STYLE_THEME)
			theme.SetWindowTheme(m_hWnd, NULL, L"REBAR");
		else
			theme.SetWindowTheme(m_hWnd, L" ", L" ");
	  #endif
	}


	void _SetBackGround(HBITMAP hBitmap)
	{
		REBARBANDINFO rbbi	 = { sizeof (REBARBANDINFO) };

		rbbi.fMask	 = RBBIM_BACKGROUND;
		rbbi.hbmBack = hBitmap;

		int 		  nCount = GetBandCount();

		for (int i = 0; i < nCount; i++)
			SetBandInfo(i, &rbbi);
	}


	void _SetBoader(BOOL bShow)
	{
		int nStyle = GetWindowLong(GWL_STYLE);

		if (bShow)
			nStyle |= RBS_BANDBORDERS;
		else
			nStyle &= ~RBS_BANDBORDERS;

		SetWindowLong(GWL_STYLE, nStyle);
	}

};



/////////////////////////////////////////////////////////////////////////////////////
// CTransparentToolBarCtrl

class CTransparentToolBarCtrl : public CWindowImpl<CTransparentToolBarCtrl, CToolBarCtrl> 
{
public:
	// Message map
	BEGIN_MSG_MAP(CTransparentToolBarCtrl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackGround)
	END_MSG_MAP()

private:
	LRESULT OnEraseBackGround(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		HWND	hWnd	= GetParent();
		CPoint	pt;

		MapWindowPoints(hWnd, &pt, 1);
		::OffsetWindowOrgEx( (HDC) wParam, pt.x, pt.y, NULL );
		LRESULT lResult = ::SendMessage(hWnd, WM_ERASEBKGND, wParam, 0L);
		::SetWindowOrgEx( (HDC) wParam, 0, 0, NULL );
		return lResult;
	}
};



//////////////////////////////////////////////////////////////////////
// CDonutExplorerReBarCtrl

class CDonutExplorerReBarCtrl : public CDonutReBarCtrl 
{
public:
	CDonutExplorerReBarCtrl& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	// Message map
	BEGIN_MSG_MAP(CDonutExplorerReBarCtrl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackGround)
	END_MSG_MAP()


private:
	LRESULT OnEraseBackGround(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		HWND	hWnd	= GetParent();
		CPoint	pt;

		MapWindowPoints(hWnd, &pt, 1);
		::OffsetWindowOrgEx( (HDC) wParam, pt.x, pt.y, NULL );
		LRESULT lResult = ::SendMessage(hWnd, WM_ERASEBKGND, wParam, 0L);
		::SetWindowOrgEx( (HDC) wParam, 0, 0, NULL );
		return lResult;
	}
};



