/**
 *	@file	ChildFrame.cpp
 *	@brief	タブページ１つの処理.
 */

#include "stdafx.h"
#include "ChildFrame.h"
#include "include\cef.h"
#include "Include\cef_runnable.h"
#include "cef\util.h"
#include <fstream>
#include "MtlBrowser.h"
#include "MtlWin.h"
#include "DonutView.h"
#include "MultiThreadManager.h"
#include "ChildFrameCommandUIUpdater.h"
#include "option\MainOption.h"
#include "option\DLControlOption.h"
#include "option\MouseDialog.h"
#include "option\IgnoreURLsOption.h"
#include "option\CloseTitleOption.h"
#include "option\UrlSecurityOption.h"
#include "option\SearchPropertyPage.h"
#include "option\AddressBarPropertyPage.h"
#include "FaviconManager.h"
#include "ToolTipManager.h"
#include "PluginManager.h"
#include "Download\DownloadManager.h"
#include "MainFrame.h"
#include "AutoLogin\LoginData.h"


#define WM_DOMENUPOPUP	(WM_APP + 2)

DECLARE_REGISTERED_MESSAGE(GetMarshalIWebBrowserPtr)

#define MSG_WM_GETMARSHALIWEBBROWSERPTR()							   \
{															   \
	static UINT WM_GETMARSHALIWEBBROWSERPTR = GET_REGISTERED_MESSAGE(GetMarshalIWebBrowserPtr);	\
	if ( uMsg == WM_GETMARSHALIWEBBROWSERPTR ) { \
		SetMsgHandled(TRUE);								   \
		IStream*	pStream = nullptr;							\
		CoMarshalInterThreadInterfaceInStream(IID_IWebBrowser2, m_spBrowser, &pStream);	\
		lResult = (LRESULT)pStream;									   \
		if ( IsMsgHandled() )								   \
			return TRUE;									   \
	}														   \
}


#define IMPLEMENTCHILDRAME_REFCOUNTING(ClassName)            \
  public:                                           \
    int AddRef() { return refct_.AddRef(); }        \
    int Release() {                                 \
      int retval = refct_.Release();                \
      if(retval == 0)                               \
        delete m_pParentChild;                                \
      return retval;                                \
    }                                               \
    int GetRefCt() { return refct_.GetRefCt(); }    \
  private:                                          \
    CefRefCount refct_;

namespace {

	
static const LPCTSTR	g_lpszLight[] = {
	_T("<span id='unDonutHilight' style='color:black;background:#FFFF00'>"),
	_T("<span id='unDonutHilight' style='color:black;background:#00FFFF'>"),
	_T("<span id='unDonutHilight' style='color:black;background:#FF00FF'>"),
	_T("<span id='unDonutHilight' style='color:black;background:#7FFF00'>"),
	_T("<span id='unDonutHilight' style='color:black;background:#1F8FFF'>"),
};

static const int	g_LIGHTMAX	= _countof(g_lpszLight);


struct _Function_SelectEmpt {
	void operator ()(IHTMLDocument2 *pDocument)
	{
		CComPtr<IHTMLSelectionObject> spSelection;
		HRESULT 	hr = pDocument->get_selection(&spSelection);
		if (spSelection)
			spSelection->empty();
	}
};

struct _Function_Hilight2 {
	LPCTSTR 	m_lpszKeyWord;
	BOOL		m_bHilight;

	_Function_Hilight2(LPCTSTR lpszKeyWord, BOOL bHilight)
		: m_lpszKeyWord(lpszKeyWord), m_bHilight(bHilight)
	{	}

	void operator ()(IHTMLDocument2* pDocument)
	{
		if (m_bHilight) {
			if ( !FindHilight(pDocument) ) {
				MakeHilight(pDocument);
			}
		} else {
			RemoveHilight(pDocument);
		}
	}

