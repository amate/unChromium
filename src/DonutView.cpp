/**
 *	@file	DonutView.cpp
 *	@brief	View
 */

#include "stdafx.h"
#include "DonutView.h"
#include "mshtmdid.h"
#include "option\DLControlOption.h"
#include "option\MouseDialog.h"
#include "option\MenuDialog.h"
#include "Donut.h"

#include "ScriptErrorCommandTargetImpl.h"
#include "Download/DownloadManager.h"
#include "AtlHostEx.h"
#include "ParseInternetShortcutFile.h"
#include "ChildFrameCommandUIUpdater.h"

// Constants
enum {
	#if 1	//+++ xp�����ł̓t���b�g�X�N���[���o�[��ݒ�Axp�Ȍ�ł�theme��K�p.
	DOCHOSTUIFLAG_FLATVIEW		= (DOCHOSTUIFLAG_FLAT_SCROLLBAR | DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE | DOCHOSTUIFLAG_THEME),
	DOCHOSTUIFLAG_THEME_VIEW    = (DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE | DOCHOSTUIFLAG_THEME),
	DOCHOSTUIFLAG_NOT_FLATVIEW	= ( /*DOCHOSTUIFLAG_NO3DBORDER*/ DOCHOSTUIFLAG_NO3DOUTERBORDER | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE),
	#elif 1	// unDonut+
	DOCHOSTUIFLAG_FLATVIEW		= (DOCHOSTUIFLAG_FLAT_SCROLLBAR | DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE | DOCHOSTUIFLAG_THEME),
	DOCHOSTUIFLAG_NOT_FLATVIEW	= ( /*DOCHOSTUIFLAG_NO3DBORDER*/ DOCHOSTUIFLAG_NO3DOUTERBORDER | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE),
	#else
	//docHostUIFlagDEFAULT		= (docHostUIFlagFLAT_SCROLLBAR | docHostUIFlagNO3DBORDER | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE |DOCHOSTUIFLAG_THEME),
	//docHostUIFlagNotFlatView	= (docHostUIFlagNO3DBORDER | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE),		// UDT DGSTR ( added by dai
	// DonutRAPT(1.26)�̒l.
	//docHostUIFlagDEFAULT 		= (docHostUIFlagFLAT_SCROLLBAR | docHostUIFlagNO3DBORDER | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE | DOCHOSTUIFLAG_THEME),
	//docHostUIFlagNotFlatScrBar= (docHostUIFlagNO3DBORDER     | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE | DOCHOSTUIFLAG_THEME),
	#endif
};

///////////////////////////////////////////////////////////////////////////////
// CDonutView

/// Constructor

CDonutView::CDonutView(CChildFrameUIStateChange& UI)
	: m_UIChange(UI)
	, m_nDDCommand(0)
   #if _ATL_VER >= 0x700
	, m_ExternalUIDispatch(this)
   #endif
   #if defined USE_ATL3_BASE_HOSTEX == 0 /*_ATL_VER >= 0x700*/	//+++
	//, m_ExternalAmbientDispatch()
   #endif
	, m_bUseCustomDropTarget(false)
	, m_bDragAccept(false)
	, m_bExternalDrag(false)
	, m_bLightRefresh(false)
	, m_dwAutoRefreshStyle(0)
	, m_dwExStyle(0)
	, m_dwCurrentThreadId(0)
{ }


void	CDonutView::SetDefaultFlags(DWORD dwDefaultDLCtrl, DWORD dwDefaultExStyle, DWORD dwAutoRefresh)
{
	m_dwDefaultDLControlFlags	= dwDefaultDLCtrl;
	m_dwDLControlFlags			= dwDefaultDLCtrl;
	m_dwExStyle					= dwDefaultExStyle;
	m_dwAutoRefreshStyle		= dwAutoRefresh;
}

