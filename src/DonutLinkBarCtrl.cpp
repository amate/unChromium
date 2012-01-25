/**
 *	@file	DonutLinkBarCtrl.cpp
 *	@brief	リンクバー
 */

#include "stdafx.h"
#include "DonutLinkBarCtrl.h"
#include <codecvt>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "MtlMisc.h"
#include "MtlWin.h"
#include "MtlDragDrop.h"
#include "HlinkDataObject.h"
#include "Donut.h"

using boost::property_tree::wptree;
using namespace boost::property_tree::xml_parser;

	
struct LinkItem
{
	CString strName;
	CString strUrl;
	vector<unique_ptr<LinkItem> >*	pFolder;

	CRect	rcItem;
	enum LinkItemState { kItemNormal, kItemHot, kItemPressed, };
	DWORD	state;

	// Attributes
	bool ModifyState(DWORD newState)
	{
		DWORD stateOld = state;
		state = newState;
		return state != stateOld;
	}

	LinkItem() : pFolder(nullptr), state(kItemNormal)
	{}
};
typedef vector<unique_ptr<LinkItem> >	LinkFolder;
typedef vector<unique_ptr<LinkItem> >*	LinkFolderPtr;

HHOOK	g_Hhook = NULL;
HWND	g_hWndLinkBar = NULL;

#define WM_CLOSEBASESUBMENU		(WM_APP + 1)

/// メニュー以外の場所をクリックしたときにメニューを閉じる
LRESULT CALLBACK LowLevelMouseProc(
  int nCode,     // フックコード
  WPARAM wParam, // メッセージ識別子
  LPARAM lParam  // メッセージデータ
)
{
	if (nCode < 0)
		return CallNextHookEx(g_Hhook, nCode, wParam, lParam);
	if (nCode == HC_ACTION) {
		switch (wParam) {
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN: {
			LPMSLLHOOKSTRUCT pllms = (LPMSLLHOOKSTRUCT)lParam;
			HWND hWnd = WindowFromPoint(pllms->pt);
			if (hWnd) {
				CString	strClassName;
				GetClassName(hWnd, strClassName.GetBuffer(24), 24);
				strClassName.ReleaseBuffer();
				if (strClassName != _T("DonutMenuWindow")) {
					if (g_hWndLinkBar)
						::SendMessage(g_hWndLinkBar, WM_CLOSEBASESUBMENU, 0, 0);
				}
			}
							 }
			break;

		default:
			break;
		}
	}
	return CallNextHookEx(g_Hhook, nCode, wParam, lParam);
}


