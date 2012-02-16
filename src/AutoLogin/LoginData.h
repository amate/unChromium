/**
*	@file	LoginData.h
*	@brief	���O�C�������Ǘ����܂�
*/

#pragma once

#include <map>

typedef std::map<CString, CString>	NameValueMap;
typedef std::map<CString, bool>		CheckboxMap;

struct LoginInfomation
{
	CString strLoginUrl;
	NameValueMap	mapNameValue;
	CheckboxMap		mapCheckbox;
};


////////////////////////////////////////
/// ���O�C�����̓��o�͂ƌ����A�Ǘ����s��

class CLoginDataManager
{
public:

	static void	Load();
	static void	Save();
	static void	Import(LPCTSTR strPath);
	static void	Export(LPCTSTR strPath);

	static int		Find(LPCTSTR strUrl);
	static void		GetNameValueMap(int nIndex, NameValueMap*& pmap);
	static void		GetCheckboxMap(int nIndex, CheckboxMap*& pmapCheckbox);

protected:
	// Data members
	static vector<LoginInfomation>	s_vecLoginInfo;
	static CString	s_strLoginDataPath;

private:
	static bool	_ReadData(LPCTSTR strPath, vector<char>& vecData);
	static void	_DeSerializeLoginData(const std::wstring& strData);
	static void	_SerializeLoginData(std::wstring& strData);
	static bool	_WriteData(LPCTSTR strPath, BYTE* pData, DWORD size);
};



#include "resource.h"

////////////////////////////////////////
/// ���O�C�����ҏW�_�C�A���O

class CLoginInfoEditDialog : 
	public CDialogImpl<CLoginInfoEditDialog>,
	public CWinDataExchange<CLoginInfoEditDialog>,
	protected CLoginDataManager
{
public:
	enum { IDD = IDD_LOGININFOEDITOR };

	enum { s_cMaxNameValue = 3, s_cMaxCheckbox = 2 };

	CLoginInfoEditDialog(const LoginInfomation& info);

	void	SetAutoLoginfunc(function<void ()> func) { m_funcAutoLogin = func; }

	// DDX map
    BEGIN_DDX_MAP( CLoginInfoEditDialog )
        DDX_CONTROL_HANDLE(IDC_URL_LIST, m_UrlList)

		DDX_TEXT(IDC_EDIT_URL, m_Url)
		DDX_TEXT(IDC_NAME1	, m_Name[0])
		DDX_TEXT(IDC_NAME2	, m_Name[1])
		DDX_TEXT(IDC_NAME3	, m_Name[2])
		DDX_TEXT(IDC_VALUE1	, m_Value[0])
		DDX_TEXT(IDC_VALUE2	, m_Value[1])
		DDX_TEXT(IDC_VALUE3	, m_Value[2])
		DDX_TEXT(IDC_CHECKNAME1, m_CheckboxName[0])
		DDX_TEXT(IDC_CHECKNAME2, m_CheckboxName[1])
		DDX_CHECK(IDC_CHECKBOX1, m_bCheck[0])
		DDX_CHECK(IDC_CHECKBOX2, m_bCheck[1])
    END_DDX_MAP()

	// Message map
	BEGIN_MSG_MAP_EX( CLoginInfoEditDialog )
		MSG_WM_INITDIALOG( OnInitDialog )
		COMMAND_ID_HANDLER_EX( IDC_BTN_APPLY, OnApply )
		COMMAND_ID_HANDLER_EX( IDC_BTN_ADD	, OnAdd )
		COMMAND_ID_HANDLER_EX( IDC_BTN_DELETE, OnDelete )
		COMMAND_ID_HANDLER_EX( IDC_BTN_NEW	, OnNew )
		COMMAND_ID_HANDLER_EX( IDC_BTN_IMPORT, OnImport )
		COMMAND_ID_HANDLER_EX( IDC_BTN_EXPORT, OnExport )
		COMMAND_ID_HANDLER_EX( IDC_BTN_TEST	, OnTest )
		COMMAND_ID_HANDLER_EX( IDOK		, OnFinish )
		COMMAND_ID_HANDLER_EX( IDCANCEL	, OnFinish )
		COMMAND_HANDLER_EX(IDC_URL_LIST, LBN_SELCHANGE, OnUrlListChange )
	END_MSG_MAP()

	BOOL	OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void	OnApply(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnAdd(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnDelete(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnNew(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnImport(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnExport(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnTest(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnFinish(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnUrlListChange(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	void	_SetLoginInfoData();
	void	_SetCopyLoginInfo();

	// Data members
	LoginInfomation	m_info;

	CListBox	m_UrlList;

	CString	m_Url;
	CString	m_Name[s_cMaxNameValue];
	CString	m_Value[s_cMaxNameValue];
	CString m_CheckboxName[s_cMaxCheckbox];
	bool	m_bCheck[s_cMaxCheckbox];

	function<void ()>	m_funcAutoLogin;
	
};