/// �����X�V�ύX
void	CDonutView::SetAutoRefreshStyle(DWORD dwStyle)
{
	m_dwAutoRefreshStyle = 0;
	DWORD dwCurFlag = DVS_AUTOREFRESH_USER;
	while (dwCurFlag > 0) {
		if (dwCurFlag & dwStyle) {
			m_dwAutoRefreshStyle = dwCurFlag;
			break;
		}
		dwCurFlag >>= 1;
	}

	KillTimer(AutoRefreshTimerID);
	_SetTimer();
}


/// DL�R���g���[����ݒ肷��
void CDonutView::PutDLControlFlags(DWORD dwDLControlFlags)
{
	m_dwDLControlFlags	= dwDLControlFlags;
	m_dwDefaultDLControlFlags = dwDLControlFlags;

	CComQIPtr<IDispatch>	spDisp = m_spHost;
	if (spDisp) {
		VARIANT 		   varResult;
		DISPPARAMS		   params = { 0 };
		HRESULT hr = spDisp->Invoke(DISPID_AMBIENT_DLCONTROL, IID_NULL, 1041 /*JP*/, DISPATCH_PROPERTYPUT, &params, &varResult, NULL, NULL);
	}
	m_UIChange.SetDLCtrl(m_dwDLControlFlags);
#if 0
	HRESULT	hr;
	CComQIPtr<IOleObject>	spOleObject = m_spBrowser;
	ATLASSERT(spOleObject);
	CComPtr<IOleClientSite>	spOleOrgSite;
	hr = spOleObject->GetClientSite(&spOleOrgSite);	// ���݂̃T�C�g��ۑ�

	hr = spOleObject->SetClientSite((IOleClientSite*)this);
	CComQIPtr<IOleControl>	spOleControl = m_spBrowser;
	ATLASSERT(spOleControl);
	hr = spOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_DLCONTROL);

	spOleObject->SetClientSite(spOleOrgSite);
#endif
}

void	CDonutView::SetExStyle(DWORD dwExStyle)
{
	m_dwExStyle	= dwExStyle;
	m_UIChange.SetExStyle(m_dwExStyle);
}




void CDonutView::SetIeMenuNoCstm(int nStatus)
{
  #if _ATL_VER >= 0x700
	m_ExternalUIDispatch.SetIEContextMenuCustom(!nStatus);
  #else
	m_spAxAmbient->SetIeMenuNoCstm(nStatus);
  #endif
}



//�h���b�O�h���b�v���̑���𐧌䂷�邩IE�R���|�ɔC���邩
void CDonutView::SetOperateDragDrop(BOOL bOn, int nCommand)
{
	CComPtr<IAxWinHostWindow> spAxWindow;
	HRESULT hr = QueryHost(&spAxWindow);
	if ( FAILED(hr) )
		return;

	//SetRegisterAsDropTarget(bOn != 0/*? true : false*/);	// false����DD���󂯕t���Ȃ�
  #if 0 //_ATL_VER >= 0x700
	if (bOn)
		m_ExternalUIDispatch.SetExternalDropTarget(this);	// DD���󂯕t����
	else
		m_ExternalUIDispatch.SetExternalDropTarget(NULL);	// DD���󂯕t���Ȃ�
  #endif
	if (bOn) {
		m_bUseCustomDropTarget = true;
	} else {
		m_bUseCustomDropTarget = false;
	}
	m_nDDCommand = nCommand;
}



// Overrides
BOOL CDonutView::PreTranslateMessage(MSG *pMsg)
{
	if (   (pMsg->message < WM_KEYFIRST   || pMsg->message > WM_KEYLAST )
		&& (pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST) )
	{
		return FALSE;
	}

	// give HTML page a chance to translate this message
	return SendMessage(WM_FORWARDMSG, 0, (LPARAM) pMsg) != 0;
}



