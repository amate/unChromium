// RightClickMenuDialog.cpp

#include "stdafx.h"
#include "../IniFile.h"
#include "../DonutPFunc.h"
#include "../ToolTipManager.h"
#include "MainOption.h"
#include "RightClickMenuDialog.h"
#include "DonutSearchBar.h"

///////////////////////////////////////////////////////////
// CCustomContextMenuOption

CMenu	CCustomContextMenuOption::s_menuDefault;
CMenu	CCustomContextMenuOption::s_menuImage;
CMenu	CCustomContextMenuOption::s_menuTextSelect;
CMenu	CCustomContextMenuOption::s_menuAnchor;
CMenu	CCustomContextMenuOption::s_menuHoldLeftButton;

CMenu	CCustomContextMenuOption::s_menuTabItem;


void	CCustomContextMenuOption::GetDefaultContextMenu(CMenu& rMenu, DWORD dwID)
{
	enum { IDR_BROWSE_CONTEXT_MENU 	= 24641 };

	// DLLからのメニューリソース読み込み
	{
		HINSTANCE	hDll = NULL;

		hDll = ::LoadLibrary(_T("SHDOCLC.DLL"));
		if (hDll == NULL) {
			hDll = ::LoadLibrary(_T("ieframe.dll"));	// for Vista
		}

		if (hDll == NULL) {
			ATLASSERT(FALSE);
			return;
		}

		HMENU hMenu = ::LoadMenu(hDll, MAKEINTRESOURCE(IDR_BROWSE_CONTEXT_MENU));
		ATLASSERT(hMenu);
		rMenu = ::GetSubMenu(hMenu, dwID);
		ATLASSERT(rMenu);

		::FreeLibrary(hDll);
	}
#if 0
	/* 文字列が設定されていないメニュー項目を削除する */
	for (int i = rMenu.GetMenuItemCount() - 1; i >= 0 ; --i) {
		CString strText;
		rMenu.GetMenuString(i, strText, MF_BYPOSITION);
		if (strText.IsEmpty() && rMenu.GetMenuItemID(i) != 0) {
			rMenu.DeleteMenu(i, MF_BYPOSITION);
		}
	}
#endif
}

HMENU	CCustomContextMenuOption::GetContextMenuFromID(DWORD dwID)
{
	switch (dwID) {
	case CONTEXT_MENU_DEFAULT:
		return s_menuDefault;
		break;

	case CONTEXT_MENU_IMAGE:
		return s_menuImage;
		break;

	case CONTEXT_MENU_TEXTSELECT:
		return s_menuTextSelect;
		break;

	case CONTEXT_MENU_ANCHOR:
		return s_menuAnchor;
		break;

	case CONTEXT_MENU_HOLDLEFTBUTTON:
		return s_menuHoldLeftButton;
		break;

	case CONTEXT_MENU_TABITEM:
		return s_menuTabItem;
		break;

	default:
		ATLASSERT(FALSE);
		return NULL;
		break;
	}
}

void	CCustomContextMenuOption::SetContextMenuFromID(HMENU hMenu, DWORD dwID)
{
	switch (dwID) {
	case CONTEXT_MENU_DEFAULT:
		s_menuDefault		= hMenu;
		break;

	case CONTEXT_MENU_IMAGE:
		s_menuImage			= hMenu;
		break;

	case CONTEXT_MENU_TEXTSELECT:
		s_menuTextSelect	= hMenu;
		break;

	case CONTEXT_MENU_ANCHOR:
		s_menuAnchor		= hMenu;
		break;

	case CONTEXT_MENU_HOLDLEFTBUTTON:
		s_menuHoldLeftButton= hMenu;
		break;

	case CONTEXT_MENU_TABITEM:
		s_menuTabItem		= hMenu;
		break;

	default:
		ATLASSERT(FALSE);
		break;
	}
}

