/**
 *	@file	ExStyle.h
 *	@brief	�g���v���p�e�B
 */

#pragma once

#include <boost\optional.hpp>
#include "DonutView.h"
#include "option\DLControlOption.h"

#define DONUT_SECTION			_T("unDonut")
#define EXPROP_KEY_ENABLED		_T("Enabled")
#define EXPROP_KEY				_T("ExProperty")
#define EXPROP_OPTION			_T("ExPropOpt") 	//+++	�ǉ�.


enum EExProp {
	EXPROP_DEFAULT			= 2,
	EXPROP_OFF				= 0,
	EXPROP_ON				= 1,

	EXPROP_DLIMAGE			= 0x00000003,
	EXPROP_VIDEO			= 0x0000000C,
	EXPROP_SOUND			= 0x00000030,
	EXPROP_RUNACTIVEX		= 0x000000C0,
	EXPROP_DLACTIVEX		= 0x00000300,
	EXPROP_SCRIPT			= 0x00000C00,
	EXPROP_JAVA 			= 0x00003000,
	EXPROP_NAVI 			= 0x00030000,
	EXPROP_FILTER			= 0x000C0000,
	EXPROP_GETSTURE 		= 0x00300000,
	EXPROP_MAILTO			= 0x00C00000,

	EXPROP_REFRESH_NONE 	= 0x01000000,
	EXPROP_REFRESH_15SEC	= 0x02000000,
	EXPROP_REFRESH_30SEC	= 0x04000000,
	EXPROP_REFRESH_1MIN 	= 0x08000000,
	EXPROP_REFRESH_2MIN 	= 0x10000000,
	EXPROP_REFRESH_5MIN 	= 0x20000000,
	EXPROP_REFRESH_USER 	= 0x40000000,
	EXPROP_REFRESH_DEFAULT	= 0x80000000,
};


enum {
	EXPROPOPT_ADDRESSBAR	=	0x00000001,	//+++ �����ŃA�h���X�o�[�̕������Ώۂɂ���.
	EXPROPOPT_FLATVIEW		=   0x0000000C,	//+++ ���̃y�[�W�𕽖ʓI�ȕ\���ɂ���
};



/*
	�g���v���p�e�B�`���f�[�^��Donut�W���̃f�[�^�t���O�Ƃ̑��ݕϊ����s���N���X

	�g���v���p�e�B�`���f�[�^�̓t���O���Ƃ�On,Off,Default�̎O�Ԃ��Ƃ�
	Default�̓R���X�g���N�^�Ŏw�肵���f�t�H���g�̒l�𗘗p����

	release12���_�ł�Default�l�͗��p����Ă��Ȃ��iOn,Off�̂ݐݒ�j
 */

class CExProperty {
private:
	DWORD		m_dwExProp;
	DWORD		m_dwExPropOpt;		//+++

public:
	DWORD		m_dwDefDLFlag;
	DWORD		m_dwDefExStyle;
	DWORD		m_dwDefReloadFlag;


public:
	CExProperty(DWORD dwDefaultDLFlag, DWORD dwDefaultExStyle, DWORD dwDefaultReloadFlag, DWORD dwExProp, DWORD dwExPropOpt)
		: m_dwDefDLFlag(dwDefaultDLFlag)
		, m_dwDefExStyle(dwDefaultExStyle)
		, m_dwDefReloadFlag(dwDefaultReloadFlag)
		, m_dwExProp(dwExProp)
		, m_dwExPropOpt(dwExPropOpt)		//+++�����ǉ�.
	{
	}


	int GetPropFlag(DWORD dwExProp, DWORD dwFlag) const
	{
		return (dwExProp & dwFlag) / (dwFlag / 3);
	}


