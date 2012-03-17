/**
*	@file	LinkPopupMenu.h
*	@brief	�����N�o�[�p�̃|�b�v�A�b�v���j���[�E�B���h�E
*/

#pragma once

#include <boost\optional.hpp>
#include <atlscrl.h>
#include "MtlWin.h"
#include "MtlMisc.h"
#include "MtlCom.h"
#include "MtlFile.h"
#include "MtlDragDrop.h"
#include "DonutPFunc.h"
#include "Misc.h"

struct LinkItem
{
	CString strName;
	CString strUrl;
	vector<unique_ptr<LinkItem> >*	pFolder;

	CRect	rcItem;
	enum LinkItemState { kItemNormal, kItemHot, kItemPressed, };
	DWORD	state;
	CIcon	icon;

	bool	bExPropEnable;
	boost::optional<DWORD>	dwExProp;
	boost::optional<DWORD>	dwExPropOpt;

	// Attributes
	bool ModifyState(DWORD newState)
	{
		DWORD stateOld = state;
		state = newState;
		return state != stateOld;
	}

	LinkItem() : pFolder(nullptr), state(kItemNormal), bExPropEnable(false)
	{}

};
typedef vector<unique_ptr<LinkItem> >	LinkFolder;
typedef vector<unique_ptr<LinkItem> >*	LinkFolderPtr;


#define WM_CLOSEBASESUBMENU		(WM_APP + 1)
#define WM_SAVELINKBOOKMARK		(WM_APP + 2)

/////////////////////////////////////////////////////////////
// CLinkItemDataObject

extern const UINT CF_DONUTLINKITEM;

class CLinkItemDataObject
	: public CComCoClass<CLinkItemDataObject, &CLSID_NULL>
	, public CComObjectRootEx<CComSingleThreadModel>
	, public IDataObjectImpl<CLinkItemDataObject>
{
public:

	CComPtr<IDataAdviseHolder>	m_spDataAdviseHolder;	// expected by IDataObjectImpl

	DECLARE_NO_REGISTRY()
	DECLARE_NOT_AGGREGATABLE(CLinkItemDataObject)

	BEGIN_COM_MAP(CLinkItemDataObject)
		COM_INTERFACE_ENTRY(IDataObject)
		COM_INTERFACE_ENTRY_FUNC(IID_NULL, 0, _This)
	END_COM_MAP()

	static CComPtr<IDataObject>	CreateInstance(LinkFolderPtr pFolder, int nIndex);

	// Constructor
	CLinkItemDataObject();
	~CLinkItemDataObject();

	static bool	IsAcceptDrag(IDataObject* pDataObject);
	static std::pair<CString, CString> GetNameAndURL(IDataObject* pDataObject);

public:

	std::pair<LinkFolderPtr, int>	GetFolderAndIndex();

	// Overrides
	HRESULT IDataObject_GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
	STDMETHOD	(QueryGetData) (FORMATETC* pformatetc);
	STDMETHOD	(EnumFormatEtc) (DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);
	STDMETHOD	(SetData) (FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease);

private:
	// Implementation

	// For DragSourceHelper
    typedef struct {
        FORMATETC   fe;
        STGMEDIUM   stgm;
    } DATAENTRY, *LPDATAENTRY;  /* �A�N�e�B�u�� FORMATETC ���Ƃɂ����̂����� 1 ���擾  */

    HRESULT FindFORMATETC(FORMATETC *pfe, LPDATAENTRY *ppde, BOOL fAdd);
    HRESULT AddRefStgMedium(STGMEDIUM *pstgmIn, STGMEDIUM *pstgmOut, BOOL fCopyIn);

    LPDATAENTRY m_rgde;            /* �A�N�e�B�u�� DATAENTRY �G���g���̔z�� */
    int m_cde;                     /* m_rgde �̃T�C�Y  */


	/// �V���[�g�J�b�g�t�@�C�����쐬����
	void _InitFileNamesArrayForHDrop();
	static CString _CompactFileName(const CString &strDir, const CString &strFile, const CString &strExt);
	HGLOBAL _CreateText();
	bool _CreateInternetShortcutFile(const CString &strFileName, const CString &strUrl);
	/// arrFileNames�ɓ����Ă�t�@�C�����Ɣ��Ȃ��t�@�C������Ԃ�
	CString _UniqueFileName(CSimpleArray<CString> &arrFileNames, const CString &strFileName);


	// Data members
	bool					m_bInitialized;
	CSimpleArray<CString>	m_arrFileNames;
	
	LinkFolderPtr	m_pBaseFolder;
	int				m_nIndex;		// m_pBaseFolder �̃C���f�b�N�X
	bool			m_bItemIsFolder;
};


class CLinkEditDialog;

////////////////////////////////////////////////////////////
// CLinkPopupMenu