// dwCmdを見てサブメニューを挿入します
// メニューを表示し終わったらロードしたメニューは削除すること
void	CCustomContextMenuOption::AddSubMenu(CMenuHandle menu, HWND hWndTopLevel, CSimpleArray<HMENU>& arrDestroyMenu)
{
	for (int i = 0; i < menu.GetMenuItemCount(); ++i) {
		MENUITEMINFO mii  = { sizeof (MENUITEMINFO) };
		mii.fMask  = MIIM_SUBMENU;

		DWORD	dwID	= 0;
		DWORD	dwCmd	= menu.GetMenuItemID(i);
		switch (dwCmd) {
		case ID_FAVORITES_DROPDOWN:
			mii.hSubMenu = (HMENU) ::SendMessage(hWndTopLevel, WM_MENU_GET_FAV	  , 0, 0);
			break;

		case ID_FAVORITES_GROUP_DROPDOWN:
			mii.hSubMenu = (HMENU) ::SendMessage(hWndTopLevel, WM_MENU_GET_FAV_GROUP, 0, 0);
			break;

		case ID_SCRIPT:
			mii.hSubMenu = (HMENU) ::SendMessage(hWndTopLevel, WM_MENU_GET_SCRIPT   , 0, 0);
			break;

		case ID_SEARCHENGINE_MENU:
			mii.hSubMenu = CDonutSearchBar::GetInstance()->GetSearchEngineMenuHandle();
			break;

		case ID_BINGTRANSLATOR_MENU:
			mii.hSubMenu = (HMENU) ::SendMessage(hWndTopLevel, WM_MENU_GET_BINGTRANSLATE, 0, 0);
			break;

		case ID_DLCTL_CHG_MULTI:	dwID = IDR_MULTIMEDIA;		break;	// マルチメディア
		case ID_DLCTL_CHG_SECU: 	dwID = IDR_SECURITY;		break;	// セキュリティ
		case ID_VIEW_FONT_SIZE: 	dwID = IDR_VIEW_FONT_SIZE;	break;	// フォントサイズ
		case ID_COOKIE_IE6: 		dwID = IDR_COOKIE_IE6;		break;	// クッキー(IE6)
		case ID_HTMLZOOM_MENU: 		dwID = IDR_ZOOM_MENU;		break;	// 拡大
		case ID_AUTO_REFRESH:		dwID = IDR_AUTO_REFRESH;	break;	// 自動更新

		default: 
			continue;
		}

		if (dwID != 0) {
			CMenuHandle  menu;
			menu.LoadMenu(dwID);	// リソースからメニューを読み込む
			HMENU	hMenuSub = menu.GetSubMenu(0);
			mii.hSubMenu	 = hMenuSub;
			arrDestroyMenu.Add(hMenuSub);
		}

		/* サブメニューを追加する */
		menu.SetMenuItemInfo(dwCmd, FALSE, &mii);
	}
}

// お片づけ
void	CCustomContextMenuOption::RemoveSubMenu(CMenuHandle menu, CSimpleArray<HMENU>& arrDestroyMenu)
{
	for (int ii = 0; ii < arrDestroyMenu.GetSize(); ii++) {
		::DestroyMenu(arrDestroyMenu[ii]);
	}

	int nMenuItemCount = menu.GetMenuItemCount();
	/* サブメニューを削除 */
	for (int i = 0; i < nMenuItemCount; ++i) {
		CString strText;
		menu.GetMenuString(i, strText, MF_BYPOSITION);
		//if (strText.Left(5) == _T("お気に入り")) continue;
		if (strText.IsEmpty())
			continue;

		CMenuItemInfo	mii;
		mii.fMask	   = MIIM_ID;
		menu.GetMenuItemInfo(i, TRUE, &mii);

		menu.RemoveMenu(i, MF_BYPOSITION);

		mii.fMask		|= MIIM_STRING;
		mii.dwTypeData	= strText.GetBuffer(0);
		menu.InsertMenuItem(i, TRUE, &mii);
	}
}