	//�Z�L�����e�B�֘A�̃t���O�ϐ����擾����
	int GetDLControlFlags() const
	{
		DWORD	dwDLFlags = 0;
		DWORD	dwExProp	= m_dwExProp;

		switch ( GetPropFlag(dwExProp, EXPROP_DLIMAGE) ) {
		case EXPROP_DEFAULT:	dwDLFlags |= (m_dwDefDLFlag & DLCTL_DLIMAGES);			break;
		case EXPROP_ON: 		dwDLFlags |= DLCTL_DLIMAGES;							break;
		}

		switch ( GetPropFlag(dwExProp, EXPROP_VIDEO) ) {
		case EXPROP_DEFAULT:	dwDLFlags |= (m_dwDefDLFlag & DLCTL_VIDEOS);			break;
		case EXPROP_ON: 		dwDLFlags |= DLCTL_VIDEOS;								break;
		}

		switch ( GetPropFlag(dwExProp, EXPROP_SOUND) ) {
		case EXPROP_DEFAULT:	dwDLFlags |= (m_dwDefDLFlag & DLCTL_BGSOUNDS);			break;
		case EXPROP_ON: 		dwDLFlags |= DLCTL_BGSOUNDS;							break;
		}

		switch ( GetPropFlag(dwExProp, EXPROP_RUNACTIVEX) ) {
		case EXPROP_DEFAULT:	dwDLFlags |= (m_dwDefDLFlag & DLCTL_NO_RUNACTIVEXCTLS); break;
		case EXPROP_OFF:		dwDLFlags |= DLCTL_NO_RUNACTIVEXCTLS;					break;
		}

		switch ( GetPropFlag(dwExProp, EXPROP_DLACTIVEX) ) {
		case EXPROP_DEFAULT:	dwDLFlags |= (m_dwDefDLFlag & DLCTL_NO_DLACTIVEXCTLS);	break;
		case EXPROP_OFF:		dwDLFlags |= DLCTL_NO_DLACTIVEXCTLS;					break;
		}

		switch ( GetPropFlag(dwExProp, EXPROP_SCRIPT) ) {
		case EXPROP_DEFAULT:	dwDLFlags |= (m_dwDefDLFlag & DLCTL_NO_SCRIPTS);		break;
		case EXPROP_OFF:		dwDLFlags |= DLCTL_NO_SCRIPTS;							break;
		}

		switch ( GetPropFlag(dwExProp, EXPROP_JAVA) ) {
		case EXPROP_DEFAULT:	dwDLFlags |= (m_dwDefDLFlag & DLCTL_NO_JAVA);			break;
		case EXPROP_OFF:		dwDLFlags |= DLCTL_NO_JAVA; 							break;
		}

		return dwDLFlags;
	}


	//�^�u�̊g���X�^�C�����擾����
	int GetExtendedStyleFlags() const
	{
		DWORD	dwExFlag = 0;
		DWORD	dwExProp = m_dwExProp;

		switch ( GetPropFlag(dwExProp, EXPROP_NAVI) ) {
		case EXPROP_DEFAULT:	dwExFlag |= (m_dwDefExStyle & DVS_EX_OPENNEWWIN);		break;
		case EXPROP_ON: 		dwExFlag |= DVS_EX_OPENNEWWIN;							break;
		}

		switch ( GetPropFlag(dwExProp, EXPROP_FILTER) ) {
		case EXPROP_DEFAULT:	dwExFlag |= (m_dwDefExStyle & DVS_EX_MESSAGE_FILTER);	break;
		case EXPROP_ON: 		dwExFlag |= DVS_EX_MESSAGE_FILTER;						break;
		}

		switch ( GetPropFlag(dwExProp, EXPROP_GETSTURE) ) {
		case EXPROP_DEFAULT:	dwExFlag |= (m_dwDefExStyle & DVS_EX_MOUSE_GESTURE);	break;
		case EXPROP_ON: 		dwExFlag |= DVS_EX_MOUSE_GESTURE;						break;
		}

		switch ( GetPropFlag(dwExProp, EXPROP_MAILTO) ) {
		case EXPROP_DEFAULT:	dwExFlag |= (m_dwDefExStyle & DVS_EX_BLOCK_MAILTO); 	break;
		case EXPROP_ON: 		dwExFlag |= DVS_EX_BLOCK_MAILTO;						break;
		}

	  #if 1	//+++
		switch ( GetPropFlag(m_dwExPropOpt, EXPROPOPT_FLATVIEW) ) {
		case EXPROP_DEFAULT:	dwExFlag |= (m_dwDefExStyle & DVS_EX_FLATVIEW);	 	break;
		case EXPROP_ON: 		dwExFlag |= DVS_EX_FLATVIEW;						break;
		}
	  #endif
		return dwExFlag;
	}