	// ハイライト作成
	void MakeHilight(IHTMLDocument2* pDocument)
	{
	try {
		// キーワードの最初の一語を取得
		CString		strKeyWord = m_lpszKeyWord;

		//+++ 単語区切りを調整
		LPCTSTR		strExcept	= _T(" \t\"\r\n　");
		strKeyWord = _tcstok( strKeyWord.GetBuffer(0), strExcept );
		strKeyWord.TrimLeft(strExcept);
		strKeyWord.TrimRight(strExcept);

		int 	nLightIndex = 0;
		HRESULT hr;

		// キーワードが空になるまで繰り返し
		while ( !strKeyWord.IsEmpty() ) {
			CComPtr<IHTMLElement>		spHTMLElement;
			// <body>を取得
			hr = pDocument->get_body(&spHTMLElement);
			if (spHTMLElement == NULL) 
				break;

			CComQIPtr<IHTMLBodyElement>	spHTMLBody = spHTMLElement;
			if (spHTMLBody == NULL) 
				break;

			// テキストレンジを取得
			CComPtr<IHTMLTxtRange>	  spHTMLTxtRange;
			hr = spHTMLBody->createTextRange(&spHTMLTxtRange);
			if (!spHTMLTxtRange)
				AtlThrow(hr);			

			//+++ 最大キーワード数(無限ループ対策)
			static unsigned maxKeyword	= Misc::getIEMejourVersion() <= 6 ? 1000 : 10000;
			//+++ 無限ループ状態を強制終了させるため、ループをカウントする
			unsigned num = 0;

			// キーワードを検索
			CComBSTR		bstrText= strKeyWord;
			VARIANT_BOOL	vBool	= VARIANT_FALSE;
			while (spHTMLTxtRange->findText(bstrText, 1, 0, &vBool), vBool == VARIANT_TRUE) {
				// 現在選択しているHTMLテキストを取得
				CComBSTR	bstrTextNow;
				hr = spHTMLTxtRange->get_text(&bstrTextNow);
				if (FAILED(hr))
					AtlThrow(hr);

				// <span>を付加
				CComBSTR	bstrTextNew;
				bstrTextNew.Append(g_lpszLight[nLightIndex]);	// <span ～
				bstrTextNew.Append(bstrTextNow);
				bstrTextNew.Append(_T("</span>"));


				CComPtr<IHTMLElement> spParentElement;
				hr = spHTMLTxtRange->parentElement(&spParentElement);
				if (FAILED(hr))
					AtlThrow(hr);

				CComBSTR	bstrParentTag;
				hr = spParentElement->get_tagName(&bstrParentTag);
				if (FAILED(hr))
					AtlThrow(hr);

				if (   bstrParentTag != _T("SCRIPT")
					&& bstrParentTag != _T("NOSCRIPT")
					&& bstrParentTag != _T("TEXTAREA")
					&& bstrParentTag != _T("STYLE"))
				{
					hr = spHTMLTxtRange->pasteHTML(bstrTextNew);	// ハイライトする
					if (FAILED(hr))
						AtlThrow(hr);

					//+++ 通常のページでハイライト置換がこんなにもあることはないだろうで、無限ループ扱いでうちどめしとく
					if (++num > maxKeyword)		
						break;
				}
				spHTMLTxtRange->collapse(VARIANT_FALSE);	// Caretの位置を選択したテキストの一番下に
			}

			++nLightIndex;
			if (nLightIndex >= g_LIGHTMAX)
				nLightIndex = 0;

			// 次のキーワードに
			strKeyWord = _tcstok(NULL, strExcept);
			strKeyWord.TrimLeft(strExcept);
			strKeyWord.TrimRight(strExcept);
		}

	} catch (const CAtlException& e) {
			e;	// 例外を握りつぶす
	}	// try
	}

	// ハイライトを解除する
	void RemoveHilight(IHTMLDocument2* pDocument)
	{
		CComBSTR	hilightID(L"unDonutHilight");
		CComBSTR	hilightTag(L"SPAN");

		CComPtr<IHTMLElementCollection> pAll;

		if (SUCCEEDED( pDocument->get_all(&pAll) ) && pAll != NULL) {
			CComVariant 		id(L"unDonutHilight");
			CComPtr<IDispatch>	pDisp;
			CComVariant 		vIndex(0);
			pAll->item(id, vIndex, &pDisp);
			if (pDisp == NULL)
				return;

			CComPtr<IUnknown>	pUnk;

			if (SUCCEEDED( pAll->get__newEnum(&pUnk) ) && pUnk != NULL) {
				CComQIPtr<IEnumVARIANT> pEnumVariant = pUnk;

				if (pEnumVariant != NULL) {
					VARIANT  v;
					ULONG	 ul;
					CComBSTR bstrTagName;
					CComBSTR bstrID;
					CComBSTR bstrTmp;

					while (pEnumVariant->Next(1, &v, &ul) == S_OK) {
						CComQIPtr<IHTMLElement> pElement = v.pdispVal;
						VariantClear(&v);

						if (pElement != NULL) {
							bstrTagName.Empty();
							bstrID.Empty();
							pElement->get_tagName(&bstrTagName);
							pElement->get_id(&bstrID);

							if (bstrTagName == hilightTag && bstrID == hilightID) {
								bstrTmp.Empty();
								pElement->get_innerHTML(&bstrTmp);
								pElement->put_outerHTML(bstrTmp);
							}
						}
					}
				}
			}
		}
	}

