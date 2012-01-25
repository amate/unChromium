/**
 *	@file KeyBoardDialog.cpp
 *	@brief	donut�̃I�v�V���� : �L�[
 */

#include "stdafx.h"
#include "KeyBoardDialog.h"
#include "../MtlBase.h"
#include "../DonutPFunc.h"
#include "../ToolTipManager.h"
#include "MainOption.h"			//+++


#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




CKeyBoardPropertyPage::CKeyBoardPropertyPage(HACCEL hAccel, HMENU hMenu)
{
	m_hAccel	 	= hAccel;
	m_hMenu 	 	= hMenu;
	m_nCmdUpdate 	= 0;
	m_nCmdMove	 	= 0;
	//m_nMinBtn2Tray= 0;		//+++

	int    nAccelSize = ::CopyAcceleratorTable(m_hAccel, NULL, 0);
	ACCEL *lpAccel	  = new ACCEL [nAccelSize];
	::CopyAcceleratorTable(hAccel, lpAccel, nAccelSize);
	m_hAccel	 = ::CreateAcceleratorTable(lpAccel, nAccelSize);
	delete[] lpAccel;
}



CKeyBoardPropertyPage::~CKeyBoardPropertyPage()
{
	::DestroyAcceleratorTable (m_hAccel);
}



// Overrides
BOOL CKeyBoardPropertyPage::OnSetActive()
{
	SetModified(TRUE);

	if (m_editNow.m_hWnd == NULL)
		m_editNow.Attach( GetDlgItem(IDC_EDIT_NOW_KEY) );

	if (m_editNew.m_hWnd == NULL)
		m_editNew.Attach( GetDlgItem(IDC_EDIT_NEW_KEY) );

	if (m_cmbCategory.m_hWnd == NULL)
		m_cmbCategory.Attach( GetDlgItem(IDC_CMB_CATEGORY) );

	if (m_cmbCommand.m_hWnd == NULL)
		m_cmbCommand.Attach( GetDlgItem(IDC_CMB_COMMAND) );

	if (m_ltAccel.m_hWnd == NULL) {
		m_ltAccel.Attach( GetDlgItem(IDC_LISTBOX1) );	//+++���x���ύX
	}

	if (m_editNow.m_hWnd && m_editNew.m_hWnd && m_cmbCategory.m_hWnd && m_cmbCommand.m_hWnd)
		_SetData();

	return DoDataExchange(FALSE);
}



BOOL CKeyBoardPropertyPage::OnKillActive()
{
	return DoDataExchange(TRUE);
}



BOOL CKeyBoardPropertyPage::OnApply()
{
	if ( DoDataExchange(TRUE) ) {
		_GetData();
		return TRUE;
	} else {
		return FALSE;
	}
}



// Constructor
// �f�[�^�𓾂�
void CKeyBoardPropertyPage::_SetData()
{
	// �R���{�{�b�N�̏�����
	InitialCombbox();
	// ���X�g�{�b�N�X�̏�����
	InitialListbox();

  #if 0 //+++ �ʂ̃I�v�V�������Ǘ����Ă���ϐ����Ԏ؂肵�Ēǉ�.
	m_nMinBtn2Tray	  = (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_MINBTN2TRAY) != 0;		//+++ �ǉ�.
  #endif
}



// �f�[�^��ۑ�
void CKeyBoardPropertyPage::_GetData()
{
	CAccelerManager accelManager(m_hAccel);

	accelManager.SaveAccelaratorState();

  #if 0 //+++ �ʂ̃I�v�V�������Ǘ����Ă���ϐ����Ԏ؂肵�Ēǉ�.
	if (m_nMinBtn2Tray)	CMainOption::s_dwMainExtendedStyle2 	 |=  MAIN_EX2_MINBTN2TRAY;		//+++ �ǉ�.
	else				CMainOption::s_dwMainExtendedStyle2 	 &= ~MAIN_EX2_MINBTN2TRAY; 		//+++ �ǉ�.
  #endif
}



LRESULT CKeyBoardPropertyPage::OnChkBtn(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	DoDataExchange(TRUE);

	// �ݒ�{�^����Enable����
	EnableSetBtn();
	return 0;
}



