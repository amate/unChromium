/**
 *	@file	PluginManager.cpp
 *	@brief	�v���O�C���E�}�l�[�W��
 */

#include "stdafx.h"
#include "PluginManager.h"
#include "IniFile.h"
#include "DonutDefine.h"
#include "DonutPFunc.h"
#include "option/PluginDialog.h"


//////////////////////////////////////////////////////////
// CPluginManager

// ��`
std::array<CPluginManager::PluginArray, PLUGIN_TYPECNT + 1>	CPluginManager::m_arrPluginData;

/*
	nKind �� 1 Origin 1toPLUGIN_TYPECNT
	�Ƃ������Ƃ�Y��ď����ĂЂǂ��ڂɂ�����
 */


//--------------------------------------------
/// �v���O�C���������\���̂̃|�C���^���擾����
CPluginManager::PluginData *CPluginManager::GetDataPtr(int nKind, int nIndex)
{
	if (nKind <= 0 || PLUGIN_TYPECNT < nKind)
		return NULL;

	PluginArray *pary = &m_arrPluginData[nKind];

	if (nIndex < 0 || (int) pary->size() <= nIndex)
		return NULL;

	return &pary->at(nIndex);
}


//--------------------------------------------
/// ���[�h�Ɏ��s�����v���O�C����Еt����
void CPluginManager::SettleBadPlugin(int nKind, int nIndex, PluginData &data, HWND hWnd)
{
	TCHAR 	szBuf[MAX_PATH] = _T("\0");
	::GetModuleFileName( data.hInstDLL, szBuf, MAX_PATH );
	ErrMsg_FailLoad(data.strCaption, szBuf, hWnd);

	RemovePlugin(nKind, nIndex);
}



BOOL CPluginManager::ReleasePluginData(PluginData &data)
{
	if ( ::IsWindow(data.hWnd) ) {
		try {	//+++
			::DestroyWindow(data.hWnd);
			data.hWnd = NULL;
		} catch (...) {
		  #ifdef _DEBUG
			FILE *fp = _wfopen(Misc::GetExeDirectory() + L"errlog.txt", L"a");
			if (fp) {
				fprintf(fp, "�v���O�C��\"%s\"�̏I�����ɃG���[���������܂����B\n", data.strCaption);
				fclose(fp);
			}
		  #endif //_DEBUG
		}
	}

	if (data.hInstDLL) {
		::FreeLibrary(data.hInstDLL);
		data.hInstDLL = NULL;
	}

	return TRUE;
}


//----------------------------------------------
/// �v���O�C������z�񂩂�폜����i�E�B���h�E���̊J�����s���j
BOOL CPluginManager::RemovePlugin(int nKind, int nIndex)
{
	if (nKind <= 0 || PLUGIN_TYPECNT < nKind)
		return FALSE;

	PluginArray *pary = &m_arrPluginData[nKind];

	if (nIndex < 0 || (int) pary->size() <= nIndex)
		return FALSE;

	if ( ReleasePluginData( pary->at(nIndex) ) )
		pary->erase(pary->begin() + nIndex);
	else
		return FALSE;

	return TRUE;
}



void CPluginManager::ErrMsg_FailLoad(LPCTSTR lpstrName, LPCTSTR lpstrPath, HWND hWnd)
{
	CString strMsg;
	strMsg.Format(_T("\"%s\"(%s)\n�����[�h���邱�Ƃ��ł��܂���ł����B\n����N�����͖����ɂȂ�܂��B"), lpstrName, lpstrPath);
	MessageBox(hWnd, strMsg, _T("Plugin���"), MB_OK | MB_ICONSTOP);
}



CPluginManager::CPluginManager()
{
}



CPluginManager::~CPluginManager()
{
}



void CPluginManager::Init()
{
}



void CPluginManager::Term()
{
	DeleteAllPlugin();
}


//---------------------------------------------
/// �v���O�C���̃^�C�g�����擾����
/// BOOL�^�̗̈�ւ̃|�C���^���w�肳���ΐ����E���s�������ɕԂ�
CString CPluginManager::GetCaption(int nKind, int nIndex, BOOL *pbRet)
{
	PluginData *pdata = GetDataPtr(nKind, nIndex);

	if (pdata) {
		if (pbRet)
			*pbRet = TRUE;

		return pdata->strCaption;
	} else {
		if (pbRet)
			*pbRet = FALSE;

		return _T("");
	}
}


//----------------------------------------------
/// �v���O�C���̃E�B���h�E�n���h�����擾����
/// ���s�����Ƃ��A�������͍쐬����Ă��Ȃ��ꍇ��NULL��Ԃ�
HWND CPluginManager::GetHWND(PLUGIN_TYPE nKind, int nIndex)
{
	PluginData *pdata = GetDataPtr(nKind, nIndex);

	if (!pdata)
		return NULL;

	return pdata->hWnd;
}

