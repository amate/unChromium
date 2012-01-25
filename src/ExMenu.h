/**
 *	@file	ExMenu.h
 *	@brief	�g�����j���[ : ���[�U�[��`���j���[�̂���
 */
#pragma once

#include "SimpleTree.h"
#include <vector>
#include "resource.h"
#include "DonutPFunc.h" 	//+++


class CExMenuItem {
public:
	CString 	strText;
	CString 	strCommand; 	//�R�}���h������
	CString 	strArg; 		//�R�}���h����
	int 		nID;			//���ڂ̃R�}���hID
	int 		nStyle; 		//���j���[���ڂ̃X�^�C�� ���L�萔�Q��
};


#define COMMAND_EXMENU_RANGE(min, max)												 \
	{																				 \
		if (uMsg == WM_COMMAND && LOWORD(wParam) >= min  && LOWORD(wParam) <= max) { \
			BOOL bRet = CExMenuManager::ExecCommand( (int) LOWORD(wParam) );		 \
			if (bRet)																 \
				return 0;															 \
		}																			 \
	}



typedef CSimpleTree<CExMenuItem>::NODEPTR  HMTREENODE;


///////////////////////////////////////////////////////
// CExMenuCtrl

class CExMenuCtrl 
{
private:
	CSimpleTree<CExMenuItem> *		m_pTree;

	//�f�[�^�s�̗v�f��
	enum EData_Count {
		DATA_COUNT_PARENT		= 2,
		DATA_COUNT_SEPARATOR	= 1,
		DATA_COUNT_ITEM 		= 4,
		DATA_COUNT_LEVEL		= 1,
	};


public:

	enum EExMenu_Style {
		EXMENU_STYLE_NORMAL 	= 0,
		EXMENU_STYLE_PARENT 	= 1,
		EXMENU_STYLE_SEPARATOR	= 2,
	};


	CExMenuCtrl(CSimpleTree<CExMenuItem> *pTree = NULL)
		: m_pTree(pTree)
	{
	}


	void	SetTree(CSimpleTree<CExMenuItem> *pTree) { m_pTree = pTree; }
	CSimpleTree<CExMenuItem>*	GetTree() { return m_pTree; }