void CKeyBoardPropertyPage::OnBtnDel(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/)
{
	CString 		strAccel;
	TCHAR			szAccel[MAX_PATH];
	szAccel[0]	= 0;	//+++

	if (m_editNow.GetWindowText(szAccel, MAX_PATH) == 0)
		return;

	int 			nIndexCmd = m_cmbCommand.GetCurSel();
	UINT			nCmdID	  = (UINT) m_cmbCommand.GetItemData(nIndexCmd);

	CAccelerManager accelManager(m_hAccel);
	m_hAccel = accelManager.DeleteAccelerator(nCmdID);

	// �����I�ɁA�ύX���ꂽ���ɂ���(;^_^A �������
	OnSelChangeCmd(0, 0, NULL);

	SetAccelList();
}



void CKeyBoardPropertyPage::OnBtnSel(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/)
{
	// CommandID
	int 			nIndexCmd = m_cmbCommand.GetCurSel();
	UINT			nCmdID	  = (UINT) m_cmbCommand.GetItemData(nIndexCmd);

	// �L�[�𓾂�
	ACCEL			accel;

	memcpy( &accel, m_editNew.GetAccel(), sizeof (ACCEL) );
	accel.fVirt |= FNOINVERT;
	accel.fVirt |= FVIRTKEY;
	accel.cmd	 = nCmdID;

	//
	CAccelerManager accelManager(m_hAccel);

	TCHAR			cBuff[MAX_PATH];
	cBuff[0]		= 0;	//+++
	CString 		strAccelNow;
	CString 		strCmd;

	m_editNow.GetWindowText(cBuff, MAX_PATH);
	strAccelNow  = CString(cBuff);

	::GetWindowText(GetDlgItem(IDC_STC01), cBuff, MAX_PATH);
	strCmd		 = CString(cBuff);

	if ( strAccelNow.IsEmpty() ) {
		// �����R�}���h����������
		if ( !strCmd.IsEmpty() ) {
			UINT uOldCmd = accelManager.FindCommandID(&accel);
			m_hAccel = accelManager.DeleteAccelerator(uOldCmd);
		}

		m_hAccel = accelManager.AddAccelerator(&accel);
	} else {
		if ( !strAccelNow.IsEmpty() && !strCmd.IsEmpty() ) {
			UINT uOldCmd = accelManager.FindCommandID(&accel);

			m_hAccel = accelManager.DeleteAccelerator(uOldCmd);
			m_hAccel = accelManager.DeleteAccelerator(nCmdID);
			m_hAccel = accelManager.AddAccelerator(&accel);
		} else {
			m_hAccel = accelManager.DeleteAccelerator(nCmdID);
			m_hAccel = accelManager.AddAccelerator(&accel);
		}
	}

	// �����I�ɁA�ύX���ꂽ���ɂ���(;^_^A �������
	OnSelChangeCmd(0, 0, NULL);

	SetAccelList();
}



// �R���{�{�b�N�̏�����
void CKeyBoardPropertyPage::InitialCombbox()
{
	if (_SetCombboxCategory(m_cmbCategory, m_hMenu) == FALSE)
		return;

	// ���鎖
	m_cmbCommand.SetDroppedWidth(250);
	OnSelChangeCate(0, 0, 0);
	::SetWindowText( GetDlgItem(IDC_STC01), _T("") );
}



void CKeyBoardPropertyPage::InitialListbox()
{
	static const TCHAR *titles[] = { _T("�R�}���h"), _T("�V���[�g�J�b�g") };
	static const int	widths[] = { 150, 350 };

	LVCOLUMN			col;

	col.mask = LVCF_TEXT | LVCF_WIDTH;

	for (int i = 0; i < _countof(titles); ++i) {
		col.pszText = (LPTSTR) titles[i];
		col.cx		= widths[i];
		m_ltAccel.InsertColumn(i, &col);
	}

	m_ltAccel.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	SetAccelList();
}



void CKeyBoardPropertyPage::SetAccelList()
{
	m_ltAccel.DeleteAllItems();

	CAccelerManager accelManager(m_hAccel);

	for (int ii = 0; ii < accelManager.GetCount(); ii++) {
		LPACCEL    lpAccel = accelManager.GetAt(ii);

		CString    strShortCut;
		CKeyHelper helper(lpAccel);
		helper.Format(strShortCut);

		CString    strCmdName;
		CToolTipManager::LoadToolTipText(lpAccel->cmd, strCmdName);

		m_ltAccel.InsertItem(ii, strCmdName);
		m_ltAccel.SetItemText(ii, 1, strShortCut);
	}
}



// �J�e�S���ύX��
void CKeyBoardPropertyPage::OnSelChangeCate(UINT code, int id, HWND hWnd)
{
	m_editNew.EnableWindow(FALSE);
	int nIndex = m_cmbCategory.GetCurSel();

	// �R�}���h�I��
	_PickUpCommand(m_hMenu, nIndex, m_cmbCommand);
}



