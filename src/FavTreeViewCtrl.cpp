/**
 *	@file	FavTreeViewCtrl.cpp
 *	@brief	���C�ɓ��胁�j���[,���C�ɓ���O���[�v,����,���[�U�[��`,�f�X�N�g�b�v�E�X�N���v�g �֌W.
 */
#include "stdafx.h"
#include "FavTreeViewCtrl.h"
#include "Donut.h"
#include "DonutPFunc.h"


#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



namespace {
	//	�N���X���Ŏg���\���͖̂������O��Ԃɓ����

	struct _Function_OpenFile {
		HWND _hWnd;

		_Function_OpenFile(HWND hWnd) : _hWnd(hWnd) { }

		void operator ()(IShellFolder *pFolder, const CItemIDList &idlFolder, const CItemIDList &idlFile)
		{
			CItemIDList idlFull = idlFolder + idlFile;

			if ( !idlFull.IsNull() )
				DonutOpenFile( idlFull.GetPath() );
		}
	};


	struct _FavoritesTreeItemCompare : public std::binary_function<const TV_INSERTSTRUCT &, const TV_INSERTSTRUCT &, bool> {
		CFavoritesOrder &		m_order;

		_FavoritesTreeItemCompare(CFavoritesOrder &order) : m_order(order){ }

		bool operator ()(const TV_INSERTSTRUCT &x, const TV_INSERTSTRUCT &y)
		{
			CItemIDList idlA   = (LPCITEMIDLIST) x.item.lParam;
			ATLASSERT( !idlA.IsNull() );

			CItemIDList idlB   = (LPCITEMIDLIST) y.item.lParam;
			ATLASSERT( !idlB.IsNull() );

			const int	enough = 10000;
			CString 	strA   = MtlGetFileName( idlA.GetPath() );
			CString 	strB   = MtlGetFileName( idlB.GetPath() );
			ATLASSERT( !strA.IsEmpty() && !strB.IsEmpty() );

			int 		itA    = m_order.Lookup(strA);
			int 		itB    = m_order.Lookup(strB);

			if (itA == -1 || itA == FAVORITESORDER_NOTFOUND)  // fixed by fub, thanks.
				itA = enough;

			if (itB == -1 || itB == FAVORITESORDER_NOTFOUND)
				itB = enough;

			if (itA == enough && itB == enough) {
				bool bParentA = (x.item.cChildren == 1);
				bool bParentB = (y.item.cChildren == 1);

				if (bParentA == bParentB) {
					int nRet = ::lstrcmp(strA, strB);
					return (nRet < 0);
				} else {
					return bParentA;
				}
			} else {
				return itA < itB;
			}
		}


		CString _GetFileName(const TV_INSERTSTRUCT &tvi)
		{
			bool		bDir   = (tvi.item.cChildren == 1);

			if (bDir) {
				return tvi.item.pszText;
			}

			CItemIDList idl    = (LPCITEMIDLIST) tvi.item.lParam;
			CString 	str    = idl.GetPath();

			if ( str.IsEmpty() )
				return CString();

			int 		nIndex = str.ReverseFind( _T('\\') );
			str = str.Right(str.GetLength() - nIndex - 1);

			return str;
		}


		CString _GetFilePath(const TV_INSERTSTRUCT &tvi)
		{
			CItemIDList idl = (LPCITEMIDLIST) tvi.item.lParam;
			CString 	str = idl.GetPath();

			return str;
		}


		FILETIME _GetFileTime(const CString &strPath_)
		{
			CString 		strPath(strPath_);

			MtlRemoveTrailingBackSlash(strPath);
			WIN32_FIND_DATA data;
			HANDLE			h = ::FindFirstFile(strPath, &data);
			ATLASSERT(h != INVALID_HANDLE_VALUE);
			::FindClose(h);

			return data.ftCreationTime;
		}
	};

}



// Attributes
bool CFavoritesTreeViewCtrl::_IsNeedFavoritesFix()
{
	if (	_check_flag( ETV_EX_FAVORITES, GetExplorerTreeViewExtendedStyle() )
		 || _check_flag( ETV_EX_USERDEFINED, GetExplorerTreeViewExtendedStyle() ) )
		return true;
	else
		return false;
}



// Overrides
void CFavoritesTreeViewCtrl::OnExplorerTreeViewInit()
{
	baseClass::OnExplorerTreeViewInit();

	_GetProfile();
}



void CFavoritesTreeViewCtrl::OnExplorerTreeViewTerm()
{
	baseClass::OnExplorerTreeViewTerm();

	_WriteProfile();
}



