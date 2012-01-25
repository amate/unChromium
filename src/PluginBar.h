/**
 *	@file	PluginBar.h
 *	@brief	�v���O�C���o�[
 */

#pragma once

#include "./include/PluginInfo.h"
#include "ChevronHandler.h"

#include "PluginManager.h"
#include "MtlWin.h"
#include "MtlCtrls.h"
#include "IniFile.h"
#include "DonutPFunc.h"
#include "resource.h"


class CPluginBar
	: public CWindowImpl<CPluginBar>
	, public CChevronHandler<CPluginBar>
{
private:
	//typedef CWindowImpl<CPluginBar> baseClass;

	//�����o�ϐ�
	HWND			m_hWndCur;		//���\�����Ă���v���O�C���̃E�B���h�E�n���h��
	int 			m_nCurIndex;	//���̃v���O�C���̃C���f�b�N�X�ԍ�

	CReBarCtrl		m_wndReBar; 	//���o�[(�c�[���o�[������Ȃ�����s�v����)
	CToolBarCtrl	m_wndToolBar;	//�c�[���o�[


public:
	DECLARE_WND_CLASS_EX(_T("Donut_PluginBar"), CS_DBLCLKS, COLOR_WINDOW)

	//�R���X�g���N�^
	CPluginBar()
		: m_hWndCur(NULL)
		, m_nCurIndex(-1)
	{
	}

private:
	//���\�b�h

	/// �v���O�C����\��(�Ȃ���΍��)
	///
	void ShowBar(int nIndex)
	{
		if (CPluginManager::GetCount(PLT_EXPLORERBAR) == 0)
			return;

		if (m_nCurIndex == nIndex)
			return;

		if ( !IsWindowVisible() )
			ShowWindow(SW_SHOWNORMAL);

		HWND hWndNew	= CPluginManager::GetHWND(PLT_EXPLORERBAR, nIndex);

		if (!hWndNew) {
			//�Ȃ��̂ō��
			if ( !CPluginManager::LoadPlugin(PLT_EXPLORERBAR, nIndex, m_hWnd) )
				return;

			hWndNew = CPluginManager::GetHWND(PLT_EXPLORERBAR, nIndex);

			if (!hWndNew)
				return;
		}

		HWND hWndBefore = m_hWndCur;

		//��ԕϐ���ύX
		m_nCurIndex = nIndex;
		m_hWndCur	= hWndNew;

		//���T�C�Y���ĕ\��
		UpdateLayout();
		::ShowWindow(m_hWndCur, SW_SHOWNORMAL);

		//���\�����Ă���v���O�C�����B��
		if ( ::IsWindow(hWndBefore) ) {
			::ShowWindow(hWndBefore, SW_HIDE);
		}

		//�t�H�[�J�X���ڂ�
		::SetFocus(hWndNew);
	}


	void HideBar()
	{
		ShowWindow(SW_HIDE);
	}


public:
	BYTE PreTranslateMessage(MSG *pMsg)
	{
		//�L�[�֘A�̃��b�Z�[�W��]��
		if ( (WM_KEYFIRST <= pMsg->message) && (pMsg->message <= WM_KEYLAST) ) {
			int nCount = CPluginManager::GetCount(PLT_EXPLORERBAR);

			for (int i = 0; i < nCount; i++) {
				if ( CPluginManager::Call_PreTranslateMessage(PLT_EXPLORERBAR, i, pMsg) )
					return _MTL_TRANSLATE_HANDLE;
			}
		}

		return _MTL_TRANSLATE_PASS;
	}


	void ReloadSkin()
	{
		if ( !m_wndToolBar.IsWindow() )
			return;

		CImageList imgs, imgsHot;
		imgs	= m_wndToolBar.GetImageList();
		imgsHot = m_wndToolBar.GetHotImageList();

		_ReplaceImageList(GetSkinFavBarPath(FALSE), imgs);
		_ReplaceImageList(GetSkinFavBarPath(TRUE) , imgsHot);
		m_wndToolBar.InvalidateRect(NULL		  , TRUE);
	}


public:
	//���b�Z�[�W�}�b�v
	BEGIN_MSG_MAP(CPluginBar)
		MESSAGE_HANDLER(WM_CREATE		, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY		, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE 		, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND	, OnEraseBackGround)
		NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnTbnDropDown)
		//CHAIN_MSG_MAP(baseclass)
		CHAIN_MSG_MAP(CChevronHandler<CPluginBar>)
	END_MSG_MAP()