class CMenuWindow : 
	public CDoubleBufferWindowImpl<CMenuWindow>,
	public CThemeImpl<CMenuWindow>,
	public CTrackMouseLeave<CMenuWindow>,
	public CMessageFilter
{
public:
	DECLARE_WND_CLASS_EX(_T("DonutMenuWindow"), CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW, COLOR_MENU)

	// Constants
	enum {
		kBoundMargin = 2,
		kBoundBorder = 1,
		kItemHeight = 22,
		kIconWidth	= 28,
		kVerticalLineWidth = 2,
		kTextMargin	= 4,
		kLeftTextPos = kBoundMargin + kIconWidth + kVerticalLineWidth + kTextMargin,
		kArrowWidth	= 25,
		kNoneTextWidth = 100,
		kBiteWidth	= 3,

		kSubMenuPopupCloseTimerID = 1,
		kSubMenuPopupCloseTime	= 500,
		kSubMenuPopupTimerID = 2,
		kSubMenuPopupTime	= 300,
	};

	// Constructor
	CMenuWindow(LinkFolderPtr pFolder, int nInheritIndex = -1) : 
		m_pFolder(pFolder), 
		m_nHotIndex(-1),
		m_pMenuWindow(nullptr),
		m_nInheritFolderIndex(nInheritIndex)
	{
		WTL::CLogFont	lf;
		lf.SetMenuFont();
		m_font = lf.CreateFontIndirect();
	}

	int	ComputeWindowWidth()
	{
		int nMaxTextWidth = 0;
		for (auto it = m_pFolder->cbegin(); it != m_pFolder->cend(); ++it) {
			int nNameWidth = MTL::MtlComputeWidthOfText(it->get()->strName, m_font);
			if (nMaxTextWidth < nNameWidth)
				nMaxTextWidth = nNameWidth;
		}
		if (nMaxTextWidth == 0)
			nMaxTextWidth = kNoneTextWidth;
		nMaxTextWidth += kLeftTextPos + kTextMargin + kArrowWidth + (kBoundMargin * 2);
		return nMaxTextWidth;
	}
	int ComputeWindowHeight()
	{
		return (kItemHeight * (int)m_pFolder->size())
			+ (kBoundMargin * 2) + (kBoundBorder * 2);
	}

	LinkFolderPtr	GetLinkFolderPtr() const { return m_pFolder; }

	// Overrides
	virtual void OnFinalMessage(HWND /*hWnd*/) { delete this; }
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		/*if (pMsg->hwnd == m_hWnd && pMsg->message == WM_LBUTTONDOWN) {
			POINT pt = { GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam) };
			OnLButtonDown((UINT)pMsg->wParam, pt);
			return TRUE;
		}*/
		return FALSE;
	}

	void DoPaint(CDCHandle dc)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		HFONT hFontPrev = dc.SelectFont(m_font);
		dc.SetBkMode(TRANSPARENT);

		if (IsThemeNull() == false) {
			DrawThemeBackground(dc, MENU_POPUPBACKGROUND, 0, &rcClient);
			CRect rcGutter;
			rcGutter.left = kBoundMargin + kIconWidth;
			rcGutter.right= rcGutter.left + kVerticalLineWidth;
			rcGutter.bottom= rcClient.bottom;
			DrawThemeBackground(dc, MENU_POPUPGUTTER, 0, &rcGutter);
		}
		for (auto it = m_pFolder->cbegin(); it != m_pFolder->cend(); ++it) {
			LinkItem& item = *it->get();
			if (IsThemeNull() == false) {
				int iState = MPI_NORMAL;
				if (item.state == item.kItemHot)
					iState = MPI_HOT;
				DrawThemeBackground(dc, MENU_POPUPITEM, iState, &item.rcItem);

				CRect	rcText = item.rcItem;
				rcText.left	= kLeftTextPos;
				rcText.right	-= kArrowWidth + kBoundMargin;
				DrawThemeText(dc, MENU_POPUPITEM, MPI_NORMAL, item.strName, item.strName.GetLength(), DT_SINGLELINE | DT_VCENTER, 0, &rcText);
				if (item.pFolder) {
					CRect rcArrow = rcText;
					rcArrow.left	= rcText.right;
					rcArrow.right	= rcClient.right;
					DrawThemeBackground(dc, MENU_POPUPSUBMENU, MSM_NORMAL, &rcArrow);
				}
			}
		}
		dc.SelectFont(hFontPrev);
	}

	/// カーソル上のアイテムをHOT状態にする
	void OnTrackMouseMove(UINT nFlags, CPoint pt)
	{

		if (auto value = _HitTest(pt)) {
			if (value.get() != m_nHotIndex) {
				_HotItem(value.get());
				SetTimer(kSubMenuPopupTimerID, kSubMenuPopupTime);
			}
		}
	}

	/// アイテムのHOT状態を解除する
	void OnTrackMouseLeave()
	{
		_HotItem(-1);
		KillTimer(kSubMenuPopupTimerID);
		if (m_pMenuWindow) {	// カーソルが出て行ったときにサブメニューを閉じる
			CPoint	pt;
			GetCursorPos(&pt);
			HWND hWnd = WindowFromPoint(pt);
			if (hWnd != m_pMenuWindow->m_hWnd)
				SetTimer(kSubMenuPopupCloseTimerID, kSubMenuPopupCloseTime);
		}
	}

	BEGIN_MSG_MAP( CMenuWindow )
		MSG_WM_CREATE( OnCreate )
		MSG_WM_DESTROY( OnDestroy )
		MSG_WM_TIMER( OnTimer )
		MSG_WM_MOUSEACTIVATE( OnMouseActivate )
		MSG_WM_KEYDOWN( OnKeyDown )
		MSG_WM_LBUTTONDOWN( OnLButtonDown )
		CHAIN_MSG_MAP( CDoubleBufferWindowImpl<CMenuWindow> )
		CHAIN_MSG_MAP( CThemeImpl<CMenuWindow> )
		CHAIN_MSG_MAP( CTrackMouseLeave<CMenuWindow> )
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		OpenThemeData(VSCLASS_MENU);

		CRect rcClient;
		GetClientRect(&rcClient);
		int nTop = kBoundMargin;
		for (auto it = m_pFolder->begin(); it != m_pFolder->end(); ++it) {
			LinkItem& item = *it->get();
			item.rcItem.top	= nTop;
			item.rcItem.left	= kBoundMargin;
			item.rcItem.right	= rcClient.right - kBoundMargin;
			item.rcItem.bottom	= nTop + kItemHeight;
			nTop	+= kItemHeight;
		}
		if (m_nInheritFolderIndex == -1 && g_Hhook == NULL) {
			g_Hhook = ::SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, _Module.GetModuleInstance(), 0);
			ATLASSERT(g_Hhook);
		}

		CMessageLoop *pLoop = _Module.GetMessageLoop();
		pLoop->AddMessageFilter(this);

		return 0;
	}

	void OnDestroy()
	{
		if (m_nInheritFolderIndex == -1 && g_Hhook) {
			::UnhookWindowsHookEx(g_Hhook);
			g_Hhook = NULL;
		}
		std::for_each(m_pFolder->begin(), m_pFolder->end(), [this](unique_ptr<LinkItem>& pItem) {
			pItem->state = pItem->kItemNormal;
		});

		CMessageLoop *pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);
	}

	void OnTimer(UINT_PTR nIDEvent)
	{
		if (nIDEvent == kSubMenuPopupCloseTimerID) {
			KillTimer(nIDEvent);
			if (m_pMenuWindow) {
				m_pMenuWindow->DestroyWindow();
				m_pMenuWindow = nullptr;
			}
		} else if (nIDEvent == kSubMenuPopupTimerID) {
			KillTimer(nIDEvent);
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			_DoExec(pt);
		}
	}

	/// クリックでアクティブにならないようにする
	int OnMouseActivate(CWindow wndTopLevel, UINT nHitTest, UINT message)
	{
		return MA_NOACTIVATE;
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (nChar == VK_ESCAPE) {
			TRACEIN(_T("OnKeyDown : VK_ESCAPE"));
		}
	}

	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		_DoExec(point, true);
	}