void CFavoritesTreeViewCtrl::OnChangeImageList()
{
	if (_check_flag( ETV_EX_FAVORITES, GetExplorerTreeViewExtendedStyle() ) && m_bOrgImage) {
		SetImageList(m_imgDonut.m_hImageList, TVSIL_NORMAL);
	} else {
		SHFILEINFO sfi;
		HIMAGELIST hImgs = (HIMAGELIST) ::SHGetFileInfo(_T("C:\\"), 0, &sfi, sizeof (sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		ATLASSERT(hImgs != NULL);
		SetImageList(hImgs, TVSIL_NORMAL);
	}
}



CItemIDList CFavoritesTreeViewCtrl::OnInitRootFolder()
{
	// change Image
	OnChangeImageList();

	if ( _check_flag( ETV_EX_MYCOMPUTER, GetExplorerTreeViewExtendedStyle() ) ) {
		CItemIDList idl;
		HRESULT 	hr = ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_DESKTOP, &idl);
		ATLASSERT( SUCCEEDED(hr) );
		return idl;

	} else if ( _check_flag( ETV_EX_FAVORITEGROUP, GetExplorerTreeViewExtendedStyle() ) ) {
		CString strDir = DonutGetFavoriteGroupFolder();
		return (LPCTSTR) strDir;

	} else if ( _check_flag( ETV_EX_USERDEFINED, GetExplorerTreeViewExtendedStyle() ) ) {
		//+++	��t���p�X���̃t���p�X��(���Ք�)
		return (LPCTSTR) Misc::GetFullPath_ForExe( CMainOption::GetExplorerUserDirectory() );
	} else if ( _check_flag( ETV_EX_SCRIPT, GetExplorerTreeViewExtendedStyle() ) ) {
		return (LPCTSTR) ( Misc::GetExeDirectory() + _T("Script\\") );
	}
	// UDT DGSTR ��ꂫ
	else if ( _check_flag( ETV_EX_HISTORY, GetExplorerTreeViewExtendedStyle() ) ) {
		CItemIDList idl;

		if ( _check_flag( ETV_EX_HISTORY_TIME, GetExplorerTreeViewExtendedStyle() ) )
			return idl;

		HRESULT 	hr = ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_HISTORY, &idl);
		ATLTRACE( "$$$$ : %s\n", idl.GetPath() );
		ATLASSERT( SUCCEEDED(hr) );
		return idl;

	} else if ( _check_flag( ETV_EX_HISTORY, GetExplorerTreeViewExtendedStyle() ) ) {
		CString 	strDir;
		CItemIDList idl;

		if ( MtlGetHistoryFolder(strDir) )
			return (LPCTSTR) strDir;
		else {
			HRESULT hr = ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_HISTORY, &idl);
			ATLASSERT( SUCCEEDED(hr) );
			return idl;
		}
	}
	// ENDE
	else {
		CString strDir;
		MtlGetFavoritesFolder(strDir);
		return (LPCTSTR) strDir;
	}
}



// U.H
void CFavoritesTreeViewCtrl::OnFolderEmpty()
{
	if ( !_check_flag( ETV_EX_HISTORY, GetExplorerTreeViewExtendedStyle() ) )
		return;

	SYSTEMTIME			sysTimeNow;
	::GetSystemTime(&sysTimeNow);
	::GetLocalTime(&sysTimeNow);

	CSimpleMap<CString, HTREEITEM> mapRoot;

	IUrlHistoryStg2 *	pHistory;
	IEnumSTATURL *		pEnumUrls;
	ULONG				ulFetched = 1;

	CoInitialize(NULL);
	HRESULT  hr = CoCreateInstance( CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg, reinterpret_cast<void **>(&pHistory) );
	if ( SUCCEEDED(hr) ) {
		pHistory->EnumUrls(&pEnumUrls);
		LPCOLESTR lpFilter = OLESTR("http");
		pEnumUrls->SetFilter(lpFilter, STATURLFLAG_ISTOPLEVEL);
		_STATURL  StatUrl;

		while ( ( hr = pEnumUrls->Next(1, &StatUrl, &ulFetched) ) == S_OK ) {
			//+++ 2005 Express �ł�com_util::�̃����N�G���[�΍�� _bstr_t �̎g�p������Ă݂�( wchar_t* -> char* �ϊ��̂��߂̎g�p�H )
			// Err
			if (StatUrl.pwcsTitle == NULL)
				continue;

			if (StatUrl.pwcsUrl == NULL)
				continue;

			// �������
			CString 	strTitle( (LPCWSTR) StatUrl.pwcsTitle );
			CString 	strUrl( (LPCWSTR) StatUrl.pwcsUrl );
			FILETIME	ftLocal;
			SYSTEMTIME	sysTimeHist;
			FileTimeToLocalFileTime(&StatUrl.ftLastVisited, &ftLocal);
			FileTimeToSystemTime(&ftLocal, &sysTimeHist);

			// �Ώ̂́A�����̂�
			if (sysTimeHist.wYear	!= sysTimeNow.wYear)
				continue;

			if (sysTimeHist.wMonth	!= sysTimeNow.wMonth)
				continue;

			if (sysTimeHist.wDay	!= sysTimeNow.wDay)
				continue;

			CString 		strTime;
			strTime.Format(_T("%02d ��"), sysTimeHist.wHour);
			HTREEITEM		hParent  = mapRoot.Lookup(strTime);

			if (hParent == NULL) {
				hParent = AddItem(strTime);
				mapRoot.Add(strTime, hParent);
			}

			// TreeItem
			LPTSTR			lpszText = NULL;
			ATLTRY(lpszText = new TCHAR[strTitle.GetLength() + 1]);
			::lstrcpy(lpszText, strTitle);

			TV_INSERTSTRUCT 	tvis;
			tvis.hParent			 = hParent;
			tvis.item.mask			 = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
			tvis.item.pszText		 = lpszText;
			tvis.item.iImage		 = I_IMAGECALLBACK; //MtlGetNormalIconIndex(idlFull, _pidlHtm);
			tvis.item.iSelectedImage = I_IMAGECALLBACK; //MtlGetSelectedIconIndex(idlFull, false, _pidlHtm);
			tvis.hInsertAfter		 = TVI_SORT;

			// set up full item id list!
			CItemIDList 	idlFull(strUrl);

			// set up!!
			tvis.item.lParam		 = (LPARAM) idlFull.m_pidl;
			idlFull.Detach();		// *********don't forget clean up the lParam!!!!!!!********

			// �ǉ�
			InsertItem(&tvis);
			delete[] lpszText;
		}

		pEnumUrls->Release();
	}

	pHistory->Release();			// release our reference
	CoUninitialize();
}