	// ハイライトがすでにされているか確認する
	BOOL FindHilight(IHTMLDocument2* pDocument)
	{
		CComPtr<IHTMLElementCollection> 	pAll;

		if (SUCCEEDED(pDocument->get_all(&pAll)) && pAll != NULL) {
			CComVariant 		id(L"unDonutHilight");
			CComPtr<IDispatch>	pDisp;
			CComVariant 		vIndex(0);
			pAll->item(id, vIndex, &pDisp);
			if (pDisp != NULL) {
				return TRUE;
			}
		}
		return FALSE;
	}
};


void	HilightWords(CefRefPtr<CefBrowser> browser, CString strKeywords)
{
	if (strKeywords.IsEmpty())
		return ;

	LPCTSTR		strExcept	= _T(" \t\"\r\n　");
	strKeywords.TrimLeft(strExcept);
	strKeywords.TrimRight(strExcept);

	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	CefString url = frame->GetURL();

	std::wstring straddkeyword = L"var unDonutHighlightKeywords = ";
	straddkeyword += _T("\"");
	straddkeyword += strKeywords;
	straddkeyword += _T("\";\r\n");
	frame->ExecuteJavaScript(straddkeyword, url, 0);

	static CString strHilightScriptBody;
	if (strHilightScriptBody.IsEmpty()) {
		FILE* fp = _wfopen(Misc::GetExeDirectory() + _T("script/hilight.js"), L"r,ccs=UTF-8");
		if (fp == nullptr)
			return ;

		enum { kBuffSize = 512 };
		WCHAR	buff[kBuffSize];
		while (!feof(fp)) {
			size_t	readsize = fread(buff, sizeof(WCHAR), kBuffSize - 1, fp);
			buff[readsize] = L'\0';
			strHilightScriptBody += buff;
		}
		fclose(fp);
	}
	frame->ExecuteJavaScript((LPCTSTR)strHilightScriptBody, url, 0);

}

void	UnHilight(CefRefPtr<CefBrowser> browser)
{
	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	CefString url = frame->GetURL();

	std::string strscript =
		"for(q in hiliteElements) {"
		"	if(hiliteElements[q]) {"
		"		for(var i = 0; i < hiliteElements[q].length; i++) {"
		"			hiliteElements[q][i].outerHTML = hiliteElements[q][i].innerText;"
		"		}"
		"	}"
		"}";
	frame->ExecuteJavaScript(strscript, url, 0);
}


void	FindKeyword(CString strKeyword, bool bFindDown, CefRefPtr<CefBrowser> browser)
{
	std::wstring strPrepare = L"var unDonutKeyword = ";
	strPrepare += _T("\"");
	strPrepare += strKeyword;
	strPrepare += _T("\";\r\n");
	strPrepare += _T("var unDonutFindDown = ");
	strPrepare += bFindDown ? _T("true;") : _T("false;");

	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	CefString url = frame->GetURL();

	frame->ExecuteJavaScript(strPrepare, url, 0);

	static CString strScriptBody;
	if (strScriptBody.IsEmpty()) {
		FILE* fp = _wfopen(Misc::GetExeDirectory() + _T("script/findkeyword.js"), L"r,ccs=UTF-8");
		if (fp == nullptr)
			return ;

		enum { kBuffSize = 512 };
		WCHAR	buff[kBuffSize];
		while (!feof(fp)) {
			size_t	readsize = fread(buff, sizeof(WCHAR), kBuffSize - 1, fp);
			buff[readsize] = L'\0';
			strScriptBody += buff;
		}
		fclose(fp);
	}
	frame->ExecuteJavaScript((LPCTSTR)strScriptBody, url, 0);
}


void	AutoLogin(const NameValueMap& NameValue, const CheckboxMap& CheckBox, CefRefPtr<CefBrowser> browser)
{
	std::wstringstream	strstream;
	strstream << L"var unDonutNameValue = {";
	for (auto it = NameValue.cbegin(); it != NameValue.cend(); ++it) {
		strstream << (LPCTSTR)it->first << L":" << L"\"" << (LPCTSTR)it->second << L"\",";
	}
	strstream << L"};\r\n";
	
	strstream << L"var unDonutCheckBox = {";
	for (auto it = CheckBox.cbegin(); it != CheckBox.cend(); ++it) {
		strstream << (LPCTSTR)it->first << L":" << L"\"" << it->second << L"\",";
	}
	strstream << L"};\r\n";

	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	CefString url = frame->GetURL();

	frame->ExecuteJavaScript(strstream.str(), url, 0);

	static CString strScriptBody;
	if (strScriptBody.IsEmpty()) {
		FILE* fp = _wfopen(Misc::GetExeDirectory() + _T("script/autologin.js"), L"r,ccs=UTF-8");
		if (fp == nullptr)
			return ;

		enum { kBuffSize = 512 };
		WCHAR	buff[kBuffSize];
		while (!feof(fp)) {
			size_t	readsize = fread(buff, sizeof(WCHAR), kBuffSize - 1, fp);
			buff[readsize] = L'\0';
			strScriptBody += buff;
		}
		fclose(fp);
	}
	frame->ExecuteJavaScript((LPCTSTR)strScriptBody, url, 0);
}

};	// namespace

/////////////////////////////////////////////////////////////
// CChildFrame::Impl

