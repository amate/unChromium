/**
 *	@file	PluginManager.h
 *	@brief	�v���O�C���E�}�l�[�W��
 */
#pragma once

//�v���O�C���̃f�[�^�^�E�萔�̒�`
#include "./include/PluginInfo.h"
#include <array>

//+++ GetProcAddress��TCHAR���Ή��Ȃ̂ŁA_T�͍폜.

#define ENTRY_PLUGINPROC(func, type)						 \
	static type GetPtr_##func(HINSTANCE hInst)				 \
	{														 \
		if (!hInst) 										 \
			return NULL;									 \
		/*return (type) ::GetProcAddress( hInst, _T(# func) );*/ \
		return (type) ::GetProcAddress( hInst, #func );		\
	}


/////////////////////////////////////////////////
/// �v���O�C���Ɋւ���N���X

class CPluginManager 
{
public:
	typedef int  (WINAPI* LPFPLUGINEVENT)(UINT, FPARAM, SPARAM);
	typedef void (WINAPI* LPFGETPLUGININFO)(PLUGININFO *);
	typedef HWND (WINAPI* LPFCREATEPLUGINWINDOW)(HWND);
	typedef HWND (WINAPI* LPFCREATEEXPLORERPANE)(HWND, UINT);
	typedef HWND (WINAPI* LPFCREATETOOLBAR)(HWND, UINT);
	typedef void (WINAPI* LPFSHOWEXPLORERMENU)(int, int);
	typedef BOOL (WINAPI* LPFPRETRANSLATEMESSAGE)(MSG *);
	typedef void (WINAPI* LPFPLUGINSETTING)();
	typedef void (WINAPI* LPFSHOWTOOLBARMENU)(UINT);
	typedef BOOL (WINAPI* LPFEVENT_TABCHANGED)(int, IWebBrowser *); //obsolete

	//���������p�̃v���O�C���f�[�^�\����
	struct PluginData {
		HINSTANCE	   hInstDLL;
		HWND		   hWnd;
		CString 	   strCaption;
		CString 	   strIniKey;
		LPFPLUGINEVENT fpEvent;
		int 		   nStyle;
		//int		   flags;	//+++	�������ȏ����Ŏg��...����ς��.

		PluginData() : hInstDLL(NULL), hWnd(NULL), fpEvent(NULL), nStyle(PLUGIN_STYLE_DERAYLOADING)
		{	}
	};

	static CString PluginDir(); 	//+++ �v���O�C���t�H���_��Ԃ�. �Ō��\��.

private:
	//�����o�ϐ�
	typedef std::vector<PluginData> 	PluginArray;
	static std::array<PluginArray, PLUGIN_TYPECNT + 1> m_arrPluginData;	//�v���O�C���^�C�v���̃f�[�^�̔z��(vector)

  #ifdef _DEBUG
	static int				m_nLoadCount;
	static int				m_nCreateCount;
  #endif	//_DEBUG

	//�v���O�C���֐��̃|�C���^�擾�֐��ɓW�J�����}�N��
	//GetPtr_�֐�����DLL�̃C���X�^���X�n���h���������ɂ��ČĂяo��
	//ex. LPFGETPLUGININFO pfGetPInfo = GetPtr_GetPluginInfo(hInstDLL);
	ENTRY_PLUGINPROC(GetPluginInfo		, LPFGETPLUGININFO		)
	ENTRY_PLUGINPROC(PluginEvent		, LPFPLUGINEVENT		)
	ENTRY_PLUGINPROC(CreatePluginWindow , LPFCREATEPLUGINWINDOW )
	ENTRY_PLUGINPROC(CreateExplorerPane , LPFCREATEEXPLORERPANE )
	ENTRY_PLUGINPROC(CreateToolBar		, LPFCREATETOOLBAR		)
	ENTRY_PLUGINPROC(ShowExplorerMenu	, LPFSHOWEXPLORERMENU	)
	ENTRY_PLUGINPROC(PreTranslateMessage, LPFPRETRANSLATEMESSAGE)
	ENTRY_PLUGINPROC(PluginSetting		, LPFPLUGINSETTING		)
	ENTRY_PLUGINPROC(Event_TabChanged	, LPFEVENT_TABCHANGED	)
	ENTRY_PLUGINPROC(ShowToolBarMenu	, LPFSHOWTOOLBARMENU	)

	//�v���O�C���������\���̂̃|�C���^���擾����
	static PluginData * GetDataPtr(int nKind, int nIndex);

	//���[�h�Ɏ��s�����v���O�C����Еt����
	static void 		SettleBadPlugin(int nKind, int nIndex, PluginData &data, HWND hWnd = NULL);
	static BOOL 		ReleasePluginData(PluginData &data);

	//�v���O�C������z�񂩂�폜����i�E�B���h�E���̊J�����s���j
	static BOOL 		RemovePlugin(int nKind, int nIndex);
	static void 		ErrMsg_FailLoad(LPCTSTR lpstrName, LPCTSTR lpstrPath, HWND hWnd = NULL);

public:
	CPluginManager();
	~CPluginManager();
	static void 		Init();
	static void 		Term();


	//�v���O�C���̃^�C�g�����擾����
	//BOOL�^�̗̈�ւ̃|�C���^���w�肳���ΐ����E���s�������ɕԂ�
	static CString		GetCaption(int nKind, int nIndex, BOOL *pbRet = NULL);


	//�v���O�C���̃E�B���h�E�n���h�����擾����
	//���s�����Ƃ��A�������͍쐬����Ă��Ȃ��ꍇ��NULL��Ԃ�
	static HWND 		GetHWND(PLUGIN_TYPE nKind, int nIndex);

	bool				IsEarlyLoading(PLUGIN_TYPE nKind, int nIndex);

	static int			GetCount(PLUGIN_TYPE nKind);


	//DLL�̃��[�h�y�уv���O�C�����̓ǂݍ��݂��s��
	static BOOL 		ReadPluginData(PLUGIN_TYPE nKind, HWND hWnd = NULL);


	//�v���O�C���̃��[�h���s��
	//�����̓v���O�C���̎�ނƔԍ��ƁA�g�ݍ��ސe�E�B���h�E�̃n���h��
	static BOOL 		LoadPlugin(PLUGIN_TYPE nKind, int nIndex, HWND hWndParent, bool Force = true);

	static BOOL 		LoadAllPlugin(PLUGIN_TYPE nKind, HWND hWndParent, bool bForce = false);

	static void 		DeleteAllPlugin(int nKind = -1);


	//�v���O�C���̊֐��Ăяo�����[�`��
	static void 		Call_ShowExplorerMenu(int nIndex, int x, int y);
	static int			Call_PluginEvent(int nKind, int nIndex, UINT uMsg , FPARAM fParam, SPARAM sParam);
	static BOOL 		Call_PreTranslateMessage(int nKind, int nIndex, MSG *lpMsg);
	static void 		Call_PluginSetting(int nKind, int nIndex);
	static BOOL 		Call_Event_TabChanged(int nKind, int nIndex, int nNewTabIndex, IWebBrowser *pWB);
	static void 		Call_ShowToolBarMenu(int nKind, int nIndex, UINT uID);


	//�S�Ẵv���O�C���ɑ΂��ăC�x���g�����𑗂�
	static void 		BroadCast_PluginEvent(UINT uMsg, FPARAM fParam, SPARAM sParam);


	//�S�Ẵv���O�C���ɑ΂��ăC�x���g���������ɑ���A��0�̒l���Ԃ��ꂽ���_�ŏI������
	static int			ChainCast_PluginEvent(UINT uMsg, FPARAM fParam, SPARAM sParam);

};