HTREEITEM CFavoritesTreeViewCtrl::AddItem(CString strText)
{
	// TreeItem
	LPTSTR			lpszText = NULL;

	ATLTRY(lpszText = new TCHAR[strText.GetLength() + 1]);
	::lstrcpy(lpszText, strText);

	CString 		strDir;
	MtlGetHistoryFolder(strDir);
	CItemIDList 	idlHist(strDir);

	TV_INSERTSTRUCT 	tvis = {0};
	tvis.hParent			 = NULL;
	tvis.item.mask			 = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvis.item.pszText		 = lpszText;
	tvis.item.iImage		 = MtlGetNormalIconIndex(idlHist);
	tvis.item.iSelectedImage = MtlGetSelectedIconIndex(idlHist, true);
	tvis.item.mask			|= TVIF_CHILDREN;
	tvis.item.cChildren 	 = 1;
	tvis.hInsertAfter		 = TVI_SORT;

	// set up full item id list!
	CItemIDList 	idlFull(strText);
	// set up!!
	tvis.item.lParam		 = (LPARAM) idlFull.m_pidl;
	idlFull.Detach();				// *********don't forget clean up the lParam!!!!!!!********

	HTREEITEM	hItem = InsertItem(&tvis);
	delete[]	lpszText;
	return hItem;
}



void CFavoritesTreeViewCtrl::OnInitialUpdateTreeItems(CSimpleArray<TV_INSERTSTRUCT> &items, LPCITEMIDLIST pidl)
{
	ATLASSERT(pidl != NULL);

	if ( CFavoritesMenuOption::s_bIEOrder && _check_flag( ETV_EX_FAVORITES, GetExplorerTreeViewExtendedStyle() ) ) {
		CFavoritesOrder 	order;
		MtlGetFavoritesOrder( order, CItemIDList(pidl).GetPath() );
		std::sort( _begin(items), _end(items), _FavoritesTreeItemCompare(order) );

	} else if ( _check_flag( ETV_EX_HISTORY, GetExplorerTreeViewExtendedStyle() ) ) {
		CItemIDList idl 	= pidl;
		CString 	strPath = idl.GetPath();

		if ( strPath.IsEmpty() )
			baseClass::OnInitialUpdateTreeItems(items, pidl);

	} else {	// UDT DGSTR ��ꂫ
		baseClass::OnInitialUpdateTreeItems(items, pidl);
	}
}


#if 0	//+++
////////////////////////////////////////////////////////////////////
//�@�A�C�e���h�c���X�g�̒������Ă����܂��B

CString GetStrOfItemIdList(LPITEMIDLIST p_pItemIDList)
{
	HRESULT			hRes;
	STRRET			stFileName;
	LPITEMIDLIST	pCurIDList;
	LPSHELLFOLDER	pCurFolder, pTempFolder;
	CString			cPrintStr;

	SHGetDesktopFolder( &pCurFolder );
	pCurIDList = p_pItemIDList;

	CString			strRslt;
	do
	{
		LPITEMIDLIST pCopyIDList;

		//�@�A�C�e���h�c���R�s�[���܂��B
		pCopyIDList = CopyItemID( pCurIDList );
		if( pCopyIDList == NULL )
		{
			//TRACE0( "Copy FALSE\n" );
			break;
		}

		//�@�t�H���_�̎擾�B
		hRes = pCurFolder->GetDisplayNameOf( pCopyIDList
				, SHGDN_NORMAL, &stFileName );
		if( hRes == NOERROR )
			cPrintStr = TFileName( pCopyIDList, &stFileName );
		else
			cPrintStr = "";

		//�@�t�H���_��\�����܂��B
		strRslt += cPrintStr;
		//TRACE( "%s\\", (LPCTSTR)cPrintStr );

		//�@���̃t�H���_�ւƃo�C���h���܂��B
		hRes = pCurFolder->BindToObject( pCopyIDList, NULL, IID_IShellFolder
			, (LPVOID *)&pTempFolder );
		if( hRes != NOERROR )
		{
			//TRACE0( "\nBind DTFolder FALSE\n" );
			pTempFolder = NULL;
		}
		//�@���݂̃t�H���_����ꊷ���܂��B
		pCurFolder->Release();
		pCurFolder = pTempFolder;

		//�@�R�s�[�����A�C�e���h�c��������܂��B
		m_pMalloc->Free( pCopyIDList );

		//�@���̃A�C�e���h�c�ւƃ|�C���^��i�߂܂��B
		pCurIDList = GetNextItemID( pCurIDList );
	}while( pCurIDList != NULL );	//�I�[�܂ŗ����甲���o�܂��B

	if( pCurFolder != NULL )
		pCurFolder->Release();

	return strRslt;
}
#endif