// IUnknown
STDMETHODIMP CDonutView::QueryInterface(REFIID iid, void ** ppvObject)
{
    if (ppvObject == NULL) 
		return E_INVALIDARG;
	*ppvObject = NULL;
	HRESULT hr = S_OK;
// IID_IServiceProvider
	if (iid == IID_IServiceProvider) {
		*ppvObject = (IServiceProvider*)this;
// IID_IDropTarget
	} else if (iid == IID_IDropTarget) {
		*ppvObject = (IDropTarget*)this;
// IID_IUnknown	IDropTarget�̂��߁H
	} else if (iid == IID_IUnknown) {
		if (m_bUseCustomDropTarget)
			*ppvObject = (IUnknown*)(IDropTarget*)this;
// IID_IDownloadManager
	} else if (iid == IID_IDownloadManager) {
		*ppvObject = (IDownloadManager*)this;
	}

	if (*ppvObject == NULL)
		return E_NOINTERFACE;
	return S_OK;
}


// QueryService
STDMETHODIMP CDonutView::QueryService(REFGUID guidService, REFIID riid, void** ppv)
{
	if (guidService == SID_SDownloadManager && CDownloadManager::UseDownloadManager()) {
		*ppv = (IDownloadManager*)this;
		return S_OK;
	}

	*ppv = NULL;
    return E_NOINTERFACE;
}



// IDropTarget
STDMETHODIMP CDonutView::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	m_spDropTargetHelper->DragEnter(m_hWnd, pDataObj, (LPPOINT)&pt, *pdwEffect);
	m_bExternalDrag = MtlIsDataAvailable(pDataObj, ::RegisterClipboardFormat(CFSTR_FILENAME));
	if (m_bExternalDrag)
		*pdwEffect |= DROPEFFECT_LINK;
	m_bDragAccept = _MtlIsHlinkDataObject(pDataObj);
	if (m_bDragAccept && m_bUseCustomDropTarget)
		*pdwEffect |= DROPEFFECT_LINK | DROPEFFECT_COPY;
	//*pdwEffect |= DROPEFFECT_LINK;// | DROPEFFECT_COPY | DROPEFFECT_MOVE;
	return S_OK;
}

STDMETHODIMP CDonutView::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	m_spDropTargetHelper->DragOver((LPPOINT)&pt, *pdwEffect);

	if (m_bExternalDrag)
		*pdwEffect |= DROPEFFECT_LINK;

	if (m_bDragAccept && m_bUseCustomDropTarget)
		*pdwEffect |= DROPEFFECT_LINK | DROPEFFECT_COPY;

	return S_OK;
}

STDMETHODIMP CDonutView::DragLeave()
{
	m_spDropTargetHelper->DragLeave();
	return S_OK;
}

STDMETHODIMP CDonutView::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	m_spDropTargetHelper->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);

	if (m_bDragAccept && m_bUseCustomDropTarget) {
		CSimpleArray<CString>	arrFiles;
		if ( MtlGetDropFileName(pDataObj, arrFiles) ) {	// �t�@�C����Drop���ꂽ
			unsigned  df   = DonutGetStdOpenFlag();
			unsigned  size = arrFiles.GetSize();
			//if (size == 1)
			//	df |= D_OPENFILE_NOCREATE;
			for (unsigned i = 0; i < size; ++i)
				DonutOpenFile(arrFiles[i], df);
			*pdwEffect = DROPEFFECT_COPY;
		} else {
			CString strText;
			if (   MtlGetHGlobalText(pDataObj, strText)
				|| MtlGetHGlobalText(pDataObj, strText, CF_SHELLURLW) )
			{
				::SendMessage(GetTopLevelParent(), WM_COMMAND_DIRECT, m_nDDCommand, (LPARAM) (LPCTSTR) strText);
				*pdwEffect = DROPEFFECT_NONE;
			}
		}
	} else if (MTL::MtlIsDataAvailable(pDataObj, CF_SHELLURLW)) {	// �^�u�Ȃǂ���
		CString strURL;
		MtlGetHGlobalText(pDataObj, strURL, CF_SHELLURLW);
		if (strURL.IsEmpty() == FALSE) {
			MTL::ParseInternetShortcutFile(strURL);	// �t�@�C���p�X->URL
			Navigate2(strURL);
		}

	} else if (m_bExternalDrag) {	// �O������
		CString strURL;
		MtlGetHGlobalText(pDataObj, strURL, ::RegisterClipboardFormat(CFSTR_FILENAME));
		if (strURL.IsEmpty() == FALSE) {
			MTL::ParseInternetShortcutFile(strURL);	// �t�@�C���p�X->URL
			Navigate2(strURL);
		}
	}
	return S_OK;
}