class CChildFrame::Impl :
	public CWindowImpl<Impl>,
	public CMessageFilter,
	public CefClient,
	public CefLifeSpanHandler,
	public CefLoadHandler,
	public CefRequestHandler,
	public CefDisplayHandler,
	public CefFocusHandler,
	public CefKeyboardHandler,
	public CefMenuHandler,
	public CefPrintHandler
{
	friend class CChildFrame;

public:
	DECLARE_WND_CLASS_EX(_T("DonutChildFrame"), 0, 0/*COLOR_APPWORKSPACE*/)

	Impl(CChildFrame* pChild);
	~Impl();

	void	SetThreadRefCount(int* pCount) { m_pThreadRefCount = pCount; }

	DWORD	GetExStyle() const { return m_view.GetExStyle(); }
	void	SetExStyle(DWORD dwStyle);
	void	SetDLCtrl(DWORD dwDLCtrl) { m_view.PutDLControlFlags(dwDLCtrl); }
	void	SetMarshalDLCtrl(DWORD dwDLCtrl) { m_dwMarshalDLCtrlFlags = dwDLCtrl; }
	void	SetAutoRefreshStyle(DWORD dwAutoRefresh) { m_view.SetAutoRefreshStyle(dwAutoRefresh); }
	void	SaveSearchWordflg(bool bSave) { m_bSaveSearchWordflg = bSave; }
	void 	SetSearchWordAutoHilight(const CString& str, bool bAutoHilight);
	void	SetTravelLog(const vector<std::pair<CString, CString> >& fore, const vector<std::pair<CString, CString> >& back) {
		m_TravelLogFore = fore; m_TravelLogBack = back;
	}

	CString	GetLocationURL() { return m_UIChange.GetUIData().strLocationURL; }
	CString GetTitle() { return m_UIChange.GetUIData().strTitle; }

	void	GetSelectedText(function<void (const CString&)> callback);
	CString GetSelectedTextLine();

	bool	CanGoBack() { return m_Browser->CanGoBack(); }
	bool	CanGoForward() { return m_Browser->CanGoForward(); }
	void	GoBack(bool bNavigate = true);
	void	GoForward(bool bNavigate = true);

	// CefClient methods
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
		{ return this; }
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE
		{ return this; }
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE
		{ return this; }
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE
		{ return this; }
	virtual CefRefPtr<CefFocusHandler> GetFocusHandler() OVERRIDE
		{ return this; }
	virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE
		{ return this; }
	virtual CefRefPtr<CefMenuHandler> GetMenuHandler() OVERRIDE
		{ return this; }
	virtual CefRefPtr<CefPrintHandler> GetPrintHandler() OVERRIDE
		{ return this; }
	virtual CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE
		{ return NULL; }

	// CefLifeSpanHandler methods
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> parentBrowser,
								const CefPopupFeatures& popupFeatures,
								CefWindowInfo& windowInfo,
								const CefString& url,
								CefRefPtr<CefClient>& client,
								CefBrowserSettings& settings) OVERRIDE;
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

	// CefLoadHandler methods
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
							CefRefPtr<CefFrame> frame) OVERRIDE;
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
							CefRefPtr<CefFrame> frame,
							int httpStatusCode) OVERRIDE;
	virtual bool OnLoadError(CefRefPtr<CefBrowser> browser,
							CefRefPtr<CefFrame> frame,
							ErrorCode errorCode,
							const CefString& failedUrl,
							CefString& errorText) OVERRIDE;

	// CefRequestHandler methods
	virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefRequest> request,
                              NavType navType,
                              bool isRedirect) OVERRIDE;
	virtual bool OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefRequest> request,
                                    CefString& redirectUrl,
                                    CefRefPtr<CefStreamReader>& resourceStream,
                                    CefRefPtr<CefResponse> response,
                                    int loadFlags) OVERRIDE;
	virtual bool GetDownloadHandler(CefRefPtr<CefBrowser> browser,
                                  const CefString& mimeType,
                                  const CefString& fileName,
                                  int64 contentLength,
                                  CefRefPtr<CefDownloadHandler>& handler) OVERRIDE;
	virtual bool GetAuthCredentials(CefRefPtr<CefBrowser> browser,
									  bool isProxy,
									  const CefString& host,
									  int port,
									  const CefString& realm,
									  const CefString& scheme,
									  CefString& username,
									  CefString& password) OVERRIDE;

	// CefDisplayHandler methods
	virtual void OnNavStateChange(CefRefPtr<CefBrowser> browser,
								bool canGoBack,
								bool canGoForward) OVERRIDE;
	virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								const CefString& url) OVERRIDE;
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
								const CefString& title) OVERRIDE;
	virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser,
                               const CefString& value,
                               StatusType type) OVERRIDE;

	// CefKeyboardHandler methods
	virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser,
                          KeyEventType type,
                          int code,
                          int modifiers,
                          bool isSystemKey,
                          bool isAfterJavaScript) OVERRIDE;

	// CefMenuHandler methods
	virtual bool OnBeforeMenu(CefRefPtr<CefBrowser> browser,
                            const CefMenuInfo& menuInfo) OVERRIDE;