private:
	bool	_IsValidIndex(int nIndex) const { 
		return 0 <= nIndex && nIndex < (int)m_pFolder->size();
	}

	boost::optional<int>	_HitTest(const CPoint& point)
	{
		int nCount = (int)m_pFolder->size();
		for (int i = 0; i < nCount; ++i) {
			if (m_pFolder->at(i)->rcItem.PtInRect(point))
				return i;
		}
		return boost::none;
	}

	void	_HotItem(int nNewHotIndex)
	{
		// clean up
		if ( _IsValidIndex(m_nHotIndex) ) {
			LinkItem& item = *m_pFolder->at(m_nHotIndex);
			item.state	= item.kItemNormal;
			//if ( item.ModifyState(item.kItemHot, 0) )
				InvalidateRect(item.rcItem);
		}

		m_nHotIndex = nNewHotIndex;

		if ( _IsValidIndex(m_nHotIndex) ) {
			LinkItem& item = *m_pFolder->at(m_nHotIndex);
			item.state	= item.kItemHot;
			//if ( item.ModifyState(0, item.kItemHot) )
				InvalidateRect(item.rcItem);
		}
	}

	void	_DoExec(const CPoint& pt, bool bLButtonDown = false)
	{
		if (auto value = _HitTest(pt)) {
			LinkItem& item = *m_pFolder->at(value.get());
			if (m_pMenuWindow) {
				if (m_pMenuWindow->m_nInheritFolderIndex == value.get())
					return ;	// 派生元フォルダなら何もしない
				m_pMenuWindow->DestroyWindow();
				m_pMenuWindow = nullptr;
			}
			// サブメニューをポップアップさせる
			if (item.pFolder) {
				m_pMenuWindow = new CMenuWindow(item.pFolder, value.get());
				CRect rcWindow;
				int nTop	= item.rcItem.top - (kBoundBorder + kBoundMargin);
				int nLeft	= item.rcItem.right	- kBiteWidth;
				rcWindow.right	= m_pMenuWindow->ComputeWindowWidth();
				rcWindow.bottom	= m_pMenuWindow->ComputeWindowHeight();
				rcWindow.MoveToXY(nLeft, nTop);
				ClientToScreen(&rcWindow);
				m_pMenuWindow->Create(m_hWnd, rcWindow, nullptr, /*WS_VISIBLE| */WS_POPUP | WS_BORDER, WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
				m_pMenuWindow->ShowWindow(SW_SHOWNOACTIVATE);

			} else if (bLButtonDown) {
				// アイテムを実行...
				DonutOpenFile(item.strUrl);

				::SendMessage(g_hWndLinkBar, WM_CLOSEBASESUBMENU, 0, 0);
				// メニューを閉じる
			}
		}
	}

	// Data members
	LinkFolderPtr	m_pFolder;
	CFont	m_font;
	int		m_nHotIndex;
	CMenuWindow*	m_pMenuWindow;
	const int m_nInheritFolderIndex;
};

