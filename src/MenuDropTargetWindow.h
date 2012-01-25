/**
 *	@file	MenuDropTargetWindow.h
 *	@brief	���j���[�ł̃h���b�O���h���b�v����(IE���C�ɓ���̕��בւ�����)
 */
#pragma once

#include "FavoriteOrder.h"
//#include "DonutExplorerBar.h"		//+++ ���C�ɓ��胁�j���[�ňړ������������Ƃ��A���C�ɓ���o�[���ɂ����f
#include "Donut.h"
#include "ParseInternetShortcutFile.h"

template <class T>
class ATL_NO_VTABLE 	CMenuDropTargetWindowImpl
	: public CWindowImpl<T>
	, public IDropTargetImpl<T>
	, public IDropSourceImpl<T>
{
	enum {
		MNGOF_CENTERGAP 	= 0,
	};

	HMENU			m_hDDMenu;
	bool			m_bDragAccept;
	//+++ char		_text[MAX_PATH];		//+++ ���g�p

	struct DragMenuData {
		int 		nPos;
		DWORD		dwPos;
		HMENU		hMenu;
		CString 	strFilePath;
		int 		nDir;
		BOOL		bBottom;
		HWND		hWndParent;		//+++ �e

	public:
		DragMenuData()
			: bBottom(FALSE)
			, hMenu(NULL)
		  #if 1 //*+++
			, nPos(0)
			, dwPos(0)
			, nDir(0)
			, hWndParent(0)	//+++
		  #endif
		{
		}
	};

	DragMenuData	m_SrcData;
	DragMenuData	m_DstData;


public:
	CMenuDropTargetWindowImpl()
		: m_hDDMenu(NULL)
	  #if 1 	//*+++
		, m_bDragAccept(0)
	  #endif
	{
		//+++ _text[0] = 0;		//+++���g�p
	}


	void SetTargetMenu(HMENU hMenu)
	{
		m_hDDMenu	 = hMenu;
		MENUINFO		info			= { sizeof (MENUINFO) };
		info.fMask	 = MIM_STYLE | MIM_APPLYTOSUBMENUS;
		info.dwStyle = MNS_DRAGDROP;
		::SetMenuInfo(hMenu, &info);
		HMENU hSubMenu = ::GetSubMenu(hMenu, 2);

		if (hSubMenu) {
			info.dwStyle = 0;
			::SetMenuInfo(hSubMenu, &info);
		}
	}


	BEGIN_MSG_MAP(CMenuDropTargetWindowImpl)
		MESSAGE_HANDLER(WM_CREATE		, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY		, OnDestroy)
		MESSAGE_HANDLER(WM_MENUDRAG 	, OnMenuDrag)
		MESSAGE_HANDLER(WM_MENUGETOBJECT, OnMenuGetObject)
	END_MSG_MAP()


	DROPEFFECT OnDragEnter(IDataObject *pDataObject, DWORD dwKeyState, CPoint point)
	{
		m_bDragAccept = _MtlIsHlinkDataObject(pDataObject);
		return _MtlStandardDropEffect(dwKeyState);
	}


	DROPEFFECT OnDragOver(IDataObject *pDataObject, DWORD dwKeyState, CPoint point, DROPEFFECT dropOkEffect)
	{
		if (!m_bDragAccept)
			return DROPEFFECT_NONE;

		return _MtlStandardDropEffect(dwKeyState) | _MtlFollowDropEffect(dropOkEffect) | DROPEFFECT_COPY;
	}


	DROPEFFECT OnDrop(IDataObject *pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropEffectList, CPoint point)
	{
		CString strText;

		if (  MtlGetHGlobalText( pDataObject, strText)
		   || MtlGetHGlobalText( pDataObject, strText, ::RegisterClipboardFormat(CFSTR_SHELLURL) ) )
		{
			//�h���b�v����
			if (MoveItemOrder()) {							//���j���[���ڂ̈ړ�
				//+++ ���C�ɓ��胁�j���[�ňړ������Ƃ��A���C�ɓ���o�[���X�V�����悤�ɂ��Ƃ�
				// CDonutExplorerBar::GetInstance()->RefreshExpBar(0);
				Donut_ExplorerBar_RefreshFavoriteBar();		//����
			}
			return DROPEFFECT_NONE;
		}

		return DROPEFFECT_NONE;
	}



	LRESULT OnMenuDrag(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if (CFavoritesMenuOption::s_bCstmOrder == false) return MND_ENDMENU;

		int 					nPos		= (int) wParam;
		CComPtr<IDataObject>	spDataObject;

		HRESULT hr	= CHlinkDataObject::_CreatorClass::CreateInstance(NULL,IID_IDataObject, (void **) &spDataObject);
		if ( FAILED(hr) )
			return MND_CONTINUE;

		CHlinkDataObject *	 pHlinkDataObject = NULL;	//�����[�X�̕K�v�Ȃ�

		hr		  = spDataObject->QueryInterface(IID_NULL, (void **) &pHlinkDataObject);
		if ( FAILED(hr) )
			return MND_CONTINUE;

		HMENU			hMenu	= (HMENU) lParam;
		MENUITEMINFO	mii 	= { sizeof (MENUITEMINFO) };
		mii.fMask = MIIM_DATA;

		if ( !::GetMenuItemInfo(hMenu, nPos, TRUE, &mii) )
			return MND_CONTINUE;

		CMenuItem * 	 pmi	= (CMenuItem *) mii.dwItemData;

		if (!pmi)
			return MND_CONTINUE;

		CString 		strFileName   = pmi->m_strText;
		CString 		strUrl		  = pmi->m_strPath;

		MTL::ParseInternetShortcutFile(strUrl);

		pHlinkDataObject->m_arrNameAndUrl.Add( std::make_pair(strFileName, strUrl) );

		if ( PreDoDragDrop(m_hWnd) && CheckFolder(pmi->m_strPath) ) {
			//�ʒu�i���j��ۑ�
			HMENU	   hMenu	  = (HMENU) lParam;
			int 	   nOffSet;

			if ( !GetPosOffset(hMenu, nOffSet) )
				return MND_CONTINUE;

			m_SrcData.nPos		  = nPos;
			m_SrcData.dwPos 	  = nPos - nOffSet;
			m_SrcData.hMenu 	  = hMenu;
			m_SrcData.strFilePath = pmi->m_strPath;
			m_SrcData.hWndParent  = GetParent();		//++++

			MtlRemoveTrailingBackSlash(m_SrcData.strFilePath);
			//�h���b�O�J�n
			DROPEFFECT dropEffect = DoDragDrop(spDataObject, DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK);

			if (dropEffect != DROPEFFECT_NONE)
				return MND_ENDMENU;
		}

		return MND_CONTINUE;
	}


	LRESULT OnMenuGetObject(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		MENUGETOBJECTINFO *pInfo = (MENUGETOBJECTINFO *) lParam;

		//pInfo��dwFlags��MNGOF_CENTERCGAP(0)�̂Ƃ��̓��j���[�̒������ɃJ�[�\�������邱�Ƃ��Ӗ����邪
		//����ȊO�̎��̓��j���[�̏㕔�ɂ��邱�Ƃ������BMSDN�ɂ͗�ɂ���ĉR�������Ă���B

	  #if 1	//+++	�t�H���_�Ԉړ�������
		if (CheckMenuPosition(pInfo->hmenu, pInfo->uPos)) {
			CComPtr<IDropTarget> pTarget;
			_LocDTQueryInterface(IID_IDropTarget, (void **) &pTarget);

			if (!pTarget)
				return MNGO_NOINTERFACE;

			if (pInfo->hmenu == m_SrcData.hMenu) {		 //�܂��t�H���_�Ԉړ��̓T�|�[�g���Ȃ�
				//�������g�͎󂯕t���Ȃ�
				if (pInfo->uPos == m_SrcData.nPos && m_SrcData.hMenu == pInfo->hmenu)
					return MNGO_NOINTERFACE;

				if (pInfo->uPos == m_SrcData.nPos + 1 && pInfo->dwFlags != MNGOF_CENTERGAP)
					return MNGO_NOINTERFACE;

				if (pInfo->dwFlags != MNGOF_CENTERGAP) {
					if (m_SrcData.nPos < (int) pInfo->uPos)
						pInfo->uPos--;
				}
			}

			//�ʒu�i��j��ۑ�
			int 	nOffSet;
			if ( !GetPosOffset(pInfo->hmenu, nOffSet) )
				return MNGO_NOINTERFACE;

			if ( (int) pInfo->uPos - nOffSet < 0 )
				pInfo->uPos++;

			m_DstData.nPos			= pInfo->uPos;
			m_DstData.dwPos			= pInfo->uPos - nOffSet;
			m_DstData.hMenu			= pInfo->hmenu;
			m_DstData.hWndParent	= GetParent();		//++++

			CMenuItem * 	 pItem	= GetMenuData(pInfo->hmenu, pInfo->uPos);

			if (pItem)
				m_DstData.strFilePath = pItem->m_strPath;
			else
				m_DstData.strFilePath = _T("");

			m_DstData.nDir	= pInfo->dwFlags;

			if (::GetMenuItemCount(pInfo->hmenu) == pInfo->uPos) {
				m_DstData.bBottom	  = TRUE;
				m_DstData.strFilePath = MtlGetDirectoryPath( GetDirPathForBottom(pInfo->hmenu, pInfo->uPos) );
			} else {
				m_DstData.bBottom = FALSE;
			}

			MtlRemoveTrailingBackSlash(m_DstData.strFilePath);

			pInfo->pvObj	= pTarget;
			return MNGO_NOERROR;
		}
		return MNGO_NOINTERFACE;
	  #else
		if (CheckMenuPosition(pInfo->hmenu, pInfo->uPos)
		   && pInfo->hmenu == m_SrcData.hMenu)		 //�܂��t�H���_�Ԉړ��̓T�|�[�g���Ȃ�
		{
			CComPtr<IDropTarget> pTarget;
			_LocDTQueryInterface(IID_IDropTarget, (void **) &pTarget);

			if (!pTarget)
				return MNGO_NOINTERFACE;

			//�������g�͎󂯕t���Ȃ�
			if (pInfo->uPos == m_SrcData.nPos && m_SrcData.hMenu == pInfo->hmenu)
				return MNGO_NOINTERFACE;

			if (pInfo->uPos == m_SrcData.nPos + 1 && pInfo->dwFlags != MNGOF_CENTERGAP)
				return MNGO_NOINTERFACE;

			if (pInfo->dwFlags != MNGOF_CENTERGAP) {
				if (m_SrcData.nPos < (int) pInfo->uPos)
					pInfo->uPos--;
			}

			//�ʒu�i��j��ۑ�
			int 	nOffSet;
			if ( !GetPosOffset(pInfo->hmenu, nOffSet) )
				return MNGO_NOINTERFACE;

			if ( (int) pInfo->uPos - nOffSet < 0 )
				pInfo->uPos++;

			m_DstData.nPos	= pInfo->uPos;
			m_DstData.dwPos = pInfo->uPos - nOffSet;
			m_DstData.hMenu = pInfo->hmenu;
			CMenuItem * 		 pItem = GetMenuData(pInfo->hmenu, pInfo->uPos);

			if (pItem)
				m_DstData.strFilePath = pItem->m_strPath;
			else
				m_DstData.strFilePath = _T("");

			m_DstData.nDir	= pInfo->dwFlags;

			if (::GetMenuItemCount(pInfo->hmenu) == pInfo->uPos) {
				m_DstData.bBottom	  = TRUE;
				m_DstData.strFilePath = MtlGetDirectoryPath( GetDirPathForBottom(pInfo->hmenu, pInfo->uPos) );
			} else {
				m_DstData.bBottom = FALSE;
			}

			MtlRemoveTrailingBackSlash(m_DstData.strFilePath);

			pInfo->pvObj	= pTarget;
			return MNGO_NOERROR;
		}
		return MNGO_NOINTERFACE;
	  #endif
	}


private:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		RegisterDragDrop();
		return 0;
	}


	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		RevokeDragDrop();
		return 0;
	}


	BOOL CheckFolder(const CString& strDirPath) const
	{
		//���[�U�[��`�̏ꍇ�͓����Ȃ�
		if ( _check_flag(CFavoritesMenuOption::s_dwStyle, EMS_USER_DEFINED_FOLDER) )
			return FALSE;

		//���C�ɓ���t�H���_�ȉ��łȂ��ꍇ�������Ȃ�
		CString 	strFavRoot;
		if ( !MtlGetFavoritesFolder(strFavRoot) )
			return FALSE;

		if (strDirPath.Find(strFavRoot) == -1)
			return FALSE;

		//IE�̕��я��łȂ��ꍇ�������Ȃ�
		if (CFavoritesMenuOption::s_bIEOrder == false)
			return FALSE;

		return TRUE;
	}


	CString GetDirPathForBottom(HMENU hMenu, UINT uPos)
	{
		MENUITEMINFO mii = { sizeof (MENUITEMINFO) };

		mii.fMask = MIIM_DATA;

		for (int i = uPos - 1; i >= 0; i--) {
			if ( !::GetMenuItemInfo(hMenu, i, TRUE, &mii) )
				return _T("");

			CMenuItem *pItem = (CMenuItem *) mii.dwItemData;

			if (pItem)
				return pItem->m_strPath;
		}

		return _T("");
	}


	CMenuItem *GetMenuData(HMENU hMenu, UINT uPos)
	{
		MENUITEMINFO	mii = { sizeof (MENUITEMINFO) };
		mii.fMask = MIIM_TYPE | MIIM_DATA;

		if ( !::GetMenuItemInfo(hMenu, uPos, TRUE, &mii) )
			return NULL;

		return (CMenuItem *) mii.dwItemData;
	}


	BOOL GetPosOffset(HMENU hMenu, int &nPosOffset)
	{
		MENUITEMINFO mii	= { sizeof (MENUITEMINFO) };

		mii.fMask = MIIM_TYPE | MIIM_DATA;
		int 		 nCount = ::GetMenuItemCount(hMenu);

		for (int i = 0; i < nCount; i++) {
			if ( !::GetMenuItemInfo(hMenu, i, TRUE, &mii) )
				return FALSE;

			CMenuItem *pItem = (CMenuItem *) mii.dwItemData;

			if (pItem) {
				nPosOffset = i;
				return TRUE;
			}
		}

		return FALSE;
	}


	BOOL CheckMenuPosition(HMENU hMenu, UINT uPos)
	{
		MENUITEMINFO mii	= { sizeof (MENUITEMINFO) };

		mii.fMask = MIIM_TYPE | MIIM_DATA;
		int 		 nCount = ::GetMenuItemCount(hMenu);

		if (nCount == uPos)
			return TRUE;

		//uPos�Ԗڂ̍��ڂ��`�F�b�N:�Z�p���[�^�����ɂ���ꍇ��FALSE;
		if ( !::GetMenuItemInfo(hMenu, uPos, TRUE, &mii) )
			return FALSE;

		CMenuItem *  pItem	= (CMenuItem *) mii.dwItemData;

		if (!pItem || mii.fType == MFT_SEPARATOR /*|| !MtlIsInternetFile(pItem->m_strPath)*/)
			return FALSE;

		return TRUE;
	}


	///+++ ���ڈړ�
	bool	MoveItemOrder()
	{
		if (CFavoritesMenuOption::s_bCstmOrder == false) return false;

		CString strSrcDir = MtlGetDirectoryPath(m_SrcData.strFilePath);
		if (m_SrcData.bBottom)	//+++ ���j���[�̈�ԍŌ�̏ꍇ�́A����url�̃t�@�C�����łȂ��A���̃t�H���_���݂̂������Ă���.
			strSrcDir = m_SrcData.strFilePath;
		CString strDstDir = MtlGetDirectoryPath(m_DstData.strFilePath);
		if (m_DstData.bBottom)	//+++ ���j���[�̈�ԍŌ�̏ꍇ�́A����url�̃t�@�C�����łȂ��A���̃t�H���_���݂̂������Ă���.
			strDstDir = m_DstData.strFilePath;

		if (m_SrcData.hMenu == m_DstData.hMenu) {
			//���f�B���N�g�����ړ�
			CFavoriteOrderHandler	order;
			order.ReadData(strSrcDir);

			//�f�[�^�̃`�F�b�N:������΍č\�z�����āA���������؂�m�F����
			if (GetOrderItemIndex(order, m_SrcData) == -1)
				return false;

			if (!m_DstData.bBottom && GetOrderItemIndex(order, m_DstData) == -1)
				return false;

			//�ړ����ĕۑ�
			if (m_DstData.bBottom)
				m_DstData.dwPos--;

			order.MoveData(m_SrcData.dwPos, m_DstData.dwPos);
			order.SaveData();

			//���\�����Ă��郁�j���[���̂̕��בւ�
			if (m_DstData.bBottom)
				m_DstData.nPos--;

			MoveMenuItems(m_SrcData.hMenu, m_SrcData.nPos, m_DstData.nPos);
			::InvalidateRect( (HWND) m_DstData.hMenu, NULL, TRUE );
			::DrawMenuBar( GetParent() );
		} else {
			//�قȂ�f�B���N�g���Ԃ̈ړ�
		  #if 1	//+++	���Ȃ薳�����...
			CFavoriteOrderHandler	srcOrder;
			srcOrder.ReadData(strSrcDir);
			CFavoriteOrderHandler	dstOrder;
			dstOrder.ReadData(strDstDir);
			if (strSrcDir == strDstDir)
				return false;

			//�f�[�^�̃`�F�b�N:������΍č\�z�����āA���������؂�m�F����
			if (m_SrcData.bBottom)
				;
			else if (GetOrderItemIndex(srcOrder, m_SrcData) == -1)
				return false;

			//�ړ����ĕۑ�
			if (m_DstData.bBottom)
				;	//--m_DstData.dwPos;
			else if (GetOrderItemIndex(dstOrder, m_DstData) == -1)
				return false;

		  #if 1	//+++
			dstOrder.MoveData(srcOrder, m_SrcData.strFilePath, m_DstData.dwPos);
		  #elif 1
			CString name     = GetFileBaseName(m_SrcData.strFilePath);
			//CString dstName= (m_SrcData.bBottom) ? m_DstData.strFilePath : GetDirName(m_DstData.strFilePath);
			//dstName		+= _T('\\') + name;
			CString dstName  = strDstDir + _T('\\') + name;
			bool	bCopy	 = false;
			if (::MoveFile( m_SrcData.strFilePath, dstName ) == 0) {
				ErrorLogPrintf(_T("���C�ɓ��胁�j���[�ł́A�t�H���_���܂������ړ��ŁA�t�@�C���ړ����s(%s)\n"), LPCTSTR(name));
				return false;
			} else {
				bCopy = ::CopyFile(dstName, m_SrcData.strFilePath, TRUE) != 0;	//+++ ���ڂ̍폜���̎��̂Ƃ��āA�_�~�[���쐬
			}
			if (srcOrder.DeleteData( m_SrcData.strFilePath, true ) == 0) {
				ErrorLogPrintf(_T("���C�ɓ��胁�j���[�ł́A�t�H���_���܂������ړ��ŁA���t�H���_���ō폜������Ȃ�����(%s)\n"), LPCTSTR(name));
			}
			if (bCopy)	//+++ �����������s���Ă����ꍇ�̂��߂̍폜.
				::DeleteFile(m_SrcData.strFilePath);
			if (dstOrder.AddData(m_SrcData.strFilePath, m_DstData.dwPos) == 0) {
				ErrorLogPrintf(_T("���C�ɓ��胁�j���[�ł́A�t�H���_���܂������ړ��ŁA�s����ւ̒ǉ����s(%s)\n"), LPCTSTR(name));
				return false;
			}
		  #else
			//srcOrder.MoveData(m_SrcData.dwPos, dstOrder, m_DstData.dwPos);
		  #endif
			srcOrder.SaveData();
			dstOrder.SaveData();

			//���\�����Ă��郁�j���[���̂̕��בւ�
			//if (m_SrcData.bBottom)
			//	m_SrcData.nPos--;
			MoveMenuItems(m_SrcData.hMenu, m_SrcData.nPos, m_DstData.hMenu, m_DstData.nPos);
			::InvalidateRect( (HWND) m_SrcData.hMenu, NULL, TRUE );
			::InvalidateRect( (HWND) m_DstData.hMenu, NULL, TRUE );
			::DrawMenuBar( m_SrcData.hWndParent );
			::DrawMenuBar( GetParent() );
			m_SrcData = m_DstData;			//+++ �ړ���������ɁA����Ɉړ�����Ƃ��p�̒��날�킹
		  #endif
		}
		return true;
	}


	int GetOrderItemIndex(CFavoriteOrderHandler &order, DragMenuData &data)
	{
		int 	nIndex;
		int 	dwPosition = data.dwPos;

		nIndex = order.FindPosition(dwPosition);

		if (nIndex == -1) {
			order.ReConstructData(order.DirPath());				//+++ �����o�[�ϐ�m_strDirPath���֐�DirPath()�ɕύX.
			nIndex = order.FindPosition(dwPosition);
			if (nIndex == -1) {
				nIndex	= order.FindName(data.strFilePath);
				if (nIndex == -1)
					return -1;
				//�f�[�^����ꂿ����Ă�̂ŕ�C
				//+++ order.m_aryData[nIndex].dwPosition = dwPosition;
				order.setAryData_position(nIndex, dwPosition);	//+++ �����o�[�ϐ����B��
			}
		}

		return nIndex;
	}


	void MoveMenuItems(HMENU hMenu, int nSrcPos, int nDstPos)
	{
		if (nSrcPos == nDstPos)
			return;

		if (nSrcPos < 0 || nDstPos < 0)
			return;

		MENUITEMINFO mii = { sizeof (MENUITEMINFO) };
		TCHAR		 buf[MAX_PATH] = _T("\0");
		mii.fMask	   = MIIM_TYPE | MIIM_STATE | MIIM_CHECKMARKS | MIIM_ID | MIIM_SUBMENU | MIIM_DATA;
		mii.dwTypeData = buf;
		mii.cch 	   = MAX_PATH;
		::GetMenuItemInfo(hMenu, nSrcPos, TRUE, &mii);
		::RemoveMenu(hMenu, nSrcPos, MF_BYPOSITION);
		::InsertMenuItem(hMenu, nDstPos, TRUE, &mii);
	}


	void MoveMenuItems(HMENU hSrcMenu, int nSrcPos, HMENU hDstMenu, int nDstPos)
	{
		if (nSrcPos < 0 || nDstPos < 0 || hSrcMenu == 0 || hDstMenu == 0)
			return;
		if (hSrcMenu == hDstMenu)
			return MoveMenuItems(hSrcMenu, nSrcPos, nDstPos);

		MENUITEMINFO mii = { sizeof (MENUITEMINFO) };
		TCHAR		 buf[MAX_PATH] = _T("\0");
		mii.fMask	   = MIIM_TYPE | MIIM_STATE | MIIM_CHECKMARKS | MIIM_ID | MIIM_SUBMENU | MIIM_DATA;
		mii.dwTypeData = buf;
		mii.cch 	   = MAX_PATH;
		::GetMenuItemInfo(hSrcMenu, nSrcPos, TRUE, &mii);
		::RemoveMenu(hSrcMenu, nSrcPos, MF_BYPOSITION);
		::InsertMenuItem(hDstMenu, nDstPos, TRUE, &mii);
	}
};



class CMenuDropTargetWindow : public CMenuDropTargetWindowImpl<CMenuDropTargetWindow> {
public:
	DECLARE_WND_SUPERCLASS( _T("Donut_MenuDDTarget"), GetWndClassName() )
};