// サブメニューを削除しておかないとMenu.xmlに登録できないので
void	CCustomContextMenuOption::ResetMenu()
{
	HMENU	arrMenu[] = {
		s_menuDefault,
		s_menuImage,
		s_menuTextSelect,
		s_menuAnchor,
		s_menuHoldLeftButton,
	
		s_menuTabItem,
	};

	for (int i = 0; i < sizeof(arrMenu) / sizeof(arrMenu[0]); ++i) {
		HMENU	menu			= arrMenu[i];
		int		nMenuItemCount	= GetMenuItemCount(menu);
		/* サブメニューを削除 */
		for (int i = 0; i < nMenuItemCount; ++i) {
			CMenuItemInfo	mii;
			mii.fMask	   = MIIM_SUBMENU;
			mii.hSubMenu   = NULL;
			SetMenuItemInfo(menu, i, TRUE, &mii);
		}
	}
}

void	CCustomContextMenuOption::_ReadAllMenuItem(LPCWSTR strElement, CMenu& rMenu, CXmlFileRead& xmlRead)
{
	ATLASSERT(rMenu.m_hMenu == NULL);

	rMenu.CreatePopupMenu();
	ATLASSERT(rMenu.m_hMenu);

	CString strName;
	UINT	uCmdID = 0;
	
	CString strItem;
	while (xmlRead.GetInternalElement(strElement, strItem)) {
		if (strItem == L"item" && xmlRead.MoveToFirstAttribute() && xmlRead.GetLocalName() == L"name") {
			strName = xmlRead.GetValue();
			if (xmlRead.MoveToNextAttribute() && xmlRead.GetLocalName() == L"command") {
				uCmdID = _ttoi(xmlRead.GetValue());
				
				if (uCmdID == 0) {
					rMenu.AppendMenu(MF_SEPARATOR, (UINT_PTR)0, _T(""));
				} else {
					rMenu.AppendMenu(0, uCmdID, strName);
				}
			}
		}
	}
}

void	CCustomContextMenuOption::_WriteAllMenuItem(LPCTSTR strElement,const CMenu& rMenu, CXmlFileWrite& xmlWrite)
{
	xmlWrite.WriteStartElement(strElement);
	for (int i = 0; i < rMenu.GetMenuItemCount(); ++i) {
		xmlWrite.WriteStartElement(L"item");
		CString strName;
		UINT	uCmdID = rMenu.GetMenuItemID(i);
		if (uCmdID == 0) {
			strName = g_cSeparater;
		} else {
			rMenu.GetMenuString(i, strName, MF_BYPOSITION);
			if (strName.Left(5) == _T("エンコード") && uCmdID == -1) {
				uCmdID = IDM_LANGUAGE;
			}
		}
		
		xmlWrite.WriteAttributeString(L"name"	, strName);
		xmlWrite.WriteAttributeValue(L"command"	, uCmdID);
		xmlWrite.WriteFullEndElement();
	}
	xmlWrite.WriteFullEndElement();	// </strElement>
}


