/**
 *	@file	ToolBarDialog.cpp
 *	@brief	donut�̃I�v�V���� : �c�[���o�[
 */

#include "stdafx.h"
#include "ToolBarDialog.h"
#include "../IniFile.h"
#include "../DonutPFunc.h"
#include "../XmlFile.h"
#include "../ToolTipManager.h"
#include "../MtlWin.h"

using namespace MTL;


//----------------------------
/// �f�t�H���g�̃c�[���o�[�{�^����ݒ�
void	SetDefaultToolBarButton(bool bLoadOnlyIndex = false)
{
	if (bLoadOnlyIndex == false) {
		static const STD_TBBUTTON btns[] = {
			{ ID_FILE_NEW,					BTNS_BUTTON | BTNS_DROPDOWN 											 },	//  1
			{ ID_VIEW_BACK, 				BTNS_BUTTON | BTNS_STD_LIST | BTNS_DROPDOWN 							 },	//  2
			{ ID_VIEW_FORWARD,				BTNS_BUTTON | BTNS_STD_LIST | BTNS_DROPDOWN 							 }, //	3
			{ ID_VIEW_STOP, 				BTNS_BUTTON 															 },	//	4
			{ ID_VIEW_REFRESH,				BTNS_BUTTON 															 },	//  5
			{ ID_VIEW_HOME, 				BTNS_BUTTON 															 },	//	6
			{ ID_FILE_NEW_CLIPBOARD2,		BTNS_BUTTON | BTNS_DROPDOWN 											 },	//	7

			{ ID_VIEW_STOP_ALL, 			BTNS_BUTTON 															 },	//	8
			{ ID_VIEW_REFRESH_ALL,			BTNS_BUTTON 															 },	//	9

			{ ID_VIEW_FAVEXPBAR,			BTNS_BUTTON | BTNS_STD_LIST 											 }, //BTNS_DROPDOWN},
			{ ID_VIEW_FAVEXPBAR_HIST,		BTNS_BUTTON | BTNS_STD_LIST 											 },	// 10
			{ ID_VIEW_CLIPBOARDBAR, 		BTNS_BUTTON | BTNS_STD_LIST 											 },	// 11

			{ ID_FILE_PRINT,				BTNS_BUTTON 															 },	// 12

			{ ID_VIEW_FULLSCREEN,			BTNS_BUTTON 															 },	// 13
			{ ID_WINDOW_CASCADE,			BTNS_BUTTON 															 },	// 14
			{ ID_WINDOW_TILE_HORZ,			BTNS_BUTTON 															 },	// 15
			{ ID_WINDOW_TILE_VERT,			BTNS_BUTTON 															 },	// 16

			{ ID_VIEW_TABBAR_MULTI, 		BTNS_BUTTON 															 },	// 17
			{ ID_TAB_LEFT,					BTNS_BUTTON 															 },	// 18
			{ ID_TAB_RIGHT, 				BTNS_BUTTON 															 },	// 19

			{ ID_EDIT_CUT,					BTNS_BUTTON 															 },	// 20
			{ ID_EDIT_COPY, 				BTNS_BUTTON 															 },	// 21
			{ ID_EDIT_PASTE,				BTNS_BUTTON 															 },	// 22

			{ ID_VIEW_FONT_SIZE,			BTNS_BUTTON | BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN						 },	// 23
			{ ID_DOCHOSTUI_OPENNEWWIN,		BTNS_BUTTON 															 },	// 24

			{ ID_DLCTL_DLIMAGES,			BTNS_BUTTON 															 },	// 25
			{ ID_DLCTL_SCRIPTS, 			BTNS_BUTTON 															 },	// 26
			{ ID_DLCTL_JAVA,				BTNS_BUTTON 															 },	// 27
			{ ID_DLCTL_RUNACTIVEXCTLS,		BTNS_BUTTON 															 },	// 28
			{ ID_DLCTL_DLACTIVEXCTLS,		BTNS_BUTTON 															 },	// 29
			{ ID_DLCTL_BGSOUNDS,			BTNS_BUTTON 															 },	// 30
			{ ID_DLCTL_VIDEOS,				BTNS_BUTTON 															 },	// 31

			{ ID_FILE_CLOSE,				BTNS_BUTTON 															 },	// 32
			{ ID_WINDOW_CLOSE_ALL,			BTNS_BUTTON 															 },	// 33
			{ ID_WINDOW_CLOSE_EXCEPT,		BTNS_BUTTON 															 },	// 34

			{ ID_MAIN_EX_NEWWINDOW, 		BTNS_BUTTON 															 },	// 35
			{ ID_MAIN_EX_NOACTIVATE,		BTNS_BUTTON 															 },	// 36
			{ ID_MAIN_EX_NOACTIVATE_NEWWIN, BTNS_BUTTON 															 },	// 37
			{ ID_REGISTER_AS_BROWSER,		BTNS_BUTTON 															 },	// 38
			{ ID_FAVORITES_DROPDOWN,		BTNS_BUTTON | BTNS_STD_LIST | BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN		 },	// 39
			{ ID_EDIT_OPEN_SELECTED_REF,	BTNS_BUTTON 															 },	// 40
			{ ID_EDIT_OPEN_SELECTED_TEXT,	BTNS_BUTTON 															 },	// 41

			// U.H
			{ ID_WINDOW_REFRESH_EXCEPT, 	BTNS_BUTTON 															 },	// 42

			//+++
			{ ID_VIEW_UP				,	BTNS_BUTTON 															 },	// 43
			{ ID_VIEW_FAVEXPBAR_GROUP	,	BTNS_BUTTON 															 },	// 44
			{ ID_VIEW_SOURCE			,	BTNS_BUTTON 															 },	// 45
			{ ID_VIEW_SOURCE_SELECTED	,	BTNS_BUTTON 															 },	// 46
			{ 57609						,	BTNS_BUTTON 															 },	// 47	//+++ "����v���r���[(&V)..."
			{ 57636						,	BTNS_BUTTON 															 },	// 48	//+++ "���̃y�[�W�̌���(&F)..."
			{ ID_EDIT_FIND_MAX			,	BTNS_BUTTON 															 },	// 49
			{ ID_VIEW_SEARCHBAR			,	BTNS_BUTTON 															 },	// 50
			{ ID_RECENT_DOCUMENT		,	BTNS_BUTTON | BTNS_DROPDOWN												 },	// 51
			{ ID_DLCTL_CHG_SECU			,	BTNS_BUTTON | BTNS_DROPDOWN												 },	// 52
			{ ID_DLCTL_CHG_MULTI		,	BTNS_BUTTON | BTNS_DROPDOWN												 },	// 53
			{ ID_URLACTION_COOKIES_CHG	,	BTNS_BUTTON | BTNS_DROPDOWN												 },	// 54
			{ ID_VIEW_FAVEXPBAR_USER	,	BTNS_BUTTON 															 },	// 55
			{ ID_URLACTION_COOKIES		,	BTNS_BUTTON 															 },	// 56
			{ ID_URLACTION_COOKIES_SESSION,	BTNS_BUTTON 															 },	// 57
			{ ID_FAVORITES_GROUP_DROPDOWN,	BTNS_BUTTON | BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN						 },	// 58
			{ ID_AUTO_REFRESH			,	BTNS_BUTTON | BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN						 },	// 59
			{ ID_VIEW_OPTION_DONUT		,	BTNS_BUTTON 															 },	// 60
			{ ID_VIEW_OPTION			,	BTNS_BUTTON 															 },	// 61
			{ ID_APP_HELP				,	BTNS_BUTTON 															 },	// 62
			{ ID_LEFT_CLOSE				,	BTNS_BUTTON 															 },	// 63
			{ ID_RIGHT_CLOSE			,	BTNS_BUTTON 															 },	// 64
			//{ 0						,	BTNS_BUTTON 															 },
			//{ 0						,	BTNS_BUTTON 															 },
		};
		CToolBarOption::s_vecTBbtns.insert(CToolBarOption::s_vecTBbtns.begin(), btns, btns + _countof(btns));
	}

	static const int defaultBtns[] = { 0, 1, 2, 3, 4, 5, 51, 60, -1, 9, 10, -1, 12 };
	CToolBarOption::s_vecShowBtn.insert(CToolBarOption::s_vecShowBtn.begin(), defaultBtns, defaultBtns + _countof(defaultBtns));

}