// IDispatch
STDMETHODIMP	CDonutView::Invoke(
		DISPID			dispidMember,
		REFIID			riid,
		LCID			lcid,
		WORD			wFlags,
		DISPPARAMS *	pdispparams,
		VARIANT *		pvarResult,
		EXCEPINFO * 	pexcepinfo,
		UINT *			puArgErr)
{
	if (!pvarResult) {
		return E_INVALIDARG;
	}

	if (dispidMember == DISPID_AMBIENT_DLCONTROL) {
		pvarResult->vt	 = VT_I4;
		pvarResult->lVal = m_dwDLControlFlags;
		return S_OK;
	}

	return DISP_E_MEMBERNOTFOUND;
}

// IDownloadManager

//--------------------------------------------
/// DL���J�n�����Ƃ��ɌĂ΂��
STDMETHODIMP CDonutView::Download(
	IMoniker* pmk,  
	IBindCtx* pbc,  
	DWORD	  dwBindVerb,  
	LONG	  grfBINDF,  
	BINDINFO* pBindInfo,  
	LPCOLESTR pszHeaders,  
	LPCOLESTR pszRedir,  
	UINT	  uiCP )
{
	if (CDLControlOption::s_bUseDLManager == false)
		return E_FAIL;

	if (::GetKeyState(VK_SHIFT) < 0)
		return E_FAIL;	// shift�������Ă���ƃf�t�H���g�ɔC����

	if (CDLOptions::bShowWindowOnDL)
		CDownloadManager::GetInstance()->OnShowDLManager(0, 0, NULL);

	CString strReferer = GetLocationURL();
	CCustomBindStatusCallBack* pCBSCB = CDownloadManager::GetInstance()->_CreateCustomBindStatusCallBack();
	IBindStatusCallback* pbscbPrev;
	HRESULT hr = ::RegisterBindStatusCallback(pbc, (IBindStatusCallback*)pCBSCB, &pbscbPrev, 0);
	if (FAILED(hr) && pbscbPrev) {
		hr = pbc->RevokeObjectParam(L"_BSCB_Holder_");
		if (SUCCEEDED(hr)) {
			TRACEIN(_T("Download() : _BSCB_Holder_"));
			// ���x�͐�������
			hr = ::RegisterBindStatusCallback(pbc, (IBindStatusCallback*)pCBSCB, NULL, 0);
			if (SUCCEEDED(hr)) {				
				pCBSCB->SetReferer(strReferer);
				pCBSCB->SetBSCB(pbscbPrev);
				pCBSCB->SetBindCtx(pbc);
			}
		}
	} else {
#if 0	//\\ ���O�ł�����Ⴄ�Ƃ܂������ۂ�
		// pbscbPrev��NULL�������Ƃ��̏ꍇ
		LPOLESTR strUrl;
		hr = pmk->GetDisplayName(pbc, NULL, &strUrl);
		if (SUCCEEDED(hr)) {
			DownloadStart(strUrl);
			::CoTaskMemFree(strUrl);
			return S_OK;
		}
#endif
		if (pszHeaders == nullptr) {
			TRACEIN(_T("Download() : Referer : %s"), (LPCTSTR)strReferer);
			pCBSCB->SetReferer(strReferer);
			IBindCtx* pBC;
			hr = ::CreateAsyncBindCtx(0, pCBSCB, NULL, &pBC);
			pbc = pBC;
		}
	}
	if (SUCCEEDED(hr)) {
		pCBSCB->SetThreadId(m_dwCurrentThreadId);
		GetParent().SendMessage(WM_INCREMENTTHREADREFCOUNT);
		CComPtr<IStream>	spStream;
		hr = pmk->BindToStorage(pbc, NULL, IID_IStream, (void**)&spStream);
	} else {
		delete pCBSCB;
	}

	return hr;
}