// iniファイルから設定を読み込む
void	CCustomContextMenuOption::GetProfile()
{
	try {
		CString strPath = _GetFilePath(_T("Menu.xml"));
		if (::PathFileExists(strPath)) {
			CXmlFileRead xmlRead(strPath);

			XmlNodeType	nodeType;
			while (xmlRead.Read(&nodeType) == S_OK) {
				switch (nodeType) {
				case XmlNodeType_Element:
					CMenu*	pMenu = NULL;
					CString strLocalName = xmlRead.GetLocalName();
					if (strLocalName == L"CONTEXT_MENU_DEFAULT") {
						pMenu = &s_menuDefault;
					} else if (strLocalName == L"CONTEXT_MENU_IMAGE") {
						pMenu = &s_menuImage;
					} else if (strLocalName == L"CONTEXT_MENU_TEXTSELECT") {
						pMenu = &s_menuTextSelect;
					} else if (strLocalName == L"CONTEXT_MENU_ANCHOR") {
						pMenu = &s_menuAnchor;
					} else if (strLocalName == L"CONTEXT_MENU_HOLDLEFTBUTTON") {
						pMenu = &s_menuHoldLeftButton;
					} else if (strLocalName == L"CONTEXT_MENU_TABITEM") {
						pMenu = &s_menuTabItem;
					} else {
						continue;
					}
					_ReadAllMenuItem(strLocalName, *pMenu, xmlRead);
				}
			}
		}
	} catch (LPCTSTR strError) {
		MessageBox(NULL, strError, NULL, NULL);
	}

	/* ファイルが無かったり、エレメントが存在しないとき */
	if (s_menuDefault.m_hMenu == NULL) {
		GetDefaultContextMenu(s_menuDefault		, CONTEXT_MENU_DEFAULT);
	}
	if (s_menuImage.m_hMenu == NULL) {
		GetDefaultContextMenu(s_menuImage		, CONTEXT_MENU_IMAGE);
	}
	if (s_menuTextSelect.m_hMenu == NULL) {
		GetDefaultContextMenu(s_menuTextSelect	, CONTEXT_MENU_TEXTSELECT);
	}
	if (s_menuAnchor.m_hMenu == NULL) {
		GetDefaultContextMenu(s_menuAnchor		, CONTEXT_MENU_ANCHOR);
	}
	if (s_menuHoldLeftButton.m_hMenu == NULL) {
		s_menuHoldLeftButton.CreatePopupMenu();
	}

	if (s_menuTabItem.m_hMenu == NULL) {
		CMenuHandle tabmenu;
		tabmenu.LoadMenu(IDR_MENU_TAB);
		s_menuTabItem = tabmenu.GetSubMenu(0);
	}
}

// iniファイルに設定を書き込む
void	CCustomContextMenuOption::WriteProfile()
{
	try {
		CString strPath = _GetFilePath(_T("Menu.xml"));
		CXmlFileWrite xmlWrite(strPath);

		xmlWrite.WriteStartElement(L"CustomContextMenu");
		_WriteAllMenuItem(L"CONTEXT_MENU_DEFAULT"	, s_menuDefault		, xmlWrite);
		_WriteAllMenuItem(L"CONTEXT_MENU_IMAGE"		, s_menuImage		, xmlWrite);
		_WriteAllMenuItem(L"CONTEXT_MENU_TEXTSELECT", s_menuTextSelect	, xmlWrite);
		_WriteAllMenuItem(L"CONTEXT_MENU_ANCHOR"	, s_menuAnchor		, xmlWrite);
		_WriteAllMenuItem(L"CONTEXT_MENU_HOLDLEFTBUTTON", s_menuHoldLeftButton, xmlWrite);
		_WriteAllMenuItem(L"CONTEXT_MENU_TABITEM"	, s_menuTabItem		, xmlWrite);
		xmlWrite.WriteFullEndElement();	// </CustomContextMenu>
	} catch (LPCTSTR str) {
		MessageBox(NULL, str, NULL, NULL);
	}
}




//////////////////////////////////////////////////////////
// CRightClickPropertyPage

// Constructor
CRightClickPropertyPage::CRightClickPropertyPage(HMENU hMenu)
	: m_menu(hMenu)
	, m_bInit(false)
{
}


void	CRightClickPropertyPage::_InitComboBox()
{
	m_cmbTarget		= GetDlgItem(IDC_CMB_TARGET);
	m_cmbCategory	= GetDlgItem(IDC_CMB_CATEGORY);

	m_cmbTarget.AddString(_T("通常"));
	m_cmbTarget.AddString(_T("画像上"));
	m_cmbTarget.AddString(_T("テキスト選択"));
	m_cmbTarget.AddString(_T("リンク上"));
	m_cmbTarget.AddString(_T("左ボタン押しながら"));
	m_cmbTarget.AddString(_T("タブバー"));

	m_cmbTarget.SetItemData(0, CONTEXT_MENU_DEFAULT);
	m_cmbTarget.SetItemData(1, CONTEXT_MENU_IMAGE);
	m_cmbTarget.SetItemData(2, CONTEXT_MENU_TEXTSELECT);
	m_cmbTarget.SetItemData(3, CONTEXT_MENU_ANCHOR);
	m_cmbTarget.SetItemData(4, CONTEXT_MENU_HOLDLEFTBUTTON);
	m_cmbTarget.SetItemData(5, CONTEXT_MENU_TABITEM);

	m_cmbTarget.SetCurSel(0);
	OnSelChangeTarget(0, 0, 0);

	/* メニューの文字列をカテゴリに登録する */
	_SetCombboxCategory(m_cmbCategory, m_menu);
	m_cmbCategory.AddString( _T("グループ・メニュー") );
	OnSelChangeCate(0, 0, 0);
}