	//�c���[�r���[����f�[�^�ǂݍ���
	BOOL LoadFromTreeView(CTreeViewCtrl &TreeC)
	{
		ATLASSERT(m_pTree);

		if ( !::IsWindow(TreeC.m_hWnd) )
			return FALSE;

		m_pTree->Clear();

		HTREEITEM hRoot = TreeC.GetRootItem();

		if (!hRoot)
			return TRUE;

		LoadFromTreeViewSub(NULL, hRoot, TreeC);

		return TRUE;
	}


private:
	BOOL LoadFromTreeViewSub(HMTREENODE hNode, HTREEITEM hItem, CTreeViewCtrl& TreeC)
	{
		//hItem�̎q�m�[�h��pNode�̎q�m�[�h�Ƃ��ēo�^����

		HTREEITEM hChild = TreeC.GetChildItem(hItem);
		while(hChild){
			//�f�[�^�̓c���[�r���[�̃A�v����`���玝���Ă���
			CExMenuItem *pExMenuItem = (CExMenuItem *)TreeC.GetItemData(hChild);
			if(pExMenuItem){
				HMTREENODE hNewNode = m_pTree->Insert(hNode,*pExMenuItem);
				LoadFromTreeViewSub(hNewNode,hChild,TreeC);
			}

			hChild = TreeC.GetNextSiblingItem(hChild);
		}

		return TRUE;
	}


public:
	//�f�[�^����c���[�r���[���\�z
	BOOL MakeTreeView(CTreeViewCtrl &TreeC)
	{
		ATLASSERT(m_pTree);

		if ( !::IsWindow(TreeC.m_hWnd) )
			return FALSE;

		TreeC.DeleteAllItems();

		HTREEITEM hRoot = TreeC.GetRootItem();
		//if(!hRoot) return TRUE;

		MakeTreeViewSub(NULL, hRoot, TreeC);

		return TRUE;
	}


private:
	BOOL MakeTreeViewSub(HMTREENODE hNode, HTREEITEM hItem, CTreeViewCtrl &TreeC)
	{
		HMTREENODE hChildNode = m_pTree->GetFirstChild(hNode);

		while (hChildNode) {
			CExMenuItem *pExMenuItem = (CExMenuItem *) m_pTree->GetData(hChildNode);

			if (pExMenuItem) {
				HTREEITEM hNewItem = TreeC.InsertItem(pExMenuItem->strText, hItem, TVI_LAST);

				if (hNewItem) {
					TreeC.SetItemData(hNewItem, (DWORD_PTR) pExMenuItem);
					MakeTreeViewSub(hChildNode, hNewItem, TreeC);
				}
			}

			hChildNode = m_pTree->GetNext(hChildNode);
		}

		return TRUE;
	}


public:
	//���j���[�\�z
	BOOL MakeMenu(HMENU& hMenuRoot)
	{
		ATLASSERT(m_pTree);

		if (!::IsMenu(hMenuRoot)) return FALSE;

		MakeMenuSub(NULL, hMenuRoot);

		return TRUE;
	}


private:
	BOOL MakeMenuSub(HMTREENODE hNode, HMENU hMenu)
	{
		HMTREENODE hChildNode = m_pTree->GetFirstChild(hNode);
		int 	   nIndex	  = 0;

		while (hChildNode) {
			CExMenuItem*	pItem = m_pTree->GetData(hChildNode);
			MENUITEMINFO	info  = { sizeof (MENUITEMINFO) };

			if (pItem->nStyle == EXMENU_STYLE_NORMAL) {
				//���ʂ̃��j���[����
				info.fMask		= MIIM_DATA | MIIM_ID | MIIM_TYPE;
				info.cch		= pItem->strText.GetLength() + 1;
				info.dwTypeData = pItem->strText.GetBuffer(0);
				info.wID		= pItem->nID;
				info.dwItemData = (DWORD_PTR) pItem;
				info.fType		= MFT_STRING;

				if ( !InsertMenuItem(hMenu, nIndex, TRUE, &info) )
					return FALSE;

			} else if (pItem->nStyle == EXMENU_STYLE_PARENT) {
				//�q�K�w�������j���[����
				info.fMask		= MIIM_DATA | MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
				info.cch		= pItem->strText.GetLength() + 1;
				info.dwTypeData = pItem->strText.GetBuffer(0);
				info.wID		= pItem->nID;
				info.dwItemData = (DWORD_PTR) pItem;
				info.fType		= MFT_STRING;
				//�T�u���j���[�̍쐬
				HMENU hSubMenu = ::CreatePopupMenu();
				info.hSubMenu	= hSubMenu;

				if ( !InsertMenuItem(hMenu, nIndex, TRUE, &info) )
					return FALSE;

				//�q�K�w�̍\�z
				if ( !MakeMenuSub(hChildNode, hSubMenu) )
					return FALSE;

			} else if (pItem->nStyle == EXMENU_STYLE_SEPARATOR) {
				//�Z�p���[�^
				info.fMask = MIIM_TYPE;
				info.fType = MFT_SEPARATOR;

				if ( !InsertMenuItem(hMenu, nIndex, TRUE, &info) )
					return FALSE;
			}

			hChildNode = m_pTree->GetNext(hChildNode);
			nIndex++;
		}

		return TRUE;
	}


public:
	//�t�@�C������f�[�^�ǂݍ���
	BOOL ReadData(CString &strFile)
	{
		ATLASSERT(m_pTree);
		m_pTree->Clear();

		FILE *fp   = _tfopen( strFile, _T("r") );
		if (fp == NULL)
			return FALSE;

		BOOL  bRet = ReadDataSub(fp, NULL);
		fclose(fp);
		return bRet;
	}


private:
	BOOL ReadDataSub(FILE *fp, HMTREENODE hNode)
	{
		//�܂����ʊK�w�ւ̈ړ��L��"��"��T��
		std::vector<CString> aryValue;
		CString 			 strBuf;
		BOOL				 bStarted = FALSE;
		int 				 nCount;

		while (1) {
			if ( !ReadString(fp, strBuf) )
				return FALSE;

			strBuf.TrimRight();

			//��s�ƃR�����g�͔�΂�
			if ( strBuf.IsEmpty() || strBuf.Left(2) == _T("//") )
				continue;

			if (!bStarted) {
				nCount	 = ParseDataString(strBuf, aryValue);

				if ( nCount != DATA_COUNT_LEVEL || aryValue[0] != _T("��") )
					continue;

				bStarted = TRUE;
			}

			if (bStarted) {
				nCount = ParseDataString(strBuf, aryValue);

				int		aryValue0 = _ttoi(aryValue[0]);		//+++ UNICODE�Ή�
				if (nCount == DATA_COUNT_LEVEL /*DATA_COUNT_SEPARATOR*/) {
					//��ʊK�w�ւ̈ړ��L��"��"����������I���
					if ( aryValue[0] == _T("��") )
						break;

					//�Z�p���[�^�ł���ꍇ
					//+++ if (atoi(aryValue[0]) == CExMenuCtrl::EXMENU_STYLE_SEPARATOR)
					if (aryValue0 == CExMenuCtrl::EXMENU_STYLE_SEPARATOR)	//+++ 	UNICODE�Ή�
					{
						CExMenuItem item;
						item.nStyle  = aryValue0;	//+++ item.nStyle  = atoi(aryValue[0]);	UNICODE�Ή�.
						HMTREENODE	hItem = m_pTree->Insert(hNode, item);

						if (!hItem)
							return FALSE;
					}
				} else if (nCount == DATA_COUNT_PARENT) {
					//�e���j���[�Ƃ��̉��ʍ��ڂ�o�^����

					CExMenuItem item;
					item.nStyle  = aryValue0;	//+++ item.nStyle  = atoi(aryValue[0]);	UNICODE�Ή�.
					item.strText = aryValue[1];
					HMTREENODE	hParent = m_pTree->Insert(hNode, item);

					if (!hParent)
						return FALSE;

					if ( !ReadDataSub(fp, hParent) )
						return FALSE;

				} else if (nCount >= DATA_COUNT_ITEM) {
					//����o�^���� ��̃R�s�y
					CExMenuItem item;
					item.nStyle		= aryValue0;					//+++ item.nStyle  = atoi(aryValue[0]);	UNICODE�Ή�.
					item.nID		= _ttoi(aryValue[1]);			//+++ item.nID		= atoi(aryValue[1]);
					item.strText	= aryValue[2];
					item.strCommand = aryValue[3];

					if (aryValue.size() > DATA_COUNT_ITEM) {
						for (int i = 4; i < (int) aryValue.size(); i++) {
							item.strArg += aryValue[i] + _T(" ");
						}
					}

					HMTREENODE	hItem = m_pTree->Insert(hNode, item);

					if (!hItem)
						return FALSE;
				} else {
					return FALSE;
				}
			}
		}

		return TRUE;
	}