private:
	//���b�Z�[�W�n���h��
	LRESULT CPluginBar::OnCreate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL &bHandled)
	{
		bHandled = FALSE;
		CString 	strSection;
		strSection.Format(_T("Plugin%02d"), PLT_EXPLORERBAR);
		DWORD		dwIndex = 0;
		{
			CIniFileI pr(g_szIniFileName, strSection);
			pr.QueryValue( dwIndex, _T("SelIndex") );
			//x pr.Close(); //++++
		}

		InitToolBar();
		CPluginManager::ReadPluginData(PLT_EXPLORERBAR, m_hWnd); //�v���O�C�����ǂݍ���
	  #if 1	//+++
		CPluginManager::ReadPluginData(PLT_DOCKINGBAR , m_hWnd); //�v���O�C�����ǂݍ���
	  #endif
		ShowBar(dwIndex);

		return 0;
	}


	LRESULT CPluginBar::OnDestroy(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL &bHandled)
	{
		bHandled = FALSE;
		//�v���O�C���̌�Еt����CMainFrame�ňꊇ���čs��
		//CPluginManager::DeleteAllPlugin(PLT_EXPLORERBAR); //�v���O�C�����

		CString 	strKey;
		strKey.Format(_T("Plugin%02d"), PLT_EXPLORERBAR);
		CIniFileO	pr( g_szIniFileName, strKey);
		pr.SetValue( m_nCurIndex, _T("SelIndex") );
		//x pr.Close(); 	//+++

		return 0;
	}


	LRESULT CPluginBar::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & /*bHandled*/)
	{
		UpdateLayout();
		return 0;
	}


	LRESULT CPluginBar::OnEraseBackGround(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
	{
		if ( !::IsWindow(m_hWndCur) )
			return DefWindowProc(uMsg, wParam, lParam);

		return 1;
	}


	//�����֐�
	void CPluginBar::UpdateLayout()
	{
		CRect rc;

		GetClientRect(&rc);
		UpdateLayout( rc.Width(), rc.Height() );
	}


	void CPluginBar::UpdateLayout(int nWidth, int nHeight)
	{
		CRect rcToolBar;

		m_wndReBar.GetClientRect(&rcToolBar);
		m_wndReBar.MoveWindow( 0, 0, nWidth, rcToolBar.Height() );

		int   nOffsetY = rcToolBar.Height();
		::MoveWindow(m_hWndCur, 0, nOffsetY, nWidth, nHeight - nOffsetY, TRUE);	
	}


	void CPluginBar::DelIniKey(CString strKey)
	{
		CString 	strSection;

		strSection.Format( _T("Plugin%02d"), PLT_EXPLORERBAR );

		CIniFileO	pr( g_szIniFileName, strSection );
		pr.DeleteValue( strKey );
		//x pr.Close(); 	//+++
	}


	void CPluginBar::InitToolBar()
	{
		int 	cx , cy;
		int 	nFlags	= ILC_COLOR24 | ILC_MASK;
		cx	 = cy = 16;

		m_wndReBar = _CreateSimpleReBarCtrl(m_hWnd, ATL_SIMPLE_REBAR_NOBORDER_STYLE | CCS_NOPARENTALIGN);

		CImageList	imgs;
		CImageList	imgsHot;
		imgs.Create   (cx, cy, nFlags , 1, 1);
		imgsHot.Create(cx, cy, nFlags , 1, 1);

		m_wndToolBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0);
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
		ATLASSERT( m_wndToolBar.IsWindow() );
		m_wndToolBar.SetButtonStructSize( sizeof (TBBUTTON) );

		m_wndToolBar.SetImageList(imgs);
		m_wndToolBar.SetHotImageList(imgsHot);

		TBBUTTON	btnPlg	= { 4		   , ID_PLUGIN_PLACEMENT , TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN, 0, 0 };
		TBBUTTON	btnMenu = { I_IMAGENONE, ID_PLUGIN_MENU 	 , TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN, 0, 0 };
		m_wndToolBar.InsertButton(-1, &btnPlg);
		m_wndToolBar.InsertButton(-1, &btnMenu);

		CVersional<TBBUTTONINFO> bi;
		bi.dwMask  = TBIF_TEXT;
		bi.pszText = _T("���X�g");
		MTLVERIFY( m_wndToolBar.SetButtonInfo(ID_PLUGIN_PLACEMENT, &bi) );
		bi.pszText = _T("���j���[");
		MTLVERIFY( m_wndToolBar.SetButtonInfo(ID_PLUGIN_MENU, &bi) );

		_AddSimpleReBarBandCtrl(m_wndReBar, m_wndToolBar);

		ReloadSkin();
	}


	CString CPluginBar::GetSkinFavBarPath(BOOL bHot)
	{
		CString strBmp;

		if (bHot)
			strBmp = _T("FavBarHot.bmp");
		else
			strBmp = _T("FavBar.bmp");

		return _GetSkinDir() + strBmp;
	}


	LRESULT CPluginBar::OnTbnDropDown(int idCtrl, LPNMHDR pnmh, BOOL &bHandled)
	{
		LPNMTOOLBAR lpnmtb = (LPNMTOOLBAR) pnmh;

		int 		nID    = lpnmtb->iItem;
		int 		index  = -1;
		int 		nCount = m_wndToolBar.GetButtonCount();

		for (int i = 0; i < nCount; i++) {
			TBBUTTON tb;
			m_wndToolBar.GetButton(i, &tb);

			if (nID == tb.idCommand) {
				index = i;
				break;
			}
		}

		if (index == -1)
			return 0;

		CRect		rc;
		m_wndToolBar.GetItemRect(index, &rc);
		ClientToScreen(&rc);

		if (nID == ID_PLUGIN_PLACEMENT) {
			CMenu menu		   = GetPluginListMenu();
			int   nNewSelIndex = menu.TrackPopupMenu(
										TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
										rc.left   + 1,
										rc.bottom + 2,
										m_hWnd );

			if (nNewSelIndex > 0)
				ShowBar(nNewSelIndex - 1);
		} else if (nID == ID_PLUGIN_MENU) {
			ShowExplorerMenu(rc.left + 1, rc.bottom + 2);
		}

		return TBDDRET_DEFAULT;
	}


	CMenuHandle CPluginBar::GetPluginListMenu()
	{
		CMenuHandle 	menu;
		menu.CreatePopupMenu();

		CString 	strCap;
		int 		nCount = CPluginManager::GetCount(PLT_EXPLORERBAR);
		for (int i = 0; i < nCount; i++) {
			strCap = CPluginManager::GetCaption(PLT_EXPLORERBAR, i);
			menu.AppendMenu(MF_ENABLED | MF_STRING, i + 1, strCap);
		}
		menu.CheckMenuItem(m_nCurIndex, MF_BYPOSITION | MF_CHECKED);

		return menu;
	}


	void CPluginBar::ShowExplorerMenu(int x, int y)
	{
		if (m_nCurIndex == -1)
			return;
		CPluginManager::Call_ShowExplorerMenu(m_nCurIndex, x, y);
	}

};