class CLinkPopupMenu : 
	public CScrollWindowImpl<CLinkPopupMenu>,
	public CThemeImpl<CLinkPopupMenu>,
	public CTrackMouseLeave<CLinkPopupMenu>,
	public IDropTargetImpl<CLinkPopupMenu>,
	public IDropSourceImpl<CLinkPopupMenu>,
	public CMessageFilter
{
	friend class CLinkEditDialog;
public:
	DECLARE_WND_CLASS_EX(_T("DonutLinkPopupMenu"), CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW, COLOR_MENU)

	// Constants
	enum {
		kBoundMargin = 2,
		kBoundBorder = 1,
		kItemHeight = 22, 
		kIconWidth	= 26,
		kVerticalLineWidth = 2,
		kTextMargin	= 4,
		kLeftTextPos = kBoundMargin + kIconWidth + kVerticalLineWidth + kTextMargin,
		kArrowWidth	= 25,
		kNoneTextWidth = 50,
		kMaxMenuTextWidth	= 400,
		kBiteWidth	= 3,

		kcxIcon = 16,
		kcyIcon = 16,
		kTopIconMargin = 3,
		kLeftIconMargin = kBoundMargin + 4,

		kSubMenuPopupCloseTimerID = 1,
		kSubMenuPopupCloseTime	= 500,
		kSubMenuPopupTimerID = 2,
		kSubMenuPopupTime	= 300,

		kDragInsertHitWidthOnFolder = 3,
	};

	// Constructor
	CLinkPopupMenu(LinkFolderPtr pFolder, int nInheritIndex = -1);

	static void	SetLinkBarHWND(HWND hWnd) { s_wndLinkBar = hWnd; }

	int	ComputeWindowWidth();
	int ComputeWindowHeight();

	LinkFolderPtr	GetLinkFolderPtr() const { return m_pFolder; }
	CLinkPopupMenu*	GetSubMenu() const { return m_pSubMenu;	}

	static void	OpenLink(const LinkItem& item, DWORD openFlag);
	static void	ShowRClickMenuAndExecCommand(LinkFolderPtr pFolder, LinkItem* pLinkItem, HWND hwnd);
	static bool	s_bNowShowRClickMenu;
	static void SaveLinkBookmark();
	static void SortByName(LinkFolderPtr pFolder);

	static void	GetFaviconToLinkItem(const CString& url, LinkFolderPtr pFolder, LinkItem* pItem, HWND hwnd);

	static CIcon	s_iconFolder;
	static CIcon	s_iconLink;

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

	void DoPaint(CDCHandle dc);

	void DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine);

	/// �J�[�\����̃A�C�e����HOT��Ԃɂ���
	void OnTrackMouseMove(UINT nFlags, CPoint pt);

	/// �A�C�e����HOT��Ԃ���������
	void OnTrackMouseLeave();

	// IDropTargetImpl
	DROPEFFECT OnDragEnter(IDataObject *pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(IDataObject *pDataObject, DWORD dwKeyState, CPoint point, DROPEFFECT dropOkEffect);
	DROPEFFECT OnDrop(IDataObject *pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropEffectList, CPoint point);
	void	OnDragLeave();

	BEGIN_MSG_MAP( CLinkPopupMenu )
		MSG_WM_CREATE( OnCreate )
		MSG_WM_DESTROY( OnDestroy )
		MSG_WM_TIMER( OnTimer )
		MSG_WM_ERASEBKGND	( OnEraseBkgnd )
		MSG_WM_MOUSEACTIVATE( OnMouseActivate )
		MSG_WM_KEYDOWN( OnKeyDown )
		MSG_WM_LBUTTONDOWN( OnLButtonDown )
		MSG_WM_RBUTTONUP	( OnRButtonUp )
		MSG_WM_MBUTTONDOWN( OnMButtonDown )
		NOTIFY_CODE_HANDLER_EX(TTN_GETDISPINFO, OnTooltipGetDispInfo)
		CHAIN_MSG_MAP( CScrollWindowImpl<CLinkPopupMenu> )
		CHAIN_MSG_MAP( CThemeImpl<CLinkPopupMenu> )
		CHAIN_MSG_MAP( CTrackMouseLeave<CLinkPopupMenu> )
		CHAIN_MSG_MAP_ALT(CScrollWindowImpl<CLinkPopupMenu>, 1)
	END_MSG_MAP()

	int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnTimer(UINT_PTR nIDEvent);
	BOOL OnEraseBkgnd(CDCHandle dc) { return TRUE; }

	/// �N���b�N�ŃA�N�e�B�u�ɂȂ�Ȃ��悤�ɂ���
	int  OnMouseActivate(CWindow wndTopLevel, UINT nHitTest, UINT message) { return MA_NOACTIVATE; }

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnRButtonUp(UINT nFlags, CPoint point);
	void OnMButtonDown(UINT nFlags, CPoint point);
	LRESULT OnTooltipGetDispInfo(LPNMHDR pnmh);

private:
	void	_InitTooltip();
	void	_UpdateItemPosition();
	bool	_IsValidIndex(int nIndex) const;
	CRect	_GetClientItemRect(const CRect& rcItem);
	boost::optional<int>	_HitTest(const CPoint& point);
	void	_HotItem(int nNewHotIndex);
	void	_DoExec(const CPoint& pt, bool bLButtonDown = false);
	void	_CloseSubMenu();
	static void	_CloseBaseSubMenu();

	void	_DrawInsertEdge(const CRect& rcEdge);
	void	_ClearInsertionEdge();
	void	_DrawDragImage(CDCHandle dc, const LinkItem& item);

	enum hitTestFlag {
		htItemNone,
		htItemTop,
		htItemBottom,
		htFolder,
	};
	int		_HitTestOnDragging(const CPoint& pt, hitTestFlag& htflag);

	// Data members
	LinkFolderPtr	m_pFolder;
	CFont	m_font;
	int		m_nHotIndex;
	CLinkPopupMenu*	m_pSubMenu;
	const int m_nInheritFolderIndex;
	static HWND	s_wndLinkBar;

	CRect	m_rcInvalidateOnDrawingInsertionEdge;
	int		m_nNowDragItemIndex;
	std::pair<LinkFolderPtr, int>	m_DragItemData;
	bool			m_bAcceptDrag;
	bool			m_bDragItemIsLinkFile;
	bool			m_bDrawDisableHot;
	static bool		s_bNoCloseBaseSubMenu;

	CToolTipCtrl	m_tip;
	CString			m_strTipText;
};



