	//�����X�V�̃t���O�ϐ����擾����
	int GetAutoRefreshFlag() const
	{
		DWORD dwRefreshFlag = m_dwExProp / (EXPROP_REFRESH_15SEC / DVS_AUTOREFRESH_15SEC);
		DWORD dwCurFlag 	= DVS_AUTOREFRESH_USER;

		while (dwCurFlag > 0) {
			if (dwCurFlag & dwRefreshFlag) {
				return dwCurFlag;
			}

			dwCurFlag >>= 1;
		}

		return 0;
	}


	int GetAutoRefreshNo() const
	{
		DWORD dwRefreshFlag = m_dwExProp / (EXPROP_REFRESH_15SEC / DVS_AUTOREFRESH_15SEC);
		DWORD dwCurFlag 	= DVS_AUTOREFRESH_USER;

		DWORD n				= 6;
		while (dwCurFlag > 0) {
			if (dwCurFlag & dwRefreshFlag) {
				return n;
			}

			dwCurFlag >>= 1;
			--n;
		}

		return 0;
	}


	DWORD GetExProperty() const
	{
		return 	m_dwExProp;
	}


	void SetExProperty(DWORD dwExProp)
	{
		m_dwExProp = dwExProp;
	}


	BOOL LoadExProperty(const CString &strIniFile, const CString &strSection = DONUT_SECTION)
	{
		CIniFileI	pr(strIniFile, strSection);
		m_dwExProp	  = pr.GetValue(EXPROP_KEY	 , 0xAAAAAA);		//+++ �����l���f�t�H���g�ݒ�ɂȂ�悤��.
		m_dwExPropOpt = pr.GetValue(EXPROP_OPTION, 8|0);			//+++
	}


	static BOOL CheckExPropertyFlag(DWORD &dwFlags, DWORD& dwFlags2, const CString& strIniFile, CString strSection = DONUT_SECTION)
	{
		CIniFileI	pr(strIniFile, strSection);
		DWORD	dwEnabled = pr.GetValue(EXPROP_KEY_ENABLED, 0);
		DWORD	dwExProp  = pr.GetValue(EXPROP_KEY        , 0xAAAAAA);		//+++ �����l�ύX.
		DWORD	dwExProp2 = pr.GetValue(EXPROP_OPTION     , 0x8);			//+++ �ǉ�.
		pr.Close();
		if (dwEnabled) {
			dwFlags  = dwExProp;
			dwFlags2 = dwExProp2;	//+++
		}
		return dwEnabled != 0;
	}
};



