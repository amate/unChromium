/**
 *	@file	CloseTitleOption.h
 *	@brief	donut�̃I�v�V����: �^�C�g���ɂ��\���}�~
 */

#pragma once

#include "../resource.h"


/**
	CCloseTitlesOption
	�^�C�g���ɂ��\���}�~�̂��߂̃f�[�^�x�[�X�Ƌ@�\�����N���X

	�\���֎~�y�[�W�^�C�g���̃��X�g�ƌ����@�\�A
	���X�g�̃t�@�C���ւ̓ǂݏ����@�\��L���܂��B

	�g�����̓V���v���ŁA�v���O�����J�n����GetProfile�A�I������WriteProfile���Ăяo���܂��B
	���Ƃ̓y�[�W���J���ۂɂ��̃^�C�g����SearchString�Ō������āA
	true���Ԃ�΃y�[�W���J���̂��L�����Z�����鏈��������΂����킯�ł��B
 */
class CCloseTitlesOption {
	friend class CCloseTitlesPropertyPage;

	typedef std::list<CString>	CStringList;			//�^�C�g�����X�g�̌^��
	static CStringList *		s_pCloseTitles; 		//�^�C�g�����X�g
public:
	static bool 				s_bValid;				//���̋@�\�͗L���ł��邩

	static void GetProfile();							//�t�@�C������̃��X�g�ǂݍ��ݑ�����������
	static void WriteProfile(); 						//�t�@�C���ւ̃��X�g�����o�����I������
	static bool SearchString(const CString &strTitle);	//�^�C�g���̌���
	static void Add(const CString &strTitle);			//���X�g�ւ̍��ڒǉ�
};



/**
	CCloseTitlesPropertyPage
	�y�[�W�^�C�g���ɂ��\���}�~��ݒ肷�邽�߂̃v���p�e�B�x�[�W�_�C�A���O�N���X

	�\���֎~�^�C�g���̈ꗗ��ҏW���邽�߂̃_�C�A���O
 */
class CCloseTitlesPropertyPage
	: public CPropertyPageImpl<CCloseTitlesPropertyPage>
	, public CWinDataExchange<CCloseTitlesPropertyPage>
{
private:
	//�����o�ϐ�
	CCloseTitlesOption::CStringList   m_urls;			//�֎~�^�C�g���̃��X�g(�����͂�����Ƃ�������)
	CString 						  m_strAddressBar;	//���J���Ă���y�[�W�̃^�C�g��
	CListBox						  m_listbox;		//���X�g�{�b�N�X�̑���N���X
	CEdit							  m_edit;			//�e�L�X�g�{�b�N�X�̑���N���X
	int 							  m_nValid; 		//�^�C�g���}�~�@�\�͗L����
	CContainedWindow				  m_wndList;		//���X�g�{�b�N�X�̑���N���X

	//�֐��I�u�W�F�N�g
	struct AddToListBox : public std::unary_function<const CString &, void> {
		CListBox &m_box;
		AddToListBox(CListBox &box) : m_box(box) { }
		result_type operator ()(argument_type src)
		{
			m_box.AddString(src);
		}
	};

	//���b�Z�[�W�n���h��
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);


	//�R�}���h�n���h��
	void	OnDelCmd(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/);
	void	OnDelAllCmd(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/);
	void	OnAddCmd(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/);
	void	OnSelChange(UINT code, int id, HWND hWnd);
	void	OnListKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);


	//�����֐�
	void	GetData();									//�_�C�A���O����f�[�^�̎擾
	BOOL	DataExchange(BOOL bSaveAndValidate);		//�R���g���[���̏�Ԃƃf�[�^�̕ϊ�


public:
	//�_�C�A���O�̃��\�[�XID
	enum { IDD = IDD_PROPPAGE_CLOSETITLES };

	//�R���X�g���N�^
	CCloseTitlesPropertyPage(const CString &strAddressBar);

	//DDX�}�b�v
	BEGIN_DDX_MAP(CCloseTitlesPropertyPage) 			//+++
		DDX_CHECK( IDC_CHK_TITLECLOSE, m_nValid )
	END_DDX_MAP()

	//�v���p�e�B�x�[�W�Ƃ��ẴI�[�o�[���C�h�֐�
	BOOL	OnSetActive();
	BOOL	OnKillActive();
	BOOL	OnApply();

public:
	//���b�Z�[�W�}�b�v
	BEGIN_MSG_MAP( CCloseTitlesPropertyPage )
		MESSAGE_HANDLER 	 ( WM_INITDIALOG	, OnInitDialog	)
		MESSAGE_HANDLER 	 ( WM_DESTROY		, OnDestroy 	)
		COMMAND_ID_HANDLER_EX( IDC_ADD_BUTTON	, OnAddCmd		)
		COMMAND_ID_HANDLER_EX( IDC_DELALL_BUTTON, OnDelAllCmd	)
		COMMAND_ID_HANDLER_EX( IDC_DEL_BUTTON	, OnDelCmd		)
		COMMAND_HANDLER_EX	 ( IDC_IGNORED_URL_LIST, LBN_SELCHANGE, OnSelChange )
		CHAIN_MSG_MAP( CPropertyPageImpl<CCloseTitlesPropertyPage> )
	ALT_MSG_MAP(1)
		MSG_WM_KEYUP( OnListKeyUp )
	END_MSG_MAP()
};