/////////////////////////////////////////////////
// CToolBarOption

// ��`
vector<STD_TBBUTTON>	CToolBarOption::s_vecTBbtns;
vector<int>				CToolBarOption::s_vecShowBtn;
DWORD					CToolBarOption::s_dwToolbarStyle = STD_TBSTYLE_DEFAULT;


/// �ݒ���擾����
void CToolBarOption::GetProfile()
{
	s_vecTBbtns.clear();
	s_vecShowBtn.clear();
	bool bSucceeded = false;

	CString strToolbarxmlPath = _GetSkinDir() + _T("Toolbar.xml");	// �V�`��
	CString strToolbarShowButtonPath = _GetSkinDir() + _T("ToolbarShowButton.xml");
	if (::PathFileExists(strToolbarxmlPath)) {
		try {
		CXmlFileRead2 xmlRead(strToolbarxmlPath);
		XmlNodeType 	nodeType;
		while (xmlRead.Read(&nodeType)) {
			if (nodeType == XmlNodeType_Element
				&& xmlRead.GetLocalName() == _T("DonutToolBar")) 
			{
				CString strElement;
				while (xmlRead.GetInternalElement(_T("DonutToolBar"), strElement)) {
					if (strElement == _T("item")) {
						STD_TBBUTTON tbbtn;
						xmlRead.MoveToFirstAttribute();
						tbbtn.idCommand = xmlRead.GetValuei();
						xmlRead.MoveToNextAttribute();
						tbbtn.fsStyle = xmlRead.GetValuei();
						s_vecTBbtns.push_back(tbbtn);
					}
				}
			}
		}
		
		bSucceeded = true;

		} catch (LPCTSTR strError) {
			MessageBox(NULL, strError, NULL, NULL);
		}
	}
	if (::PathFileExists(strToolbarShowButtonPath) && bSucceeded) {
		try {
		CXmlFileRead2	xmlReadShowBtn(strToolbarShowButtonPath);
		XmlNodeType	nodeType;
		while (xmlReadShowBtn.Read(&nodeType)) {
			if (nodeType == XmlNodeType_Element
				&& xmlReadShowBtn.GetLocalName() == _T("ToolBarShowButton")) 
			{
				xmlReadShowBtn.MoveToFirstAttribute();
				if (xmlReadShowBtn.GetLocalName() == _T("ToolbarStyle"))
					s_dwToolbarStyle = _wtol(xmlReadShowBtn.GetValue());
				else
					s_dwToolbarStyle = STD_TBSTYLE_DEFAULT;
				CString Element;
				while (xmlReadShowBtn.GetInternalElement(_T("ToolBarShowButton"), Element)) {
					if (Element == _T("button")) {
						xmlReadShowBtn.MoveToFirstAttribute();
						s_vecShowBtn.push_back(xmlReadShowBtn.GetValuei());
					}
				}
			}
		}
		} catch (LPCTSTR strError) {
			MessageBox(NULL, strError, NULL, NULL);
		}
	} else if (::PathFileExists(strToolbarShowButtonPath) == FALSE && bSucceeded) {	// ToolBarShowButton.xml�݂̂Ȃ�����
		SetDefaultToolBarButton(true);	// �f�t�H���g���g�p
		bSucceeded = true;
	}

	if (bSucceeded == false) {	// ���^��
		CString strToolbarIniPath = _GetSkinDir() + _T("Toolbar.ini");
		if (::PathFileExists(strToolbarIniPath)) {
			CIniFileI	pr(strToolbarIniPath, _T("TOOLBAR"));
			int nCount = pr.GetValuei(_T("TOOLBAR_CNT"));
			for (int i = 0; i < nCount; ++i) {
				STD_TBBUTTON tbbtn;
				CString strID = _T("ID_");
				CString strStyle = _T("STYLE_");
				strID.Append(i);
				strStyle.Append(i);
				tbbtn.idCommand = pr.GetValuei(strID);
				tbbtn.fsStyle	= pr.GetValuei(strStyle);
				s_vecTBbtns.push_back(tbbtn);
			}
			nCount = pr.GetValuei(_T("button.count"));
			for (int i = 0; i < nCount; ++i) {
				CString strtemp;
				strtemp.Format(_T("button%d.iBitmap"), i);
				s_vecShowBtn.push_back(pr.GetValuei(strtemp));
			}
			s_dwToolbarStyle = pr.GetValue(_T("Std_ToolBar_Style"), STD_TBSTYLE_DEFAULT);
			WriteProfileToolbar();	// �V�`���ɂ��Ă���
			bSucceeded = true;
		}
	}

	if (bSucceeded == false) {	// �f�t�H���g
		SetDefaultToolBarButton();
		WriteProfileToolbar();	// �V�`���ɂ��Ă���
		bSucceeded = true;
	}
	ATLASSERT(s_vecTBbtns.size() > 0);
	ATLASSERT(s_vecShowBtn.size() > 0);
}