///////////////////////////////////////////////////////
// CDonutLinkBarCtrl::Impl

class CDonutLinkBarCtrl::Impl : 
	public CDoubleBufferWindowImpl<Impl>,
	public CThemeImpl<Impl>,
	public CTrackMouseLeave<Impl>,
	public IDropTargetImpl<Impl>,
	public IDropSourceImpl<Impl>
{
public:
	DECLARE_WND_CLASS(_T("DonutLinkBarCtrl"))

	// Constants
	enum { 
		kTopBottomPadding = 6,
		kRightLeftPadding = 6,
		kMaxItemTextWidth	= 100,
		kLeftMargin	= 2,
		kDragInsertHitWidthOnFolder = 7,
	};

	// Constructer
	Impl();

	void	SetFont(HFONT hFont);

	// Overrides
	void DoPaint(CDCHandle dc);
	void OnTrackMouseMove(UINT nFlags, CPoint pt);
	void OnTrackMouseLeave();
	// IDropTargetImpl
	DROPEFFECT OnDragOver(IDataObject *pDataObject, DWORD dwKeyState, CPoint point, DROPEFFECT dropOkEffect);
	DROPEFFECT OnDrop(IDataObject *pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropEffectList, CPoint point);
	void	OnDragLeave();

	BEGIN_MSG_MAP( Impl )
		MSG_WM_CREATE( OnCreate )
		MSG_WM_DESTROY( OnDestroy )
		MSG_WM_SIZE( OnSize )
		MSG_WM_LBUTTONDOWN( OnLButtonDown )
		MSG_WM_LBUTTONUP( OnLButtonUp )
		MESSAGE_HANDLER_EX( WM_CLOSEBASESUBMENU, OnCloseBaseSubMenu )
		CHAIN_MSG_MAP( CDoubleBufferWindowImpl<Impl> )
		CHAIN_MSG_MAP( CThemeImpl<Impl> )
		CHAIN_MSG_MAP( CTrackMouseLeave<Impl> )
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnSize(UINT nType, CSize size);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	LRESULT OnCloseBaseSubMenu(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	void	_RefreshBandInfo();
	void	_AddLinkItem(LinkFolderPtr pFolder, wptree pt);
	void	_UpdateItemPosition();
	bool	_IsValidIndex(int nIndex) const { 
		return 0 <= nIndex && nIndex < (int)m_BookmarkList.size(); 
	}
	boost::optional<int>	_HitTest(const CPoint& point);
	void	_HotItem(int nNewHotIndex);
	void	_PressItem(int nPressedIndex);
	void	_CloseSubMenu();
	void	_DoPopupSubMenu(int nIndex);

	void	_DoDragDrop(const CPoint& pt, UINT nFlags, int nIndex);
	void	_DrawInsertEdge(const CPoint& ptRightBottom);
	void	_ClearInsertionEdge();

	// Data members
	CFont	m_font;
	vector<unique_ptr<LinkItem> >	m_BookmarkList;
	int		m_nHotIndex;
	int		m_nPressedIndex;
	CMenuWindow*	m_pSubMenu;
	CPoint	m_ptInvalidateOnDrawingInsertionEdge;
};

#include "DonutLinkBarCtrl.inl"


///////////////////////////////////////////////////////
// CDonutLinkBarCtrl

CDonutLinkBarCtrl::CDonutLinkBarCtrl()
	: pImpl(new Impl)
{}

CDonutLinkBarCtrl::~CDonutLinkBarCtrl()
{
	delete pImpl;
}

HWND	CDonutLinkBarCtrl::Create(HWND hWndParent)
{
	return pImpl->Create(hWndParent, CRect(0, 0, -1, 20));
}


void	CDonutLinkBarCtrl::SetFont(HFONT hFont)
{
	pImpl->SetFont(hFont);
}