	BOOL ReadString(FILE *fp, CString &strBuf)
	{
		char buf[0x4000+2];
		buf[0] = 0;	//+++
		if (fgets(buf, 0x4000, fp) == NULL) {
			return FALSE;
		}
		strBuf = buf;
		return TRUE;
	}


	//�������^�u�A�J���}����؂��vector�ɕ�������
	int ParseDataString(const CString &strLine, std::vector<CString> &vecText)
	{
		vecText.clear();

		int 	nCount = 0;
		CString strBuf, strText = strLine;
		strText.TrimLeft( _T(" \t,�@") );
		strText.TrimRight( _T(" \t,�@") );

		while ( !strText.IsEmpty() ) {
			int nPos = strText.FindOneOf( _T("\t,") );

			if (nPos == -1) {
				if ( !strText.IsEmpty() ) {
					vecText.push_back(strText);
					strText.Empty();
				}
			} else {
				strBuf	= strText.Left(nPos);
				strText = strText.Mid(nPos);
				vecText.push_back(strBuf);
			}

			nCount++;
			strText.TrimLeft(_T(" \t,�@"));
		}

		return nCount;
	}


public:
	//�t�@�C���փf�[�^�ۑ�
	BOOL SaveData(CString &strFile)
	{
		ATLASSERT(m_pTree);
		FILE *fp   = _tfopen( strFile, _T("w") );
		if (fp == NULL)
			return FALSE;

		BOOL  bRet = SaveSub(fp, NULL, -1);
		fclose(fp);
		return bRet;
	}


private:
	BOOL SaveSub(FILE *fp, HMTREENODE hNode, int nDepth)
	{
		SaveLevelMark(fp, TRUE, nDepth + 1);
		BOOL bRet = PutOutData(fp, hNode, nDepth + 1);
		SaveLevelMark(fp, FALSE, nDepth + 1);
		return bRet;
	}