//--------------------------------------
/// �w�肳�ꂽ��ނ̃v���O�C������Ԃ�
int CPluginManager::GetCount(PLUGIN_TYPE nKind)
{
	if (nKind <= 0 || PLUGIN_TYPECNT < nKind)
		return -1;

	return (int)m_arrPluginData[nKind].size();
}



bool CPluginManager::IsEarlyLoading(PLUGIN_TYPE nKind, int nIndex)
{
	PluginData *pdata = GetDataPtr(nKind, nIndex);
	if (!pdata)
		return false;

	return (pdata->nStyle & PLUGIN_STYLE_EARLYLOADING) != 0;	//+++ ? true : false;
}


//----------------------------------------
/// DLL�̃��[�h�y�уv���O�C�����̓ǂݍ��݂��s��
BOOL CPluginManager::ReadPluginData(PLUGIN_TYPE nKind, HWND hWnd)
{
	if (nKind <= 0 || PLUGIN_TYPECNT < nKind)
		return FALSE;

	CString 	strKey;
	strKey.Format(_T("Plugin%02d"), nKind);

	CIniFileIO	pr(g_szIniFileName, strKey);

	CString 	strDLLPath = Misc::GetExeDirectory() + PluginDir();
	int	nCount = pr.GetValuei(_T("Count"));
	for (int nIndex = 0; nIndex < nCount; ++nIndex) {
		try {	//+++
			strKey.Format(_T("%02d"), nIndex);
			//DLL�̃��[�h
			CString   strDLLFile = strDLLPath + pr.GetString( strKey );
			HINSTANCE hInstDLL	 = ::LoadLibrary( strDLLFile );

			if (hInstDLL) {
				//�f�[�^�̏�����
				PluginData		 data;
				data.hInstDLL  = hInstDLL;
				data.strIniKey = strKey;
				data.fpEvent   = GetPtr_PluginEvent(hInstDLL);

				//GetPluginInfo�֐����g���ăv���O�C���ŗL�̏����擾
				LPFGETPLUGININFO	pfGetPluginInfo = GetPtr_GetPluginInfo(hInstDLL);
				if (pfGetPluginInfo) {
					PLUGININFO *	ppi = new PLUGININFO;
					pfGetPluginInfo(ppi);	// �擾
					data.strCaption 	= ppi->name;
					data.nStyle 		= ppi->type & 0xFFFFFFF0;
					delete 		ppi;
				  #if 1	//+++ �h�b�L���O�o�[�v���O�C�����G�N�X�v���[���o�[�v���O�C�������ɂ���.
					if (nKind == PLT_DOCKINGBAR) {
						nKind 	    =  PLT_EXPLORERBAR;
						//+++ data.flags |= 1;
					}
				  #endif
				}

				//�f�[�^��o�^
				m_arrPluginData[nKind].push_back(data);
			} else {
				//���[�h�Ɏ��s�����̂œo�^��������
				ErrMsg_FailLoad(_T("N/A"), strDLLFile, hWnd);
				pr.DeleteValue(strKey);
			}
		} catch (...) {
			ATLASSERT(0);
		}
	}

	return TRUE;
}