// UDT DGSTR
void CDonutView::OnMultiChg(WORD, WORD, HWND)
{
	_ToggleFlag(ID_DLCTL_BGSOUNDS, DLCTL_BGSOUNDS);
	_ToggleFlag(ID_DLCTL_VIDEOS  , DLCTL_VIDEOS  );
	_ToggleFlag(ID_DLCTL_DLIMAGES, DLCTL_DLIMAGES);
	_LightRefresh();
}


/// ���݂�DL�R���g���[���𔽓]������
void CDonutView::OnSecuChg(WORD, WORD, HWND)
{
	_ToggleFlag(ID_DLCTL_SCRIPTS		, DLCTL_NO_SCRIPTS			, TRUE);
	_ToggleFlag(ID_DLCTL_JAVA			, DLCTL_NO_JAVA 			, TRUE);
	_ToggleFlag(ID_DLCTL_DLACTIVEXCTLS	, DLCTL_NO_DLACTIVEXCTLS	, TRUE);
	_ToggleFlag(ID_DLCTL_RUNACTIVEXCTLS , DLCTL_NO_RUNACTIVEXCTLS	, TRUE);
	_LightRefresh();
}


/// �S����DL�R���g���[���̃I��/�I�t�؂�ւ�
void CDonutView::OnAllOnOff(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	switch (wID) {
	case ID_DLCTL_ON_OFF_MULTI:
		if ( ( GetDLControlFlags() & (DLCTL_DLIMAGES | DLCTL_BGSOUNDS | DLCTL_VIDEOS) ) == (DLCTL_DLIMAGES | DLCTL_BGSOUNDS | DLCTL_VIDEOS) )
			_RemoveFlag(DLCTL_BGSOUNDS | DLCTL_VIDEOS | DLCTL_DLIMAGES);
		else
			_AddFlag(DLCTL_BGSOUNDS | DLCTL_VIDEOS | DLCTL_DLIMAGES);
		break;

	case ID_DLCTL_ON_OFF_SECU:

		if ( ( ( GetDLControlFlags() & (DLCTL_NO_RUNACTIVEXCTLS | DLCTL_NO_DLACTIVEXCTLS | DLCTL_NO_SCRIPTS | DLCTL_NO_JAVA) ) == 0 ) ) {
			//�`�F�b�N�͑S�����Ă���
			_AddFlag(DLCTL_NO_SCRIPTS | DLCTL_NO_JAVA | DLCTL_NO_DLACTIVEXCTLS | DLCTL_NO_RUNACTIVEXCTLS);
		} else {
			//�`�F�b�N����Ă��Ȃ����ڂ�����
			_RemoveFlag(DLCTL_NO_SCRIPTS | DLCTL_NO_JAVA | DLCTL_NO_DLACTIVEXCTLS | DLCTL_NO_RUNACTIVEXCTLS);
		}

		break;
	}

	_LightRefresh();
}