void CFavoritesTreeViewCtrl::OnTreeItemClicked(HTREEITEM hTreeItem, UINT uFlags)
{
	//+++ ����ł�Shell���ĂԂ̂͂��������d�l�Ƃ������ƂȂ̂��낤��?.. �}�C�R���s���[�^����ie�ň����Ȃ��t�@�C�������邩�炽�Ԃ񂻂�.
	//CTreeItem	treeItem( hTreeItem );
	CItemIDList 	idl  			= _GetItemIDList( hTreeItem );
	DWORD 			dwViewExStyle 	= GetExplorerTreeViewExtendedStyle();
	CString			strPath			= idl.GetPath();
	if (strPath.IsEmpty())
		strPath = idl.GetUrl();			//+++ �������B����A���s��.

	if (!_check_flag( ETV_EX_FAVORITEGROUP,dwViewExStyle )
	  && _check_flag( ETV_EX_MYCOMPUTER | ETV_EX_SHELLEXECUTE | ETV_EX_HISTORY | ETV_EX_SCRIPT, dwViewExStyle ) )
	{
	  #if 1 //+++ �g���q��donut�Ō������̂�donut�ɔC����.
			//+++ ���A�����̏ꍇ���̂̃t�@�C�������݂��Ȃ����� idl.GetPath()�Ńp�X���擾�ł��Ȃ��͗l...
		CString ext 	= Misc::GetFileExt(strPath);
		ext.MakeLower();
		if (ext == _T("html") || ext == _T("shtml") || ext == _T("url") || ext == _T("htm") //|| ext == _T("cgi")
			|| strPath.Left(5).CompareNoCase(_T("http:")) == 0 || strPath.Left(4).CompareNoCase(_T("ftp:")) == 0)
		{
			;
		} else {
			MtlShellExecute( m_hWnd, idl );
			return;
		}
	  #else
		MtlShellExecute( m_hWnd, idl );
		return;
	  #endif
	}

	HWND		hWnd = NULL;		//+++ ���ǎg���Ă��Ȃ�... ����킵����...

	if ( !idl.IsNull() )
		DonutOpenFile( strPath );

	if (m_hWnd) {	//+++ �`�F�b�N�ǉ�...
		CWindow 	wnd(m_hWnd);
		::SetFocus( wnd.GetTopLevelWindow() );
	}
}



bool CFavoritesTreeViewCtrl::OnGetInfoToolTip(HTREEITEM hItem, CString &strTip, int cchTextMax)
{
	if ( _IsItemFolder(hItem) )
		return false;

	CItemIDList idl 	= _GetItemIDList(hItem);

	if ( idl.IsNull() )
		return false;

	// ���ԑѕʗ�����
	if ( _check_flag( ETV_EX_HISTORY | ETV_EX_HISTORY_TIME, GetExplorerTreeViewExtendedStyle() ) ) {
		CString strText;
		GetItemText(hItem, strText);
		// strTip = MtlMakeFavoriteToolTipText(strTitle,strUrl, cchTextMax);
		strTip = strText;
		return true;
	}

	CString 	strPath = idl.GetPath();

	if ( strPath.IsEmpty() )
		return false;

	CString 	strUrl	= MtlGetInternetShortcutUrl(strPath);

	if ( strUrl.IsEmpty() )
		strUrl = strPath;

	CString 	strName;

	if ( !GetItemText(hItem, strName) )
		return false;

	strTip = MtlMakeFavoriteToolTipText(strName, strUrl, cchTextMax);
	return true;
}



bool CFavoritesTreeViewCtrl::OnCmdContextMenu(HTREEITEM hTreeItem, int nID)
{
	if (nID == s_nIDExProp) {
		CItemIDList idl = _GetItemIDList(hTreeItem);

		if ( idl.IsNull() )
			return false;

		::SendMessage(GetTopLevelParent(), WM_SET_EXPROPERTY, (WPARAM) (LPCTSTR) idl.GetPath(), 0);
		return true;
	} else if (nID == s_nIDOpenAll) {
		MtlForEachFile( _GetItemIDList(hTreeItem), _Function_OpenFile(m_hWnd) );
		return true;
	}

	return false;
}



/*
	bool CFavoritesTreeViewCtrl::OnCheckContextItem(HTREEITEM hTreeItem)
	{
	//�����̂Ƃ�"��T"�Ȃǂ̃p�X�����݂��Ȃ����ڂŃR���e�L�X�g���j���[�����s����Ɨ�����̂�
	//�`�F�b�N���Ă��̏ꍇ�����O���� minit
	BOOL bHistory = _check_flag(ETV_EX_HISTORY, GetExplorerTreeViewExtendedStyle());
	if(bHistory && ItemHasChildren(hTreeItem))
		return false;
	return true;
	}
 */

