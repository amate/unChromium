/**
 *	@file	CommandSelectDialog.cpp
 *	@brief	"�R�}���h��I��"�_�C�A���O
 */
#include "stdafx.h"
#include "CommandSelectDialog.h"
#include "../DonutPFunc.h"

#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CCommandSelectDialog::CCommandSelectDialog(HMENU hMenu)
	: m_dwCommandID(0)
{
	m_hMenu = hMenu;
}



DWORD_PTR CCommandSelectDialog::GetCommandID()
{
	return m_dwCommandID;
}



LRESULT CCommandSelectDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
	CenterWindow( GetParent() );
	DoDataExchange(FALSE);

	m_cmbCategory.Attach( GetDlgItem(IDC_CMB_CATEGORY) );
	m_cmbCommand.Attach( GetDlgItem(IDC_CMB_COMMAND) );

	if (_SetCombboxCategory(m_cmbCategory, m_hMenu) == FALSE)
		return FALSE;

	// ���鎖
	m_cmbCommand.SetDroppedWidth(250);
	OnSelChangeCate(0, 0, 0);
	return TRUE;
}



LRESULT CCommandSelectDialog::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}



void CCommandSelectDialog::OnSelChangeCate(UINT code, int id, HWND hWnd)
{
	int nIndex = m_cmbCategory.GetCurSel();

	// �R�}���h�I��
	_PickUpCommand(m_hMenu, nIndex, m_cmbCommand);

	m_dwCommandID = 0;
	SetGuideMsg();
}



// �R�}���h�ύX��
void CCommandSelectDialog::OnSelChangeCmd(UINT code, int id, HWND hWnd)
{
	int nIndex = m_cmbCommand.GetCurSel();

	if (nIndex == -1) return;

	m_dwCommandID = m_cmbCommand.GetItemData(nIndex);
	SetGuideMsg();
}



void CCommandSelectDialog::SetGuideMsg()
{
	CString strGuide;

	if (m_dwCommandID != 0)
		strGuide.LoadString( UINT(m_dwCommandID) );
	else
		strGuide = _T("�R�}���h��I�肵�܂���B");

	int nFindIt = strGuide.Find( _T('\t') );
	if (nFindIt != -1)
		strGuide = strGuide.Mid(nFindIt + 1);

	nFindIt = strGuide.Find( _T('\n') );

	if (nFindIt != -1)
		strGuide = strGuide.Left(nFindIt);

	::SetWindowText(::GetDlgItem(m_hWnd, IDC_STC01), strGuide);
}