/// �E�B���h�E�̏�����
int CDonutView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Let me initialize itself
	LRESULT lRet = DefWindowProc();

	m_dwCurrentThreadId = ::GetCurrentThreadId();
	try {
		HRESULT hr = QueryControl(IID_IWebBrowser2, (void**)&m_spBrowser);
		ATLASSERT(m_spBrowser);

		BOOL	bCheck	= GetRegisterAsDropTarget();
		hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                     IID_IDropTargetHelper, (LPVOID*)&m_spDropTargetHelper);
		ATLASSERT(hr == S_OK);

		// Set flat scrollbar style
		CComPtr<IAxWinHostWindow>	spAxHostWindow;
		hr	= QueryHost(&spAxHostWindow);	// �z�X�g���擾
		if (FAILED(hr))
			AtlThrow(hr);

		m_spAxAmbient	= spAxHostWindow;
		ATLASSERT(m_spAxAmbient);

		//�t���b�g�r���[�̉ۂ�K�p
		//hr = m_spAxAmbient->put_DocHostFlags(_bFlatView ? docHostUIFlagNotFlatView : docHostUIFlagDEFAULT); // UDT DGSTR ( added by dai
	  #if 1	//+++ xp�ȍ~�� theme�ɂ���ĕ��ʓI�ł��邱�Ƃ����҂��āA�t���b�g�X�N���[���o�[�ɂ��Ȃ�theme on�̐ݒ�ɂ���.
		//BOOL	bFlatView	= (m_dwDefaultExtendedStyleFlags & DVS_EX_FLATVIEW) != 0;	//+++ ? 1 : 0;
		BOOL	bFlatView	= (m_dwExStyle      & DVS_EX_FLATVIEW) != 0;	//+++ ? 1 : 0;

		unsigned flags      = DOCHOSTUIFLAG_NOT_FLATVIEW;
		if (bFlatView) {
			flags = _CheckOsVersion_XPLater() ? DOCHOSTUIFLAG_THEME_VIEW : DOCHOSTUIFLAG_NOT_FLATVIEW;
		}
	  #else
		BOOL	bFlatView	= (CDLControlOption::s_dwExtendedStyleFlags & DVS_EX_FLATVIEW) != 0;	//+++ ? 1 : 0;
		unsigned flags      = bFlatView ? DOCHOSTUIFLAG_FLATVIEW : DOCHOSTUIFLAG_NOT_FLATVIEW;
	  #endif
		hr	= m_spAxAmbient->put_DocHostFlags(flags);		//\\ flags��ݒ肷��
		if (FAILED(hr))
			AtlThrow(hr);

		// �O��UI��IDispatch�C���^�[�t�F�C�X��ݒ肷��
		CComQIPtr<IDocHostUIHandler>	   pDefaultHandler = spAxHostWindow;
		m_ExternalUIDispatch.SetDefaultUIHandler(pDefaultHandler);
		spAxHostWindow->SetExternalUIHandler(&m_ExternalUIDispatch);

		// �O��AmbientIDispatch�C���^�[�t�F�C�X��ݒ肷��
		CComQIPtr<IAxWinAmbientDispatchEx> pAmbient 	   = spAxHostWindow;
		m_spHost = spAxHostWindow;
		//m_ExternalAmbientDispatch.SetHostWindow(spAxWindow);
		pAmbient->SetAmbientDispatch((IDispatch*)this);//(&m_ExternalAmbientDispatch);

		// DLMnager�p��IServiceProvider��o�^����
		CComQIPtr<IObjectWithSite>	spObjectWithSite = spAxHostWindow;
		ATLASSERT(spObjectWithSite);
		hr = spObjectWithSite->SetSite((IUnknown*)(IServiceProvider*)this);

		_InitDLControlFlags();

		SetIeMenuNoCstm(CMenuOption::s_bNoCustomIEMenu);
		if (CMouseOption::s_nDragDropCommandID)
			SetOperateDragDrop(TRUE, CMouseOption::s_nDragDropCommandID);

	  #if 1	//+++
		if (m_dwExStyle == (DWORD)-1)
			m_dwExStyle = CDLControlOption::s_dwExtendedStyleFlags; //_dwViewStyle;
		m_UIChange.SetExStyle(m_dwExStyle);
	  #else
		ATLASSERT(m_dwExStyle == 0);
		m_dwExStyle = CDLControlOption::s_dwExtendedStyleFlags; //_dwViewStyle;
	  #endif
		_SetTimer();	// �ݒ肳��Ă���Ȃ玩���X�V���J�n

		RegisterDragDrop(m_hWnd, this);
	}
	catch (const CAtlException& e) {
		MessageBox(GetLastErrorString(e));
	}
	return 0;
}