//-----------------------------
/// Toolbar.xml��ۑ�
void CToolBarOption::WriteProfileToolbar()
{
	CString strToolbarOldPath = _GetSkinDir() + _T("Toolbar.ini");
	if (::PathFileExists(strToolbarOldPath)) {
		WriteProfileToolbarShowButton();
		Misc::MoveToBackupFile(strToolbarOldPath);	// ���^����bak��
	}
	CString SkinDir = _GetSkinDir();
	CString strToolbarxmlPath = SkinDir + _T("Toolbar.xml");
	if (::PathIsDirectory(SkinDir) == FALSE)
		::SHCreateDirectory(NULL, SkinDir);
	try {
		CXmlFileWrite	xmlWrite(strToolbarxmlPath);
		xmlWrite.WriteStartElement(L"DonutToolBar");
		for (auto it = s_vecTBbtns.cbegin(); it != s_vecTBbtns.cend(); ++it) {
			xmlWrite.WriteStartElement(L"item");
			xmlWrite.WriteAttributeValue(L"command", it->idCommand);
			xmlWrite.WriteAttributeValue(L"style", it->fsStyle);
			xmlWrite.WriteFullEndElement();
		}
		xmlWrite.WriteFullEndElement();
	} catch (LPCTSTR strError) {
		MessageBox(NULL, strError, NULL, NULL);
	}
	
}