// �R�}���h�ύX��
void CKeyBoardPropertyPage::OnSelChangeCmd(UINT code, int id, HWND hWnd)
{
	int 			nIndex = m_cmbCommand.GetCurSel();

	if (nIndex == -1)
		return;

	// �R�}���hID
	UINT			nCmdID = (UINT) m_cmbCommand.GetItemData(nIndex);

	// �܂��A�L����
	if (nCmdID == 0)
		m_editNew.EnableWindow(FALSE);
	else
		m_editNew.EnableWindow(TRUE);

	// �V���[�g�J�b�g�L�[�𓾂�
	CString 		strAccel;
	CAccelerManager accelManager(m_hAccel);

	if ( accelManager.FindAccelerator(nCmdID, strAccel) ) {
		m_editNow.SetWindowText(strAccel);
		::EnableWindow(GetDlgItem(IDC_BTN_DEL), TRUE);
	} else {
		m_editNow.SetWindowText(_T(""));
		::EnableWindow(GetDlgItem(IDC_BTN_DEL), FALSE);
	}

	m_editNew.SetWindowText(_T(""));
	::EnableWindow(GetDlgItem(IDC_BTN_SET), FALSE);
	::SetWindowText( GetDlgItem(IDC_STC01), _T("") );
}



BOOL CKeyBoardPropertyPage::OnTranslateAccelerator(LPMSG lpMsg)
{
	BOOL bSts = m_editNew.OnTranslateAccelerator(lpMsg);

	if (bSts) {
		// �L�[�𓾂�
		ACCEL			accel;
		memcpy( &accel, m_editNew.GetAccel(), sizeof (ACCEL) );
		CAccelerManager accelManager(m_hAccel);
		UINT			nCmdID = accelManager.FindCommandID(&accel);

		// New �A�N�Z�X�L�[�e�L�X�g
		TCHAR			cBuff[MAX_PATH + 1];
		cBuff[0]	= 0;	//+++
		::GetWindowText(m_editNew.m_hWnd, cBuff, MAX_PATH);
		CString 		strAccelNew(cBuff);

		// Now �A�N�Z�X�L�[�e�L�X�g
		::GetWindowText(m_editNow.m_hWnd, cBuff, MAX_PATH);
		CString 		strAccelNow(cBuff);

		// �����R�}���h�e�L�X�g
		CString 		strCmd;

		if (nCmdID != 0)
			CToolTipManager::LoadToolTipText(nCmdID, strCmd);

		::SetWindowText(GetDlgItem(IDC_STC01), strCmd);

		// �ݒ�{�^����Enable����
		EnableSetBtn();
	}

	return bSts;
}



// �ݒ�{�^����Enable����
void CKeyBoardPropertyPage::EnableSetBtn()
{
	// New �A�N�Z�X�L�[�e�L�X�g
	TCHAR		cBuff[MAX_PATH];
	cBuff[0]	= 0;	//+++

	::GetWindowText(m_editNew.m_hWnd, cBuff, MAX_PATH);
	CString strAccelNew(cBuff);

	// Now �A�N�Z�X�L�[�e�L�X�g
	::GetWindowText(m_editNow.m_hWnd, cBuff, MAX_PATH);
	CString strAccelNow(cBuff);

	// �����R�}���h�e�L�X�g
	::GetWindowText(GetDlgItem(IDC_STC01), cBuff, MAX_PATH);
	CString strCmd(cBuff);

	BOOL	bEnableSetBtn = TRUE;

	if (strAccelNew.IsEmpty() == FALSE) {
		// Now �A�N�Z�X�L�[���Ȃ���
		if ( strAccelNow.IsEmpty() ) {
			if ( strCmd.IsEmpty() )
				bEnableSetBtn = TRUE;
			else if (m_nCmdMove == 1)
				bEnableSetBtn = TRUE;
			else
				bEnableSetBtn = FALSE;
		} else {
			if ( strCmd.IsEmpty() ) {
				if (m_nCmdUpdate == 1)
					bEnableSetBtn = TRUE;
				else
					bEnableSetBtn = FALSE;
			} else {
				if (m_nCmdMove == 1 && m_nCmdUpdate == 1)
					bEnableSetBtn = TRUE;
				else
					bEnableSetBtn = FALSE;
			}
		}
	} else {
		bEnableSetBtn = FALSE;
	}
	::EnableWindow(GetDlgItem(IDC_BTN_SET), bEnableSetBtn);
}