	BOOL PutOutData(FILE *fp, HMTREENODE hNode, int nDepth)
	{
		HMTREENODE hChild = m_pTree->GetFirstChild(hNode);

		while (hChild) {
			if ( m_pTree->GetFirstChild(hChild) ) {
				//�q�m�[�h�����̂Ŏ���̏��������o������q�m�[�h�̏����o�͂���
				PutOutParentItemData(fp, *m_pTree->GetData(hChild), nDepth);
				SaveSub(fp, hChild, nDepth);
			} else {
				//�q�m�[�h�������Ȃ����j���[�������m�[�h
				PutOutItemData(fp, *m_pTree->GetData(hChild), nDepth);
			}

			hChild = m_pTree->GetNext(hChild);
		}

		return TRUE;
	}


	BOOL SaveLevelMark(FILE *fp, BOOL bDown, int nDepth)
	{
		CString		strOut;

		if (bDown) {
			strOut = _T("��");
		} else {
			strOut = _T("��");
		}

		strOut += _T("\n");

		CString 	strWhite;

		for (int i = 0; i < nDepth; i++)
			strWhite += _T("\t");

		strOut	= strWhite + strOut;

		return _fputts(strOut, fp) != EOF;		//+++ return fputs(strOut, fp) != EOF /*? TRUE : FALSE*/;	UNICODE�C��
	}


	BOOL PutOutParentItemData(FILE *fp, CExMenuItem &item, int nDepth)
	{
		CString 	strBuf;

		strBuf.Format(_T("%d\t%s\n"), item.nStyle, item.strText);

		CString strWhite;

		for (int i = 0; i < nDepth; i++)
			strWhite += _T("\t");

		strBuf = strWhite + strBuf;

	  #if 1	//+++ UNICODE�C��
		return _fputts(strBuf, fp) != EOF;
	  #else
		if (fputs(strBuf, fp) == EOF)
			return FALSE;

		return TRUE;
	  #endif
	}


	BOOL PutOutItemData(FILE *fp, CExMenuItem &item, int nDepth)
	{
		CString 	strBuf;

		strBuf.Format(_T("%d\t%d\t%s\t%s\n"), item.nStyle, item.nID, item.strText, item.strCommand);

		CString 	strWhite;

		for (int i = 0; i < nDepth; i++)
			strWhite += _T("\t");

		strBuf = strWhite + strBuf;

		if (_fputts(strBuf, fp) == EOF)
			return FALSE;

		return TRUE;
	}

};



class CExMenuManager {
public:
	enum EExMenu {
		EXMENU_ID_FIRST 		= 0,
		//EXMENU_ID_ADDRESSBAR	= 1,
		EXMENU_COUNT			= 1,
	};

	static CSimpleArray<CSimpleTree<CExMenuItem> *> *	s_aryTree;

	static void Initialize()
	{
		static LPCTSTR	EXMENU_FILENAME[]	= { _T("usermenu.txt")		  , NULL };


		ATLASSERT(!s_aryTree);
		s_aryTree = new CSimpleArray<CSimpleTree<CExMenuItem> *>();

		CExMenuCtrl menuC;

		for (int i = 0; i < EXMENU_COUNT; i++) {
			CSimpleTree<CExMenuItem> *pTree 	  = new CSimpleTree<CExMenuItem>();
			menuC.SetTree(pTree);

			if (EXMENU_FILENAME[i] == NULL)
				break;

			CString 	strFileName = _GetFilePath( _T("menu\\") ) + EXMENU_FILENAME[i];
			menuC.ReadData(strFileName);
			s_aryTree->Add(pTree);
		}
	}


	static void Terminate()
	{
		ATLASSERT(s_aryTree);
		if (s_aryTree == 0) 	//+++ �O�̂��ߒe��.
			return;
		int nCount = s_aryTree->GetSize();

		for (int i = 0; i < nCount; i++) {
			delete (*s_aryTree)[i];
		}

		delete s_aryTree;
		s_aryTree = NULL;		//+++ �폜��͋����N���A���Ƃ�.
	}