//----------------------------
/// ToolbarShowButton.xml��ۑ�
void CToolBarOption::WriteProfileToolbarShowButton()
{
	CString strToolbarShowButtonPath = _GetSkinDir() + _T("ToolbarShowButton.xml");

	try {
		CXmlFileWrite	xmlWrite(strToolbarShowButtonPath);
		xmlWrite.WriteStartElement(L"ToolBarShowButton");
		xmlWrite.WriteAttributeValue(L"ToolbarStyle", s_dwToolbarStyle);
		for (auto it = s_vecShowBtn.cbegin(); it != s_vecShowBtn.cend(); ++it) {
			xmlWrite.WriteStartElement(L"button");
			xmlWrite.WriteAttributeValue(L"index", *it);
			xmlWrite.WriteFullEndElement();
		}
		xmlWrite.WriteFullEndElement();
	} catch (LPCTSTR strError) {
		MessageBox(NULL, strError, NULL, NULL);
	}
}










extern const UINT	g_uDropDownCommandID[];
extern const UINT	g_uDropDownWholeCommandID[];
extern const int	g_uDropDownCommandCount;
extern const int	g_uDropDownWholeCommandCount;


namespace {

CString GetBigFilePath()
{
	CString strPath = _GetSkinDir();

	strPath += _T("BigHot.bmp");
	return strPath;
}



CString GetSmallFilePath()
{
	CString strPath = _GetSkinDir();

	strPath += _T("SmallHot.bmp");
	return strPath;
}

};	// namespace


/////////////////////////////////////////////////////////
// CToolBarPropertyPage

CToolBarPropertyPage::CToolBarPropertyPage(HMENU hMenu, BOOL *pbSkinChange, function<void ()> funcReloadSkin)
	: m_bInit(false), m_bChanged(false), m_funcReloadSkin(funcReloadSkin)
{
	m_hMenu 		 = hMenu;
	m_bExistSkin	 = FALSE;
	m_pbSkinChanged  = pbSkinChange;
}


// Overrides