/// �g���v���p�e�B�f�[�^�ݒ�p�_�C�A���O
template<int IDD_DLG>					//+++ �ʏ�̃_�C�A���O�Ƃ͕ʂɁAURL�Z�L�����e�B�����ɕʃ_�C�A���O��p�ӂ��邽�߂Ƀe���v���[�g��... ���������A����ς���
class CExPropertyDialogT : 
	public CDialogImpl<CExPropertyDialogT<IDD_DLG> > , 
	public CWinDataExchange<CExPropertyDialogT<IDD_DLG> >
{
public:
	enum { IDD = IDD_DLG/*IDD_DIALOG_EXPROPERTY*/ };

	/// �R���X�g���N�^
	CExPropertyDialogT(const CString& strUrlFile, 
					   const CString& strSection = DONUT_SECTION, 
					   bool urlMode = true)
		: m_strUrlFile(strUrlFile)
		, m_strSection(strSection)
		, m_bURLMode(urlMode)			//+++ �ǉ�
		, m_urlEditBtnSw(0)				//+++ �ǉ�
		, m_bEnabled(FALSE)
	{
		// strUrlFile �� strSection ����ݒ�ǂݍ���
		CIniFileI	pr(strUrlFile, strSection);
		DWORD	dwExProp    = pr.GetValue(EXPROP_KEY   , 0xAAAAAA/*�����l���f�t�H���g�ݒ�ɂȂ�悤��.*/);
		DWORD	dwExPropOpt = pr.GetValue(EXPROP_OPTION, 0x8);		
		m_bEnabled	= pr.GetValue(EXPROP_KEY_ENABLED, FALSE) != 0;

		pr.ChangeSectionName(_T("InternetShortcut"));
		m_strUrlBase = pr.GetString(_T("URL"));

		Init(dwExProp, dwExPropOpt);
	}

	CExPropertyDialogT(const CString& url, bool bEnabled, DWORD dwExProp = 0xAAAAAA, DWORD dwExPropOpt = 0x8)
		: m_strUrlBase(url), m_bURLMode(true), m_urlEditBtnSw(0), m_bEnabled(bEnabled)
	{
		Init(dwExProp, dwExPropOpt);
	}

	void	Init(DWORD dwExProp, DWORD dwExPropOpt)
	{
		// �_�E�����[�h�R���g���[��
		m_nImage			= exProp2btn(dwExProp, EXPROP_DLIMAGE	);
		m_nVideo			= exProp2btn(dwExProp, EXPROP_VIDEO		);
		m_nSound			= exProp2btn(dwExProp, EXPROP_SOUND		);
		m_nRunActiveX		= exProp2btn(dwExProp, EXPROP_RUNACTIVEX );
		m_nDLActiveX		= exProp2btn(dwExProp, EXPROP_DLACTIVEX	);
		m_nScript			= exProp2btn(dwExProp, EXPROP_SCRIPT 	);
		m_nJava 			= exProp2btn(dwExProp, EXPROP_JAVA		);

		// ����I�v�V����
		m_nNaviLock 		= exProp2btn(dwExProp, EXPROP_NAVI		);
		m_nFilter			= exProp2btn(dwExProp, EXPROP_FILTER 	);
		m_nGesture			= exProp2btn(dwExProp, EXPROP_GETSTURE	);
		m_nBlockMailTo		= exProp2btn(dwExProp, EXPROP_MAILTO 	);

		m_nFlatView 		= exProp2btn(dwExPropOpt, EXPROPOPT_FLATVIEW  );	//+++

		//+++ m_nNoCloseNaviLock =	�g���v���p�e�B�ł͐ݒ�ł��Ȃ��Ƃ���.

		// �����X�V
		int   nFlag 		= dwExProp / EXPROP_REFRESH_NONE;
		m_nReload			= 0;
		while ( (nFlag >>= 1) > 0 )
			m_nReload++;

	  #if 1 //+++ �����֌W�ǉ�
		if (IDD == IDD_DIALOG_EXPROPERTY) {
			m_exs_bUsePost = false;
			if (m_strUrlFile.GetLength() > 0) {
				CIniFileI	pr(m_strUrlFile, m_strSection);
				//DWORD	dwExPropOpt= pr.GetValue(EXPROP_OPTION);
				m_exs_adrBar	   = dwExPropOpt & EXPROPOPT_ADDRESSBAR;
				//x m_exs_adrBar   = pr.GetValue(  _T("AddressBar") 	);
				m_exs_frontURL	   = pr.GetString( _T("FrontURL")		);
				m_exs_backURL	   = pr.GetString( _T("BackURL")		);
				m_exs_frontKeyword = pr.GetString( _T("FrontKeyWord")	);
				m_exs_backKeyword  = pr.GetString( _T("BackKeyWord")	);
				m_exs_encode	   = pr.GetValue ( _T("Encode") 		);
				m_exs_bUsePost	   = pr.GetValue ( _T("UsePost")		) != 0;
				m_exs_ShortcutCode = pr.GetString( _T("ShortcutCode")   );
			}
		}
	  #endif
	}

	/// �_�C���O�ɕ\������^�C�g����ݒ�
	void SetTitle(CString strTitle) { m_strTitle = strTitle; }

	// attributes
	CString GetURL() const { return m_strUrlBase; }
	boost::optional<DWORD>	GetExProp() const 
	{ 
		if (m_bEnabled)
			return m_dwExProp;
		else
			return boost::none; 
	}
	boost::optional<DWORD>	GetExPropOpt() const 
	{
		if (m_bEnabled)
			return m_dwExPropOpt;
		else
			return boost::none;
	}


	BEGIN_DDX_MAP(CExPropertyDialogT<IDD_DLG> )
		DDX_CHECK( IDC_CHECK_DLACTIVEXCTLS	, m_nDLActiveX	)
		DDX_CHECK( IDC_CHECK_IMAGE			, m_nImage		)
		DDX_CHECK( IDC_CHECK_JAVA			, m_nJava		)
		DDX_CHECK( IDC_CHECK_NAVILOCK		, m_nNaviLock	)
		DDX_CHECK( IDC_CHECK_SCROLLBAR		, m_nFlatView	)	//+++
		DDX_CHECK( IDC_CHECK_RUNACTIVEXCTLS	, m_nRunActiveX )
		DDX_CHECK( IDC_CHECK_SCRIPTS 		, m_nScript 	)
		DDX_CHECK( IDC_CHECK_SOUND			, m_nSound		)
		DDX_CHECK( IDC_CHECK_VIDEO			, m_nVideo		)
		DDX_CHECK( IDC_CHK_BLOCK_MAILTO		, m_nBlockMailTo)
		DDX_CHECK( IDC_CHK_MOUSE_GESTURE 	, m_nGesture	)
		DDX_CHECK( IDC_CHK_MSG_FILTER		, m_nFilter 	)
		DDX_RADIO( IDC_RADIO_RELOAD_NONE 	, m_nReload 	)
		DDX_CHECK( IDC_CHECK_ENABLE			, m_bEnabled	)
		//+++ �T�[�`�֌W�ǉ�
		if (IDD == IDD_DIALOG_EXPROPERTY) {
			DDX_TEXT ( IDC_EDIT_EXSEARCH_FRONTURL	, m_exs_frontURL	)
			DDX_TEXT ( IDC_EDIT_EXSEARCH_BACKURL 	, m_exs_backURL 	)
			DDX_TEXT ( IDC_EDIT_EXSEARCH_FRONTKEYWORD,m_exs_frontKeyword)
			DDX_TEXT ( IDC_EDIT_EXSEARCH_BACKKEYWORD, m_exs_backKeyword )
			DDX_CHECK( IDC_CHECK_EXSEARCH_ADDRESSBAR, m_exs_adrBar		)
			DDX_RADIO( IDC_RADIO_EXSEARCH_NONE		, m_exs_encode		)
			DDX_CHECK( IDC_CHECK_USEPOST			, m_exs_bUsePost	)
			DDX_TEXT ( IDC_EDIT_SHORTCUTCODE		, m_exs_ShortcutCode)
		}
	END_DDX_MAP()


	BEGIN_MSG_MAP(CExPropertyDialogT<IDD_DLG> )
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK				, OnOK			)
		COMMAND_ID_HANDLER_EX(IDCANCEL			, OnCancel		)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_DELETE , OnDelete		)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_DEFAULT, OnDefault 	)
		COMMAND_ID_HANDLER_EX(IDC_BTN_DFLTBASE  , OnDfltBase 	)
		COMMAND_ID_HANDLER_EX(IDC_BTN_EXPROP_URL, OnBtnExpropUrl)
	END_MSG_MAP()