void	CRightClickPropertyPage::_AddCommandtoListFromSubMenu(CMenuHandle subMenu)
{
	int nAddCnt = 0;
	int nPopStartIndex = m_listCommand.AddString(g_cSeparater);

	for (int i = 0; i < subMenu.GetMenuItemCount(); ++i) {
		HMENU hMenuSub = subMenu.GetSubMenu(i);
		if (hMenuSub) {
			_AddCommandtoListFromSubMenu(hMenuSub);
		}

		UINT nCmdID = subMenu.GetMenuItemID(i);
		if ( _DontUseID(nCmdID) ) break;

		CString strMenu;
		CToolTipManager::LoadToolTipText(nCmdID, strMenu);
		if ( strMenu.IsEmpty() ) continue;

		int nIndex = m_listCommand.AddString(strMenu);
		m_listCommand.SetItemData(nIndex, nCmdID);
		nAddCnt++;
	}

	if (nAddCnt != 0) {
		m_listCommand.AddString(g_cSeparater);
	} else {
		m_listCommand.GetItemData(nPopStartIndex);
		m_listCommand.DeleteString(nPopStartIndex);
	}
	
}

// nIndexはカテゴリコンボボボックスの現在選択されているセル
// コマンドリストにカテゴリで選択されたメニューを登録する
void	CRightClickPropertyPage::_SetCommandList(int nIndex)
{
	m_listCommand.ResetContent();

	if (nIndex + 1 == m_cmbCategory.GetCount()) {
		// グループメニューのとき
		static const UINT uCommandExID[] = {
			ID_FAVORITES_DROPDOWN,
			ID_FAVORITES_GROUP_DROPDOWN,
			ID_SCRIPT,
			ID_DLCTL_CHG_MULTI,
			ID_DLCTL_CHG_SECU,
			ID_VIEW_FONT_SIZE,
			ID_COOKIE_IE6,
			ID_HTMLZOOM_MENU,			//+++
			ID_SEARCHENGINE_MENU,		//+++
		};

		for (int ii = 0; ii < sizeof (uCommandExID) / sizeof (UINT); ii++) {
			CString strMenu;
			CToolTipManager::LoadToolTipText(uCommandExID[ii], strMenu);

			if ( strMenu.IsEmpty() ) continue;

			int nIndex = m_listCommand.AddString(strMenu);
			m_listCommand.SetItemData(nIndex, uCommandExID[ii]);
		}
	} else {
		CMenuHandle	subMenu = m_menu.GetSubMenu(nIndex);
		for (int i = 0; i < subMenu.GetMenuItemCount(); ++i) {
			HMENU hMenuSub = subMenu.GetSubMenu(i);
			if (hMenuSub) {
				_AddCommandtoListFromSubMenu(hMenuSub);
			}

			UINT nCmdID = subMenu.GetMenuItemID(i);
			if ( _DontUseID(nCmdID) ) break;

			CString strMenu;
			CToolTipManager::LoadToolTipText(nCmdID, strMenu);
			if ( strMenu.IsEmpty() ) continue;

			int nIndex = m_listCommand.AddString(strMenu);
			m_listCommand.SetItemData(nIndex, nCmdID);
		}
	}

	{	// 不要なセパレータの削除
		int   nCountSep = 0;
		int   nCountCmb = m_listCommand.GetCount();

		for (int i = 0; i < nCountCmb - 1; i++) {
			if (m_listCommand.GetItemData(i) == 0) {
				nCountSep++;

				if (m_listCommand.GetItemData(i + 1) == 0) {
					m_listCommand.DeleteString(i);
					nCountCmb--;
					i--;
				}
			}
		}

		if (nCountSep > 2) {
			if (m_listCommand.GetItemData(0) == 0) {
				m_listCommand.DeleteString(0);
			}

			int nIndexLast = m_listCommand.GetCount() - 1;

			if (m_listCommand.GetItemData(nIndexLast) == 0) {
				m_listCommand.DeleteString(nIndexLast);
			}
		}
	}
}