void CDonutView::OnDestroy()
{
	RevokeDragDrop(m_hWnd);

	if (m_spBrowser) {
#if 0
		HRESULT hr = m_spBrowser->Stop();
		CComQIPtr<IOleInPlaceObject>	spInPlaceObject = m_spBrowser;
		ATLASSERT(spInPlaceObject);
		hr = spInPlaceObject->InPlaceDeactivate();
		ATLASSERT(SUCCEEDED(hr));
		spInPlaceObject.Release();

		// Set Client Site
		CComPtr<IOleObject>	spOleObject;
		hr = m_spBrowser->QueryInterface(IID_IOleObject, (void**)&spOleObject);
		ATLASSERT(SUCCEEDED(hr));
		hr = spOleObject->Close(OLECLOSE_NOSAVE);
#endif
		//spOleObject.Release();
		m_spBrowser.Release();
		m_spAxAmbient.Release();
		m_spHost.Release();
	}

	if (m_bUseCustomDropTarget)
		SetOperateDragDrop(FALSE, 0);

	KillTimer(AutoRefreshTimerID);

	DefWindowProc();
}



void CDonutView::OnMultiBgsounds(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	_ToggleFlag(ID_DLCTL_BGSOUNDS, DLCTL_BGSOUNDS);
	_LightRefresh();
}



void CDonutView::OnMultiVideos(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	_ToggleFlag(ID_DLCTL_VIDEOS, DLCTL_VIDEOS);
	_LightRefresh();
}



void CDonutView::OnMultiDlImages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	ATLTRACE2( atlTraceGeneral, 4, _T("CDonutView::OnMultiDlImages\n") );

	if ( _ToggleFlag(ID_DLCTL_DLIMAGES, DLCTL_DLIMAGES) )
		_LightRefresh();
}



void CDonutView::OnSecurRunactivexctls(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	_ToggleFlag(ID_DLCTL_RUNACTIVEXCTLS, DLCTL_NO_RUNACTIVEXCTLS, TRUE);
	_LightRefresh();
}



void CDonutView::OnSecurDlactivexctls(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if ( !_ToggleFlag(ID_DLCTL_DLACTIVEXCTLS, DLCTL_NO_DLACTIVEXCTLS, TRUE) )
		_LightRefresh();
}



void CDonutView::OnSecurScritps(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	_ToggleFlag(ID_DLCTL_SCRIPTS, DLCTL_NO_SCRIPTS, TRUE);
	_LightRefresh();
}



void CDonutView::OnSecurJava(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	_ToggleFlag(ID_DLCTL_JAVA, DLCTL_NO_JAVA, TRUE);
	_LightRefresh();
}


void	CDonutView::OnBlockMailto(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (m_dwExStyle & DVS_EX_BLOCK_MAILTO)
		m_dwExStyle &= ~DVS_EX_BLOCK_MAILTO;
	else
		m_dwExStyle |= DVS_EX_BLOCK_MAILTO;
	m_UIChange.SetExStyle(m_dwExStyle);
}

void	CDonutView::OnMouseGesture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (m_dwExStyle & DVS_EX_MOUSE_GESTURE)
		m_dwExStyle &= ~DVS_EX_MOUSE_GESTURE;
	else
		m_dwExStyle |= DVS_EX_MOUSE_GESTURE;
	m_UIChange.SetExStyle(m_dwExStyle);
}

void	CDonutView::OnMessageFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (m_dwExStyle & DVS_EX_MESSAGE_FILTER)
		m_dwExStyle &= ~DVS_EX_MESSAGE_FILTER;
	else
		m_dwExStyle |= DVS_EX_MESSAGE_FILTER;
	m_UIChange.SetExStyle(m_dwExStyle);
}

void	CDonutView::OnAutoRefreshNone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_dwAutoRefreshStyle = 0;
	KillTimer(AutoRefreshTimerID);
}

void	CDonutView::OnAutoRefresh15sec(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_dwAutoRefreshStyle  = 0;
	m_dwAutoRefreshStyle |= DVS_AUTOREFRESH_15SEC;
	KillTimer(AutoRefreshTimerID);
	_SetTimer();
}

void	CDonutView::OnAutoRefresh30sec(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_dwAutoRefreshStyle  = 0;
	m_dwAutoRefreshStyle |= DVS_AUTOREFRESH_30SEC;
	KillTimer(AutoRefreshTimerID);
	_SetTimer();
}