#if 0
	// Event handlers
	void	OnBeforeNavigate2(IDispatch*		pDisp,
						   const CString&		strURL,
						   DWORD				nFlags,
						   const CString&		strTargetFrameName,
						   CSimpleArray<BYTE>&	baPostedData,
						   const CString&		strHeaders,
						   bool&				bCancel );
	void	OnDownloadBegin() { }
	void	OnDownloadComplete();
	void	OnTitleChange(const CString& strTitle);
	void	OnProgressChange(long progress, long progressMax);
	void	OnStatusTextChange(const CString& strText);
	void	OnSetSecureLockIcon(long nSecureLockIcon);
	void	OnPrivacyImpactedStateChange(bool bPrivacyImpacted);
	void	OnStateConnecting();
	void	OnStateDownloading();
	void	OnStateCompleted();
	void	OnDocumentComplete(IDispatch *pDisp, const CString& strURL);
	void	OnCommandStateChange(long Command, bool bEnable);
	void	OnNewWindow2(IDispatch **ppDisp, bool& bCancel);
	void	OnNewWindow3(IDispatch **ppDisp, bool& bCancel, DWORD dwFlags, BSTR bstrUrlContext,  BSTR bstrUrl);
	void	OnWindowClosing(bool IsChildWindow, bool& bCancel);
#endif
	// PreTranslateMessage用
	BOOL	OnRButtonHook(CPoint ptDown);
	BOOL	OnMButtonHook(MSG* pMsg);
	BOOL	OnXButtonUp(WORD wKeys, WORD wButton);

	// Overrides
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnFinalMessage(HWND /*hWnd*/) { Release()/*delete m_pParentChild*/;/*this;*/ }
	void	searchEngines(const CString &strKeyWord);

	// Message map
	BEGIN_MSG_MAP( Impl )
		MSG_WM_CREATE		( OnCreate )
		MSG_WM_DESTROY		( OnDestroy )
		MSG_WM_CLOSE		( OnClose )
		MSG_WM_SIZE			( OnSize )
		MSG_WM_ERASEBKGND	( OnEraseBkgnd )