void	CRightClickPropertyPage::_AddStringToMenuListFromMenu(const CMenuHandle menu)
{
	for (int i = 0; i < menu.GetMenuItemCount(); ++i) {
		CString strText;
		UINT uCmdID = menu.GetMenuItemID(i);
		if (uCmdID == 0) {
			strText = g_cSeparater;
		} else {
			menu.GetMenuString(i, strText, MF_BYPOSITION);
			//if (strText.IsEmpty()) continue;
		}
		m_listMenu.AddString(strText);
		m_listMenu.SetItemData(i, uCmdID);
	}
}

// nIndexのメニューからメニューリストを作る
void	CRightClickPropertyPage::_SetMenuList(int nIndex)
{
	m_listMenu.ResetContent();

	DWORD dwID = (DWORD)m_cmbTarget.GetItemData(nIndex);
	HMENU hMenu = GetContextMenuFromID(dwID);
	_AddStringToMenuListFromMenu(hMenu);
}

// メニューリストからメニューを作る
void	CRightClickPropertyPage::_CreateMenuFromMenuList(CMenuHandle &rMenu)
{
	ATLASSERT(rMenu.m_hMenu == NULL);

	rMenu.CreatePopupMenu();
	for (int i = 0; i < m_listMenu.GetCount(); ++i) {
		
		UINT uCmdID = (UINT)m_listMenu.GetItemData(i);
		if (uCmdID == 0) {
			rMenu.InsertMenu(-1, MF_BYPOSITION | MF_SEPARATOR, (UINT_PTR)0, _T(""));
		} else {
			CString strText;
			m_listMenu.GetText(i, strText);
			if (strText.Left(5) == _T("エンコード") && uCmdID == -1) {
				uCmdID = IDM_LANGUAGE;
			}
			rMenu.InsertMenu(-1, MF_BYPOSITION, uCmdID, strText);
		}
		
	}
}


// Overrides
BOOL	CRightClickPropertyPage::OnSetActive()
{
	if (m_bInit == false) {
		m_bInit = true;

		m_btnAdd			= GetDlgItem(IDC_BUTTON_ADD);
		m_btnRemove			= GetDlgItem(IDC_BUTTON_REMOVE);
		m_btnAddSeparator	= GetDlgItem(IDC_BUTTON_ADD_SEPARATOR);
		m_btnApplyMenu		= GetDlgItem(IDC_BUTTON_APPLYMENU);
		m_btnApplyMenu.EnableWindow(FALSE);

		m_listCommand	= GetDlgItem(IDC_LIST_COMMAND);
		m_listMenu		= GetDlgItem(IDC_LIST_MENU);
		m_listMenu.MakeDragList();

		_InitComboBox();

		ResetMenu();
	}

	return TRUE;
}

BOOL	CRightClickPropertyPage::OnKillActive()
{

	return TRUE;
}

BOOL	CRightClickPropertyPage::OnApply()
{
	/* 設定を保存する */
	WriteProfile();
	return TRUE;
}


BOOL	CRightClickPropertyPage::OnBeginDrag(int nCtlID, HWND hWndDragList, POINT ptCursor)
{
	// カーソル位置のアイテムの左側に挿入マークを描画
	//m_listMenu.DrawInsert(m_listMenu.LBItemFromPt(ptCursor));
	return TRUE;
}

void	CRightClickPropertyPage::OnCancelDrag(int nCtlID, HWND hWndDragList, POINT ptCursor)
{
	// 挿入マークを消去
	m_listMenu.DrawInsert(-1);
}

