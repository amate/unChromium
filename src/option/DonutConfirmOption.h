/**
 *	@file	DonutConfirmOption.h
 *	@brief	donut�I�v�V����: �m�F�_�C�A���O�Ɋւ���ݒ�v���p�e�B�y�[�W. �I�����Ƀ��[�U�[�֊m�F���o���������܂�.
 *	@note
 *			�I�����Ƀ��[�U�[�֊m�F���o���������܂݂܂��B
 *			�I�����Ɋm�F���s��CDonutConfirmOption�Ƃ��̐ݒ���s��
 *			�v���p�e�B�y�[�WCDonutConfirmPropertyPage��L���܂��B
 */

#pragma once

#include "../resource.h"
#include "../MtlWin.h"



/**
	CDonutConfirmOption
	�e��m�F�_�C�A���O���o�����߂̃N���X
 */
class CDonutConfirmOption {
	friend class CDonutConfirmPropertyPage;

private:
	//�m�F�_�C�A���O�̐ݒ�t���O�̒萔
	enum EDonut_Confirm {
		DONUT_CONFIRM_EXIT			 = 0x00000001L,
		DONUT_CONFIRM_CLOSEALL		 = 0x00000002L,
		DONUT_CONFIRM_CLOSEALLEXCEPT = 0x00000004L,
		DONUT_CONFIRM_STOPSCRIPT	 = 0x00000008L,
		DONUT_CONFIRM_CLOSELEFTRIGHT = 0x00000010L,
	};

	//�����o�ϐ�
	static DWORD	s_dwFlags;
	//x static BOOL s_bStopScript;
	static DWORD	s_dwStopScript;

public:
	//�����o�֐�
	static void 	GetProfile();
	static void 	WriteProfile();

	static bool 	OnDonutExit(HWND hWnd = NULL);
	static bool 	OnCloseAll(HWND hWnd = NULL);
	static bool 	OnCloseAllExcept(HWND hWnd = NULL);
	static bool		OnCloseLeftRight(HWND hWnd = NULL, bool bLeft = false);

	static BOOL 	WhetherConfirmScript();

private:
	static bool 	_SearchDownloadingDialog();

	//�����Ŏg�p����\����
	struct _Function_Searcher {
		bool	m_bFound;

		_Function_Searcher() : m_bFound(false) { }

		bool operator ()(HWND hWnd)
		{
			if ( MtlIsWindowCurrentProcess(hWnd) ) {
				CString strCaption = MtlGetWindowText(hWnd);

				if ( (strCaption.Find( _T('%') ) != -1 && strCaption.Find( _T("�������܂���") ) != -1)
				   || strCaption.Find( _T("�t�@�C���̃_�E�����[�h") ) != -1 )
				{
					m_bFound = true;
					return false;
				}
			}

			return true; // continue finding
		}
	};

};


/**
	CDonutConfirmPropertyPage
	�m�F�_�C�A���O�Ɋւ���ݒ�v���p�e�B�y�[�W
 */
class CDonutConfirmPropertyPage
	: public CPropertyPageImpl< CDonutConfirmPropertyPage >
	, public CWinDataExchange < CDonutConfirmPropertyPage >
{
public:
	//�_�C�A���O���\�[�XID
	enum { IDD = IDD_PROPPAGE_CONFIRMDLG };

private:
	//�����o�ϐ�
	int 	m_nExit;
	int 	m_nCloseAll;
	int 	m_nCloseAllExcept;
	int 	m_nStopScript;
	int 	m_nCloseLeftRight;

public:
	//DDX�}�b�v
	BEGIN_DDX_MAP(CDonutConfirmPropertyPage)
		DDX_CHECK( IDC_CHECK_CFD_EXIT,			m_nExit 		 )
		DDX_CHECK( IDC_CHECK_CFD_CLOSEALL,		m_nCloseAll 	 )
		DDX_CHECK( IDC_CHECK_CFD_CLOSEEXCEPT,	m_nCloseAllExcept)
		DDX_CHECK( IDC_CHECK_CFD_STOPSCRIPT,	m_nStopScript	 )
		DDX_CHECK( IDC_CHECK_CFD_CLOSELEFTRIGHT,m_nCloseLeftRight)
	END_DDX_MAP()

	//�R���X�g���N�^
	CDonutConfirmPropertyPage();

	//�v���p�e�B�y�[�W�̃I�[�o�[���C�h�֐�
	BOOL	OnSetActive();
	BOOL	OnKillActive();
	BOOL	OnApply();

	//�����֐�
private:
	void	_GetData();
	void	_SetData();
};