//		MSG_WM_GETMARSHALIWEBBROWSERPTR()
		USER_MSG_WM_CHILDFRAMEACTIVATE( OnChildFrameActivate )
		USER_MSG_WM_SET_CHILDFRAME( OnGetChildFrame )
		USER_MSG_WM_GETCHILDFRAMEDATA( OnGetChildFrameData )
		USER_MSG_WM_MENU_GOBACK 	( OnMenuGoBack		)
		USER_MSG_WM_MENU_GOFORWARD	( OnMenuGoForward	)
		USER_MSG_WM_GETSELECTEDTEXT	( OnGetSelectedText	)
		USER_MSG_WM_EXECUTEUSERJAVASCRIPT( OnExecuteUserJavascript )
		USER_MSG_WM_SETPAGEBITMAP	( OnSetPageBitmap )
		USER_MSG_WM_DRAWCHILDFRAMEPAGE( OnDrawChildFramePage )
		USER_MSG_WM_FAVICONCHANGE	( OnFaviconChange	)
		USER_MSG_WM_INCREMENTTHREADREFCOUNT()
		
		MESSAGE_HANDLER_EX( WM_DOMENUPOPUP, OnDoMenuPopup )

		// ファイル
		COMMAND_ID_HANDLER_EX( ID_EDIT_OPEN_SELECTED_REF, OnEditOpenSelectedRef 	)	// リンクを開く
		COMMAND_ID_HANDLER_EX( ID_EDIT_OPEN_SELECTED_TEXT,OnEditOpenSelectedText	)	// URLテキストを開く
		COMMAND_ID_HANDLER_EX( ID_FILE_PRINT		, OnFilePrint		)

		// 編集
		COMMAND_ID_HANDLER_EX( ID_EDIT_CUT				, OnEditCut 		)
		COMMAND_ID_HANDLER_EX( ID_EDIT_COPY 			, OnEditCopy		)
		COMMAND_ID_HANDLER_EX( ID_EDIT_PASTE			, OnEditPaste		)
		COMMAND_ID_HANDLER_EX( ID_EDIT_SELECT_ALL		, OnEditSelectAll	)
		COMMAND_ID_HANDLER_EX( ID_EDIT_FIND 			, OnEditFind				)
		COMMAND_ID_HANDLER_EX( ID_EDIT_FIND_MAX 		, OnEditFindMax 			)
		COMMAND_ID_HANDLER_EX( ID_TITLE_COPY			, OnTitleCopy				)
		COMMAND_ID_HANDLER_EX( ID_URL_COPY				, OnUrlCopy 				)
		COMMAND_ID_HANDLER_EX( ID_COPY_TITLEANDURL		, OnTitleAndUrlCopy 		)

		// 表示
		COMMAND_ID_HANDLER_EX( ID_SHOW_AUTOLOGINEDITDIALOG, OnShowAutoLoginEditDialog )
		COMMAND_ID_HANDLER_EX( ID_VIEW_SETFOCUS 		, OnViewSetFocus	)
		COMMAND_ID_HANDLER_EX( ID_VIEW_BACK 			, OnViewBack		)
		COMMAND_ID_HANDLER_EX( ID_VIEW_FORWARD			, OnViewForward 	)
		COMMAND_ID_HANDLER_EX( ID_VIEW_HOME 			, OnViewHome		)
		COMMAND_ID_HANDLER_EX( ID_VIEW_STOP				, OnViewStop		)
		COMMAND_ID_HANDLER_EX( ID_VIEW_UP				, OnViewUp			)
		COMMAND_ID_HANDLER_EX( ID_VIEW_GOTOROOT			, OnViewGoToRoot	)
		COMMAND_ID_HANDLER_EX( ID_VIEW_REFRESH			, OnViewRefresh 	)
		COMMAND_ID_HANDLER_EX( ID_VIEW_SOURCE			, OnViewSource		)
		COMMAND_ID_HANDLER_EX( ID_VIEW_DEVTOOL			, OnViewDevTool		)

		// ツール
		COMMAND_ID_HANDLER_EX( ID_EDIT_IGNORE			, OnAddClosePopupUrl		)
		COMMAND_ID_HANDLER_EX( ID_EDIT_CLOSE_TITLE		, OnAddClosePopupTitle		)

		// ウィンドウ
		COMMAND_ID_HANDLER_EX( ID_FILE_CLOSE			, OnFileClose				)

		COMMAND_RANGE_HANDLER_EX( ID_VIEW_BACK1   , ID_VIEW_BACK9	, OnViewBackX	)
		COMMAND_RANGE_HANDLER_EX( ID_VIEW_FORWARD1, ID_VIEW_FORWARD9, OnViewForwardX)
		
		// 検索バーから
		MSG_WM_USER_HILIGHT 		( OnHilight 		)
		MSG_WM_USER_FIND_KEYWORD	( OnFindKeyWord 	)
		// 独自ページ内検索バーから
		USER_MSG_WM_HILIGHTFROMFINDBAR( OnHilightFromFindBar )
		USER_MSG_WM_REMOVEHILIGHT( OnRemoveHilight )

		COMMAND_ID_HANDLER_EX( ID_HTMLZOOM_MENU			, OnHtmlZoomMenu			)
		// Special command
		COMMAND_ID_HANDLER_EX( ID_HTMLZOOM_ADD			, OnHtmlZoom				)
		COMMAND_ID_HANDLER_EX( ID_HTMLZOOM_SUB			, OnHtmlZoom				)
		COMMAND_ID_HANDLER_EX( ID_HTMLZOOM_100TOGLE		, OnHtmlZoom				)
		COMMAND_RANGE_HANDLER_EX( ID_HTMLZOOM_400 , ID_HTMLZOOM_050 , OnHtmlZoom    )
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_HOME 		, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_END		, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_PAGEUP	, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_PAGEDOWN	, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_UP		, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_DOWN 		, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SAVEIMAGE			, OnSaveImage		)

		//CHAIN_COMMANDS_MEMBER( m_view )
		//CHAIN_MSG_MAP( CWebBrowserCommandHandler<Impl> )

		if (uMsg == WM_COMMAND)
			GetTopLevelWindow().PostMessage(WM_COMMAND_FROM_CHILDFRAME, wParam, lParam);
	ALT_MSG_MAP(1)	// WebViewHost
		MSG_WM_MBUTTONUP	( OnViewMButtonUp	)
		MSG_WM_XBUTTONUP	( OnViewXButtonUp	)
		MSG_WM_RBUTTONDOWN	( OnViewRButtonDown	)
		MSG_WM_RBUTTONUP	( OnViewRButtonUp	)
		MSG_WM_CLOSE		( OnViewClose		)
	END_MSG_MAP()

	int		OnCreate(LPCREATESTRUCT /*lpCreateStruct*/);
	void	OnDestroy();
	void	OnClose();
	void	OnSize(UINT nType, CSize size);
	BOOL	OnEraseBkgnd(CDCHandle dc) { return 1; }
	void	OnChildFrameActivate(HWND hWndAct, HWND hWndDeact);	// タブの切り替えが通知される
	CChildFrame* OnGetChildFrame() { return m_pParentChild; }
	void	OnGetChildFrameData(ChildFrameDataOnClose* pData) { _CollectDataOnClose(*pData); }

	LRESULT OnDoMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnMenuGoBack(CMenuHandle menu);
	LRESULT OnMenuGoForward(CMenuHandle menu);
	void	OnGetSelectedText(LPCTSTR* ppStr);
	void	OnExecuteUserJavascript(CString* pstrScriptText);
	void	OnSetPageBitmap(HBITMAP* pBmp) { m_pPageBitmap = pBmp; }
	void	OnDrawChildFramePage(CDCHandle dc);
	void	OnFaviconChange(LPCTSTR strFaviconURL) { m_UIChange.SetFaviconURL(strFaviconURL); }

	// ファイル
	void 	OnEditOpenSelectedRef(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void 	OnEditOpenSelectedText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void	OnFilePrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	// 編集
	void	OnEditCut(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnEditCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnEditPaste(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnEditSelectAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnEditFind(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnEditFindMax(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) { /*_OnEditFindMax(0, 0, NULL);*/ }
	void	OnTitleCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnUrlCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnTitleAndUrlCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);	

	// 表示
	void	OnShowAutoLoginEditDialog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl);
	void	OnViewSetFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) { 
		m_Browser->SetFocus(true);/*m_view.SetFocus();*/ 
	}
	void	OnViewStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnViewUp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnViewGoToRoot(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnViewBack(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnViewForward(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnViewHome(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void	OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void	OnViewSource(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void	OnViewDevTool(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void	OnViewBackX(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnViewForwardX(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);

	// ツール
	void 	OnAddClosePopupUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void 	OnAddClosePopupTitle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	// ウィンドウ
	void 	OnFileClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	// 検索バーから
	LRESULT OnHilight(CString strKeyWord);
	LRESULT OnFindKeyWord(LPCTSTR lpszKeyWord, BOOL bFindDown, long Flags = 0);
	// 独自ページ内検索バーから
	int		OnHilightFromFindBar(LPCTSTR strText, bool bNoHighlight, bool bEraseOld, long Flags);
	void	OnRemoveHilight();

	void	OnHtmlZoomMenu(UINT uNotifyCode, int nID, CWindow wndCtl);
	// Specla command
	void	OnHtmlZoom(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnSpecialKeys(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnSaveImage(UINT uNotifyCode, int nID, CWindow wndCtl);

	// WebViewHost
	void OnViewMButtonUp(UINT nFlags, CPoint point);
	void OnViewXButtonUp(int fwButton, int dwKeys, CPoint ptPos);
	void OnViewRButtonDown(UINT nFlags, CPoint point);
	void OnViewRButtonUp(UINT nFlags, CPoint point);
	void OnViewClose();

private:
	DWORD	_GetInheritedDLCtrlFlags();
	DWORD	_GetInheritedExStyleFlags();
	bool	_CursorOnSelectedText();
	void	_InitTravelLog();
	void	_CollectDataOnClose(ChildFrameDataOnClose& data);
	void	_SetFavicon(CefRefPtr<CefFrame> frame);
	void	_HilightOnce(IDispatch *pDisp, LPCTSTR lpszKeyWord);
	BOOL 	_FindKeyWordOne(IHTMLDocument2* pDocument, const CString& strKeyword, BOOL bFindDown, long Flags = 0);

	// DOM visitors will be called after the associated path is loaded.
	void AddDOMVisitor(const std::string& path, CefRefPtr<CefDOMVisitor> visitor);
	CefRefPtr<CefDOMVisitor> GetDOMVisitor(const std::string& path);

	// Data members
	CChildFrame*	m_pParentChild;

	// The child browser window
	CefRefPtr<CefBrowser> m_Browser;

	// The child browser window handle
	CefWindowHandle m_BrowserHwnd;

	// Support for DOM visitors.
	typedef std::map<std::string, CefRefPtr<CefDOMVisitor> > DOMVisitorMap;
	DOMVisitorMap m_DOMVisitors;

	CContainedWindow	m_wndWebViewHost;
	CDonutView	m_view;
	CChildFrameUIStateChange	m_UIChange;
	int*	m_pThreadRefCount;
	bool	m_bNowActive;
	bool	m_bSaveSearchWordflg;
	CString	m_strSearchWord;
	CComBSTR	m_strBookmark;
	int 		m_nPainBookmark;
	CString		m_strOldKeyword;
	bool	m_bNowHilight;
	bool	m_bAutoHilight;
	CString m_strStatusText;
	bool	m_bExecutedNewWindow;	// for OnMButtonHook
	DWORD	m_dwMarshalDLCtrlFlags;
	bool	m_bInitTravelLog;
	vector<std::pair<CString, CString> >	m_TravelLogFore;
	vector<std::pair<CString, CString> >	m_TravelLogBack;

	int		m_nImgScl;					//+++ zoom,imgサイズ自動設定での設定値.
	int		m_nImgSclSav;				//+++ zoom,imgサイズの100%とのトグル切り替え用
	int		m_nImgSclSw;				//+++ zoom,imgサイズの100%とのトグル切り替え用
	bool	m_bImagePage;	// 画像ファイルを開いたかどうか
	CSize	m_ImageSize;

	bool	m_bReload;
	bool	m_bNowNavigate;
	bool	m_bClosing;
	//CBitmap	m_bmpPage;
	HBITMAP*	m_pPageBitmap;
	bool	m_bMClick;
	CefMenuInfo	m_menuInfo;
	CefBrowser* m_pmenuBrowser;
	CefFrame*	m_pmenuFrame;
	static bool s_bRButtonUpCancel;

	CString m_strReferer;

	// Include the default reference counting implementation.
	IMPLEMENTCHILDRAME_REFCOUNTING(Impl);
	// Include the default locking implementation.
	IMPLEMENT_LOCKING(Impl);
};

bool CChildFrame::Impl::s_bRButtonUpCancel = false;

#include "ChildFrame.inl"


/////////////////////////////////////////////////////////////
// CChildFrame

CChildFrame::CChildFrame() : pImpl(new Impl(this))
{
}

CChildFrame::~CChildFrame()
{
	delete pImpl;
}

/// スレッドを立ててCChildFrameのインスタンスを作る
void	CChildFrame::AsyncCreate(NewChildFrameData& data)
{
	CChildFrame*	pChild = new CChildFrame;
	if (data.dwDLCtrl == -1)
		data.dwDLCtrl	= CDLControlOption::s_dwDLControlFlags;
	if (data.dwExStyle == -1)
		data.dwExStyle	= CDLControlOption::s_dwExtendedStyleFlags;
	pChild->pImpl->m_view.SetDefaultFlags(data.dwDLCtrl, data.dwExStyle, data.dwAutoRefresh);
	pChild->pImpl->m_strReferer	= data.strReferer;

	MultiThreadManager::ExecuteChildFrameThread(pChild, &data);
}

void	CChildFrame::SetThreadRefCount(int* pCount)
{
	pImpl->SetThreadRefCount(pCount);
}

HWND	CChildFrame::CreateEx(HWND hWndParent, LPCTSTR strUrl)
{
	RECT rc;
	::GetClientRect(hWndParent, &rc);
	return pImpl->Create(hWndParent, rc, NULL, WS_CHILD /*| WS_VISIBLE*/, 0, 0U, (LPVOID)strUrl);
}

void	CChildFrame::Navigate2(LPCTSTR lpszURL)
{
	if (pImpl->m_Browser.get())
		pImpl->m_Browser->GetMainFrame()->LoadURL(lpszURL);
	//:::pImpl->Navigate2(lpszURL);
}

HWND	CChildFrame::GetHwnd() const
{
	return pImpl->m_hWnd;
}

DWORD	CChildFrame::GetExStyle() const
{
	return pImpl->GetExStyle();
}

void	CChildFrame::SetExStyle(DWORD dwStyle)
{
	pImpl->SetExStyle(dwStyle);
}

void	CChildFrame::SetDLCtrl(DWORD dwDLCtrl)
{
	pImpl->SetDLCtrl(dwDLCtrl);
}

void	CChildFrame::SetMarshalDLCtrl(DWORD dwDLCtrl)
{
	pImpl->SetMarshalDLCtrl(dwDLCtrl);
}

void	CChildFrame::SetAutoRefreshStyle(DWORD dwAutoRefresh)
{
	pImpl->SetAutoRefreshStyle(dwAutoRefresh);
}

void	CChildFrame::SaveSearchWordflg(bool bSave)
{
	pImpl->SaveSearchWordflg(bSave);
}

void	CChildFrame::SetSearchWordAutoHilight(const CString& str, bool bAutoHilight)
{
	pImpl->SetSearchWordAutoHilight(str, bAutoHilight);
}

void	CChildFrame::SetTravelLog(const vector<std::pair<CString, CString> >& fore, const vector<std::pair<CString, CString> >& back)
{
	pImpl->SetTravelLog(fore, back);
}


CComPtr<IWebBrowser2>	CChildFrame::GetIWebBrowser()
{
	return nullptr;
//	return pImpl->m_spBrowser;
}


CComPtr<IWebBrowser2>	CChildFrame::GetMarshalIWebBrowser()
{
	IStream* pStream = (IStream*)pImpl->SendMessage(GET_REGISTERED_MESSAGE(GetMarshalIWebBrowserPtr));
	if (pStream == nullptr)
		return nullptr;
	CComPtr<IWebBrowser2>	spBrowser;
	CoGetInterfaceAndReleaseStream(pStream, IID_IWebBrowser2, (void**)&spBrowser);
	return spBrowser;
}

CString	CChildFrame::GetLocationURL()
{
	return pImpl->GetLocationURL();
}