int		CRightClickPropertyPage::OnDragging(int nCtlID, HWND hWndDragList, POINT ptCursor)
{
	// カーソル位置のアイテムの左側に挿入マークを描画
	m_listMenu.DrawInsert(m_listMenu.LBItemFromPt(ptCursor));
	return 0;
}

void	CRightClickPropertyPage::OnDropped(int nCtlID, HWND hWndDragList, POINT ptCursor)
{
	// 挿入マークを消去
	m_listMenu.DrawInsert(-1);

	int nSrcIndex	= m_listMenu.GetCurSel();             // 移動元のインデックス
	int nDestIndex	= m_listMenu.LBItemFromPt(ptCursor);  // 移動先のインデックス

	if (nSrcIndex == -1 || nDestIndex == -1 
	 || nDestIndex == nSrcIndex || nDestIndex == nSrcIndex + 1) return;

	/* 移動元のアイテムを保存しておく */
	CString strText;
	UINT	uCmdID = (UINT)m_listMenu.GetItemData(nSrcIndex);
	m_listMenu.GetText(nSrcIndex, strText);
	
	/* 挿入する */
	m_listMenu.InsertString(nDestIndex, strText);
	m_listMenu.SetItemData(nDestIndex, uCmdID);
	m_listMenu.SetCurSel(nDestIndex);

	/* 上に移動した場合 */
	if (nDestIndex < nSrcIndex) nSrcIndex++;
	m_listMenu.DeleteString(nSrcIndex);

	m_btnApplyMenu.EnableWindow(TRUE);
}



// [対象]のコンボボックスのセルが変わったとき
void	CRightClickPropertyPage::OnSelChangeTarget(UINT code, int id, HWND hWnd)
{
	int nCurSel = m_cmbTarget.GetCurSel();
	_SetMenuList(nCurSel);

	m_btnRemove.EnableWindow(FALSE);
	m_btnApplyMenu.EnableWindow(FALSE);
}

// [カテゴリ]のコンボボックスのセルが変わったとき
void	CRightClickPropertyPage::OnSelChangeCate(UINT code, int id, HWND hWnd)
{
	int nCurSel = m_cmbCategory.GetCurSel();
	_SetCommandList(nCurSel);

	m_btnAdd.EnableWindow(FALSE);
}

// コマンドリストの項目が選択された
void	CRightClickPropertyPage::OnSelChangeCommandList(UINT code, int id, HWND hWnd)
{
	m_btnAdd.EnableWindow(TRUE);
}

// コマンドリストの項目がダブルクリックされた
void	CRightClickPropertyPage::OnSelDblclkCommandList(UINT code, int id, HWND hWnd)
{
	OnAdd(0, 0, 0);
}

// メニューリストの項目がダブルクリックされた
void	CRightClickPropertyPage::OnSelDblclkMenuList(UINT code, int id, HWND hWnd)
{
	CAccessKeyWindow m_wndAccesskey;

	int nCurSel = m_listMenu.GetCurSel();
	if (nCurSel == -1) return;
	UINT uCmdID = (UINT)m_listMenu.GetItemData(nCurSel);
	if (uCmdID == 0) return;

	CString strKey;							// &E
	CString strText;						// ダブルクリックした項目の文字列
	m_listMenu.GetText(nCurSel, strText);
	int	nIndex = strText.Find(_T('&'));
	if (nIndex != -1) {
		strKey = strText.Mid(nIndex, 2);
		m_wndAccesskey.m_strKey = strKey.Mid(1, 1);
	}

	if (m_wndAccesskey.DoModal(GetParent()) == IDOK) {
		
		m_listMenu.DeleteString(nCurSel);

		if (m_wndAccesskey.m_strKey.IsEmpty()) {
			if (strKey.IsEmpty() == FALSE) {
				// アクセスキーを削除する
				strKey.Insert(0, _T("("));
				strKey += _T(")");
				strText.Replace(strKey, _T(""));
			}
		} else {
			if (nIndex != -1) {
				// アクセスキーを置き換える
				CString strNewKey;
				strNewKey.Format(_T("&%s"), m_wndAccesskey.m_strKey);
				strText.Replace(strKey, strNewKey);
			} else {
				// アクセスキーを追加する
				strText += _T("(&");
				strText += m_wndAccesskey.m_strKey;
				strText += _T(")");
			}
		}

		m_listMenu.InsertString(nCurSel, strText);
		m_listMenu.SetItemData(nCurSel, uCmdID);
		m_listMenu.SetCurSel(nCurSel);

		m_btnApplyMenu.EnableWindow(TRUE);
	}	
}