//------------------------------
/// �_�C�A���O���\�����ꂽ�Ƃ�
BOOL CToolBarPropertyPage::OnSetActive()
{
	SetModified(TRUE);

	if (m_bInit == false) {
		m_bInit = true;
		DoDataExchange(DDX_LOAD);

		_SetData();
	} else {
		// ���X�g�r���[����������
		InitialListCtrl();
	}
	return TRUE;
}

//-------------------------------
/// �_�C�A���O����\���ɂ����Ƃ�
BOOL CToolBarPropertyPage::OnKillActive()
{
	return DoDataExchange(DDX_SAVE);
}

//-------------------------
/// �K�p���������Ƃ�
BOOL CToolBarPropertyPage::OnApply()
{
	if ( DoDataExchange(DDX_SAVE) ) {
		_GetData();
		return TRUE;
	} else {
		return FALSE;
	}
}


void	CToolBarPropertyPage::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (lpDrawItemStruct->CtlID == IDC_LIST_ICON) {
		CDCHandle dc = lpDrawItemStruct->hDC;

		// Save these value to restore them when done drawing.
		COLORREF crOldTextColor = dc.GetTextColor();
		COLORREF crOldBkColor = dc.GetBkColor();

		// If this item is selected, set the background color 
		// and the text color to appropriate values. Also, erase
		// rect by filling it with the background color.
		if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
			(lpDrawItemStruct->itemState & ODS_SELECTED))
		{
			dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, 
				::GetSysColor(COLOR_HIGHLIGHT));
		} else
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, crOldBkColor);
#if 0
		// If this item has the focus, draw a red frame around the
		// item's rect.
		if ((lpDrawItemStruct->itemAction | ODA_FOCUS) &&
			(lpDrawItemStruct->itemState & ODS_FOCUS))
		{
			CBrush br;
			br.CreateSolidBrush(RGB(255, 0, 0));
			dc.FrameRect(&lpDrawItemStruct->rcItem, br);
		}
#endif
		IconListData* pData = (IconListData*)lpDrawItemStruct->itemData;
		if (pData) {
			CIconHandle icon = m_imgList.GetIcon(pData->nIndex);
			if (icon.m_hIcon)
				icon.DrawIconEx(dc, lpDrawItemStruct->rcItem.left + cxMargin, lpDrawItemStruct->rcItem.top + cyMargin, m_iconSize.cx, m_iconSize.cy);

			lpDrawItemStruct->rcItem.left += m_iconSize.cx + cxMargin + IconTextMargin;
			// Draw the text.
			dc.DrawText(
				pData->strText,
				pData->strText.GetLength(),
				&lpDrawItemStruct->rcItem,
				DT_SINGLELINE | DT_VCENTER);
		}

		// Reset the background color and the text color back to their
		// original values.
		dc.SetTextColor(crOldTextColor);
		dc.SetBkColor(crOldBkColor);
	}
}

void	CToolBarPropertyPage::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (lpMeasureItemStruct->CtlID == IDC_LIST_ICON) {
		enum { cyMargin = 2 };
		SIZE size;
		m_imgList.GetIconSize(size);
		if (size.cy < 16)
			size.cy = 16;	// Notoolbar+�Ƃ��p
		size.cy += cyMargin*2;
		lpMeasureItemStruct->itemHeight	= size.cy;
	}
}




//-------------------------------
/// �f�[�^�𓾂�
void CToolBarPropertyPage::_SetData()
{
	// �R���{�{�b�N�̏�����
	InitialCombbox();
	// ���X�g�r���[�̏�����
	InitialListCtrl();
}


//-------------------------------
/// �f�[�^��ۑ�
void CToolBarPropertyPage::_GetData()
{
	if (m_bExistSkin && m_pbSkinChanged) {
		WriteProfileToolbar();
	}
}


//-------------------------------
// �R���{�{�b�N�X�̏�����
void CToolBarPropertyPage::InitialCombbox()
{
	if (_SetCombboxCategory(m_cmbCategory, m_hMenu) == FALSE)
		return;

	m_cmbCategory.AddString( _T("�h���b�v�_�E���E�A�C�R��") );

	// ���z�I�ɁA�I��ύX
	OnSelChangeCate(0, 0, 0);
}


//----------------------------------
/// �J�e�S���ύX��
void CToolBarPropertyPage::OnSelChangeCate(UINT code, int id, HWND hWnd)
{
	int nIndex = m_cmbCategory.GetCurSel();
	_SetCommandList(nIndex);
}