	//�t�����j���[�i���C�����j���[+���ꃁ�j���[+�g�����j���[�j�����[�h����
	//�󂯂�HMENU�͕K��DestroyMenu���邱��
	static HMENU LoadFullMenu()
	{
		static LPCTSTR	EXMENU_NAME[]		= { _T("���[�U�[��`���j���["), NULL };
		CMenuHandle 	menu, menuSpc;

		menu.LoadMenu(IDR_MAINFRAME);

		//���ꃁ�j���[�̒ǉ�
		menuSpc.LoadMenu(IDR_SPECIAL);
		menu.AppendMenu( MF_POPUP, (UINT_PTR) menuSpc.m_hMenu, _T("����") );

		//�g�����j���[�̒ǉ�
		int 		nExMenuCount = s_aryTree->GetSize();
		CExMenuCtrl menuC;
		CMenuHandle menuEx;

		for (int i = 0; i < nExMenuCount; i++) {
			menuC.SetTree( (*s_aryTree)[i] );
			menuEx.CreateMenu();

			if ( menuC.MakeMenu(menuEx.m_hMenu) ) {
				menu.AppendMenu(MF_POPUP, (UINT_PTR) menuEx.m_hMenu, EXMENU_NAME[i]);
			}
		}

		return menu.m_hMenu;
	}


	static HMENU LoadExMenu(int nID)
	{
		CMenuHandle menu;
		CExMenuCtrl menuC( (*s_aryTree)[nID] );

		menu.CreatePopupMenu();
		menuC.MakeMenu(menu.m_hMenu);
		return menu.m_hMenu;
	}


	static HMTREENODE FindFromID(int nCmdID, CSimpleTree<CExMenuItem> *pTree)
	{
		return FindFromIDSub(NULL, nCmdID, pTree);
	}


	static HMTREENODE FindFromIDSub(HMTREENODE hParent, int nCmdID, CSimpleTree<CExMenuItem> *pTree)
	{
		HMTREENODE hChild = pTree->GetFirstChild(hParent);

		while (hChild) {
			CExMenuItem *pItem = pTree->GetData(hChild);

			if (pItem) {
				if (pItem->nStyle == CExMenuCtrl::EXMENU_STYLE_NORMAL && pItem->nID == nCmdID) {
					return hChild;
				} else if (pItem->nStyle == CExMenuCtrl::EXMENU_STYLE_PARENT) {
					HMTREENODE hTarget = FindFromIDSub(hChild, nCmdID, pTree);

					if (hTarget)
						return hTarget;
				}
			}

			hChild = pTree->GetNext(hChild);
		}

		return NULL;
	}


	static CString GetToolTip(int nCmdID)
	{
		if (EXMENU_USERDEFINE_MIN <= nCmdID && nCmdID <= EXMENU_USERDEFINE_MAX) {
			CSimpleTree<CExMenuItem> *pTree = (*s_aryTree)[EXMENU_ID_FIRST];
			HMTREENODE				  hNode = FindFromID(nCmdID, pTree);

			if (hNode) {
				CExMenuItem *pItem = pTree->GetData(hNode);
				return pItem->strText;
			}
		}

		return CString();
	}


	static BOOL ExecCommand(int nID)
	{
	  #if 1 //*+++ ���������ϐ�������...��0���ƃ}�Y�C? �Ƃ肠�����A��������-1
		int 	nIndex = -1;
	  #endif

		if (EXMENU_USERDEFINE_MIN <= nID && nID <= EXMENU_USERDEFINE_MAX) {
			nIndex = EXMENU_ID_FIRST;
		}

		if (nIndex < 0 || EXMENU_COUNT <= nIndex)
			return FALSE;

		CSimpleTree<CExMenuItem>*	pTree  = (*s_aryTree)[nIndex];
		if (!pTree)
			return FALSE;

		HMTREENODE		hNode  = FindFromID(nID, pTree);
		if (!hNode)
			return FALSE;

		CExMenuItem *	pItem  = pTree->GetData(hNode);
		if (!pItem)
			return FALSE;
		if (pItem->strCommand.Left(6).CompareNoCase(_T("script")) == 0) 
			pItem->strCommand.Insert(0, Misc::GetExeDirectory());	//\\+
		::ShellExecute(NULL, _T("open"), pItem->strCommand, pItem->strArg, Misc::GetExeDirectory(), SW_SHOWNORMAL);
		return TRUE;
	}
};



__declspec(selectany) CSimpleArray<CSimpleTree<CExMenuItem> *> *	CExMenuManager::s_aryTree = NULL;