UINT CFavoritesTreeViewCtrl::OnPrepareMenuForContext(HTREEITEM hTreeItem, CMenuHandle menu, int &nPos)
{
	bool bFolder = _IsItemFolder(hTreeItem);
	UINT uFlags  = CMF_CANRENAME;

	if (bFolder)
		uFlags |= CMF_EXPLORE | CMF_NODEFAULT;
	else
		uFlags |= CMF_NORMAL;

	//	if (!_IsNeedFavoritesFix()) //���C�ɓ��肩���[�U�[��`���ᖳ�������烁�j���[�ɒǉ����Ȃ��B
	//		return uFlags;

	// set up for favorites
	if (bFolder) {
		if ( _IsItemExpanded(hTreeItem) )
			menu.AppendMenu( MF_ENABLED | MF_STRING, s_nIDCollapse, _T("�܂肽����(&A)") );
		else
			menu.AppendMenu( MF_ENABLED | MF_STRING, s_nIDExpand  , _T("�W�J(&A)") );

		//���C�ɓ���ł����[�U�[��`�ł����������炱���ŏI��
		if ( !_IsNeedFavoritesFix() ) {
			nPos += 2;
			return uFlags;
		}

		menu.AppendMenu( MF_ENABLED | MF_STRING, s_nIDOpenAll	 , _T("���g�����ׂĊJ��(&I)") );
		menu.AppendMenu( MF_ENABLED | MF_STRING, s_nIDNewFolder  , _T("�V�����t�H���_���쐬(&W)") );
		menu.AppendMenu(MF_SEPARATOR);
		nPos += 4;
	} else {
	}

	return uFlags;
}



void CFavoritesTreeViewCtrl::OnPrepareMenuForPopup(HTREEITEM hTreeItem, CMenuHandle menu)
{
	if ( !_IsNeedFavoritesFix() )
		return;

	int nCmdID = MtlGetCmdIDFromAccessKey( menu, _T("&S") );

	// remove "create shortcut" menu item
	if (nCmdID != -1)
		menu.RemoveMenu(nCmdID, MF_BYCOMMAND);

	if ( !_IsItemFolder(hTreeItem) ) {
		int nCmdID = MtlGetCmdIDFromAccessKey(menu, _T("&O"), false);

		// remove original "open" menu item
		if (nCmdID != -1)
			menu.RemoveMenu(nCmdID, MF_BYCOMMAND);

		menu.InsertMenu( 0, MF_BYPOSITION | MF_STRING, s_nIDNewFolder,	_T("�V�����t�H���_���쐬(&W)") );
		menu.InsertMenu( 0, MF_BYPOSITION | MF_STRING, s_nIDOpen,		_T("�J��(&O)") );

		menu.SetMenuDefaultItem(s_nIDOpen, FALSE);

		menu.InsertMenu(0, MF_BYPOSITION | MF_SEPARATOR, 0);
		menu.InsertMenu( 0, MF_BYPOSITION, s_nIDExProp, _T("�g���v���p�e�B(&E)") );
	}
}



void CFavoritesTreeViewCtrl::_GetProfile()
{
	CString 	strSection = _T("FavTree");
	CIniFileI	pr(g_szIniFileName, strSection);
	DWORD		dwStyle    = ETV_EX_FAVORITES | ETV_EX_SINGLEEXPAND;	// default
	pr.QueryValue( dwStyle, _T("Style") );
	pr.Close();

	SetExplorerTreeViewExtendedStyle(dwStyle);
}



void CFavoritesTreeViewCtrl::_WriteProfile()
{
	CString 	strSection = _T("FavTree");
	CIniFileO	pr(g_szIniFileName, strSection);
	pr.SetValue( GetExplorerTreeViewExtendedStyle(), _T("Style") );
}



////////////////////////////////////////////////////////////////////////////////
//CDonutFavoritesBar�̒�`
///////////////////////////////////////////////////////////////////////////////

namespace {
	//�����o�֐����Ŏg���֐��I�u�W�F�N�g�Ȃǂ͂��̖������O��ԂŒ�`����

	struct _HistoryData {
		CString 	strTitle;
		CString 	strUrl;
		FILETIME	fTime;
	};

	struct _HistoryCompare : public std::binary_function<const _HistoryData &, const _HistoryData &, bool> {
		_HistoryCompare() { }

		bool operator ()(const _HistoryData &x, const _HistoryData &y)
		{
			//+++ return CompareFileTime(&x.fTime, &y.fTime) < 0 ? false : true;
			return CompareFileTime(&x.fTime, &y.fTime) >= 0;
		}
	};

	struct _Function_SearchHistory {
		CString 				_strSearchWord;
		CFavoritesTreeViewCtrl* _ptreeview;

		_Function_SearchHistory(CString strWord, CFavoritesTreeViewCtrl *ptreeView)
			: _strSearchWord(strWord)
			, _ptreeview(ptreeView)
		{
		}