private:
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
	{
	  #if 1	//+++ URL ��ҏW�\�ɕύX. URL��\�������Ȃ��ꍇ�́AURL�{�^�����Ȃ���.
		if (m_bURLMode) {
			CEdit( GetDlgItem(IDC_EDIT_EXPROP_URL) ).SetWindowText(m_strUrlBase);
			CEdit( GetDlgItem(IDC_EDIT_EXPROP_URL) ).SetReadOnly(1);
			m_urlEditBtnSw = 0;
			CEdit( GetDlgItem(IDC_EDIT_EXPROP_URL) ).EnableWindow(1);
			CButton( GetDlgItem(IDC_BTN_EXPROP_URL) ).EnableWindow(1);
		} else {
			CButton( GetDlgItem(IDC_BTN_EXPROP_URL) ).EnableWindow(0);
			CEdit( GetDlgItem(IDC_EDIT_EXPROP_URL) ).EnableWindow(0);
		}
	  #else
		CStatic( GetDlgItem(IDC_STATIC_URL) ).SetWindowText(strUrl);
	  #endif

		if ( m_strTitle.IsEmpty() ) {
			CString strTitle = MtlGetFileName(m_strUrlFile);
			strTitle = strTitle.Left( strTitle.ReverseFind('.') );
			CStatic( GetDlgItem(IDC_STATIC_TITLE) ).SetWindowText(strTitle);
		} else {
			CStatic( GetDlgItem(IDC_STATIC_TITLE) ).SetWindowText(m_strTitle);
		}

		DoDataExchange(DDX_LOAD);
		return 1;
	}


	//+++ �g���v���p�e�B�����{�^����Ԃɕϊ�.
	static int		exProp2btn(DWORD dwExProp, DWORD flag) {
		return (dwExProp & flag) / (flag / 3);
	}


	//+++ �{�^����Ԃ��g���v���p�e�B���ɕϊ�
	static DWORD	btn2ExProp(int stat, DWORD flag) {
	  #if 1
		if (stat == BST_UNCHECKED)
			return 0;					//+++ �`�F�b�N����Ă��Ȃ��ꍇ
		if (stat == BST_INDETERMINATE)
			return 2 * flag / 3;		//+++ �f�t�H���g�ݒ��p����ꍇ
		return flag / 3;				//+++ �`�F�b�N����Ă���ꍇ
	  #else
		return stat * (flag / 3);
	  #endif
	}


	void OnOK(UINT uNotifyCode, int nID, HWND hWndCtl)
	{
		DoDataExchange(DDX_SAVE);

		DWORD		dwExProp  = 0;
		dwExProp  |= btn2ExProp( m_nImage		, EXPROP_DLIMAGE	);
		dwExProp  |= btn2ExProp( m_nVideo		, EXPROP_VIDEO 	 	);
		dwExProp  |= btn2ExProp( m_nSound		, EXPROP_SOUND 	 	);
		dwExProp  |= btn2ExProp( m_nRunActiveX	, EXPROP_RUNACTIVEX );
		dwExProp  |= btn2ExProp( m_nDLActiveX	, EXPROP_DLACTIVEX  );
		dwExProp  |= btn2ExProp( m_nScript		, EXPROP_SCRIPT	 	);
		dwExProp  |= btn2ExProp( m_nJava		, EXPROP_JAVA		);

		dwExProp  |= btn2ExProp( m_nNaviLock	, EXPROP_NAVI		);
		dwExProp  |= btn2ExProp( m_nFilter		, EXPROP_FILTER	 	);
		dwExProp  |= btn2ExProp( m_nGesture 	, EXPROP_GETSTURE	);
		dwExProp  |= btn2ExProp( m_nBlockMailTo	, EXPROP_MAILTO		);

		if (dwExProp || m_bEnabled || m_nReload > 0) {
			if (m_nReload != -1)
				dwExProp |= (EXPROP_REFRESH_NONE << m_nReload);	// �����X�V�t���O�ǉ�
		}

		m_dwExProp = dwExProp;

		//+++ �ǉ�
		DWORD		dwExPropOpt = 0;
		if (m_exs_adrBar) dwExPropOpt |= EXPROPOPT_ADDRESSBAR;
		dwExPropOpt   |= btn2ExProp( m_nFlatView, EXPROPOPT_FLATVIEW);	//+++
		m_dwExPropOpt = dwExPropOpt;

		// �Z�[�u
		if (m_strUrlFile.GetLength() > 0) {
			CIniFileIO	pr(m_strUrlFile, m_strSection);
			if (m_bEnabled || dwExProp || pr.GetValue(EXPROP_KEY_ENABLED, FALSE))	
				pr.SetValue(m_bEnabled	, EXPROP_KEY_ENABLED);
			if (m_bEnabled || dwExProp || pr.GetValue(EXPROP_KEY, 0xAAAAAA/*�����l���f�t�H���g�ݒ�ɂȂ�悤��.*/))
				pr.SetValue(dwExProp	, EXPROP_KEY); 

			//+++ �����֌W�ǉ�
			if (IDD == IDD_DIALOG_EXPROPERTY) {
				if (dwExPropOpt || pr.GetValue(EXPROP_OPTION, 8))
					pr.SetValue(dwExPropOpt , EXPROP_OPTION);

				if ( m_exs_frontURL.IsEmpty() == 0 || pr.GetString(_T("FrontURL")).IsEmpty() == 0
				  || m_exs_backURL.IsEmpty()  == 0 || pr.GetString(_T("BackURL")).IsEmpty()  == 0 )
				{
					pr.SetString(m_exs_frontURL		, _T("FrontURL")	);
					pr.SetString(m_exs_backURL		, _T("BackURL") 	);
				}

				if ( m_exs_frontKeyword.IsEmpty() == 0 || pr.GetString(_T("FrontKeyword")).IsEmpty() == 0
				  || m_exs_backKeyword.IsEmpty()  == 0 || pr.GetString(_T("BackKeyword")).IsEmpty()  == 0)
				{
					pr.SetString(m_exs_frontKeyword	, _T("FrontKeyword"));
					pr.SetString(m_exs_backKeyword	, _T("BackKeyword") );
				}

				if (m_exs_encode || pr.GetValue(_T("Encode"), 0) != 0)
					pr.SetValue(m_exs_encode	, _T("Encode")		);

				pr.SetValue(m_exs_bUsePost, _T("UsePost"));
				pr.SetString(m_exs_ShortcutCode, _T("ShortCutCode"));
			}
		}

		//+++ URL �̕ҏW���s�����ꍇ
		if (m_bURLMode) {
			CString 	strUrl = MtlGetWindowText( GetDlgItem(IDC_EDIT_EXPROP_URL) );
			if (m_strUrlBase != strUrl) {
				m_strUrlBase = strUrl;
				if (m_strUrlFile.GetLength() > 0) {
					CIniFileIO	pr( m_strUrlFile, _T("InternetShortcut") );
					pr.SetString( strUrl, _T("URL") );
				}
			}
		}

		EndDialog(nID);
	}


	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl) { EndDialog(nID); }

	/// ����
	void OnDelete(UINT uNotifyCode, int nID, HWND hWndCtl)
	{
		m_nImage		= 0;
		m_nVideo		= 0;
		m_nSound		= 0;
		m_nRunActiveX	= 0;
		m_nDLActiveX	= 0;
		m_nScript		= 0;
		m_nJava 		= 0;
		m_nNaviLock 	= 0;
		m_nFlatView		= 0;	//+++
		m_nFilter		= 0;
		m_nGesture		= 0;
		m_nBlockMailTo	= 0;
		m_nReload		= 0;

		if (m_strUrlFile.GetLength() > 0) {
			if (m_strSection == DONUT_SECTION) {
				//x MtlIniDeleteSection(m_strUrlFile, m_strSection);	//+++ �����o�[�ɒu������.
				CIniFileO 	pr(m_strUrlFile, m_strSection);
				pr.DeleteSection();
			} else {
				CIniFileO 	pr(m_strUrlFile, m_strSection);
				pr.DeleteValue(EXPROP_KEY);
				pr.DeleteValue(EXPROP_KEY_ENABLED);
			}
		}

		m_bEnabled	= FALSE;

		//+++ �����֌W�ǉ�.
		m_exs_frontURL.Empty();
		m_exs_backURL.Empty();
		m_exs_frontKeyword.Empty();
		m_exs_backKeyword.Empty();
		m_exs_adrBar	   = 0;
		m_exs_encode	   = 0;
		m_exs_ShortcutCode.Empty();

		DoDataExchange(DDX_LOAD);
	}

	/// ���݂̕W���ݒ�
	void OnDefault(UINT uNotifyCode, int nID, HWND hWndCtl)
	{
		DWORD 	dwDLFlag	= CDLControlOption::s_dwDLControlFlags;
		DWORD 	dwExStyle	= CDLControlOption::s_dwExtendedStyleFlags; //_dwViewStyle;

		m_nReload	   = 0;

		m_nImage	   =  (dwDLFlag & DLCTL_DLIMAGES		 )	!= 0;	//+++	? 1 : 0;
		m_nVideo	   =  (dwDLFlag & DLCTL_VIDEOS			 )	!= 0;	//+++	? 1 : 0;
		m_nSound	   =  (dwDLFlag & DLCTL_BGSOUNDS		 )	!= 0;	//+++	? 1 : 0;
		m_nRunActiveX  = !(dwDLFlag & DLCTL_NO_RUNACTIVEXCTLS); 		//+++	? 1 : 0;
		m_nDLActiveX   = !(dwDLFlag & DLCTL_NO_DLACTIVEXCTLS ); 		//+++	? 1 : 0;
		m_nScript	   = !(dwDLFlag & DLCTL_NO_SCRIPTS		 ); 		//+++	? 1 : 0;
		m_nJava 	   = !(dwDLFlag & DLCTL_NO_JAVA 		 ); 		//+++	? 1 : 0;

		m_nNaviLock    =  (dwExStyle & DVS_EX_OPENNEWWIN	 )	!= 0;	//+++	? 1 : 0;
		m_nFlatView    =  (dwExStyle & DVS_EX_FLATVIEW	 	 )	!= 0;	//+++
		m_nFilter	   =  (dwExStyle & DVS_EX_MESSAGE_FILTER )	!= 0;	//+++	? 1 : 0;
		m_nGesture	   =  (dwExStyle & DVS_EX_MOUSE_GESTURE  )	!= 0;	//+++	? 1 : 0;
		m_nBlockMailTo =  (dwExStyle & DVS_EX_BLOCK_MAILTO	 )	!= 0;	//+++	? 1 : 0;

		DoDataExchange(DDX_LOAD);
	}

	/// �W���ݒ�C��
	void OnDfltBase(UINT uNotifyCode, int nID, HWND hWndCtl)
	{
		m_nReload	    = 0;

		m_nImage		= BST_INDETERMINATE;
		m_nVideo		= BST_INDETERMINATE;
		m_nSound		= BST_INDETERMINATE;
		m_nRunActiveX	= BST_INDETERMINATE;
		m_nDLActiveX	= BST_INDETERMINATE;
		m_nScript		= BST_INDETERMINATE;
		m_nJava 		= BST_INDETERMINATE;
		m_nNaviLock 	= BST_INDETERMINATE;	//+++
		m_nFlatView 	= BST_INDETERMINATE;	//+++
		m_nFilter		= BST_INDETERMINATE;
		m_nGesture		= BST_INDETERMINATE;
		m_nBlockMailTo	= BST_INDETERMINATE;

		DoDataExchange(DDX_LOAD);
	}


  #if 1	//+++ URL ��ҏW�\�ɕύX. URL��\�������Ȃ��ꍇ�́AURL�{�^�����Ȃ���.
	void OnBtnExpropUrl(UINT uNotifyCode, int nID, HWND hWndCtl)
	{
		CEdit( GetDlgItem(IDC_EDIT_EXPROP_URL) ).SetReadOnly( m_urlEditBtnSw );
		m_urlEditBtnSw ^= 1;
		DoDataExchange(FALSE);
	}
  #endif


