/**
 *	@file	MenuEncode.cpp
 *	@brief	�G���R�[�h�I�����j���[.
 */

#include "stdafx.h"
#include "MenuEncode.h"
#include "ChildFrame.h"
#include "MainFrame.h"


///////////////////////////////////////////////////
// CMenuEncode


CMenuEncode::CMenuEncode(CMainFrame* pFrame) : m_pMainFrame(pFrame)
{
	m_posStart		= 0;
	m_insertPoint	= 0;
	m_hInstDLL		= NULL;
	m_hWndTarget	= NULL;
	m_bFlag 		= FALSE;
}

CMenuEncode::~CMenuEncode()
{
	spCT.Detach();
	spWnd.Detach();
}


void CMenuEncode::Init(HMENU hMenu, int insertPoint)
{
	m_Menu		  = hMenu;
	m_insertPoint = insertPoint;
}



void CMenuEncode::OnInitMenuPopup(HMENU hmenuPopup, UINT uPos, BOOL fSystemMenu)
{
	SetMsgHandled(FALSE);

	CChildFrame* pChild = m_pMainFrame->GetActiveChildFrame();// DonutGetActiveWindow(m_hWnd);

	if (hmenuPopup == m_Menu.m_hMenu) {
		_ReleaseMenu();

		if (pChild)
			CreateMenu();

		MENUITEMINFO mii = { sizeof (MENUITEMINFO) };
		mii.fMask = MIIM_TYPE;
		m_Menu.GetMenuItemInfo(m_insertPoint, TRUE, &mii);
		if (mii.fType & MFT_SEPARATOR) {
			mii.fMask	   = MIIM_SUBMENU | MIIM_TYPE;
			mii.dwTypeData = _T("�G���R�[�h(&M)");
			mii.cch 	   = 15;
			mii.fType	   = MFT_STRING;
			mii.hSubMenu   = m_EncodeMenu.m_hMenu;
			m_Menu.InsertMenuItem(m_insertPoint, TRUE, &mii);	// �G���R�[�h ���j���[��}������
		}
	}

	if (pChild) {
		EnableMenuItem(m_Menu.m_hMenu, m_insertPoint, MF_BYPOSITION | MF_ENABLED);
	} else {
		EnableMenuItem(m_Menu.m_hMenu, m_insertPoint, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	}
}


LRESULT CMenuEncode::OnReleaseContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	_ReleaseMenu();
	return 0;
}

LRESULT CMenuEncode::OnExitMenuLoop(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	_ClearMenu();
	return 0;
}


void CMenuEncode::OnRangeCommand(UINT, int nID, HWND hWndCtrl)
{
	MENUITEMINFO mii   = { sizeof (MENUITEMINFO) };
	mii.fMask = MIIM_DATA;

	HMENU		 hMenu = _SearchSubMenu(nID, m_EncodeMenu.m_hMenu);
	if (!hMenu)  return;

	::GetMenuItemInfo(hMenu, nID, FALSE, &mii);

	if (m_hWndTarget) {
		::SendMessage(m_hWndTarget, WM_COMMAND, mii.dwItemData, 0);
	}
}




HMENU CMenuEncode::_SearchSubMenu(int nID, HMENU hParentMenu)
{
	HMENU			hRet;
	MENUITEMINFO	mii   = { sizeof (MENUITEMINFO) };
	mii.fMask = MIIM_SUBMENU | MIIM_ID;
	int 	count = ::GetMenuItemCount(hParentMenu);
	for (int i = 0; i < count; i++) {
		::GetMenuItemInfo(hParentMenu, i, TRUE, &mii);
		if (mii.wID == nID)
			return hParentMenu;
		if (mii.hSubMenu) {
			hRet = _SearchSubMenu(nID, mii.hSubMenu);
			if (hRet)		return hRet;
		}
	}
	return NULL;
}


