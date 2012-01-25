/**
 *	DonutConfirmOption.cpp
 *	@brief	donut�I�v�V����: �m�F�_�C�A���O�Ɋւ���ݒ�v���p�e�B�y�[�W. �I�����Ƀ��[�U�[�֊m�F���o���������܂�.
 *	@note
 *			�I�����Ƀ��[�U�[�֊m�F���o���������܂݂܂��B
 *			�I�����Ɋm�F���s��CDonutConfirmOption�Ƃ��̐ݒ���s��
 *			�v���p�e�B�y�[�WCDonutConfirmPropertyPage��L���܂��B
 */

#include "stdafx.h"
#include "DonutConfirmOption.h"
#include "../IniFile.h"


#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



////////////////////////////////////////////////////////////////////////////////
//CDonutConfirmOption�̒�`
////////////////////////////////////////////////////////////////////////////////

//static�ϐ��̒�`
DWORD		CDonutConfirmOption::s_dwFlags		= 0/*DONUT_CONFIRM_EXIT | DONUT_CONFIRM_CLOSEALLEXCEPT*/;
DWORD		CDonutConfirmOption::s_dwStopScript = TRUE; 	//+++ sizeof(BOOL)=sizeof(DWORD)�Ɉˑ����Ȃ��悤�ɁABOOL�������̂�DWORD�ɕύX.



//�����o�֐�
void CDonutConfirmOption::GetProfile()
{
	CIniFileI		pr( g_szIniFileName, _T("Confirmation") );
	pr.QueryValue( s_dwFlags, _T("Confirmation_Flags") );
	pr.QueryValue( /*(DWORD&)*/s_dwStopScript, _T("Script") );
	pr.Close();
}



void CDonutConfirmOption::WriteProfile()
{
	CIniFileO	pr( g_szIniFileName, _T("Confirmation") );
	pr.SetValue( s_dwFlags, _T("Confirmation_Flags") );
	pr.SetValue( s_dwStopScript, _T("Script") );
	pr.Close();
}



bool CDonutConfirmOption::OnDonutExit(HWND hWnd)
{
	if ( _SearchDownloadingDialog() ) {
		if ( IDYES == ::MessageBox(hWnd,
						_T("�_�E�����[�h���̃t�@�C��������܂����ADonut���I�����Ă���낵���ł����H"),
						_T("�m�F�_�C�A���O"), MB_YESNO | MB_ICONQUESTION ) )
			return true;
		else
			return false;
	}

	if ( !_check_flag(DONUT_CONFIRM_EXIT, s_dwFlags) )
		return true;

	// Note. On debug mode, If DONUT_CONFIRM_EXIT set, the process would be killed
	//		 before Module::Run returns. What can I do?
	if ( IDYES == ::MessageBox(hWnd, _T("Donut���I�����Ă���낵���ł����H"),
								_T("�m�F�_�C�A���O"), MB_YESNO | MB_ICONQUESTION) ) {
		return true;
	}

	return false;
}



bool CDonutConfirmOption::OnCloseAll(HWND hWnd)
{
	if ( !_check_flag(DONUT_CONFIRM_CLOSEALL, s_dwFlags) )
		return true;

	if ( IDYES == ::MessageBox(hWnd, _T("�E�B���h�E�����ׂĕ��Ă���낵���ł����H"),
							   _T("�m�F�_�C�A���O"), MB_YESNO | MB_ICONQUESTION) )
		return true;

	return false;
}



bool CDonutConfirmOption::OnCloseAllExcept(HWND hWnd)
{
	if ( !_check_flag(DONUT_CONFIRM_CLOSEALLEXCEPT, s_dwFlags) )
		return true;

	if ( IDYES == ::MessageBox(hWnd, _T("����ȊO�̃E�B���h�E�����ׂĕ��Ă���낵���ł����H"),
							   _T("�m�F�_�C�A���O"), MB_YESNO | MB_ICONQUESTION) )
		return true;

	return false;
}



bool CDonutConfirmOption::OnCloseLeftRight(HWND hWnd, bool bLeft)
{
	if ( !_check_flag(DONUT_CONFIRM_CLOSELEFTRIGHT, s_dwFlags) )
		return true;
	const TCHAR* pStr = (bLeft) ? _T("���̃^�u��荶���̃^�u�����ׂĕ��Ă���낵���ł����H")
								: _T("���̃^�u���E���̃^�u�����ׂĕ��Ă���낵���ł����H") ;

	if ( IDYES == ::MessageBox(hWnd, pStr, _T("�m�F�_�C�A���O"), MB_YESNO | MB_ICONQUESTION) )
		return true;

	return false;
}



bool CDonutConfirmOption::_SearchDownloadingDialog()
{
	_Function_Searcher f;

	f = MtlForEachTopLevelWindow(_T("#32770"), NULL, f);

	return f.m_bFound;
}



BOOL CDonutConfirmOption::WhetherConfirmScript()
{
	if (s_dwStopScript)
		return TRUE;

	return FALSE;
}



////////////////////////////////////////////////////////////////////////////////
//CDonutConfirmPropertyPage�̒�`
////////////////////////////////////////////////////////////////////////////////

//�R���X�g���N�^
CDonutConfirmPropertyPage::CDonutConfirmPropertyPage()
{
	_SetData();
}



//�v���p�e�B�y�[�W�̃I�[�o�[���C�h�֐�
BOOL CDonutConfirmPropertyPage::OnSetActive()
{
	SetModified(TRUE);
	return DoDataExchange(FALSE);
}



BOOL CDonutConfirmPropertyPage::OnKillActive()
{
	return DoDataExchange(TRUE);
}



BOOL CDonutConfirmPropertyPage::OnApply()
{
	if ( DoDataExchange(TRUE) ) {
		_GetData();
		return TRUE;
	} else {
		return FALSE;
	}
}



//�����֐�
void CDonutConfirmPropertyPage::_GetData()
{
	DWORD dwFlags = 0;

	if (m_nExit /*== 1*/)
		dwFlags |= CDonutConfirmOption::DONUT_CONFIRM_EXIT;

	if (m_nCloseAll /*== 1*/)
		dwFlags |= CDonutConfirmOption::DONUT_CONFIRM_CLOSEALL;

	if (m_nCloseAllExcept /*== 1*/)
		dwFlags |= CDonutConfirmOption::DONUT_CONFIRM_CLOSEALLEXCEPT;

	if (m_nCloseLeftRight)
		dwFlags |= CDonutConfirmOption::DONUT_CONFIRM_CLOSELEFTRIGHT;

	CDonutConfirmOption::s_dwStopScript = (m_nStopScript != 0);
	CDonutConfirmOption::s_dwFlags		= dwFlags;
}



void CDonutConfirmPropertyPage::_SetData()
{
	DWORD	dwFlags = CDonutConfirmOption::s_dwFlags;

	m_nExit 		  = _check_flag(CDonutConfirmOption::DONUT_CONFIRM_EXIT 		, dwFlags);
	m_nCloseAll 	  = _check_flag(CDonutConfirmOption::DONUT_CONFIRM_CLOSEALL 	, dwFlags);
	m_nCloseAllExcept = _check_flag(CDonutConfirmOption::DONUT_CONFIRM_CLOSEALLEXCEPT,dwFlags);
	m_nStopScript	  = CDonutConfirmOption::s_dwStopScript != 0;
	m_nCloseLeftRight = _check_flag(CDonutConfirmOption::DONUT_CONFIRM_CLOSELEFTRIGHT,dwFlags);
}