void	CDonutView::OnAutoRefresh1min(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_dwAutoRefreshStyle  = 0;
	m_dwAutoRefreshStyle |= DVS_AUTOREFRESH_1MIN;
	KillTimer(AutoRefreshTimerID);
	_SetTimer();
}

void	CDonutView::OnAutoRefresh2min(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_dwAutoRefreshStyle  = 0;
	m_dwAutoRefreshStyle |= DVS_AUTOREFRESH_2MIN;
	KillTimer(AutoRefreshTimerID);
	_SetTimer();
}

void	CDonutView::OnAutoRefresh5min(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_dwAutoRefreshStyle  = 0;
	m_dwAutoRefreshStyle |= DVS_AUTOREFRESH_5MIN;
	KillTimer(AutoRefreshTimerID);
	_SetTimer();
}

void	CDonutView::OnAutoRefreshUser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_dwAutoRefreshStyle  = 0;
	m_dwAutoRefreshStyle |= DVS_AUTOREFRESH_USER;
	KillTimer(AutoRefreshTimerID);
	_SetTimer();
}

/// �y�[�W���X�V����
void	CDonutView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == AutoRefreshTimerID) {
		::PostMessage(GetParent(), WM_COMMAND, ID_VIEW_REFRESH, 0);
	} else {
		SetMsgHandled(FALSE);
	}
}


// Implementation


/// DL�t���O���g�O������
bool CDonutView::_ToggleFlag(WORD wID, DWORD dwFlag, BOOL bReverse)
{
	bool  bRet			   = false;
	DWORD dwDLControlFlags = m_dwDLControlFlags;
	if (dwDLControlFlags & dwFlag) {
		dwDLControlFlags &= ~dwFlag;
	} else {
		dwDLControlFlags |= dwFlag;
		bRet			  = true;
	}

	PutDLControlFlags(dwDLControlFlags);
	return bRet;
}


/// DL�R���g���[���̕ύX��L���ɂ��邽�߂ɍX�V����
void CDonutView::_LightRefresh()
{
	if (::GetKeyState(VK_CONTROL) < 0)
		return;

	m_bLightRefresh = true;
	CString strURL = GetLocationURL();
	Navigate2(strURL);
}


/// DL�t���O��ǉ�����
void CDonutView::_AddFlag(DWORD dwFlag) 	//minit
{
	DWORD	dwDLControlFlags = m_dwDLControlFlags;
	dwDLControlFlags |= dwFlag;

	PutDLControlFlags(dwDLControlFlags);
}

/// DL�t���O����菜��
void CDonutView::_RemoveFlag(DWORD dwFlag)	//minit
{
	DWORD dwDLControlFlags = m_dwDLControlFlags;
	dwDLControlFlags &= ~dwFlag;

	PutDLControlFlags(dwDLControlFlags);
}

/// �����X�V�p�̃^�C�}�[���J�n����
void	CDonutView::_SetTimer()
{
	auto GetElapse = [this]() -> int {
		if (m_dwAutoRefreshStyle == 0)
			return -1;
		else if (m_dwAutoRefreshStyle & DVS_AUTOREFRESH_15SEC)
			return 15 * 1000;
		else if (m_dwAutoRefreshStyle & DVS_AUTOREFRESH_30SEC)
			return 30 * 1000;
		else if (m_dwAutoRefreshStyle & DVS_AUTOREFRESH_1MIN)
			return 1 * 60 * 1000;
		else if (m_dwAutoRefreshStyle & DVS_AUTOREFRESH_2MIN)
			return 2 * 60 * 1000;
		else if (m_dwAutoRefreshStyle & DVS_AUTOREFRESH_5MIN)
			return 5 * 60 * 1000;
		else if (m_dwAutoRefreshStyle & DVS_AUTOREFRESH_USER)
			return CMainOption::s_dwAutoRefreshTime * 1000;

		return -1;
	};

	int nElapse = GetElapse();
	if (nElapse != -1)
		SetTimer(AutoRefreshTimerID, nElapse);

	m_UIChange.SetAutoRefreshStyle(m_dwAutoRefreshStyle);
}