private:
	DWORD		m_dwExProp;
	DWORD		m_dwExPropOpt;		//+++
	BOOL		m_bEnabled;

	CString 	m_strUrlFile;
	CString 	m_strSection;

	CString 	m_strTitle;

	int 		m_nDLActiveX;
	int 		m_nImage;
	int 		m_nJava;
	int 		m_nNaviLock;
	int 		m_nRunActiveX;
	int 		m_nScript;
	int 		m_nScrollBar;
	int 		m_nSound;
	int 		m_nVideo;
	int 		m_nBlockMailTo;
	int 		m_nGesture;
	int 		m_nFilter;
	int 		m_nReload;
	int			m_nFlatView;				//+++

  #if 1 //+++ �T�[�`�֌W�ǉ�
	CString 	m_exs_frontURL;
	CString 	m_exs_backURL;
	CString 	m_exs_frontKeyword;
	CString 	m_exs_backKeyword;
	int 		m_exs_adrBar;
	int 		m_exs_encode;
	bool		m_exs_bUsePost;
	CString		m_exs_ShortcutCode;
  #endif
  #if 1	//+++ url�ҏW
	bool		m_bURLMode;				//+++ 0:url��  1:url�L.
	int			m_urlEditBtnSw;			//+++ url�ҏW��on,off
	CString		m_strUrlBase;			//+++ �G�f�B�b�g�`�F�b�N�p
  #endif
};

typedef CExPropertyDialogT<IDD_DIALOG_EXPROPERTY>		CExPropertyDialog;
//typedef CExPropertyDialogT<IDD_DIALOG_EXPROPERTY2>	CExPropertyDialog2;