		void operator ()(IShellFolder *pFolder, const CItemIDList &idlFolder, const CItemIDList &idlFile, bool bFolder)
		{
			CString 		strText;

			MtlGetDisplayName(pFolder, idlFile, strText);

			if (bFolder) {
				//�ċA�I����
				CItemIDList  idlFull	= idlFolder + idlFile;
				LPITEMIDLIST pidlFolder = idlFull;
				MtlForEachObject( pidlFolder, _Function_SearchHistory(_strSearchWord, _ptreeview) );
				return;
			}

			//�������[�h�Ƀ}�b�`���Ȃ��ꍇ�͔�����
			if (strText.Find(_strSearchWord) == -1)
				return;

			LPTSTR	lpszText = NULL;
			ATLTRY(lpszText  = new TCHAR[strText.GetLength() + 1]);

			if (lpszText == NULL)
				return;

			::lstrcpy(lpszText, strText);
			TV_INSERTSTRUCT 	tvis = {0};
			tvis.hParent			 = TVI_ROOT;
			tvis.hInsertAfter		 = TVI_LAST;
			tvis.item.mask			 = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
			tvis.item.pszText		 = lpszText;
			tvis.item.iImage		 = I_IMAGECALLBACK;
			tvis.item.iSelectedImage = I_IMAGECALLBACK;

			// set up full item id list!
			CItemIDList 	idlFull 	 = idlFolder + idlFile;
			ATLASSERT(idlFull.m_pidl != NULL);

			// fill mask
			DWORD			dwAttributes = SFGAO_LINK | SFGAO_SHARE | SFGAO_GHOSTED;
			HRESULT 		hr			 = pFolder->GetAttributesOf(1, (LPCITEMIDLIST *) &idlFile.m_pidl, &dwAttributes);

			if ( SUCCEEDED(hr) && !MtlIsTooSlowIDList(idlFull) )
				_ptreeview->_TreeItemMask(tvis.item, dwAttributes);

			// set up!!
			tvis.item.lParam		 = (LPARAM) idlFull.m_pidl;

			idlFull.Detach(); // #########don't forget clean up the lParam!!!!!!!##########

			_ptreeview->InsertItem(&tvis);
		}

	};
}



CDonutFavoritesBar::CDonutFavoritesBar()
{
}



BOOL CDonutFavoritesBar::OnToolbarReconstruct()
{
	m_wndToolBar.ShowWindow(SW_HIDE);
	int nCount = m_wndToolBar.GetButtonCount();

	for (int i = 0; i < nCount; i++)
		if ( !m_wndToolBar.DeleteButton(0) )
			return FALSE;

	if ( _check_flag( ETV_EX_FAVORITES, m_view.GetExplorerTreeViewExtendedStyle() ) ) {
		//���C�ɓ��胁�j���[
		_AddNewButton(ID_FAVORITE_PLACEMENT,	_T("�\��"), 2, TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN);
		_AddNewButton(ID_FAVORITE_ADD,			_T("�ǉ�..."), 0, 0);
		_AddNewButton(ID_FAVORITE_ORGANIZE, 	_T("����..."), 1, 0);
	} else if ( _check_flag( ETV_EX_FAVORITEGROUP, m_view.GetExplorerTreeViewExtendedStyle() ) ) {
		//���C�ɓ���O���[�v
		_AddNewButton(ID_FAVORITE_PLACEMENT,	  _T("�\��"),	 2, TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN);
		_AddNewButton(ID_FAVORITE_GROUP_SAVE,	  _T("�ۑ�..."), I_IMAGENONE, 0);
		_AddNewButton(ID_FAVORITE_GROUP_ADD,	  _T("�ǉ�..."), I_IMAGENONE, 0);
		_AddNewButton(ID_FAVORITE_GROUP_ORGANIZE, _T("����..."), I_IMAGENONE, 0);
	} else if ( _check_flag( ETV_EX_HISTORY, m_view.GetExplorerTreeViewExtendedStyle() ) ) {
		//����
		_AddNewButton(ID_FAVORITE_PLACEMENT,	_T("�\��"), 2, TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN);
		_AddNewButton(ID_SEARCH_HISTORY,		_T("����"), I_IMAGENONE, 0);
		_AddNewButton(ID_VIEW_HISTORY_SORT, 	_T("����"), I_IMAGENONE, TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN);
	} else if ( _check_flag( ETV_EX_USERDEFINED, m_view.GetExplorerTreeViewExtendedStyle() ) ) {
		//���[�U�[��`
		_AddNewButton(ID_FAVORITE_PLACEMENT,	_T("�\��"), 	2, TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN);
		_AddNewButton(ID_SELECT_USERFOLDER, 	_T("�t�H���_..."), I_IMAGENONE, 0);
	} else {
		//���f�X�N�g�b�v�E�X�N���v�g
		_AddNewButton(ID_FAVORITE_PLACEMENT,	_T("�\��"), 	2, TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN);
	}

	m_wndToolBar.AddStrings( _T("NS\0") );	// for proper item height

	ReloadSkin();

	CalcSize();
	m_wndToolBar.ShowWindow(SW_SHOW);

	return TRUE;
}



void CDonutFavoritesBar::_AddNewButton(int nID, LPTSTR lpText, int nImage, int nStyleSpecial)
{
	_InsertButton(nID, nStyleSpecial, nImage);

	if (lpText)
		_SetButtonText(nID, lpText);
}



void CDonutFavoritesBar::_InsertButton(int nID, int nStyleSpecial, int nImage)
{
	TBBUTTON btn = { nImage, nID, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | nStyleSpecial, 0, 0 };

	m_wndToolBar.InsertButton(-1, &btn);
}



void CDonutFavoritesBar::_SetButtonText(int nID, LPTSTR lpstr)
{
	CVersional<TBBUTTONINFO> bi;
	bi.dwMask  = TBIF_TEXT;
	bi.pszText = lpstr;
	m_wndToolBar.SetButtonInfo(nID, &bi);
}



void CDonutFavoritesBar::ReloadSkin()
{
	CString 	strSkin    = _GetSkinDir();
	CString 	strSkinHot = strSkin;

	strSkin    += "FavBar.bmp";
	strSkinHot += "FavBarHot.bmp";

	if ( m_wndToolBar.IsWindow() ) {
		CImageList imgs, imgsHot;
		imgs	= m_wndToolBar.GetImageList();
		imgsHot = m_wndToolBar.GetHotImageList();

		_ReplaceImageList(strSkin, imgs);
		_ReplaceImageList(strSkinHot, imgsHot);
		m_wndToolBar.InvalidateRect(NULL, TRUE);
	}
}



