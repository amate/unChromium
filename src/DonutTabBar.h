/**
 *	@file	DonutTabBar.h
 */

#pragma once


#define PASS_MSG_MAP_MDICHILD_TO_TAB(x)		CHAIN_MSG_MAP_ALT_MEMBER(x, 1)

// �O���錾
class CChildFrameClient;

///////////////////////////////////////////////////////////
// �^�u�̏��

enum ETABITEMSTATES {
	TISS_NORMAL		= 0x01,
	TISS_SELECTED	= 0x02,
	TISS_MSELECTED	= 0x04,
	TISS_PRESSED	= 0x08,
	TISS_HOT		= 0x10,
	TISS_NAVIGATELOCK = 0x20,
	TISS_INACTIVE	= 0x40,
};


//--------------------------------
/// minit A_ON��GetLinkState�ŏ������B_ON�͎c��
/// SetLinkState�� LINKSTATE_B_ON ���w�肵���ꍇ
/// �蓮�� SetLinkState(LINKSTATE_OFF)���Ȃ���΂Ȃ�Ȃ�
enum ELinkState {
	LINKSTATE_OFF		= 0,
	LINKSTATE_A_ON		= 1,
	LINKSTATE_B_ON		= 2,
};


///////////////////////////////////////////////
// DonutTabBar

class CDonutTabBar
{
public:
	CDonutTabBar();
	~CDonutTabBar();

	HWND	Create(HWND hWndParent);
	void	SetChildFrameClient(CChildFrameClient* pChildClient);

	// Attributes
	HWND	GetTabHwnd(int nIndex);
	int 	GetTabIndex(HWND hWndMDIChild);
	void	SetFont(HFONT hFont);
	int		GetItemCount() const;
	void	GetItemState(int nIndex, DWORD& state);
	void	SetItemActive(int nIndex);
	void	SetItemInactive(int nIndex);
	int		GetCurSel() const;
	bool	SetCurSel(int nIndex);
	void	SetLinkState(ELinkState State);
	void	SetConnecting(HWND hWnd);
	void	SetDownloading(HWND hWnd);
	void	SetComplete(HWND hWnd);
	void	SetTitle(HWND hWnd, LPCTSTR strTitle);
	void	NavigateLockTab(HWND hWnd, bool bOn);
	void	ReloadSkin();
	void	GetWindowRect(LPRECT rect);

	// Operation
	void	OnMDIChildCreate(HWND hWnd, bool bActive = false);
	void	OnMDIChildDestroy(HWND hWnd);
	void	InsertHere(bool bUse);
	void	SetInsertIndex(int nIndex);
	bool	MoveItems(int nDestIndex, CSimpleArray<int>& arrSrcs);
	void	LeftTab();
	void	RightTab();
	void	ShowTabMenu(int nIndex);
	int 	ShowTabListMenuDefault (int nX, int nY);
	int 	ShowTabListMenuVisible (int nX, int nY);
	int 	ShowTabListMenuAlphabet(int nX, int nY);

	// �O������g���N���X
	struct CLockRedraw {
		CLockRedraw(CDonutTabBar& tabbar);
		~CLockRedraw();
		CDonutTabBar& m_TabBar;
	};

	void	ForEachWindow(function<void (HWND)> func);

	BEGIN_MSG_MAP_EX_decl(CDonutTabBar);

	void	OnSetFaviconImage(HWND hWnd, HICON hIcon);

private:
	class Impl;
	Impl*	pImpl;
};