BOOL CMenuEncode::CreateMenu()
{
	HRESULT	hr = S_OK;

	CChildFrame* pChild = m_pMainFrame->GetActiveChildFrame();
	if (pChild == nullptr) 
		return FALSE;

	CComPtr<IWebBrowser2>	spBrowser = pChild->GetMarshalIWebBrowser();
	if (spBrowser == nullptr)
		return FALSE;

	CComPtr<IDispatch>	spDisp;
	spBrowser->get_Document(&spDisp);
	CComQIPtr<IHTMLDocument2>	spDoc = spDisp;
	if (spDoc == nullptr)
		return FALSE;

	//defined on AtlhostEx.h
	enum {
		IDR_BROWSE_CONTEXT_MENU 	= 24641,
		SHDVID_GETMIMECSETMENU		=	 27,
	};

	//ICommandTarget��IOleWindow���擾�B�E�B���h�E�n���h���͕ۊ�
	hr	= spDoc->QueryInterface(IID_IOleCommandTarget, (void **) &spCT);
	hr	= spDoc->QueryInterface(IID_IOleWindow, (void **) &spWnd);
	if ( (!spCT) || (!spWnd) )
		return FALSE;

	hr	= spWnd->GetWindow(&m_hWndTarget);
	if (m_hWndTarget == NULL)
		return FALSE;

	//DLL���烁�j���[�����[�h����
	m_hInstDLL	 = LoadLibrary( TEXT("SHDOCLC.DLL") );
	if (m_hInstDLL == NULL) {
		m_hInstDLL = LoadLibrary( TEXT("ieframe.dll") );	// for vista
		if (m_hInstDLL == NULL)
			return FALSE;
	}

	m_CtxMenu	 = LoadMenu( m_hInstDLL, MAKEINTRESOURCE(IDR_BROWSE_CONTEXT_MENU) );
	m_CtxMenuSub = GetSubMenu(m_CtxMenu.m_hMenu, CONTEXT_MENU_DEFAULT);

	//�G���R�[�h���j���[���擾����
	//CComVariant 	var;
	VARIANT	var = { 0 };
	hr = spCT->Exec(&CGID_ShellDocView, SHDVID_GETMIMECSETMENU, 0, NULL, &var);

	MENUITEMINFO	mii   = { sizeof (MENUITEMINFO) };
	mii.fMask	 = MIIM_SUBMENU;
	mii.hSubMenu = (HMENU) var.byref;
	m_CtxMenuSub.SetMenuItemInfo(IDM_LANGUAGE, FALSE, &mii);

	//m_CtxMenuSub�ɂ�����G���R�[�h���j���[�̐擪���ڂ�ID��ۊ�
	MENUITEMINFO	mii2  = { sizeof (MENUITEMINFO) };
	m_posStart	 = 0;
	mii2.fMask	 = MIIM_SUBMENU | MIIM_ID;
	int count = m_CtxMenuSub.GetMenuItemCount();
	for (int i = 0; i < count; ++i) {
		m_CtxMenuSub.GetMenuItemInfo(i, TRUE, &mii2);
		if (mii2.hSubMenu == mii.hSubMenu)
			m_posStart = mii2.wID;
	}

	//�G���R�[�h���j���[���R�s�[
	int id = ID_START;
	m_EncodeMenu.CreatePopupMenu();
	CopySubMenu(m_EncodeMenu.m_hMenu, mii.hSubMenu, id);

	return TRUE;
}


HMENU CMenuEncode::CopySubMenu(HMENU hCopy , HMENU hOriginal, int &id)
{
	CMenuHandle  Original = hOriginal;
	CMenuHandle  Copy	  = hCopy;
	MENUITEMINFO mii = { sizeof (MENUITEMINFO) };
	mii.fMask  = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE ;
	CString 	 strBuf;

	int 		 count	  = Original.GetMenuItemCount();
	for (int i = 0; i < count; i++) {
		mii.dwTypeData = strBuf.GetBuffer(1024);
		mii.cch 	   = 1024;
		Original.GetMenuItemInfo(i, TRUE, &mii);
		mii.dwItemData = mii.wID;	//���̃��j���[���N���b�N�����
		mii.wID 	   = id++;		///�R�}���h�n���h�����Ăяo�����悤�ɂ���
		strBuf.ReleaseBuffer();

		if (mii.hSubMenu == NULL) {
			Copy.InsertMenuItem(i, TRUE, &mii);
		} else {
			CMenuHandle Parent;
			Parent.CreatePopupMenu();
			Parent		 = CopySubMenu(Parent.m_hMenu, mii.hSubMenu, id);
			mii.fMask	 = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE ;
			mii.hSubMenu = Parent.m_hMenu;
			Copy.InsertMenuItem(i, TRUE, &mii);
		}
	}

	return Copy.m_hMenu;
}

void CMenuEncode::_ReleaseMenu()
{
	if ( m_EncodeMenu.IsMenu() ) { m_EncodeMenu.DestroyMenu(); }
	m_posStart	 = 0;
	m_hWndTarget = NULL;
	if (spCT)		{ spCT.Release() ;			 spCT		= NULL; }
	if (spWnd)		{ spWnd.Release();			 spWnd		= NULL; }
	if (m_hInstDLL) { ::FreeLibrary(m_hInstDLL); m_hInstDLL = NULL; }
}

/// ���j���[����[�G���R�[�h] ���j���[����菜��
void CMenuEncode::_ClearMenu()
{
	MENUITEMINFO mii = { sizeof (MENUITEMINFO) };
	mii.fMask = MIIM_TYPE;
	m_Menu.GetMenuItemInfo(m_insertPoint, TRUE, &mii);
	if ( !(mii.fType & MFT_SEPARATOR) )
		m_Menu.RemoveMenu(m_insertPoint, MF_BYPOSITION);

	if ( m_CtxMenu.IsMenu() )
		m_CtxMenu.DestroyMenu();
}