//-------------------------------------------------------------------
/// �v���O�C���̃��[�h���s��
/// �����̓v���O�C���̎�ނƔԍ��ƁA�g�ݍ��ސe�E�B���h�E�̃n���h��
BOOL CPluginManager::LoadPlugin(PLUGIN_TYPE nKind, int nIndex, HWND hWndParent, bool bForce)
{
	PluginData *pdata	 = GetDataPtr(nKind, nIndex);

	if (!pdata)
		return FALSE;

	if ( ::IsWindow(pdata->hWnd) )
		return TRUE;

											//���Ƀ��[�h�ς�
	bool		bFuncPtr = false;			//*
	bool		bDelay	 = (pdata->nStyle & PLUGIN_STYLE_EARLYLOADING) == 0;

	if (bDelay && !bForce)
		return TRUE;						//��œǂݍ���

	try {	//+++
		switch (nKind) {
		case PLT_TOOLBAR:
			{
				//�c�[���o�[�v���O�C���̃��[�h - CreateToolBar�֐��̎��s
				LPFCREATETOOLBAR pfCreateToolBar = GetPtr_CreateToolBar(pdata->hInstDLL);
				bFuncPtr = pfCreateToolBar != 0;	//+++ ? true : false;

				if (bFuncPtr) {
					pdata->hWnd = pfCreateToolBar(hWndParent, IDC_PLUGIN_TOOLBAR + nIndex);
				}
			}
			break;

		case PLT_EXPLORERBAR:
			{
				//�G�N�X�v���[���o�[�v���O�C���̃��[�h - CreateExplorerPane�֐��̎��s
				LPFCREATEEXPLORERPANE pfCreateExplorerBarPane = GetPtr_CreateExplorerPane(pdata->hInstDLL);
				bFuncPtr = pfCreateExplorerBarPane != 0;	//+++ ? true : false;

				if (bFuncPtr) {
					pdata->hWnd = pfCreateExplorerBarPane(hWndParent, IDC_PLUGIN_EXPLORERBAR + nIndex);
				}
			  #if 1	//+++ �������PLT_DOCKINGBAR�o�[��PLT_TOOLBAR�����Ă���ꍇ...
				else /*if (pdata->flags & 1)*/ {
					//�h�b�L���O�v���O�C���̃��[�h - CreatePluginWindow�֐��̎��s
					LPFCREATEPLUGINWINDOW pfCreatePluginWindow = GetPtr_CreatePluginWindow(pdata->hInstDLL);
					bFuncPtr = pfCreatePluginWindow != 0;	//+++ ? true : false;
					if (bFuncPtr) {
						pdata->hWnd = pfCreatePluginWindow(hWndParent);
						::SetParent(pdata->hWnd, hWndParent);
					}
				}
			  #endif
			}
			break;

		case PLT_DOCKINGBAR:
			{
				//�h�b�L���O�v���O�C���̃��[�h - CreatePluginWindow�֐��̎��s
				LPFCREATEPLUGINWINDOW pfCreatePluginWindow = GetPtr_CreatePluginWindow(pdata->hInstDLL);
				bFuncPtr = pfCreatePluginWindow != 0;	//+++ ? true : false;

				if (bFuncPtr) {
					pdata->hWnd = pfCreatePluginWindow(hWndParent);
					::SetParent(pdata->hWnd, hWndParent);		//+++ �����ɐݒ�...
				}
			}
			break;

		case PLT_OPERATION:			//+++ �����͂��Ȃ��Ǝv���邪�A�O�̂��ߗp��.
			return TRUE;			//+++

	  #if 0
		case PLT_STATUSBAR:			//+++ �������p�ӂ��Ă݂�.
			{
				// �v���O�C���̃��[�h - CreatePluginWindow�֐��̎��s
				LPFCREATEPLUGINWINDOW pfCreatePluginWindow = GetPtr_CreatePluginWindow(pdata->hInstDLL);
				bFuncPtr = pfCreatePluginWindow != 0;
				if (bFuncPtr) {
					pdata->hWnd = pfCreatePluginWindow(hWndParent);
					::SetParent(pdata->hWnd, hWndStatusBar);
				}
			}
			break;
	   #endif

		default:
			//����ȊO�̎�ނ̃v���O�C�� - �������Ȃ̂ŃA�T�[�g����
			ATLASSERT(FALSE);
		}

		//���s�̏ꍇ�̌�Еt��
		if ( !bFuncPtr || !::IsWindow(pdata->hWnd) ) {
			SettleBadPlugin(nKind, nIndex, *pdata, hWndParent);
			return FALSE;
		}

		} catch (...) {
		ATLASSERT(0);
	}

	return TRUE;
}

//----------------------------------------
/// �S�Ẵv���O�C����ǂݍ���
BOOL CPluginManager::LoadAllPlugin(PLUGIN_TYPE nKind, HWND hWndParent, bool bForce)
{
	if (nKind <= 0 || PLUGIN_TYPECNT < nKind)
		return FALSE;

	int nCount = GetCount(nKind);

	for (int i = 0; i < nCount; i++) {
		if ( !LoadPlugin(nKind, i, hWndParent, bForce) ) {				//���[�h���s����ƃf�[�^���������
			i--;
			nCount--;
		}
	}

	return TRUE;
}

//---------------------------------------------
/// �w�肳�ꂽ��ނ̃v���O�C�������ׂĉ������(-1�ł��ׂĂ̎�ނ����)
void CPluginManager::DeleteAllPlugin(int nKind)
{
	int 	nStart;
	int		nEnd;

	if (nKind == -1) {
		nStart = 1;
		nEnd   = PLUGIN_TYPECNT;
	} else if (1 <= nKind && nKind <= PLUGIN_TYPECNT) {
		nStart = nEnd = nKind;
	} else {
		ATLASSERT(FALSE);
		return;
	}

	for (int i = nStart; i <= nEnd; i++) {
		PluginArray *		  pary = &m_arrPluginData[i];

		for (PluginArray::iterator it = pary->begin(); it != pary->end(); ++it)
			ReleasePluginData(*it);

		pary->clear();
	}
}



// �v���O�C���̊֐��Ăяo�����[�`��

void CPluginManager::Call_ShowExplorerMenu(int nIndex, int x, int y)	//�G�N�X�v���[���o�[�̂݃T�|�[�g
{
	PluginData *		pdata			   = GetDataPtr(PLT_EXPLORERBAR, nIndex);

	if (!pdata)
		return;

	LPFSHOWEXPLORERMENU fpShowExplorerMenu = GetPtr_ShowExplorerMenu(pdata->hInstDLL);

	if (!fpShowExplorerMenu)
		return;

	fpShowExplorerMenu(x, y);
}



int CPluginManager::Call_PluginEvent(int nKind, int nIndex, UINT uMsg , FPARAM fParam, SPARAM sParam)
{
	PluginData *   pdata		 = GetDataPtr(nKind, nIndex);

	if (!pdata)
		return 0;

	try {	//+++
		LPFPLUGINEVENT fpPluginEvent = GetPtr_PluginEvent(pdata->hInstDLL);

		if (!fpPluginEvent)
			return 0;

		return fpPluginEvent(uMsg, fParam, sParam);
	} catch (...) {	//+++
		ATLASSERT(0);
	}
	return 0;
}



BOOL CPluginManager::Call_PreTranslateMessage(int nKind, int nIndex, MSG *lpMsg)
{
	PluginData *		   pdata	= GetDataPtr(nKind, nIndex);
	if (!pdata)
		return FALSE;
	try {	//+++
		LPFPRETRANSLATEMESSAGE fpPreTranslateMessage = GetPtr_PreTranslateMessage(pdata->hInstDLL);

		if (!fpPreTranslateMessage)
			return FALSE;

		return fpPreTranslateMessage(lpMsg);
	} catch (...) {
		ATLASSERT(0);
	}
	return FALSE;
}



void CPluginManager::Call_PluginSetting(int nKind, int nIndex)
{
	PluginData *	 pdata	= GetDataPtr(nKind, nIndex);
	if (!pdata)
		return;

	LPFPLUGINSETTING fpPluginSetting = GetPtr_PluginSetting(pdata->hInstDLL);

	if (!fpPluginSetting)
		return;

	fpPluginSetting();
}



BOOL CPluginManager::Call_Event_TabChanged(int nKind, int nIndex, int nNewTabIndex, IWebBrowser *pWB)	//obsolete
{
	PluginData *		pdata = GetDataPtr(nKind, nIndex);
	if (!pdata)
		return FALSE;

	LPFEVENT_TABCHANGED fpEvent_TabChanged = GetPtr_Event_TabChanged(pdata->hInstDLL);

	if (!fpEvent_TabChanged)
		return FALSE;

	return fpEvent_TabChanged(nNewTabIndex, pWB);
}



void CPluginManager::Call_ShowToolBarMenu(int nKind, int nIndex, UINT uID)								//�c�[���o�[�v���O�C���݂̂̃T�|�[�g
{
	PluginData *	   pdata = GetDataPtr(nKind, nIndex);
	if (!pdata)
		return;

	LPFSHOWTOOLBARMENU fpShowToolBarMenu = GetPtr_ShowToolBarMenu(pdata->hInstDLL);

	if (!fpShowToolBarMenu)
		return;

	fpShowToolBarMenu(uID);
}


//-----------------------------------
/// �S�Ẵv���O�C���ɑ΂��ăC�x���g�����𑗂�
void CPluginManager::BroadCast_PluginEvent(UINT uMsg, FPARAM fParam, SPARAM sParam)
{
	for (int nKind = 1; nKind <= PLUGIN_TYPECNT; nKind++) {
		int nCount = (int)m_arrPluginData[nKind].size();

		for (int i = 0; i < nCount; i++) {
			Call_PluginEvent(nKind, i, uMsg, fParam, sParam);
		}
	}
}


//------------------------------------
/// �S�Ẵv���O�C���ɑ΂��ăC�x���g���������ɑ���A��0�̒l���Ԃ��ꂽ���_�ŏI������
int CPluginManager::ChainCast_PluginEvent(UINT uMsg, FPARAM fParam, SPARAM sParam)
{
	for (int nKind = 1; nKind <= PLUGIN_TYPECNT; nKind++) {
		int nCount = (int)m_arrPluginData[nKind].size();

		for (int i = 0; i < nCount; i++) {
			int nRet = Call_PluginEvent(nKind, i, uMsg, fParam, sParam);
			if (nRet != 0)
				return nRet;
		}
	}

	return 0;
}


//--------------------------------------
///+++ "Plugin"�t�H���_����Ԃ�. �܂� Plugin32\ (64�r�b�g�ł� Plugin64\) �����݂���΂����Ԃ��������"Plugin"��Ԃ�.
CString CPluginManager::PluginDir()
{
  #ifdef WIN64
	const TCHAR* dir = _T("Plugin64");
  #else
	const TCHAR* dir = _T("Plugin32");
  #endif
	if (::PathFileExists(Misc::GetExeDirectory() + dir) == FALSE)
		dir = _T("Plugin");
	return CString(dir) + _T('\\');
}