//-------------------------------------
/// ���X�g�r���[�̏�����
void CToolBarPropertyPage::InitialListCtrl()
{
	// �C���[�W���X�g����
	_InitImageList();

	CLockRedraw lock(m_ltIcon);
	int nCount = m_ltIcon.GetCount();
	for (int i = 0; i < nCount; ++i) {
		delete (IconListData*)m_ltIcon.GetItemDataPtr(i);
		m_ltIcon.SetItemDataPtr(i, NULL);
	}
	while (m_ltIcon.DeleteString(0) != LB_ERR) ;

	int nSize = (int)s_vecTBbtns.size();
	for (int i = 0; i < nSize; ++i) {
		UINT	nID = s_vecTBbtns[i].idCommand;

		IconListData* pData = new IconListData;
		CToolTipManager::LoadToolTipText(nID, pData->strText);
		pData->nIndex = i;

		m_ltIcon.AddString(pData->strText);
		m_ltIcon.SetItemDataPtr(i, pData);
	}
	m_bExistSkin = TRUE;
}

//------------------------------------
void	CToolBarPropertyPage::OnDestroy()
{
	int nCount = m_ltIcon.GetCount();
	for (int i = 0; i < nCount; ++i) {
		delete (IconListData*)m_ltIcon.GetItemDataPtr(i);
	}
	if (m_bChanged && m_funcReloadSkin)
		m_funcReloadSkin();
}


//-------------------------------------
/// �E���e�L�X�g�\����؂�ւ�
LRESULT CToolBarPropertyPage::OnChkBtnText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	DoDataExchange(TRUE);

	int nIndexLt = m_ltIcon.GetCaretIndex();

	if (nIndexLt == -1)
		return 0;

	int nChk	 = (int) ::SendMessage(GetDlgItem(IDC_CHKBTN_TXT), BM_GETCHECK, 0L, 0L);

	if (nChk == BST_CHECKED)
		s_vecTBbtns[nIndexLt].fsStyle |= BTNS_STD_LIST;
	else
		s_vecTBbtns[nIndexLt].fsStyle &= ~BTNS_STD_LIST;

	return 0;
}


//------------------------------------------
/// ���X�g�r���[�̑I���A�C�e�����ύX���ꂽ
void CToolBarPropertyPage::OnSelChangeCommandIcon(UINT code, int id, HWND hWnd)
{
	int nIndex = m_ltIcon.GetCaretIndex();
	if (nIndex == -1)
		return;

	GetDlgItem(IDC_CHKBTN_TXT).EnableWindow(TRUE);
	if (s_vecTBbtns[nIndex].fsStyle & BTNS_STD_LIST)
		::SendMessage(GetDlgItem(IDC_CHKBTN_TXT), BM_SETCHECK, BST_CHECKED, 0L);
	else
		::SendMessage(GetDlgItem(IDC_CHKBTN_TXT), BM_SETCHECK, BST_UNCHECKED, 0L);
}


//----------------------------------
/// �R�}���h�ύX
void	CToolBarPropertyPage::OnChangeCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nCmdIndex	= m_listCommand.GetCurSel();
	int nIconIndex	= m_ltIcon.GetCurSel();
	if (nCmdIndex == -1 || nIconIndex == -1)
		return;

	UINT nCommand = (UINT)m_listCommand.GetItemData(nCmdIndex);
	s_vecTBbtns[nIconIndex].idCommand = nCommand;
	s_vecTBbtns[nIconIndex].fsStyle	  = GetToolButtonStyle(nCommand);

	CString strText;
	CToolTipManager::LoadToolTipText(nCommand, strText);
	IconListData* pData = (IconListData*)m_ltIcon.GetItemDataPtr(nIconIndex);
	pData->strText = strText;

	CRect rc;
	m_ltIcon.GetItemRect(nIconIndex, &rc);
	m_ltIcon.InvalidateRect(&rc);

	m_bChanged = true;
}



