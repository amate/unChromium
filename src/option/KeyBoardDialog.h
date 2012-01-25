/**
 *	@file	KeyBoardDialog.h
 *	@brief	donut�̃I�v�V���� : �L�[
 */
#pragma once

#include "../resource.h"
#include "../AccelManager.h"



class CKeyBoardPropertyPage
	: public CPropertyPageImpl<CKeyBoardPropertyPage>
	, public CWinDataExchange<CKeyBoardPropertyPage>
{
public:
	// Constants
	enum { IDD = IDD_PROPPAGE_KEYBOARD };

private:
	// Data members
	CEdit		  m_editNow;
	CEditAccel	  m_editNew;
	CComboBox	  m_cmbCategory;
	CComboBox	  m_cmbCommand;
	CListViewCtrl m_ltAccel;

	HACCEL		  m_hAccel;
	HMENU		  m_hMenu;
	int 		  m_nCmdUpdate;
	int 		  m_nCmdMove;

	//int		  m_nMinBtn2Tray;	//+++ �ŏ����{�^���Ńg���C�ɓ����ݒ�

public:
	// DDX map
	BEGIN_DDX_MAP( CKeyBoardPropertyPage )
		DDX_CHECK( IDC_CHK_CMDUPDATE	, m_nCmdUpdate 	)
		DDX_CHECK( IDC_CHK_CMDMOVE		, m_nCmdMove	)
		//DDX_CHECK( IDC_CHK_MINBTN2TRAY, m_nMinBtn2Tray)		//+++
	END_DDX_MAP()

	//�����E�j��
	CKeyBoardPropertyPage(HACCEL hAccel, HMENU hMenu);
	~CKeyBoardPropertyPage();

	// Overrides
	BOOL	OnSetActive();
	BOOL	OnKillActive();
	BOOL	OnApply();

private:
	// Constructor
	// �f�[�^�𓾂�
	void	_SetData();


	// �f�[�^��ۑ�
	void	_GetData();

public:
	// Message map and handlers
	BEGIN_MSG_MAP(CKeyBoardPropertyPage)
		COMMAND_HANDLER_EX( IDC_CMB_CATEGORY, CBN_SELCHANGE, OnSelChangeCate )
		COMMAND_HANDLER_EX( IDC_CMB_COMMAND , CBN_SELCHANGE, OnSelChangeCmd  )

		COMMAND_ID_HANDLER_EX( IDC_BTN_DEL, OnBtnDel )
		COMMAND_ID_HANDLER_EX( IDC_BTN_SET, OnBtnSel )
		COMMAND_ID_HANDLER( IDC_CHK_CMDUPDATE, OnChkBtn )
		COMMAND_ID_HANDLER( IDC_CHK_CMDMOVE  , OnChkBtn )

		CHAIN_MSG_MAP(CPropertyPageImpl<CKeyBoardPropertyPage>)
	END_MSG_MAP()

private:
	LRESULT OnChkBtn(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

	void	OnBtnDel(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/);

	void	OnBtnSel(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/);

	// �R���{�{�b�N�X�̏�����
	void	InitialCombbox();
	void	InitialListbox();
	void	SetAccelList();

	// �J�e�S���ύX��
	void	OnSelChangeCate(UINT code, int id, HWND hWnd);

	// �R�}���h�ύX��
	void	OnSelChangeCmd(UINT code, int id, HWND hWnd);

public:
	BOOL	OnTranslateAccelerator(LPMSG lpMsg);

private:
	// �ݒ�{�^����Enable����
	void	EnableSetBtn();
};