LRESULT CDonutFavoritesBar::OnSearchHistory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CSearchHistoryDialog	dlg;
	INT_PTR 				nRet = dlg.DoModal();
	if (nRet == 0 || nRet == -1)
		return 0;

	//�����̍��ڂ��폜
	CLockRedraw 	lock(m_view.m_hWnd);
	m_view._CleanUpAll();

	CString 	strWord = dlg.m_strKeyWord;
	if ( strWord.IsEmpty() )
		return 0;

	strWord.MakeLower();

	if (dlg.m_bUseHiFunction) {
		CComPtr<IUrlHistoryStg2>	pHistory;

		HRESULT  hr = CoCreateInstance( CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg, reinterpret_cast<void **>(&pHistory) );
		if ( FAILED(hr) )
			return 0;

		CComPtr<IEnumSTATURL>		pEnum;

		hr = pHistory->EnumUrls(&pEnum);
		if ( FAILED(hr) )
			return 0;

		STATURL 	stat	  = { sizeof (STATURL) };
		DWORD		dwRet	  = 1;
		CString 	strUrl;
		CString 	strTitle;
		CString 	strLUrl;
		CString 	strLTitle;
		BOOL		bUseDate = dlg.m_bCheckDate;

		SYSTEMTIME	stStart,
					stEnd;

		::ZeroMemory(&stStart, sizeof stStart);
		::ZeroMemory(&stEnd  , sizeof stEnd  );

		if (bUseDate) {
			stStart = dlg.m_sysTimeStart;
			stEnd	= dlg.m_sysTimeEnd;
		}

		CSimpleArray<CString>	 arystr;
		SplitKeyWord(arystr, strWord);

		int 		nCount	 = arystr.GetSize();
		CString 	strKey;
		pEnum->SetFilter(L"http", STATURLFLAG_ISTOPLEVEL);

		while (pEnum->Next(1, &stat, &dwRet) == S_OK && dwRet == 1) {
			BOOL	bFindInTitle = FALSE;
			BOOL	bFindInUrl	 = FALSE;

			strLTitle	 = stat.pwcsTitle;
			strLTitle.MakeLower();
			strLUrl 	 = stat.pwcsUrl;
			strLUrl.MakeLower();

			for (int i = 0; i < nCount; i++) {
				strKey		 = arystr[i];
				bFindInTitle = (strLTitle.Find(strKey) != -1);	//+++ ? TRUE : FALSE;
				if (!bFindInTitle) {
					bFindInUrl = (strLUrl.Find(strKey) != -1);	//+++ ? TRUE : FALSE;
					if (!bFindInUrl)
						goto NEXT;
				}
			}

			if (bUseDate) {
				FILETIME   ftLocal;
				SYSTEMTIME sysTimeHist;
				FileTimeToLocalFileTime(&stat.ftLastVisited, &ftLocal);
				FileTimeToSystemTime(&ftLocal, &sysTimeHist);

				if (  stStart.wYear  <= sysTimeHist.wYear  && sysTimeHist.wYear  <= stEnd.wYear
				   && stStart.wMonth <= sysTimeHist.wMonth && sysTimeHist.wMonth <= stEnd.wMonth
				   && stStart.wDay	 <= sysTimeHist.wDay   && sysTimeHist.wDay	 <= stEnd.wDay	 )
				{
					strTitle = stat.pwcsTitle;
					strUrl	 = stat.pwcsUrl;
					InsertItem(strTitle, strUrl);
				}

				goto NEXT;
			}

			strTitle = stat.pwcsTitle;
			strUrl	 = stat.pwcsUrl;
			InsertItem(strTitle, strUrl);

		  NEXT:
			;
		}

	} else {
		//IE5.5�����̏ꍇ�@�@�\�͎ア���x��
		CItemIDList idl;
		HRESULT 	hr = ::SHGetSpecialFolderLocation(m_view.m_hWnd, CSIDL_HISTORY, &idl);
		MtlForEachObject( idl, _Function_SearchHistory(strWord, &m_view) );
	}

	m_view.SortChildren(NULL);

	return 0;
}



void CDonutFavoritesBar::SplitKeyWord(CSimpleArray<CString> &arystr, CString strWord)
{
	arystr.RemoveAll();

	CString strBase = strWord;
	strBase.Replace( _T("�@"), _T(" ") );
	strBase.TrimLeft();
	strBase.TrimRight();

	while (1) {
		int 	pos 	= strBase.Find( _T(" ") );

		if (pos == -1)
			break;

		CString strLeft = strBase.Left(pos);
		arystr.Add(strLeft);
		strBase.Delete(0, pos + 1);

		if ( strBase.IsEmpty() )
			return;
	}

	arystr.Add(strBase);
}



