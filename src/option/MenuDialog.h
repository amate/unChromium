/**
 *	@file	MenuOption.h
 *	@brief	donut�̃I�v�V���� : ���j���[
 */

#pragma once

#include "../resource.h"

namespace MTL { class CCommandBarCtrl2; }

enum EMenuEx {
	MENU_EX_NOCUSTOMMENU	= 0x00000001L,	// ���j���[�̕�����ҏW���Ȃ�
	MENU_EX_NOCUSTOMIEMENU	= 0x00000002L,	// �E�N���b�N���j���[���J�X�^�����Ȃ�
	MENU_EX_R_EQUAL_L		= 0x00000004L,	// �E�N���b�N���j���[��̉E�N���b�N��...
	MENU_EX_DONTSHOWBUTTON	= 0x00000008L,	// ���j���[�ɍŏ���etc..�{�^����\�����Ȃ�
};

////////////////////////////////////////////////////////////
// CMenuOption

class CMenuOption
{
public:
	static bool		s_bNoCustomMenu;	// ���j���[�̕�����ҏW���Ȃ�
	static bool		s_bNoCustomIEMenu;	// �E�N���b�N���j���[���J�X�^�����Ȃ�
	static bool		s_bR_Equal_L;		// �E�N���b�N���j���[��̉E�N���b�N��...
	static bool		s_bDontShowButton;	// ���j���[�ɍŏ���etc..�{�^����\�����Ȃ�
	static int		s_nMenuBarStyle;	// ���j���[�o�[�̍��ڂ̕\�L(0:���{�� 1:1���� 2:�p��)

public:
	static void		GetProfile();
	static void		WriteProfile();
};




////////////////////////////////////////////////////////////
// CMenuPropertyPage : [Donut�̃I�v�V����] - [���j���[]

class CMenuPropertyPage
	: public CPropertyPageImpl<CMenuPropertyPage>
	, public CWinDataExchange<CMenuPropertyPage>
{
public:
	// Constants
	enum { IDD = IDD_PROPPAGE_MENU };

private:
	// Data members
	HMENU					m_hMenu;

	int 					m_nNoCstmMenu;
	int 					m_nNoCstmIeMenu;
	int 					m_nREqualL;
	int 					m_nNoButton;

	int 					m_nMenuBarStyle;		//+++

	MTL::CCommandBarCtrl2&	m_rCmdBar;				//+++ ���j���[�o�[�֌W�p

	bool					m_bInit;

public:
	// Constructor/Destructor
	CMenuPropertyPage(HMENU hMenu, CCommandBarCtrl2& CmdBar);
	~CMenuPropertyPage();

	// Overrides
	BOOL	OnSetActive();
	BOOL	OnKillActive();
	BOOL	OnApply();

private:
	// �f�[�^��ۑ�
	void	_SaveData();

public:

	// DDX map
	BEGIN_DDX_MAP( CMenuPropertyPage )
		DDX_CHECK( IDC_NO_CSTM_TXT_MENU 	, m_nNoCstmMenu )
		DDX_CHECK( IDC_NO_CSTM_IE_MENU		, m_nNoCstmIeMenu)
		DDX_CHECK( IDC_CHECK_R_EQUAL_L		, m_nREqualL	)
		DDX_CHECK( IDC_CHECK_NOBUTTON		, m_nNoButton	)
		DDX_RADIO( IDC_MENU_BAR_STYLE_JAPAN , m_nMenuBarStyle)
	END_DDX_MAP()


	// Message map and handlers
	BEGIN_MSG_MAP( CMenuPropertyPage )
		CHAIN_MSG_MAP( CPropertyPageImpl<CMenuPropertyPage> )
	END_MSG_MAP()
};