// メニューリストの項目が選択された
void	CRightClickPropertyPage::OnSelChangeMenuList(UINT code, int id, HWND hWnd)
{
	m_btnRemove.EnableWindow(TRUE);
}



// メニューリストに追加する
void	CRightClickPropertyPage::OnAdd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_btnApplyMenu.EnableWindow(TRUE);

	/* 現在選択されているコマンドリストのテキストとコマンドIDを取得 */
	CString	strText;
	int nCurSel = m_listCommand.GetCurSel();
	UINT uCmdID = (UINT)m_listCommand.GetItemData(nCurSel);
	if (uCmdID == 0) {
		strText = g_cSeparater;
	} else {
		m_listCommand.GetText(nCurSel, strText);
	}
	
	{	// メニューリストに追加する
		int nCurSel = m_listMenu.GetCurSel();
		nCurSel = m_listMenu.InsertString(nCurSel ,strText);
		m_listMenu.SetItemData(nCurSel, uCmdID);
	}
}

// メニューリストから削除する
void	CRightClickPropertyPage::OnRemove(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_btnApplyMenu.EnableWindow(TRUE);

	int nCurSel = m_listMenu.GetCurSel();
	m_listMenu.DeleteString(nCurSel);
	m_listMenu.SetCurSel(nCurSel);
}

// メニューリストにセパレーターを追加する
void	CRightClickPropertyPage::OnAddSeparator(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_btnApplyMenu.EnableWindow(TRUE);

	int nCurSel = m_listMenu.GetCurSel();
	int nIndex	= m_listMenu.InsertString(nCurSel, g_cSeparater);
	m_listMenu.SetItemData(nIndex, 0);
}

// 適用(メニュー)
void	CRightClickPropertyPage::OnApplyMenu(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_btnApplyMenu.EnableWindow(FALSE);

	CMenuHandle	menu;
	_CreateMenuFromMenuList(menu);

	int nIndex = m_cmbTarget.GetCurSel();
	DWORD dwID = (DWORD)m_cmbTarget.GetItemData(nIndex);
	SetContextMenuFromID(menu, dwID);
}

// リセット
void	CRightClickPropertyPage::OnMenuReset(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nCurSel = m_cmbTarget.GetCurSel();
	DWORD dwID = (DWORD)m_cmbTarget.GetItemData(nCurSel);
	switch (dwID) 
	{
	case CONTEXT_MENU_HOLDLEFTBUTTON:
		s_menuHoldLeftButton.DestroyMenu();
		s_menuHoldLeftButton.CreatePopupMenu();
		break;

	case CONTEXT_MENU_TABITEM:
		{
			s_menuTabItem.DestroyMenu();
			CMenuHandle menu;
			menu.LoadMenu(IDR_MENU_TAB);
			s_menuTabItem = menu.GetSubMenu(0);
			break;
		}
	default:
		{	/* デフォルトの設定を読み込み */
			CMenu menu;
			GetDefaultContextMenu(menu, dwID);

			SetContextMenuFromID(menu, dwID);
			menu.Detach();
		}
	}

	/* メニューリストに反映 */
	OnSelChangeTarget(0, 0, 0);
}

// [...]
void	CRightClickPropertyPage::OnExample(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nCurSel = m_cmbTarget.GetCurSel();
	DWORD dwID = (DWORD)m_cmbTarget.GetItemData(nCurSel);
	CMenuHandle menu = GetContextMenuFromID(dwID);

	CPoint	pt;
	::GetCursorPos(&pt);
	menu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
}