void CDonutFavoritesBar::InsertItem(CString &strTitle, CString &strUrl)
{
	LPTSTR			lpszText = NULL;

	ATLTRY(lpszText = new TCHAR[strTitle.GetLength() + 1]);

	if (lpszText == NULL)
		return;

	::lstrcpy(lpszText, strTitle);
	TV_INSERTSTRUCT 	tvis = {0};
	tvis.hParent			 = TVI_ROOT;
	tvis.hInsertAfter		 = TVI_LAST;
	tvis.item.mask			 = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvis.item.pszText		 = lpszText;
	tvis.item.iImage		 = I_IMAGECALLBACK;
	tvis.item.iSelectedImage = I_IMAGECALLBACK;

	// set up full item id list!
	CItemIDList 	idlFull(strUrl);

	// set up!!
	tvis.item.lParam		 = (LPARAM) idlFull.m_pidl;
	idlFull.Detach(); // *********don't forget clean up the lParam!!!!!!!********

	m_view.InsertItem(&tvis);
}



LRESULT CDonutFavoritesBar::OnSelectUserFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CFolderDialog foldlg( m_hWnd, _T("���[�U�[��`�t�H���_��I�����Ă��������B") );

	if (foldlg.DoModal() != IDOK)
		return 0;

	CString 	  strPath = foldlg.GetFolderPath();
	CMainOption::SetExplorerUserDirectory(strPath);

	m_view.RefreshRootTree();

	return 0;
}



LRESULT CDonutFavoritesBar::OnTbnDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL &bHandled)
{
	LPNMTOOLBAR lpnmtb	   = (LPNMTOOLBAR) pnmh;
	int 		nID 	   = lpnmtb->iItem;
	int 		index	   = -1;

	int 		nCount	   = m_wndToolBar.GetButtonCount();

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

	UINT		uMenuFlags = TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN
							 | (!AtlIsOldWindows() ? TPM_VERPOSANIMATION : 0);

	TPMPARAMS	TPMParams;
	TPMParams.cbSize	= sizeof (TPMPARAMS);
	TPMParams.rcExclude = rc;

	CMenu		menu;

	if (nID == ID_FAVORITE_PLACEMENT) {
		menu.LoadMenu(IDR_MENU_FAVTREE_BAR);
	} else if (nID == ID_VIEW_HISTORY_SORT) {
		menu.LoadMenu(IDR_HISTORY);
	}

	::TrackPopupMenuEx(menu.GetSubMenu(0), uMenuFlags, rc.left + 1, rc.bottom + 2, m_hWnd, &TPMParams);

	return TBDDRET_DEFAULT;
}



LRESULT CDonutFavoritesBar::OnViewHistoryDay(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	m_view.RefreshRootTree();
	return 0;
}



LRESULT CDonutFavoritesBar::OnViewHistoryRecent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CSimpleArray<_HistoryData> items;
	CComPtr<IUrlHistoryStg2>   pHistory;

	HRESULT  hr = CoCreateInstance( CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg2, reinterpret_cast<void **>(&pHistory) );
	if ( FAILED(hr) )
		return 0;

	//�����̍��ڂ��폜
	CLockRedraw  lock(m_view.m_hWnd);
	m_view._CleanUpAll();

	{
		CComPtr<IEnumSTATURL> pEnum;

		hr = pHistory->EnumUrls(&pEnum);
		if ( FAILED(hr) )
			return 0;

		pEnum->Reset();

		STATURL 	stat	= { sizeof (STATURL) };
		SYSTEMTIME	stToday;
		GetLocalTime(&stToday);

		DWORD		dwRet = 0;
		pEnum->SetFilter(L"http", STATURLFLAG_ISTOPLEVEL);

		while (SUCCEEDED( pEnum->Next(1, &stat, &dwRet) ) && dwRet == 1) {
			FILETIME	ftLocal;
			SYSTEMTIME	sysTimeHist;
			FileTimeToLocalFileTime(&stat.ftLastVisited, &ftLocal);
			FileTimeToSystemTime(&ftLocal, &sysTimeHist);

			if (  stToday.wYear  == sysTimeHist.wYear
			   && stToday.wMonth == sysTimeHist.wMonth
			   && stToday.wDay	 == sysTimeHist.wDay)
			{
				_HistoryData data;
				data.strTitle = stat.pwcsTitle;

				if ( data.strTitle.IsEmpty() )
					continue;

				data.strUrl   = stat.pwcsUrl;
				data.fTime	  = stat.ftLastVisited;
				items.Add(data);
			}
		}
	}

	std::sort( _begin(items), _end(items), _HistoryCompare() );

	int 	nCount = items.GetSize();

	for (int i = 0; i < nCount; i++) {
		InsertItem(items[i].strTitle, items[i].strUrl);
	}

	return 0;
}



// -----------------------------------------------------------------------
// �����ł����̂��킩��Ȃ����Ƃ肠����...

//+++
LRESULT CDonutFavoritesBar::OnFavoriteGroupSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	::SendMessage(GetTopLevelParent(), WM_COMMAND, (WPARAM) ID_FAVORITE_GROUP_SAVE, 0);
	return 0;
}


//+++
LRESULT CDonutFavoritesBar::OnFavoriteGroupAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	::SendMessage(GetTopLevelParent(), WM_COMMAND, (WPARAM) ID_FAVORITE_GROUP_ADD, 0);
	return 0;
}


//+++
LRESULT CDonutFavoritesBar::OnFavoriteGroupOrganize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	::SendMessage(GetTopLevelParent(), WM_COMMAND, (WPARAM) ID_FAVORITE_GROUP_ORGANIZE, 0);
	return 0;
}