DWORD CToolBarPropertyPage::GetToolButtonStyle(UINT nTarID)
{
	int ii;

	for (ii = 0; ii < g_uDropDownCommandCount; ii++) {
		if (nTarID == g_uDropDownCommandID[ii])
			return BTNS_BUTTON | BTNS_DROPDOWN;
	}

	for (ii = 0; ii < g_uDropDownWholeCommandCount; ii++) {
		if (nTarID == g_uDropDownWholeCommandID[ii]) {
			if (nTarID == ID_FAVORITES_DROPDOWN)
				return BTNS_BUTTON | BTNS_STD_LIST | BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN;
			else
				return BTNS_BUTTON | BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN;
		}
	}

	return BTNS_BUTTON;
}



//---------------------------------
/// nIndex�̓J�e�S���R���{�{�{�b�N�X�̌��ݑI������Ă���Z��
/// �R�}���h���X�g�ɃJ�e�S���őI�����ꂽ���j���[��o�^����
void	CToolBarPropertyPage::_SetCommandList(int nIndex)
{
	m_listCommand.ResetContent();

	if (nIndex + 1 == m_cmbCategory.GetCount()) {
		// �h���b�v�_�E���E�A�C�R���̂Ƃ�
		for (int ii = 0; ii < g_uDropDownCommandCount; ii++) {
			CString strMenu;
			strMenu.LoadString(g_uDropDownCommandID[ii]);

			if (strMenu.Find(_T("\n")) != -1)
				strMenu = strMenu.Left( strMenu.Find(_T("\n")) );

			int 	nIndex = m_listCommand.AddString(strMenu);
			m_listCommand.SetItemData(nIndex, g_uDropDownCommandID[ii]);
		}

		for (int ii = 0; ii < g_uDropDownWholeCommandCount; ii++) {
			CString strMenu;
			strMenu.LoadString(g_uDropDownWholeCommandID[ii]);

			if (strMenu.Find(_T("\n")) != -1)
				strMenu = strMenu.Left( strMenu.Find(_T("\n")) );

			int 	nIndex = m_listCommand.AddString(strMenu);
			m_listCommand.SetItemData(nIndex, g_uDropDownWholeCommandID[ii]);
		}
	} else {
		CMenuHandle	subMenu = ::GetSubMenu(m_hMenu, nIndex);
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

	{	// �s�v�ȃZ�p���[�^�̍폜
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


void	CToolBarPropertyPage::_AddCommandtoListFromSubMenu(CMenuHandle subMenu)
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


//-----------------------
/// �C���[�W���X�g������������
void	CToolBarPropertyPage::_InitImageList()
{
	if (m_imgList.m_hImageList)
		m_imgList.Destroy();

	CBitmap bmp;
	bmp.Attach( AtlLoadBitmapImage(GetSmallFilePath().GetBuffer(0), LR_LOADFROMFILE) );
	if (bmp.IsNull()) {
		//bBig = FALSE; 								//+++ small�p�̕\�����o�O���Ă�ۂ��̂ŁA�Ƃ肠����big�����őΏ�.
		bmp.Attach( AtlLoadBitmapImage(GetBigFilePath().GetBuffer(0), LR_LOADFROMFILE) );
		if (bmp.IsNull()) {					//+++ ������bmp���g���悤�ɂ���...
			bmp.LoadBitmap(IDB_MAINFRAME_TOOLBAR_HOT);	//+++ ������bmp���g���悤�ɂ���...
			if (bmp.IsNull()) {
			/*	::MessageBox(m_hWnd, _T("�c�[���o�[�X�L���t�@�C����������܂���ł����B\n")
									 _T("�J�X�^�}�C�Y�Ɏx�Ⴊ�o��̂ő��삪�ł��Ȃ��悤�ɂȂ��Ă��܂��B\n")
									 _T("�X�L���t�H���_��BigHot.bmp�t�@�C�����������Ă��������B")			, _T("information"), MB_OK);*/
				ATLASSERT(FALSE);
				return;
			}
		}
	}

	bmp.GetSize(m_iconSize);
	int nCount	= m_iconSize.cx / m_iconSize.cy;
	m_iconSize.cx = m_iconSize.cy;
	MTLVERIFY( m_imgList.Create(m_iconSize.cx, m_iconSize.cy, ILC_COLOR24 | ILC_MASK, nCount, 1) );
	MTLVERIFY( m_imgList.Add( bmp, RGB(255, 0, 255) ) != -1 );
}









