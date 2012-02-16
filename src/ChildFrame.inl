/**
*	@file	ChildFrame.inl
*	@brief	CChildFrame::Impl の定義
*/

#pragma once


CChildFrame::Impl::Impl(CChildFrame* pChild) : 
	m_pParentChild(pChild), 
	m_pThreadRefCount(nullptr), 
	m_wndWebViewHost(this, 1),
	m_view(m_UIChange),
	m_bNowActive(false),
	m_bSaveSearchWordflg(false),
	m_nPainBookmark(0),
	m_bNowHilight(false),
	m_bAutoHilight(false),
	m_bExecutedNewWindow(false),
	m_dwMarshalDLCtrlFlags(0),
	m_bInitTravelLog(false),
	m_nImgScl(100),
	m_nImgSclSav(100),
	m_nImgSclSw(0),
	m_bImagePage(false),
	m_bReload(false),
	m_bNowNavigate(false),
	m_bClosing(false),
	m_pPageBitmap(nullptr),
	m_bMClick(false),
	m_menuInfo()
{
	AddRef();
}

CChildFrame::Impl::~Impl()
{
	if (m_menuInfo.frameUrl.str)
		cef_string_clear(&m_menuInfo.frameUrl);
	if (m_menuInfo.imageUrl.str)
		cef_string_clear(&m_menuInfo.imageUrl);
	if (m_menuInfo.linkUrl.str)
		cef_string_clear(&m_menuInfo.linkUrl);
	if (m_menuInfo.misspelledWord.str)
		cef_string_clear(&m_menuInfo.misspelledWord);
	if (m_menuInfo.pageUrl.str)
		cef_string_clear(&m_menuInfo.pageUrl);
	if (m_menuInfo.securityInfo.str)
		cef_string_clear(&m_menuInfo.securityInfo);
	if (m_menuInfo.selectionText.str)
		cef_string_clear(&m_menuInfo.selectionText);
}


void	CChildFrame::Impl::SetExStyle(DWORD dwStyle)
{
	m_view.SetExStyle(dwStyle);

	bool bNavigateLock = (dwStyle & DVS_EX_OPENNEWWIN) != 0;
	//m_MDITab.NavigateLockTab(m_hWnd, bNavigateLock);
	//m_bExPropLock					= (bNavigateLock && bExProp) /*? true : false*/;

}


void 	CChildFrame::Impl::SetSearchWordAutoHilight(const CString& str, bool bAutoHilight)
{
	m_strSearchWord = str;
	m_bAutoHilight	= bAutoHilight;
}

/// 選択範囲の文字列を取得(フレーム対応版)
void CChildFrame::Impl::GetSelectedText(function<void (const CString&)> callback)
{
	if (!m_Browser)
		return ;

	class GetSelectedTextDOMVisitor : public CefDOMVisitor
	{
	public:
		GetSelectedTextDOMVisitor(function<void (const CString&)>& func) : m_funcCallback(func) {}

		virtual void Visit(CefRefPtr<CefDOMDocument> document)
		{		
			CString strSelectedText = document->GetSelectionAsText().c_str();
			m_funcCallback(strSelectedText);
		}

	private:
		function<void (const CString&)>	m_funcCallback;

		IMPLEMENT_REFCOUNTING(GetSelectedTextDOMVisitor);
	};

	CefRefPtr<CefDOMVisitor> domvisitor(new GetSelectedTextDOMVisitor(callback));
	m_Browser->GetMainFrame()->VisitDOM(domvisitor);

#if 0
	CString 	strSelText;
	MtlForEachHTMLDocument2( m_spBrowser, [&strSelText](IHTMLDocument2 *pDocument) {
		CComPtr<IHTMLSelectionObject>		spSelection;
		HRESULT 	hr	= pDocument->get_selection(&spSelection);
		if ( SUCCEEDED( hr ) ) {
			CComPtr<IDispatch>				spDisp;
			hr	   = spSelection->createRange(&spDisp);
			if ( SUCCEEDED( hr ) ) {
				CComQIPtr<IHTMLTxtRange>	spTxtRange = spDisp;
				if (spTxtRange != NULL) {
					CComBSTR				bstrText;
					hr	   = spTxtRange->get_text(&bstrText);
					if (SUCCEEDED(hr) && !!bstrText)
						strSelText = bstrText;
				}
			}
		}
	});
#endif
#if 0	//:::
	if (strSelText.IsEmpty()) {
		//+++ 選択範囲がない場合、右クリックメニュー経由のことがあるので、それ対策を強引に...
		CCustomContextMenu* pMenu = CCustomContextMenu::GetInstance();
		if (pMenu && pMenu->GetContextMenuMode() == CONTEXT_MENU_ANCHOR && pMenu->GetAnchorUrl().IsEmpty() == 0) {
			//bstrText		= pMenu->GetAnchorUrl();
			//bstrLocationUrl = pMenu->GetAnchorUrl();
			strSelText = CString(pMenu->GetAnchorUrl());
		}
	}
#endif
//	return strSelText;
}



//+++ _OpenSelectedText()より分離.
CString CChildFrame::Impl::GetSelectedTextLine()
{
	CString str/* = GetSelectedText()*/;
	if (str.IsEmpty() == 0) {
		int 	n	= str.Find(_T("\r"));
		int 	n2	= str.Find(_T("\n"));
		if ((n < 0 || n > n2) && n2 >= 0)
			n = n2;
		if (n >= 0) {
			str = str.Left(n);
		}
	}
	return str;
}


void	CChildFrame::Impl::GoBack(bool bNavigate /*= true*/)
{
	if (CanGoBack() == false)
		return ;

	m_Browser->GoBack();
#if 0
	if (bNavigate) {
		CString strUrl = m_TravelLogBack[0].second;
		m_Browser->GetMainFrame()->LoadURL((LPCTSTR)strUrl);
	}
	std::pair<CString, CString>	willpair = m_TravelLogBack[0];
	m_TravelLogBack.erase(m_TravelLogBack.begin());

	m_TravelLogFore.insert(m_TravelLogFore.begin(), std::make_pair(GetTitle(), GetLocationURL()));

	m_UIChange.SetTitle(willpair.first);
	m_UIChange.SetLocationURL(willpair.second);
	m_UIChange.SetNavigateBack(CanGoBack());
	m_UIChange.SetNavigateForward(CanGoForward());
#endif
}

void	CChildFrame::Impl::GoForward(bool bNavigate /*= true*/)
{
	if (CanGoForward() == false)
		return ;

	m_Browser->GoForward();
#if 0
	if (bNavigate) {
		CString strUrl = m_TravelLogFore[0].second;
		m_Browser->GetMainFrame()->LoadURL((LPCTSTR)strUrl);
	}

	std::pair<CString, CString>	willpair;
	willpair = m_TravelLogFore[0];
	m_TravelLogFore.erase(m_TravelLogFore.begin());

	m_TravelLogBack.insert(m_TravelLogBack.begin(), std::make_pair(GetTitle(), GetLocationURL()));

	m_UIChange.SetTitle(willpair.first);
	m_UIChange.SetLocationURL(willpair.second);
	m_UIChange.SetNavigateBack(CanGoBack());
	m_UIChange.SetNavigateForward(CanGoForward());
#endif
}



// CefLifeSpanHandler methods

bool CChildFrame::Impl::OnBeforePopup(CefRefPtr<CefBrowser> parentBrowser,
                                  const CefPopupFeatures& popupFeatures,
                                  CefWindowInfo& windowInfo,
                                  const CefString& url,
                                  CefRefPtr<CefClient>& client,
                                  CefBrowserSettings& settings)
{
	REQUIRE_UI_THREAD();

	if (m_bMClick || (popupFeatures.x < 0 || popupFeatures.y < 0)) {
		DWORD	dwDLCtrl	= _GetInheritedDLCtrlFlags();
		DWORD	dwExStyle	= _GetInheritedExStyleFlags();
		if (CUrlSecurityOption::IsUndoSecurity(GetLocationURL())) {
			dwDLCtrl	= CDLControlOption::s_dwDLControlFlags;
			dwExStyle	= CDLControlOption::s_dwExtendedStyleFlags;
		}

		NewChildFrameData	data(GetParent());
		data.strURL	= url.c_str();
		data.strReferer	= GetLocationURL();
		data.bLink	= true;
		data.dwDLCtrl	= dwDLCtrl;
		data.dwExStyle	= dwExStyle;
		CString strSearchWord = m_strSearchWord;
		bool	bNowHilight = m_bNowHilight;
		data.funcCallAfterCreated	= [strSearchWord, bNowHilight](CChildFrame* pChild) {
			pChild->SetSearchWordAutoHilight(strSearchWord, bNowHilight);
		};
		CChildFrame::AsyncCreate(data);

		return true;
	}


	class ClientPopupHandler : public CefClient
	{
	public:

		// Include the default reference counting implementation.
		IMPLEMENT_REFCOUNTING(ClientPopupHandler);
	};
	client = new ClientPopupHandler;
#if 0
#ifdef TEST_REDIRECT_POPUP_URLS
  std::string urlStr = url;
  if(urlStr.find("chrome-devtools:") == std::string::npos) {
    // Show all popup windows excluding DevTools in the current window.
    windowInfo.m_dwStyle &= ~WS_VISIBLE;
    client = new ClientPopupHandler(m_Browser);
  }
#endif // TEST_REDIRECT_POPUP_URLS
#endif
  return false;
}


void CChildFrame::Impl::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
  REQUIRE_UI_THREAD();

  AutoLock lock_scope(this);
  if(!m_Browser.get())
  {
    // We need to keep the main child window, but not popup windows
    m_Browser = browser;
    m_BrowserHwnd = browser->GetWindowHandle();
	HWND hWndWebViewHost = ::GetWindow(m_BrowserHwnd, GW_CHILD);
	ATLASSERT( ::IsWindow(hWndWebViewHost) );
	m_wndWebViewHost.SubclassWindow(hWndWebViewHost);
  }
}

bool CChildFrame::Impl::DoClose(CefRefPtr<CefBrowser> browser)
{
  REQUIRE_UI_THREAD();

  if (m_BrowserHwnd == browser->GetWindowHandle()) {
    // Since the main window contains the browser window, we need to close
    // the parent window instead of the browser window.
    //CloseMainWindow();
	  PostMessage(WM_CLOSE);

    // Return true here so that we can skip closing the browser window 
    // in this pass. (It will be destroyed due to the call to close
    // the parent above.)
    return true;
  }

  // A popup browser window is not contained in another window, so we can let
  // these windows close by themselves.
  return false;
}

void CChildFrame::Impl::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
  REQUIRE_UI_THREAD();

  if(m_BrowserHwnd == browser->GetWindowHandle()) {
    // Free the browser pointer so that the browser can be destroyed
    m_Browser = NULL;
  }
}


// CefLoadHandler methods

void CChildFrame::Impl::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame)
{
  REQUIRE_UI_THREAD();

  if(m_BrowserHwnd == browser->GetWindowHandle() && frame->IsMain()) {
    // We've just started loading a page
	  GetTopLevelWindow().PostMessage(WM_CHILDFRAMEDOWNLOADING, (WPARAM)m_hWnd);

	// Faviconをクリアする
	m_UIChange.SetFaviconURL(_T(""));
	CFaviconManager::SetFavicon(m_hWnd, _T(""));

	m_strOldKeyword = _T("");
  }
}

void CChildFrame::Impl::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              int httpStatusCode)
{
  REQUIRE_UI_THREAD();

  if(m_BrowserHwnd == browser->GetWindowHandle() && frame->IsMain()) {
    // We've just finished loading a page
    GetTopLevelWindow().PostMessage(WM_CHILDFRAMECOMPLETE, (WPARAM)m_hWnd);

    CefRefPtr<CefDOMVisitor> visitor = GetDOMVisitor(frame->GetURL());
    if(visitor.get())
      frame->VisitDOM(visitor);

	_SetFavicon(frame);

	bool bHilight = m_bAutoHilight || CDonutSearchBar::GetInstance()->GetHilightSw();
	if (bHilight && m_strSearchWord.IsEmpty() == FALSE) {
		if (m_bNowActive)
			GetTopLevelWindow().SendMessage(WM_SETSEARCHTEXT, (WPARAM)(LPCTSTR)m_strSearchWord, true);
		CString strWords = m_strSearchWord;
		DeleteMinimumLengthWord(strWords);
		m_bNowHilight = true;
		HilightWords(browser, strWords);
		//m_Browser->Find(10, (LPCTSTR)strWords, true, false, false);
	}

	// 自動ログイン
	int nIndex = CLoginDataManager::Find(frame->GetURL().c_str());
	if (nIndex != -1) {
		NameValueMap*	pmap;
		CLoginDataManager::GetNameValueMap(nIndex, pmap);
		CheckboxMap*	pmapCheck;
		CLoginDataManager::GetCheckboxMap(nIndex, pmapCheck);
		AutoLogin(*pmap, *pmapCheck, browser);
	}
  }
}

bool CChildFrame::Impl::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& failedUrl,
                                CefString& errorText)
{
  REQUIRE_UI_THREAD();
#if 0
  if(errorCode == ERR_CACHE_MISS) {
    // Usually caused by navigating to a page with POST data via back or
    // forward buttons.
    errorText = "<html><head><title>Expired Form Data</title></head>"
                "<body><h1>Expired Form Data</h1>"
                "<h2>Your form request has expired. "
                "Click reload to re-submit the form data.</h2></body>"
                "</html>";
  } else {
    // All other messages.
    std::stringstream ss;
    ss <<       "<html><head><title>Load Failed</title></head>"
                "<body><h1>Load Failed</h1>"
                "<h2>Load of URL " << std::string(failedUrl) <<
                " failed with error code " << static_cast<int>(errorCode) <<
                ".</h2></body>"
                "</html>";
    errorText = ss.str();
  }
#endif
  return false;
}


// CefRequestHandler methods

bool CChildFrame::Impl::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefRequest> request,
                              NavType navType,
                              bool isRedirect)
{
	if (m_bMClick)
		return false;
#if 0
	//bool bLoadRequest = false;
	CefRequest::HeaderMap	map;
	request->GetHeaderMap(map);
	auto it = map.find(L"Accept-Language");
	if (it == map.end()) {
		map.insert(make_pair<CefString, CefString>(L"Accept-Language", L"ja,en-US;q=0.8,en;q=0.6"));
		bLoadRequest = true;
	}
#endif
	if (m_strReferer.GetLength() > 0) {
		CefRequest::HeaderMap	map;
		request->GetHeaderMap(map);
		auto it = map.find("Referer");
		if (it != map.end())
			it->second = (LPCTSTR)m_strReferer;
		else
			map.insert(make_pair<CefString, CefString>(L"Referer", (LPCTSTR)m_strReferer));
		m_strReferer.Empty();
		request->SetHeaderMap(map);
		frame->LoadRequest(request);
		return true;
	}

#if 0
	if (bLoadRequest) {
		request->SetHeaderMap(map);
		frame->LoadRequest(request);
		return true;
	}
#endif
	return false;
}

bool CChildFrame::Impl::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefRequest> request,
                                    CefString& redirectUrl,
                                    CefRefPtr<CefStreamReader>& resourceStream,
                                    CefRefPtr<CefResponse> response,
                                    int loadFlags)
{
	return false;
}

bool CChildFrame::Impl::GetDownloadHandler(CefRefPtr<CefBrowser> browser,
                                  const CefString& mimeType,
                                  const CefString& fileName,
                                  int64 contentLength,
                                  CefRefPtr<CefDownloadHandler>& handler)
{
	class CDownloadHandler : public CefDownloadHandler
	{
	public:
		CDownloadHandler(DLItem* pItem, HANDLE hFile) : m_pDLItem(pItem), m_hFile(hFile)
		{
			ATLASSERT(m_hFile != NULL && m_hFile != INVALID_HANDLE_VALUE);
		}

		virtual bool ReceivedData(void* data, int data_size)
		{
			if (m_pDLItem->bAbort) 
				return false;

			DWORD dwWritten = 0;
			m_pDLItem->nProgress += data_size;
			m_totalRead += data_size;
			ATLVERIFY(::WriteFile(m_hFile, data, data_size, &dwWritten, NULL));
			ATLASSERT(dwWritten == data_size);
			
			return true;
		}

		virtual void Complete()
		{
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;

			if (m_pDLItem->bAbort) {
				TRACEIN(_T("CDownloadHandler::Complete : DLは操作により中止されました : %s"), 
					m_pDLItem->strFileName);
				::DeleteFile(m_pDLItem->strIncompleteFilePath);
				::SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, 
					static_cast<LPCTSTR>(m_pDLItem->strIncompleteFilePath), nullptr);
				TRACEIN(_T("不完全ファイルを削除しました。: %s"), 
					(LPCTSTR)m_pDLItem->strIncompleteFilePath);
			} else {
				TRACEIN(_T("CDownloadHandler::Complete : 正常終了しました(%s)"), 
					(LPCTSTR)m_pDLItem->strFileName);
				::MoveFileEx(m_pDLItem->strIncompleteFilePath, m_pDLItem->strFilePath, MOVEFILE_REPLACE_EXISTING);
				/* エクスプローラーにファイルの変更通知 */
				::SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_PATH, 
					static_cast<LPCTSTR>(m_pDLItem->strIncompleteFilePath), 
					static_cast<LPCTSTR>(m_pDLItem->strFilePath));
			}

			/* DLリストから削除 */
			CDownloadManager::GetInstance()->RemoveFromDLList(m_pDLItem);
		}

	private:
		DLItem*	m_pDLItem;
		HANDLE	m_hFile;
		int64	m_totalRead;

		IMPLEMENT_REFCOUNTING(CDownloadHandler)
	};

	DLItem* pItem(new DLItem);
	ATLASSERT(contentLength < INT_MAX || contentLength == -1);
	pItem->nProgressMax	= (int)contentLength;
	pItem->strReferer	= browser->GetMainFrame()->GetURL().c_str();
	pItem->strFileName	= fileName.c_str();
	if (GetFileName(pItem) == false) {
		delete pItem;
		return false;
	}
	// フォルダが存在しなければ作成
	CString strDir = MtlGetDirectoryPath(pItem->strFilePath);
	if (::PathIsDirectory(strDir) == FALSE)
		::SHCreateDirectory(NULL, strDir);

	HANDLE hFile = ::CreateFile(pItem->strIncompleteFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		CString strError = _T("ファイルの作成に失敗しました\n");
		strError += GetLastErrorString();
		::MessageBox(NULL, strError, NULL, MB_OK | MB_ICONWARNING);
		delete pItem;
		return false;
	}

	/* DLリストに追加 */
	CDownloadManager::GetInstance()->AddToDLList(pItem);

	handler = new CDownloadHandler(pItem, hFile);
	return true;
}


bool CChildFrame::Impl::GetAuthCredentials(CefRefPtr<CefBrowser> browser,
									  bool isProxy,
									  const CefString& host,
									  int port,
									  const CefString& realm,
									  const CefString& scheme,
									  CefString& username,
									  CefString& password)
{
	class CGetAuthCredentialsDialog : public CDialogImpl<CGetAuthCredentialsDialog>
	{
	public:
		enum { IDD = IDD_GETAUTHCREDENTIALS };

		CGetAuthCredentialsDialog(const CString& host) : m_host(host) {
			m_AuthCredentialsPath = Misc::GetExeDirectory() + _T("AuthCredentials.ini");
		}

		CString GetUsername() const { return m_username; }
		CString GetPassword() const { return m_password; }

		// Message map
		BEGIN_MSG_MAP_EX( CGetAuthCredentialsDialog )
			MSG_WM_INITDIALOG( OnInitDialog )
			COMMAND_ID_HANDLER_EX(IDOK, OnOK)
			COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		END_MSG_MAP()

		BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
		{
			CenterWindow();

			SetForegroundWindow(m_hWnd);

			CStatic wndStatic = GetDlgItem(IDC_STATIC_DISCRIPTION);
			CString	strtext = MtlGetWindowText(wndStatic);
			strtext.Replace(_T("%s"), m_host);
			wndStatic.SetWindowText(strtext);
			
			CIniFileI	pr(m_AuthCredentialsPath, m_host);
			m_username = pr.GetString(_T("username"));
			m_password = pr.GetString(_T("password"));
			GetDlgItem(IDC_EDIT_USERNAME).SetWindowText(m_username);
			GetDlgItem(IDC_EDIT_PASSWORD).SetWindowText(m_password);
			if (m_username.GetLength() > 0)
				CButton(GetDlgItem(IDC_CHECK_SAVENAMEANDPASS)).SetCheck(BST_CHECKED);
			return TRUE;
		}

		void OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
		{
			m_username = MtlGetWindowText(GetDlgItem(IDC_EDIT_USERNAME));
			m_password = MtlGetWindowText(GetDlgItem(IDC_EDIT_PASSWORD));
			if (CButton(GetDlgItem(IDC_CHECK_SAVENAMEANDPASS)).GetCheck() == BST_CHECKED) {
				CIniFileO	pr(m_AuthCredentialsPath, m_host);
				pr.SetString(m_username, _T("username"));
				pr.SetString(m_password, _T("password"));
			}

			EndDialog(nID);
		}

		void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
		{
			EndDialog(nID);
		}

	private:
		CString	m_host;
		CString m_username;
		CString m_password;
		CString	m_AuthCredentialsPath;
	};

	CGetAuthCredentialsDialog	acdialog(host.c_str());
	if (acdialog.DoModal(NULL) == IDOK) {
		username = (LPCTSTR)acdialog.GetUsername();
		password = (LPCTSTR)acdialog.GetPassword();
		return true;
	}

	return false;
}

void CChildFrame::Impl::OnNavStateChange(CefRefPtr<CefBrowser> browser,
                                     bool canGoBack,
                                     bool canGoForward)
{
  REQUIRE_UI_THREAD();

  m_UIChange.SetNavigateBack(canGoBack);
	m_UIChange.SetNavigateForward(canGoForward);
}

void CChildFrame::Impl::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url)
{
  REQUIRE_UI_THREAD();

  if(m_BrowserHwnd == browser->GetWindowHandle() && frame->IsMain())
  {
	  m_UIChange.SetLocationURL(url.c_str());
  }
}

void CChildFrame::Impl::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title)
{
  REQUIRE_UI_THREAD();

  if (m_bClosing)
	  return ;

  // Set the frame window title bar
  CefWindowHandle hwnd = browser->GetWindowHandle();
  if(m_BrowserHwnd == hwnd)
  {
    // The frame window will be the parent of the browser window	
	SetWindowText(title.c_str());
	m_UIChange.SetTitle(title.c_str());
  }
}

void CChildFrame::Impl::OnStatusMessage(CefRefPtr<CefBrowser> browser,
                               const CefString& value,
                               StatusType type)
{
	if (m_bClosing)
		return ;

	if (value.c_str())
		m_UIChange.SetStatusText(value.c_str());
	else
		m_UIChange.SetStatusText(_T(""));
}

// CefKeyboardHandler methods

bool CChildFrame::Impl::OnKeyEvent(CefRefPtr<CefBrowser> browser,
                          KeyEventType type,
                          int code,
                          int modifiers,
                          bool isSystemKey,
                          bool isAfterJavaScript)
{
	if (type == KEYEVENT_RAWKEYDOWN) {
		if (modifiers & KEY_CTRL && !(modifiers & KEY_SHIFT) && !(modifiers & KEY_ALT)) {
			if (code == 0x46/*F*/) {

				class GetSelTextDOMVisitor : public CefDOMVisitor
				{
				public:
					GetSelTextDOMVisitor(CChildFrame::Impl* pThis) : m_pThis(pThis) {}

					virtual void Visit(CefRefPtr<CefDOMDocument> document)
					{
						CString strText;
						if (document->HasSelection())
							strText = document->GetSelectionAsText().c_str();

						m_pThis->GetTopLevelWindow().SendMessage(WM_OPENFINDBARWITHTEXT, (WPARAM)(LPCTSTR)strText);
					}

				private:
					CChildFrame::Impl*	m_pThis;

					IMPLEMENT_REFCOUNTING(GetSelTextDOMVisitor);
				};
				CefRefPtr<CefDOMVisitor> getSelDomVisitor(new GetSelTextDOMVisitor(this));
				browser->GetMainFrame()->VisitDOM(getSelDomVisitor);
				return true;
			}
		}
	}
	return false;
}


// CefMenuHandler methods

bool CChildFrame::Impl::OnBeforeMenu(CefRefPtr<CefBrowser> browser,
                            const CefMenuInfo& menuInfo)
{
	if (m_menuInfo.frameUrl.str)
		cef_string_clear(&m_menuInfo.frameUrl);
	if (m_menuInfo.imageUrl.str)
		cef_string_clear(&m_menuInfo.imageUrl);
	if (m_menuInfo.linkUrl.str)
		cef_string_clear(&m_menuInfo.linkUrl);
	if (m_menuInfo.misspelledWord.str)
		cef_string_clear(&m_menuInfo.misspelledWord);
	if (m_menuInfo.pageUrl.str)
		cef_string_clear(&m_menuInfo.pageUrl);
	if (m_menuInfo.securityInfo.str)
		cef_string_clear(&m_menuInfo.securityInfo);
	if (m_menuInfo.selectionText.str)
		cef_string_clear(&m_menuInfo.selectionText);

	m_menuInfo = menuInfo;
	if (menuInfo.frameUrl.str)
		cef_string_copy(menuInfo.frameUrl.str, menuInfo.frameUrl.length, &m_menuInfo.frameUrl);
	if (menuInfo.imageUrl.str)
		cef_string_copy(menuInfo.imageUrl.str, menuInfo.imageUrl.length, &m_menuInfo.imageUrl);
	if (menuInfo.linkUrl.str)
		cef_string_copy(menuInfo.linkUrl.str, menuInfo.linkUrl.length, &m_menuInfo.linkUrl);
	if (menuInfo.misspelledWord.str)
		cef_string_copy(menuInfo.misspelledWord.str, menuInfo.misspelledWord.length, &m_menuInfo.misspelledWord);
	if (menuInfo.pageUrl.str)
		cef_string_copy(menuInfo.pageUrl.str, menuInfo.pageUrl.length, &m_menuInfo.pageUrl);
	if (menuInfo.securityInfo.str)
		cef_string_copy(menuInfo.securityInfo.str, menuInfo.securityInfo.length, &m_menuInfo.securityInfo);
	if (menuInfo.selectionText.str)
		cef_string_copy(menuInfo.selectionText.str, menuInfo.selectionText.length, &m_menuInfo.selectionText);

	auto frame = browser->GetFocusedFrame();
	m_pmenuFrame = frame;
	m_pmenuFrame->AddRef();
	m_pmenuBrowser = browser;
	m_pmenuBrowser->AddRef();
	PostMessage(WM_DOMENUPOPUP);

	return true;
}



#if 0
// Event handlers
void	CChildFrame::Impl::OnBeforeNavigate2(IDispatch*		pDisp,
										const CString&		strURL,
										DWORD				nFlags,
										const CString&		strTargetFrameName,
										CSimpleArray<BYTE>&	baPostedData,
										const CString&		strHeaders,
										bool&				bCancel )
{
	bool bTopWindow = IsPageIWebBrowser(pDisp);

	{
		//プラグインイベント - ナビゲート前
		DEVTS_TAB_NAVIGATE stn;
		stn.nIndex			 = (int)GetTopLevelWindow().SendMessage(WM_GETTABINDEX, (WPARAM)m_hWnd);
		stn.lpstrURL		 = (LPCTSTR) strURL;
		stn.lpstrTargetFrame = (LPCTSTR) strTargetFrameName;
		int 	nRet		 = CPluginManager::ChainCast_PluginEvent(DEVT_TAB_BEFORENAVIGATE, stn.nIndex, (SPARAM) &stn);
		if (nRet == -1) {
			//ナビゲートキャンセル
			bCancel = true;
			return;
		} else if (nRet == -2) {
			//クローズ
			bCancel 	  = true;
			//m_bNewWindow2 = false;
			m_bClosing	  = true;
			PostMessage(WM_CLOSE);		// It's possible to post twice, but don't care.
			return;
		}
	}

	// mailto: 無効
	if (m_view.GetExStyle() & DVS_EX_BLOCK_MAILTO) {
		if (strURL.Left(7).CompareNoCase( _T("mailto:") ) == 0) {
			bCancel = true;
			return;
		}
	}

	// Navigate中かつjavescriptから始まるウィンドウはナビゲートしない
	if (m_bNowNavigate && 
		strURL.Left(15).CompareNoCase(_T("javascript:void")) == 0 || strURL.CompareNoCase(_T("javascript:;")) == 0) 
	{
		bCancel = true;
		return;
	}

	// ユーザースクリプトをインストールするかどうか
	if (strURL.Right(8).CompareNoCase(_T(".user.js")) == 0) {
		if (CUserDefinedJsOption::UserDefinedScriptInstall(strURL, m_hWnd)) {
			bCancel = true;

			CString strLocation = GetLocationURL();
			if (strLocation.IsEmpty())
				PostMessage(WM_CLOSE);
			return ;
		}
	}

	if (bTopWindow) {
		if (m_dwMarshalDLCtrlFlags) {
			m_view.PutDLControlFlags(m_dwMarshalDLCtrlFlags);
			m_dwMarshalDLCtrlFlags = 0;
		}

		if (m_view.m_bLightRefresh) {
			m_view.m_bLightRefresh = false;	// 手動でセキュリティを設定したので何もしない
		} else	{	
			// URL別セキュリティの設定
			DWORD exopts	= 0xFFFFFFFF;
			DWORD dlCtlFlg	= 0xFFFFFFFF;
			DWORD exstyle	= 0xFFFFFFFF;
			DWORD autoRefresh = 0xFFFFFFFF;
			DWORD dwExPropOpt = 8;
			if (CUrlSecurityOption::IsUndoSecurity(GetLocationURL())) {
				m_view.PutDLControlFlags(CDLControlOption::s_dwDLControlFlags);
				SetExStyle(CDLControlOption::s_dwExtendedStyleFlags);
			}
			if (CUrlSecurityOption::FindUrl( strURL, &exopts, &dwExPropOpt, 0 )) {
				CExProperty  ExProp(CDLControlOption::s_dwDLControlFlags, CDLControlOption::s_dwExtendedStyleFlags, 0, exopts, dwExPropOpt);
				dlCtlFlg	= ExProp.GetDLControlFlags();
				exstyle		= ExProp.GetExtendedStyleFlags();
				autoRefresh = ExProp.GetAutoRefreshFlag();
			}

			//+++ url別拡張プロパティの処理....
			//+++	戻る・進むでの拡張プロパティ情報の頁ごとの保存ができていないので、破綻する...
			//+++	が、バグっても反映されることのほうが意味ありそうなので利用
			if (CUrlSecurityOption::s_bValid) {
				if (exopts != 0xFFFFFFFF && CUrlSecurityOption::activePageToo()) {
					m_view.PutDLControlFlags( dlCtlFlg );
					m_view.SetAutoRefreshStyle( autoRefresh );
					SetExStyle( exstyle );	//+++メモ: マウス中ボタンクリックでの、リンク別タブ表示の場合、まだタブ位置未決定のため設定できない...
				}
			}
		}

		/* Faviconを白紙に設定 */
		//if (strURL.Left(11).CompareNoCase(_T("javascript:")) != 0) {
			//m_UIChange.SetLocationURL(strURL);
		//	_SetFavicon(strURL);
		//}

	}
	
	m_bNowNavigate = true;	// Navigate中である

	{
		//イベント発生 - ナビゲート
		DEVTS_TAB_NAVIGATE stn;
		stn.nIndex			 = (int)GetTopLevelWindow().SendMessage(WM_GETTABINDEX, (WPARAM)m_hWnd);
		stn.lpstrURL		 = (LPCTSTR) strURL;
		stn.lpstrTargetFrame = (LPCTSTR) strTargetFrameName;
		CPluginManager::BroadCast_PluginEvent(DEVT_TAB_NAVIGATE, stn.nIndex, (SPARAM) &stn);
	}
}

void	CChildFrame::Impl::OnDownloadComplete()
{
	m_bNowNavigate = false;	// Navigate終了
}

void	CChildFrame::Impl::OnTitleChange(const CString& strTitle)
{
	SetWindowText(strTitle);
	m_UIChange.SetTitle(strTitle);

	m_UIChange.SetLocationURL(GetLocationURL());
}

void	CChildFrame::Impl::OnProgressChange(long progress, long progressMax)
{
	m_UIChange.SetProgress(progress, progressMax);
}

void	CChildFrame::Impl::OnStatusTextChange(const CString& strText)
{
	m_strStatusText = strText;
	m_UIChange.SetStatusText(strText);
}

void	CChildFrame::Impl::OnSetSecureLockIcon(long nSecureLockIcon)
{
	m_UIChange.SetSecureLockIcon((int)nSecureLockIcon);
}

void	CChildFrame::Impl::OnPrivacyImpactedStateChange(bool bPrivacyImpacted)
{
	m_UIChange.SetPrivacyImpacted(bPrivacyImpacted);
}

void	CChildFrame::Impl::OnStateConnecting()
{
	READYSTATE	state;
	HRESULT hr = m_spBrowser->get_ReadyState(&state);
	if (hr == S_OK && state == READYSTATE_COMPLETE)
		return;

	//m_MDITab.SetConnecting(m_hWnd);
	GetTopLevelWindow().PostMessage(WM_CHILDFRAMECONNECTING, (WPARAM)m_hWnd);
	//m_bPrivacyImpacted = TRUE;	// ページ読み込み後もOnStateConnectingが呼ばれるページがあるので
								// クッキー制限アイコンが表示されないページがある
	//m_nSecureLockIcon = secureLockIconUnsecure;
}

void	CChildFrame::Impl::OnStateDownloading()
{
	READYSTATE	state;
	HRESULT hr = m_spBrowser->get_ReadyState(&state);
	if (hr == S_OK && state == READYSTATE_COMPLETE)
		return;

	//m_MDITab.SetDownloading(m_hWnd);
	GetTopLevelWindow().PostMessage(WM_CHILDFRAMEDOWNLOADING, (WPARAM)m_hWnd);
}

void	CChildFrame::Impl::OnStateCompleted()
{
	//m_MDITab.SetComplete(m_hWnd);
	GetTopLevelWindow().PostMessage(WM_CHILDFRAMECOMPLETE, (WPARAM)m_hWnd);

	if (m_bReload) {
		CString strUrl = GetLocationURL();
		CComQIPtr<IDispatch>	spDisp = m_spBrowser;
		OnDocumentComplete(spDisp, strUrl);
		m_bReload = false;
	}
}

/// documentが操作できるようになった
void	CChildFrame::Impl::OnDocumentComplete(IDispatch *pDisp, const CString& strURL)
{
	if ( IsPageIWebBrowser(pDisp) ) {
		// 自動リサイズの設定を初期化
		m_bImagePage	= false;
		m_nImgSclSw		= (CMainOption::s_nAutoImageResizeType == AUTO_IMAGE_RESIZE_FIRSTON);
		m_ImageSize.SetSize(0, 0);

		/* ページ内検索の情報をリセット */
		m_nPainBookmark = 0;
		m_strBookmark	= LPCOLESTR(NULL);

		_InitTravelLog();	// トラベルログを設定

		_AutoImageResize(true);	// 画像の自動リサイズ

		_SetFavicon(strURL);

		/* ユーザー定義Javascript */
		if (auto value = CUserDefinedJsOption::FindURL(strURL)) {
			for (auto it = value->cbegin(); it != value->cend(); ++it) {
				CComPtr<IDispatch>	spDisp;
				m_spBrowser->get_Document(&spDisp);
				CComQIPtr<IHTMLDocument2> spDoc = spDisp;
				if (spDoc) {
					CComPtr<IHTMLElement>	spScriptElm;
					spDoc->createElement(CComBSTR(L"script"), &spScriptElm);
					if (spScriptElm == nullptr)
						goto ADDJSFINISH;
 
					CComQIPtr<IHTMLScriptElement>	spScript = spScriptElm;
					spScript->put_type(CComBSTR(L"text/javascript"));
					spScript->put_text(*(*it));

					CComPtr<IHTMLElement>	spBodyElm;
					spDoc->get_body(&spBodyElm);
					CComQIPtr<IHTMLDOMNode>	spBodyNode = spBodyElm;
					if (spBodyNode == nullptr)
						goto ADDJSFINISH;
					CComQIPtr<IHTMLDOMNode>	spScriptNode = spScript;
					CComPtr<IHTMLDOMNode>	sptempNode;
					spBodyNode->appendChild(spScriptNode, &sptempNode);
				}
				
			}
		}
		ADDJSFINISH:
		/* ユーザー定義CSS */
		if (auto value = CUserDefinedCSSOption::FindURL(strURL)) {
			CComBSTR strCssPath = value.get();
			MtlForEachHTMLDocument2g(m_spBrowser, [strCssPath](IHTMLDocument2* pDoc) {
				CComPtr<IHTMLStyleSheet>	spStyleSheet;
				pDoc->createStyleSheet(strCssPath, -1, &spStyleSheet);
			});
		}

		bool bHilight = m_bAutoHilight || CDonutSearchBar::GetInstance()->GetHilightSw();
		if (bHilight && m_strSearchWord.IsEmpty() == FALSE) {
			if (m_bNowActive)
				GetTopLevelWindow().SendMessage(WM_SETSEARCHTEXT, (WPARAM)(LPCTSTR)m_strSearchWord, true);
			CString strWords = m_strSearchWord;
			DeleteMinimumLengthWord(strWords);
			m_bNowHilight = true;
			_HilightOnce(pDisp, strWords);
		}
	}

	{
		//プラグインイベント - ロード完了
		CComQIPtr<IWebBrowser2> 	pWB2 = pDisp;

		if (pWB2) {
			DEVTS_TAB_DOCUMENTCOMPLETE	dc;
			dc.lpstrURL   = (LPCTSTR) strURL;

			CComBSTR	bstrTitle;
			pWB2->get_LocationName(&bstrTitle);

			CString 	strTitle = bstrTitle;
			dc.lpstrTitle = (LPCTSTR) strTitle;
			dc.nIndex	  = (int)GetTopLevelWindow().SendMessage(WM_GETTABINDEX, (WPARAM)m_hWnd);
			dc.bMainDoc   = IsPageIWebBrowser(pDisp);
			dc.pDispBrowser	= pDisp;
			CPluginManager::BroadCast_PluginEvent(DEVT_TAB_DOCUMENTCOMPLETE, dc.nIndex, (DWORD_PTR) &dc);
		}
	}
}

void	CChildFrame::Impl::OnNewWindow2(IDispatch **ppDisp, bool& bCancel)
{
#if 1
	m_bExecutedNewWindow = true;

	CChildFrame*	pChild = new CChildFrame;
	pChild->pImpl->SetThreadRefCount(m_pThreadRefCount);
	DWORD	dwDLCtrl	= _GetInheritedDLCtrlFlags();
	DWORD	dwExStyle	= _GetInheritedExStyleFlags();
	if (CUrlSecurityOption::IsUndoSecurity(GetLocationURL())) {
		dwDLCtrl	= CDLControlOption::s_dwDLControlFlags;
		dwExStyle	= CDLControlOption::s_dwExtendedStyleFlags;
	}
	pChild->pImpl->m_view.SetDefaultFlags(dwDLCtrl, dwExStyle, 0);
	HWND hWnd = pChild->CreateEx(GetParent());
	ATLASSERT( ::IsWindow(hWnd) );

	pChild->pImpl->m_spBrowser->get_Application(ppDisp);
	ATLASSERT( ppDisp && *ppDisp );

	pChild->pImpl->SetSearchWordAutoHilight(m_strSearchWord, m_bNowHilight);

	GetTopLevelWindow().PostMessage(WM_TABCREATE, (WPARAM)pChild->pImpl->m_hWnd, TAB_LINK);
#endif
}

void	CChildFrame::Impl::OnNewWindow3(IDispatch **ppDisp, bool& bCancel, DWORD dwFlags, BSTR bstrUrlContext,  BSTR bstrUrl)
{
#if 0
	bCancel	= true;
	m_bExecutedNewWindow = true;

	NewChildFrameData	data(GetParent());
	data.strURL	= bstrUrl;
	DWORD	dwDLCtrl	= _GetInheritedDLCtrlFlags();
	DWORD	dwExStyle	= _GetInheritedExStyleFlags();
	if (CUrlSecurityOption::IsUndoSecurity(GetLocationURL())) {
		dwDLCtrl	= CDLControlOption::s_dwDLControlFlags;
		dwExStyle	= CDLControlOption::s_dwExtendedStyleFlags;
	}
	data.dwDLCtrl	= dwDLCtrl;
	data.dwExStyle	= dwExStyle;
	data.bLink	= true;
	CString strSearchWord = m_strSearchWord;
	bool	bNowHilight	= m_bNowHilight;
	data.funcCallAfterCreated	= [strSearchWord, bNowHilight](CChildFrame* pChild) {
		pChild->SetSearchWordAutoHilight(strSearchWord, bNowHilight);
	};
	CChildFrame::AsyncCreate(data);
#endif
}

void	CChildFrame::Impl::OnWindowClosing(bool IsChildWindow, bool& bCancel)
{
	PostMessage(WM_CLOSE);
}

#endif


BOOL	CChildFrame::Impl::OnMButtonHook(MSG* pMsg)
{
	CIniFileI	 pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
	DWORD	dwLinkOpenBtnM = pr.GetValue(_T("LinkOpenBtnM"), 0);
	if (dwLinkOpenBtnM == 0)	// ボタンが設定されていなかったら、終わり.
		return FALSE;

	//+++ メモ：デフォルトでは dwLinkOpenBtmM = IDM_FOLLOWLINKN
	// ATLASSERT(dwLinkOpenBtnM == IDM_FOLLOWLINKN);

	//+++ カーソルがリンクをさしていたらstatusBar用のメッセージがあることを利用.
	bool	bLink = false;
	if (   m_strStatusText.IsEmpty() == FALSE
		&& m_strStatusText != _T("ページが表示されました") 
		&& m_strStatusText != _T("完了") )			//+++ リンクのないページでは"ページが表示されました"というメッセージが設定されているので除外.
		bLink = true;

	::SendMessage(pMsg->hwnd, WM_LBUTTONDOWN, 0, pMsg->lParam); 				//+++ おまじない.
	//int 	nTabCnt = m_MDITab.GetItemCount();									//+++ リンクをクリックしたかどうかのチェック用.
	//pChild->m_bAllowNewWindow = true;
	m_bExecutedNewWindow = false;
	::SendMessage(pMsg->hwnd, WM_COMMAND, dwLinkOpenBtnM, 0);					//+++ リンクを新しい窓にひらく
	if (m_bExecutedNewWindow)
		return TRUE;
	//pChild->m_bAllowNewWindow = false;
	//int 	nTabCnt2 = m_MDITab.GetItemCount();
	//if (nTabCnt != nTabCnt2 || bLink)											//+++ リンクメッセージがあるか、タブが増えていたら、リンクをクリックしたとする.
	///	return TRUE;															//+++ trueを返して中ボタンクリックの処理をやったことにする.

	::SendMessage(pMsg->hwnd, WM_LBUTTONUP, 0, pMsg->lParam);					//+++ リンク以外をクリックした場合おまじないの左クリック押しを終了しておく.
	return FALSE;																//+++ falseを返すことで、IEコンポーネントにウィールクリックの処理を任せる.
}

static DWORD GetMouseButtonCommand(const MSG& msg)
{
	CString 	strKey;
	switch (msg.message) {
	case WM_LBUTTONUP:	strKey = _T("LButtonUp");					break;
	case WM_MBUTTONUP:	strKey = _T("MButtonUp");					break;
	case WM_XBUTTONUP:	strKey.Format(_T("XButtonUp%d"), GET_XBUTTON_WPARAM(msg.wParam)); break;
	case WM_MOUSEWHEEL:
		short zDelta = (short)HIWORD(msg.wParam);
		if (zDelta > 0)
			strKey = _T("WHEEL_UP");
		else
			strKey = _T("WHEEL_DOWN");
		break;
	}

	CIniFileI	pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
	return pr.GetValue(strKey, 0);;
}

static int PointDistance(const CPoint& pt1, const CPoint& pt2)
{
	return (int)sqrt( pow(float (pt1.x - pt2.x), 2.0f) + pow(float (pt1.y - pt2.y), 2.0f) );
}

BOOL CChildFrame::Impl::OnRButtonHook(CPoint ptDown)
{
	if ( !(m_view.GetExStyle() & DVS_EX_MOUSE_GESTURE) )
		return FALSE;

	m_wndWebViewHost.SetCapture();

	::ClientToScreen(m_hWnd, &ptDown);
	CPoint	ptLast = ptDown;

	bool bCursorOnSelectedText = CMouseOption::s_bUseRightDragSearch && _CursorOnSelectedText();
	HMODULE	hModule	= ::LoadLibrary(_T("ole32.dll"));
	CCursor cursor	= ::LoadCursor(hModule, MAKEINTRESOURCE(3));
	CString	strSearchEngine;
	CString strLastMark;
	CString strMove;
	DWORD	dwTime = 0;
	int		nDistance = 0;
	bool	bNoting = true;	// 何もしなかった
	MSG msg = { 0 };
	do {
		BOOL nRet = GetMessage(&msg, NULL, 0, 0);
		if (nRet == 0 || nRet == -1 || GetCapture() != m_wndWebViewHost)
			break;

		DWORD dwCommand = 0;
		switch (msg.message) {
		case WM_MOUSEWHEEL:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
			dwCommand = GetMouseButtonCommand(msg);
			break;

		case WM_MOUSEMOVE: {
			if (bNoting == false)	// 他のコマンドを実行済み
				break;
			CPoint	ptNow(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
			::ClientToScreen(msg.hwnd, &ptNow);
			if (bCursorOnSelectedText) {
				if (nDistance < 10) {
					nDistance = PointDistance(ptDown, ptNow);	// 距離を求める
					if (nDistance < 10)
						break;
				}
				SetCursor(cursor);	// カーソルを変更する

				CString strMark;
				if (CMouseOption::s_bUseRect) {
					int nAng  = (int) _GetAngle(ptDown, ptNow);	// 角度を求める
					if		  (nAng <  45 || nAng >  315) {
						strSearchEngine = CMouseOption::s_strREngine;	
						strMark = _T("[→] ");
					} else if (nAng >= 45 && nAng <= 135) {
						strSearchEngine = CMouseOption::s_strTEngine;	
						strMark = _T("[↑] ");
					} else if (nAng > 135 && nAng <  225) {
						strSearchEngine = CMouseOption::s_strLEngine;	
						strMark = _T("[←] ");
					} else if (nAng >= 225 && nAng <= 315) {
						strSearchEngine = CMouseOption::s_strBEngine;
						strMark = _T("[↓] ");
					}
				} else {
					strSearchEngine = CMouseOption::s_strCEngine;
				}
				if (strSearchEngine.IsEmpty() == FALSE) {
					CString strMsg;
					strMsg.Format(_T("検索 %s: %s"), strMark, strSearchEngine);
					m_UIChange.SetStatusText(strMsg);
				} else {
					m_UIChange.SetStatusText(_T(""));
				}
			} else {
				nDistance = PointDistance(ptLast, ptNow);	// 距離を求める
				if (nDistance < 10)
					break;
				
				CString strMark1;
				int nAng1  = (int) _GetAngle(ptLast, ptNow);	// 角度を求める
				if		(nAng1 <  45 || nAng1 >  315)
					strMark1 = _T("→");
				else if (nAng1 >= 45 && nAng1 <= 135)
					strMark1 = _T("↑");
				else if (nAng1 > 135 && nAng1 <  225)
					strMark1 = _T("←");
				else if (nAng1 >= 225 && nAng1 <= 315)
					strMark1 = _T("↓");

				if (strMark1 == strLastMark) {					// 同じ方向に動かして、かつ300ms以上経ったなら有効		
					DWORD dwTimeNow = ::GetTickCount();
					if ( (dwTimeNow - dwTime) > 300 ) {
						strLastMark = _T("");
						dwTime	 = dwTimeNow;
					}
				}
				if (strMark1 != strLastMark) {
					strMove	+= strMark1;	// 方向を追加
					strLastMark = strMark1;	

					CString strCmdName;
					CIniFileI	pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
					DWORD	dwCommand = pr.GetValue(strMove);
					if (dwCommand) {
						// 合致するコマンドがあれば表示
						CString strTemp;
						CToolTipManager::LoadToolTipText(dwCommand, strTemp);
						strCmdName.Format(_T("[ %s ]"), strTemp);
					}

					// ステータスバーに表示
					CString 	strMsg;
					strMsg.Format(_T("ジェスチャー : %s %s"), strMove, strCmdName);
					m_UIChange.SetStatusText(strMsg);
				}
				dwTime = ::GetTickCount();
			}
			ptLast = ptNow;
			break;
						   }

		case WM_LBUTTONDOWN:
			if (bCursorOnSelectedText && nDistance >= 10) {	// 右ボタンドラッグをキャンセルする
				s_bRButtonUpCancel = true;
				msg.message = WM_RBUTTONUP;
				m_UIChange.SetStatusText(_T(""));
			}
			break;
			
		case WM_RBUTTONUP:
		case 0x401:	// (WM_USRE + 1)	// なぜかメニューが出てしまう
			break;	// Dispatchさせない
		default:
			::DispatchMessage(&msg);
			break;
		}	// switch

		switch (dwCommand) {
		case 0:	
			break;
			
		case ID_FILE_CLOSE:
			s_bRButtonUpCancel = true;
			m_wndWebViewHost.PostMessage(WM_CLOSE);	// これだけじゃたぶんダメ
			msg.message = WM_RBUTTONUP;
			//::PostMessage(m_hWnd, WM_COMMAND, ID_FILE_CLOSE, 0);
			//::PostMessage(hWnd, WM_CLOSE, 0, 0);
			bNoting    = false;
			m_UIChange.SetStatusText(_T(""));
			break;

		case ID_GET_OUT:				// 退避
		case ID_VIEW_FULLSCREEN:		// 全体表示
		case ID_VIEW_UP:				// 上へ
		case ID_VIEW_BACK:				// 前に戻る
		case ID_VIEW_FORWARD:			// 次に進む
		case ID_VIEW_STOP_ALL:			// すべて中止
		case ID_VIEW_REFRESH_ALL:		// すべて更新
		case ID_WINDOW_CLOSE_ALL:		// すべて閉じる
		case ID_WINDOW_CLOSE_EXCEPT:	// これ以外閉じる
			::PostMessage(m_hWnd, WM_COMMAND, dwCommand, 0);
			bNoting    = false;
			m_UIChange.SetStatusText(_T(""));
			break;

		default:
			::PostMessage(m_hWnd, WM_COMMAND, dwCommand, 0);
			bNoting    = false;
			m_UIChange.SetStatusText(_T(""));
			break;
		}

	} while (msg.message != WM_RBUTTONUP);

	ReleaseCapture();
	::FreeLibrary(hModule);
	if (bCursorOnSelectedText) {
		SetCursor(::LoadCursor(NULL, IDC_ARROW));	// カーソルを元に戻す
		if (s_bRButtonUpCancel)
			return TRUE;
		if (strSearchEngine.IsEmpty() == FALSE) {	// 右ボタンドラッグ実行
			m_UIChange.SetStatusText(_T(""));
			GetSelectedText([strSearchEngine](const CString& str) {
				CDonutSearchBar::GetInstance()->SearchWebWithEngine(str, strSearchEngine);
			});
			bNoting = false;
		}
	}

	/* マウスジェスチャーコマンド実行 */
	if (bNoting) {
		ptLast.SetPoint(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		::ClientToScreen(msg.hwnd, &ptLast);

		m_UIChange.SetStatusText(_T(""));

		CIniFileI	pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
		DWORD dwCommand = pr.GetValue(strMove);
		if (dwCommand) {
			::SendMessage(m_hWnd, WM_COMMAND, dwCommand, 0);
			bNoting = false;
		} else if (dwCommand == -1)
			return TRUE;
	}

	if ( bNoting && strMove.IsEmpty() ) {	// 右クリックメニューを出す
		::ScreenToClient(m_hWnd, &ptLast);
		LPARAM lParam = MAKELONG(ptLast.x, ptLast.y);

		::SendMessage(m_wndWebViewHost, WM_RBUTTONUP, 0, lParam);
	}
	return !bNoting;
}

BOOL CChildFrame::Impl::OnXButtonUp(WORD wKeys, WORD wButton)
{
	CString 	strKey;
	switch (wButton) {
	case XBUTTON1: strKey = _T("Side1"); break;
	case XBUTTON2: strKey = _T("Side2"); break;
	}

	CIniFileI pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
	DWORD	dwCommand = pr.GetValue(strKey);
	if (dwCommand == 0)
		return FALSE;

	switch (dwCommand) {
	case ID_VIEW_BACK:		GoBack();		break;
	case ID_VIEW_FORWARD:	GoForward();	break;
	default:
		GetTopLevelWindow().SendMessage(WM_COMMAND, dwCommand);
	}
	return TRUE;
}

// Overrides

BOOL CChildFrame::Impl::PreTranslateMessage(MSG* pMsg)
{
	if (m_bNowActive == false)
		return FALSE;

	// ミドルクリック
	if ( pMsg->message == WM_MBUTTONDOWN && IsChild(pMsg->hwnd) && OnMButtonHook(pMsg) )
		return TRUE;

	// 右ドラッグキャンセル用
	if ( pMsg->message == WM_RBUTTONUP && s_bRButtonUpCancel ) {
		s_bRButtonUpCancel = false;
		return TRUE;
	}
	// マウスジェスチャーへ
	//if ( pMsg->message == WM_RBUTTONDOWN && OnRButtonHook(pMsg) )
	//	return TRUE;

	// サイドボタン
	if (pMsg->message == WM_XBUTTONUP) {
		if ( OnXButtonUp( GET_KEYSTATE_WPARAM(pMsg->wParam), GET_XBUTTON_WPARAM(pMsg->wParam)) )
			return TRUE;
	}

	// アクセラレータキー
	if (g_pMainWnd->m_hAccel != NULL && ::TranslateAccelerator(m_hWnd, g_pMainWnd->m_hAccel, pMsg))
			return TRUE;
	//return m_view.PreTranslateMessage(pMsg);
	return FALSE;
}

/// 選択中の文字列を検索(アドレスバーでCtrl+Enterしたときの検索エンジンが使われる)
void CChildFrame::Impl::searchEngines(const CString& strKeyWord)
{
	CString 	strSearchWord = strKeyWord;

	if (CAddressBarOption::s_bReplaceSpace)
		strSearchWord.Replace( _T("　"), _T(" ") );

	//_ReplaceCRLF(strSearchWord,CString(_T(" ")));
	//strSearchWord.Replace('\n',' ');
	//strSearchWord.Remove('\r');
	strSearchWord.Replace( _T("\r\n"), _T("") );

	CIniFileI	pr( g_szIniFileName, _T("AddressBar") );
	CString 		strEngin = pr.GetStringUW( _T("EnterCtrlEngin"), NULL, 256 );

	::SendMessage(GetTopLevelParent(), WM_SEARCH_WEB_SELTEXT, (WPARAM) (LPCTSTR) strSearchWord, (LPARAM) (LPCTSTR) strEngin);
}

static int _Pack(int hi, int low)
{
	if ( !( ( ('0' <= low && low <= '9') || ('A' <= low && low <= 'F') || ('a' <= low && low <= 'f') )
		  && ( ('0' <= hi && hi  <= '9') || ('A' <= hi	&& hi  <= 'F') || ('a' <= hi  && hi  <= 'f') ) ) )
		return 0;	//数値ではない

	int nlow = ('0' <= low && low <= '9') ? low - '0'
			 : ('A' <= low && low <= 'F') ? low - 'A' + 0xA
			 :								low - 'a' + 0xA ;
	int nhi  = ('0' <= hi  && hi  <= '9') ? hi	- '0'
			 : ('A' <= hi  && hi  <= 'F') ? hi	- 'A' + 0xA
			 :								hi	- 'a' + 0xA ;

	return (nhi << 4) + nlow;
}

// Message map

int		CChildFrame::Impl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	DefWindowProc();

	++(*m_pThreadRefCount);

	CChildFrameCommandUIUpdater::AddCommandUIMap(m_hWnd);
	m_UIChange.SetChildFrame(m_hWnd);

	// Browserウィンドウ作成
	RECT rc;
	GetClientRect(&rc);
	CefWindowInfo info;
	info.SetAsChild(m_hWnd, rc);
	CefBrowserSettings settings;
	CefString(&settings.default_encoding).FromWString(std::wstring(L"Shift_JIS"));
	settings.encoding_detector_enabled	= true;
	settings.shrink_standalone_images_to_fit	= true;

	CefString(&settings.standard_font_family).FromWString(std::wstring(L"メイリオ"));
	CefString(&settings.serif_font_family).FromWString(std::wstring(L"メイリオ"));
	CefString(&settings.sans_serif_font_family).FromWString(std::wstring(L"メイリオ"));
	CefString(&settings.cursive_font_family).FromWString(std::wstring(L"メイリオ"));
	CefString(&settings.fantasy_font_family).FromWString(std::wstring(L"メイリオ"));
	CefString(&settings.fixed_font_family).FromWString(std::wstring(L"Meiryo UI"));
	//settings.default_font_size	= 16;
	//settings.default_fixed_font_size	= 12;

	ATLVERIFY(CefBrowser::CreateBrowser(info, 
		static_cast<CefRefPtr<CefClient> >(this), 
		(LPCTSTR)lpCreateStruct->lpCreateParams, 
		settings));

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	return 0;
}

void	CChildFrame::Impl::OnDestroy()
{
	m_bClosing = true;

	if (m_wndWebViewHost.IsWindow())
		m_wndWebViewHost.UnsubclassWindow(TRUE);

	HWND wndBrowser = m_Browser->GetWindowHandle();
	::DestroyWindow(wndBrowser);

    CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop);
	pLoop->RemoveMessageFilter(this);
	
	--(*m_pThreadRefCount);
	if (*m_pThreadRefCount == 0) {
		TRACEIN(_T("ChildFreameスレッドの破棄"));
		PostQuitMessage(0);
	}

	CChildFrameCommandUIUpdater::RemoveCommandUIMap(m_hWnd);
}

void	CChildFrame::Impl::OnClose()
{
	SetMsgHandled(FALSE);

	if (m_Browser)
		m_Browser->ParentWindowWillClose();

	CWindow	wndMain = GetTopLevelWindow();

	ChildFrameDataOnClose*	pClosedTabData = new ChildFrameDataOnClose;
	_CollectDataOnClose(*pClosedTabData);
	wndMain.PostMessage(WM_ADDRECENTCLOSEDTAB, (WPARAM)pClosedTabData);

	{
		int nIndex = (int)GetTopLevelWindow().SendMessage(WM_GETTABINDEX, (WPARAM)m_hWnd);
		//プラグインイベント - クローズ
		CPluginManager::BroadCast_PluginEvent(DEVT_TAB_CLOSE, nIndex, 0);
	}

	wndMain.SendMessage(WM_TABDESTROY, (WPARAM)m_hWnd);
}

void	CChildFrame::Impl::OnSize(UINT nType, CSize size)
{
	DefWindowProc();

	if (m_BrowserHwnd) {
        HDWP hdwp = BeginDeferWindowPos(1);
        hdwp = ::DeferWindowPos(hdwp, m_BrowserHwnd, NULL,
          0, 0, size.cx, size.cy,
          SWP_NOZORDER);
        EndDeferWindowPos(hdwp);
	}
}

void	CChildFrame::Impl::OnChildFrameActivate(HWND hWndAct, HWND hWndDeact)
{
	if (hWndAct == m_hWnd) {
		m_bNowActive = true;
		if (MtlIsApplicationActive(m_hWnd) && m_Browser)
			m_Browser->SetFocus(true);

		if (CSearchBarOption::s_bSaveSearchWord) {
			CDonutSearchBar::GetInstance()->SetSearchStr(m_strSearchWord); //\\ 保存しておいた文字列を検索バーに戻す
			CDonutSearchBar::GetInstance()->ForceSetHilightBtnOn(m_bNowHilight);
		}

	} else if (hWndDeact == m_hWnd) {
		m_bNowActive = false;
		// _KillFocusToHTML
		HRESULT	hr = E_FAIL;
		//CComQIPtr<IOleInPlaceObject> spIOleInPlaceObject = m_spBrowser;
		//if(spIOleInPlaceObject) {
		//	hr = spIOleInPlaceObject->UIDeactivate(); // IEのUIを無効化
		//}

		if( m_bSaveSearchWordflg ){	//\\ 現在、検索バーにある文字列を取っておく
			CDonutSearchBar::GetInstance()->GetEditCtrl().GetWindowText(m_strSearchWord.GetBuffer(1024), 1024);
			m_strSearchWord.ReleaseBuffer();
		} else {
			m_bSaveSearchWordflg = true;
		}
	}
}

void func1()
{

}

LRESULT CChildFrame::Impl::OnDoMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const CefMenuInfo& menuInfo = m_menuInfo;

	struct menuItem {
		LPCTSTR	name;
		int		ID;
	};

	static menuItem	MenuPage[] = {
		{ _T("戻る(&B)")		, ID_VIEW_BACK	},
		{ _T("進む(&F)")		, ID_VIEW_FORWARD	},
		{ _T("再読み込み(&L)")	, ID_VIEW_REFRESH	},
		{ _T("-------------")	, 0	},
		{ _T("名前を付けて保存(&A)")	, ID_FILE_SAVE_AS	},
		{ _T("ページのソースを表示(&V)")	, ID_VIEW_SOURCE	},
		{ _T("ページの情報を表示(&I)")	, ID_FILE_PROPERTIES	},
	};
	static menuItem	MenuLink[] = {
		{ _T("新しいタブで開く(&T)")	,	ID_MENU_OPENLINKNEWTAB	},
		{ _T("名前を付けてリンク先を保存(&K)")	, ID_MENU_SAVELINKTO	},
		{ _T("リンク アドレスをコピー(&E)")	, ID_MENU_COPYLINKADDRESS	},
	};
	static menuItem	MenuImage[] = {
		{ _T("名前を付けて画像を保存(&V)")	, ID_MENU_SAVEIMAGEWITHDIALOG	},
		{ _T("画像 URL をコピー(&O)")	, ID_MENU_COPYIMAGEURL	},
		{ _T("新しいタブで画像を開く(&I)")	, ID_MENU_OPENIMAGENEWTAB	},
	};
	static menuItem	MenuFrame[] = {
		{ _T("フレームの再読み込み")	, ID_MENU_RELOADFRAMEPAGE	},
		{ _T("フレームのソースを表示")	, ID_MENU_SHOWFRAMEPAGESOURCE	},
		{ _T("フレーム情報を表示")		, ID_MENU_FRAMEPAGEPROPERTY	},
	};
	static menuItem	MenuEdit[] = {
		{ _T("取消(&U)\tCtrl+Z")	, ID_MENU_UNDO	},
		{ _T("やり直し(&R)\tCtrl+Shift+Z")	, ID_MENU_REDO	},
		{ _T("---------")	, 0	},
		{ _T("切り取り(&T)\tCtrl+X")	, ID_MENU_CUT	},
		{ _T("コピー(&C)\tCtrl+C")	, ID_MENU_COPY	},
		{ _T("貼り付け(&P)\tCtrl+Y")	, ID_MENU_PASTE	},
		{ _T("削除(&D)")	, ID_MENU_DELETE	},
		{ _T("---------")	, 0	},
		{ _T("すべて選択(&A)\tCtrl+A")	, ID_MENU_SELECT_ALL	},
	};
	static menuItem MenuSelection[] = {
		{ _T("コピー\tCtrl+C"),	ID_MENU_COPY	},
		{ _T("選択文字列を検索(&S)")	, ID_EDIT_FIND_MAX	},
	};
	
	auto funcGetFlags = [=](UINT nID) -> UINT {
		UINT nFlags = MF_ENABLED | MF_STRING | MF_BYPOSITION;		
		switch (nID) {
		case ID_VIEW_BACK:
			if (!(CanGoBack()))
				nFlags |= MF_GRAYED;
			break;
		case ID_VIEW_FORWARD:
			if (!(CanGoForward()))
				nFlags |= MF_GRAYED;
			break;			

		case ID_MENU_UNDO:
			if (!(menuInfo.editFlags & MENU_CAN_UNDO))
				nFlags |= MF_GRAYED;
			break;
		case ID_MENU_REDO:
			if (!(menuInfo.editFlags & MENU_CAN_REDO))
				nFlags |= MF_GRAYED;
			break;
		case ID_MENU_CUT:
			if (!(menuInfo.editFlags & MENU_CAN_CUT))
				nFlags |= MF_GRAYED;
			break;
		case ID_MENU_COPY:
			if (!(menuInfo.editFlags & MENU_CAN_COPY))
				nFlags |= MF_GRAYED;
			break;
		case ID_MENU_PASTE:
			if (!(menuInfo.editFlags & MENU_CAN_PASTE))
				nFlags |= MF_GRAYED;
			break;
		case ID_MENU_DELETE:
			if (!(menuInfo.editFlags & MENU_CAN_DELETE))
				nFlags |= MF_GRAYED;
			break;
		case ID_MENU_SELECT_ALL:
			if (!(menuInfo.editFlags & MENU_CAN_SELECT_ALL))
				nFlags |= MF_GRAYED;
			break;
		case 0:	nFlags |= MF_SEPARATOR;	break;
		}
		return nFlags;
	};

	CMenu	menu;
	menu.CreatePopupMenu();	
	CPoint pt(menuInfo.x, menuInfo.y);
	auto funcMenuInsert = [&](menuItem* pArr, int nSize) {
		if (menu.GetMenuItemCount() != 0) {
			menu.InsertMenu(-1, MF_BYPOSITION | MF_SEPARATOR, (UINT_PTR)0);
		}
		for (int i = 0; i < nSize; ++i) {
			menu.InsertMenu(-1, funcGetFlags(pArr[i].ID), pArr[i].ID, pArr[i].name);
		}
	};

	if (menuInfo.typeFlags & MENUTYPE_LINK) {
		funcMenuInsert(MenuLink, _countof(MenuLink));
	}
	if (menuInfo.typeFlags & MENUTYPE_IMAGE) {
		funcMenuInsert(MenuImage, _countof(MenuImage));		
	}
	if (menuInfo.typeFlags & MENUTYPE_EDITABLE) {
		funcMenuInsert(MenuEdit, _countof(MenuEdit));
	} else {
		if (menuInfo.typeFlags & MENUTYPE_SELECTION) {
			funcMenuInsert(MenuSelection, _countof(MenuSelection));
		} else if (menuInfo.typeFlags & MENUTYPE_PAGE && menu.GetMenuItemCount() == 0) {
			funcMenuInsert(MenuPage, _countof(MenuPage));	
		}
		if (menuInfo.typeFlags & MENUTYPE_FRAME && !(menuInfo.typeFlags & MENUTYPE_SELECTION)) {
			funcMenuInsert(MenuFrame, _countof(MenuFrame));	
		}
	}


	// ポップアップメニューを表示する
	if (menu.GetMenuItemCount() > 0) {
		//CefPostTask(TID_UI, NewCefRunnableFunction(&func1));
		int nCmd = menu.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
		NewChildFrameData data(GetParent());
		switch (nCmd) {
		case ID_VIEW_SOURCE:
			m_pmenuFrame->ViewSource();
			break;
		case ID_MENU_OPENLINKNEWTAB:			
			data.strURL = m_menuInfo.linkUrl.str;
			data.bLink	= true;
			CChildFrame::AsyncCreate(data);
			break;
		case ID_MENU_SAVELINKTO:
			CDownloadManager::GetInstance()->SetReferer(m_menuInfo.pageUrl.str);
			CDownloadManager::GetInstance()->DownloadStart(m_menuInfo.linkUrl.str, NULL, NULL, DLO_SHOWWINDOW);
			break;
		case ID_MENU_COPYLINKADDRESS:
			MtlSetClipboardText(m_menuInfo.linkUrl.str, m_hWnd);
			break;

		case ID_MENU_SAVEIMAGEWITHDIALOG:
			// 今は"名前を付けずに"になってるけど
			CDownloadManager::SetReferer(GetLocationURL());
			CDownloadManager::GetInstance()->DownloadStart(m_menuInfo.imageUrl.str, NULL, NULL, DLO_SAVEIMAGE);
			break;
		case ID_MENU_COPYIMAGEURL:
			MtlSetClipboardText(m_menuInfo.imageUrl.str, m_hWnd);
			break;
		case ID_MENU_OPENIMAGENEWTAB:
			data.strURL	= m_menuInfo.imageUrl.str;
			data.bLink	= true;
			CChildFrame::AsyncCreate(data);
			break;
		case ID_MENU_RELOADFRAMEPAGE:
			m_pmenuFrame->ExecuteJavaScript("document.location.reload();", "", 0);
			break;
		case ID_MENU_SHOWFRAMEPAGESOURCE:
			m_pmenuFrame->ViewSource();
			break;

		case ID_MENU_UNDO:
			m_pmenuFrame->Undo();
			break;
		case ID_MENU_REDO:
			m_pmenuFrame->Redo();
			break;
		case ID_MENU_CUT:
			m_pmenuFrame->Cut();
			break;
		case ID_MENU_COPY:
			m_pmenuFrame->Copy();
			break;
		case ID_MENU_PASTE:
			m_pmenuFrame->Paste();
			break;
		case ID_MENU_DELETE:
			m_pmenuFrame->Delete();
			break;
		case ID_MENU_SELECT_ALL:
			m_pmenuFrame->SelectAll();
			break;
		case ID_EDIT_FIND_MAX:
			searchEngines(menuInfo.selectionText.str);
			break;
		default:
			SendMessage(WM_COMMAND, nCmd);
			break;
		}
	}
	m_pmenuBrowser->Release();
	m_pmenuBrowser = nullptr;
	return 0;
}


static void SetTravelLogMenu(CMenuHandle menu, const vector<std::pair<CString, CString> >& vecLog, int BaseID)
{
	CMenuHandle submenu = menu.GetSubMenu(0);
	while (submenu.GetMenuItemCount())
		submenu.RemoveMenu(0, MF_BYPOSITION);

	int nIndex = 0;
	for (auto it = vecLog.cbegin(); it != vecLog.cend(); ++it) {
		submenu.AppendMenu(MF_STRING, BaseID + nIndex, it->first);
		++nIndex;
	}
}

LRESULT CChildFrame::Impl::OnMenuGoBack(CMenuHandle menu)
{
	SetTravelLogMenu(menu, m_TravelLogBack, ID_VIEW_BACK1);
	return 0;
}

LRESULT CChildFrame::Impl::OnMenuGoForward(CMenuHandle menu)
{
	SetTravelLogMenu(menu, m_TravelLogFore, ID_VIEW_FORWARD1); 
	return 0;
}

/// 現在選択されているテキストを返す
void	CChildFrame::Impl::OnGetSelectedText(LPCTSTR* ppStr)
{
	CString strSelectedText/* = GetSelectedText()*/;
	LPTSTR	strtemp = new TCHAR[strSelectedText.GetLength() + 1];
	::wcscpy_s(strtemp, strSelectedText.GetLength() + 1, strSelectedText);
	*ppStr = strtemp;
}

/// strScriptTextに書かれているスクリプトを実行する
void	CChildFrame::Impl::OnExecuteUserJavascript(CString* pstrScriptText)
{
	unique_ptr<CString>	pScriptText(std::move(pstrScriptText));
	if (m_Browser == nullptr)
		return ;

	auto spFrame = m_Browser->GetMainFrame();
	if (spFrame) {
		spFrame->ExecuteJavaScript(static_cast<LPCTSTR>(*pScriptText), "", 0);
	}
}

void	CChildFrame::Impl::OnDrawChildFramePage(CDCHandle dc)
{
#if 0
	if (m_bClosing)
		return ;
	CComQIPtr<IViewObject>	spViewObject = m_spBrowser;
	if (spViewObject == nullptr)
		return ;

	RECT rect;
	GetClientRect(&rect);
	spViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, dc, (RECTL*)&rect, (RECTL*)&rect, NULL, NULL);
#endif
}


/// コマンドから閉じるように要求されたので閉じる
void 	CChildFrame::Impl::OnFileClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	//+++ タブロック対策... OnClose側のだけで大丈夫のようだけれど、とりあえず.
	if (   //::: s_bMainframeClose == false											//+++ unDonut終了時以外で
		/*&&*/ _check_flag(m_view.GetExStyle(), DVS_EX_OPENNEWWIN)					//+++ ナビゲートロックのページで
		&& (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_NOCLOSE_NAVILOCK) )	//+++ ナビゲートロックのページを閉じない、の指定があれば
	{
		return; 																//+++ 閉じずに帰る
	}

	PostMessage(WM_CLOSE);
}

// 検索バーから

/// ハイライト
LRESULT CChildFrame::Impl::OnHilight(CString strKeyWord)
{
	m_bNowHilight = CDonutSearchBar::GetInstance()->GetHilightSw();
	if (m_bNowHilight) {
		m_strSearchWord = strKeyWord;
		DeleteMinimumLengthWord(strKeyWord);
		HilightWords(m_Browser, strKeyWord);
	} else {
		UnHilight(m_Browser);
	}
#if 0
	//if (m_bNowHilight != bHilightSw)
	{
		m_bNowHilight	= bHilightSw;
		if (bHilightSw) {
			m_strSearchWord = strKeyWord;
			DeleteMinimumLengthWord(strKeyWord);
		} else {
			m_strSearchWord.Empty();
		}
		MtlForEachHTMLDocument2g(m_spBrowser, _Function_SelectEmpt());
		MtlForEachHTMLDocument2g(m_spBrowser, _Function_Hilight2(strKeyWord, bHilightSw));
	}
#endif
	return TRUE;
}

/// ページ内検索
LRESULT CChildFrame::Impl::OnFindKeyWord(LPCTSTR lpszKeyWord, BOOL bFindDown, long Flags /*= 0*/)
{
	if (m_Browser == nullptr)
		return 0;

	if (Flags == 100) {	// 検索バーから
		CString	strKeyword = lpszKeyWord;
		LPCTSTR		strExcept	= _T(" \t\"\r\n　");
		strKeyword.TrimLeft(strExcept);
		strKeyword.TrimRight(strExcept);
		FindKeyword(strKeyword, bFindDown != 0, m_Browser);
		return 1;

	} else {
		bool bFindNext = true;
		if (m_strOldKeyword != lpszKeyWord) {
			m_Browser->StopFinding(true);
			bFindNext = false;
		}
		m_Browser->Find(0, lpszKeyWord, bFindDown != 0, false, bFindNext);
		m_strOldKeyword = lpszKeyWord;
		return 1;
	}
#if 0
	if (!m_spBrowser)
		return 0;

	CComPtr<IDispatch>	spDisp;
	HRESULT hr = m_spBrowser->get_Document(&spDisp);
	CComQIPtr<IHTMLDocument2>	spDocument = spDisp;	// htmlの取得
	if (!spDocument)
		return 0;

	CString strKeyword = lpszKeyWord;
	strKeyword.Replace(_T('ﾞ'), _T('゛'));

	// 検索
	BOOL	bSts = _FindKeyWordOne(spDocument, strKeyword, bFindDown, Flags);
	if (bSts)
		return TRUE;

	// フレームウィンドウの取得
	CComPtr<IHTMLFramesCollection2> 	spFrames;
	hr = spDocument->get_frames(&spFrames);
	// cf. Even if no frame, get_frames would succeed.
	if ( FAILED(hr) )
		return 0;

	// フレーム内ウィンドウの数を取得
	LONG	nCount	   = 0;
	hr = spFrames->get_length(&nCount);
	if ( FAILED(hr) )
		return 0;

	BOOL	bFindIt    = FALSE;
	if (bFindDown) {	// ページ内検索 - 下
		for (LONG ii = m_nPainBookmark; ii < nCount; ii++) {
			CComVariant 			varItem(ii);
			CComVariant 			varResult;

			// フレーム内のウィンドウを取得
			hr		= spFrames->item(&varItem, &varResult);
			if ( FAILED(hr) )
				continue;

			CComQIPtr<IHTMLWindow2> spWindow = varResult.pdispVal;
			if (!spWindow)
				continue;

			CComPtr<IHTMLDocument2> spDocumentFr;
			hr		= spWindow->get_document(&spDocumentFr);
			if ( FAILED(hr) ) {
				CComQIPtr<IServiceProvider>  spServiceProvider = spWindow;
				ATLASSERT(spServiceProvider);
				CComPtr<IWebBrowser2>	spBrowser;
				hr = spServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&spBrowser);
				if (!spBrowser)
					continue;
				CComPtr<IDispatch>	spDisp;
				hr = spBrowser->get_Document(&spDisp);
				if (!spDisp)
					continue;
				spDocumentFr = spDisp;
				if (!spDocument)
					continue;
			}

			// 検索
			bFindIt = _FindKeyWordOne(spDocumentFr, strKeyword, bFindDown, Flags);
			if (bFindIt) {
				m_nPainBookmark = ii;
				break;
			}
		}

		if (!bFindIt) {
			m_nPainBookmark = 0;
			m_strBookmark	= LPCOLESTR(NULL);
		}

	} else {			// ページ内検索 - 上
		if (m_nPainBookmark == 0 && !m_strBookmark)
			m_nPainBookmark = nCount - 1;

		for (LONG ii = m_nPainBookmark; ii >= 0; ii--) {
			CComVariant 			varItem(ii);
			CComVariant 			varResult;

			// ウィンドウの取得
			hr		= spFrames->item(&varItem, &varResult);
			if ( FAILED(hr) )
				continue;

			CComQIPtr<IHTMLWindow2> spWindow = varResult.pdispVal;
			if (!spWindow)
				continue;

			CComPtr<IHTMLDocument2> spDocumentFr;
			hr		= spWindow->get_document(&spDocumentFr);
			if ( FAILED(hr) ) {
				CComQIPtr<IServiceProvider>  spServiceProvider = spWindow;
				ATLASSERT(spServiceProvider);
				CComPtr<IWebBrowser2>	spBrowser;
				hr = spServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&spBrowser);
				if (!spBrowser)
					continue;
				CComPtr<IDispatch>	spDisp;
				hr = spBrowser->get_Document(&spDisp);
				if (!spDisp)
					continue;
				spDocumentFr = spDisp;
				if (!spDocument)
					continue;
			}

			// 検索
			bFindIt = _FindKeyWordOne(spDocumentFr, strKeyword, bFindDown, Flags);
			if (bFindIt) {
				m_nPainBookmark = ii;
				break;
			}
		}

		if (!bFindIt) {
			m_nPainBookmark = 0;
			m_strBookmark	= LPCOLESTR(NULL);
		}
	}

	return bFindIt;
#endif
}

static void _RemoveHighlight(IHTMLDocument3* pDoc3)
{
	CComQIPtr<IHTMLDocument2> spDoc2 = pDoc3;
	_MtlForEachHTMLDocument2(spDoc2, [&](IHTMLDocument2* pDoc) {
		CComPtr<IHTMLSelectionObject> spSelection;	/* テキスト選択を空にする */
		pDoc->get_selection(&spSelection);
		if (spSelection.p)
			spSelection->empty();

		vector<CComPtr<IHTMLElement> > vecElm;
		CComPtr<IHTMLElementCollection>	spCol;
		CComQIPtr<IHTMLDocument3> spDoc3 = pDoc;
		spDoc3->getElementsByTagName(CComBSTR(L"span"), &spCol);
		ForEachHtmlElement(spCol, [&vecElm](IDispatch* pDisp) -> bool {
			CComQIPtr<IHTMLElement>	spElm = pDisp;
			if (spElm.p) {
				CComBSTR strID;
				spElm->get_id(&strID);
				if (strID && strID == L"udfbh")
					vecElm.push_back(spElm);
			}
			return true;
		});
		for (auto it = vecElm.rbegin(); it != vecElm.rend(); ++it) {
			CComBSTR str;
			(*it)->get_innerText(&str);
			(*it)->put_outerHTML(str);
		}
	});
}

int		CChildFrame::Impl::OnHilightFromFindBar(LPCTSTR strText, bool bNoHighlight, bool bEraseOld, long Flags)
{
	if (strText[0] == _T('\0'))
		m_Browser->StopFinding(true);	// とりあえずの処理
	return 0;
#if 0
	CString strKeyword = strText;
	CComBSTR	strChar(L"Character");
	CComBSTR	strTextedit(L"Textedit");
	CComBSTR	strBackColor(L"BackColor");
	CComBSTR	strColor(L"greenyellow");

	CComPtr<IDispatch>	spDisp;
	m_spBrowser->get_Document(&spDisp);
	CComQIPtr<IHTMLDocument2>	spDoc = spDisp;
	if (spDoc == nullptr)
		return 0;

	/* 前のハイライト表示を消す */
	CComQIPtr<IHTMLDocument3>	spDoc3 = spDoc;
	if (bEraseOld)
		_RemoveHighlight(spDoc3);

	/* 単語をハイライトする */
	int nMatchCount = 0;
	if (strKeyword.GetLength() > 0) {
		_MtlForEachHTMLDocument2(spDoc, [&](IHTMLDocument2* pDoc) {
			CComPtr<IHTMLSelectionObject> spSelection;	/* テキスト選択を空にする */
			pDoc->get_selection(&spSelection);
			if (spSelection.p)
				spSelection->empty();

			CComPtr<IHTMLElement>	spElm;
			pDoc->get_body(&spElm);
			CComQIPtr<IHTMLBodyElement>	spBody = spElm;
			if (spBody.p == nullptr)
				return;

			CComPtr<IHTMLTxtRange>	spTxtRange;
			spBody->createTextRange(&spTxtRange);
			if (spTxtRange.p == nullptr)
				return;

			//long nMove;
			//spTxtRange->moveStart(strTextedit, -1, &nMove);
			//spTxtRange->moveEnd(strTextedit, 1, &nMove);
			VARIANT_BOOL	vResult;
			CComBSTR	strWord = strKeyword;
			while (spTxtRange->findText(strWord, 1, Flags, &vResult), vResult == VARIANT_TRUE) {
				CComPtr<IHTMLElement> spParentElement;
				spTxtRange->parentElement(&spParentElement);
				CComBSTR	bstrParentTag;
				spParentElement->get_tagName(&bstrParentTag);
				if (   bstrParentTag != _T("SCRIPT")
					&& bstrParentTag != _T("NOSCRIPT")
					&& bstrParentTag != _T("TEXTAREA")
					&& bstrParentTag != _T("STYLE")) 
				{
					if (bNoHighlight == false) {
						CComBSTR strInnerText;
						spTxtRange->get_text(&strInnerText);
						//VARIANT_BOOL	vRet;
						//spTxtRange->execCommand(strBackColor, VARIANT_FALSE, CComVariant(strColor), &vRet);
						CComBSTR strValue(L"<span id=\"udfbh\" style=\"color:black;background:greenyellow\">");//#00FFFF
						strValue += strInnerText;
						strValue += _T("</span>");
						spTxtRange->pasteHTML(strValue);
					}
					++nMatchCount;
				}
				spTxtRange->collapse(VARIANT_FALSE);
			}
		});
	}
	return nMatchCount;
#endif
}

void	CChildFrame::Impl::OnRemoveHilight()
{
#if 0
	CComPtr<IDispatch>	spDisp;
	m_spBrowser->get_Document(&spDisp);
	CComQIPtr<IHTMLDocument3> spDoc3 = spDisp;
	if (spDoc3)
		_RemoveHighlight(spDoc3);
#endif
}


/// ポップアップズームメニューを開く
void	CChildFrame::Impl::OnHtmlZoomMenu(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	::SetForegroundWindow(m_hWnd);

	CMenu	menu0;
	menu0.LoadMenu(IDR_ZOOM_MENU);
	ATLASSERT(menu0.IsMenu());
	CMenuHandle menu = menu0.GetSubMenu(0);
	ATLASSERT(menu.IsMenu());

	// ポップアップメニューを開く.
	POINT 	pt;
	::GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
}

void	CChildFrame::Impl::OnSpecialKeys(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (!m_Browser)
		return ;
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	int nCode = 0;
	switch (nID) {
	case ID_SPECIAL_HOME:		nCode = VK_HOME;	break;
	case ID_SPECIAL_END:		nCode = VK_END; 	break;
	case ID_SPECIAL_PAGEUP: 	nCode = VK_PRIOR;	break;
	case ID_SPECIAL_PAGEDOWN:	nCode = VK_NEXT;	break;
	case ID_SPECIAL_UP: 		nCode = VK_UP;		break;
	case ID_SPECIAL_DOWN:		nCode = VK_DOWN;	break;
	default:	
		ATLASSERT(FALSE);
		return ;
	}

	m_Browser->SendKeyEvent(KT_KEYDOWN, nCode, 0, false, false);
}

/// 画像を保存する
void	CChildFrame::Impl::OnSaveImage(UINT uNotifyCode, int nID, CWindow wndCtl)
{
#if 0
	auto funcGetUrl = [](CComQIPtr<IDispatch> spDisp) -> LPCTSTR {
		CComBSTR strUrl;
		CComQIPtr<IHTMLImgElement>	spImage =  spDisp;
		if (spImage != NULL) {
			spImage->get_href(&strUrl);
		} else {
			CComQIPtr<IHTMLInputElement>	spInput = spDisp;
			if (spInput != NULL) {
				spInput->get_src(&strUrl);
			} else {
				CComQIPtr<IHTMLAreaElement>	spArea = spDisp;
				if (spArea != NULL) {
					spArea->get_href(&strUrl);
				}
			}
		}
		return strUrl;
	};

	CString strUrl = m_view.GetAnchorURL();
	if (strUrl.IsEmpty() == FALSE) {
		CDownloadManager::GetInstance()->DownloadStart(strUrl, NULL, NULL, DLO_SAVEIMAGE);
	} else {
		CComPtr<IDispatch>	spDisp;
		m_spBrowser->get_Document(&spDisp);
		CComQIPtr<IHTMLDocument2>	spDocument = spDisp;
		ATLASSERT(spDocument);
		CPoint	pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		CComPtr<IHTMLElement>	spHitElement;
		spDocument->elementFromPoint(pt.x, pt.y, &spHitElement);
		// Hit先が画像の場合
		CComQIPtr<IHTMLImgElement>	spImg = spHitElement;
		if (spImg == NULL) {
			// フレームの可能性
			CComQIPtr<IHTMLFrameElement3>	spFrame = spHitElement;
			if (spFrame) {
				LONG x, y;
				spHitElement->get_offsetTop(&y);
				spHitElement->get_offsetLeft(&x);
				pt.Offset(-x, -y);
				CComPtr<IDispatch>	spFrameDisp;
				spFrame->get_contentDocument(&spFrameDisp);
				CComQIPtr<IHTMLDocument2>	spFrameDocument = spFrameDisp;
				if (spFrameDocument == NULL) 
					return;	// 別ドメイン
				ATLASSERT(spFrameDocument);
				CComPtr<IHTMLElement>	spHitFrameElement;
				spFrameDocument->elementFromPoint(pt.x, pt.y, &spHitFrameElement);
				spImg = spHitFrameElement;
			}
		}
		if (spImg) {
			CString strUrl = funcGetUrl(spImg.p);
			if (strUrl.IsEmpty() == FALSE)
				CDownloadManager::GetInstance()->DownloadStart(strUrl, NULL, NULL, DLO_SAVEIMAGE);
		}
	}
#endif
}

/// 拡大/縮小
void	CChildFrame::Impl::OnHtmlZoom(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (!m_Browser)
		return ;

	auto SetBodyStyleZoom	= [this](double addSub, double scl, bool bWheel) {
		{
			double zoomLevel = 0;
			if (addSub) {
				double NowZoomLevel = m_Browser->GetZoomLevel();
				zoomLevel = NowZoomLevel;
				zoomLevel += addSub;
			} else {
				zoomLevel = scl;
			}
			
			m_Browser->SetZoomLevel(zoomLevel);
		}
	};	// lamda

	switch (nID) {
	case ID_HTMLZOOM_ADD:	SetBodyStyleZoom(+0.5, 0, wndCtl == 0);	break;
	case ID_HTMLZOOM_SUB:	SetBodyStyleZoom(-0.5, 0, wndCtl == 0);	break;

	case ID_HTMLZOOM_100TOGLE:
		if (m_nImgScl == 100) {	// 100% なら元の拡大率に
			m_nImgScl	 = m_nImgSclSav;
			m_nImgSclSw	 = 1;
		} else {				// それ以外なら 100% に
			m_nImgSclSav = m_nImgScl;
			m_nImgScl    = 100;
			m_nImgSclSw	 = 0;
		}
		SetBodyStyleZoom(0, m_nImgScl, false);
		break;

	default: {
		ATLASSERT(ID_HTMLZOOM_400 <= nID && nID <= ID_HTMLZOOM_050);
		static const double scls[] = { 30, 10, 5, 2.5, 0, -3.5, -5 };
		int  n = nID - ID_HTMLZOOM_400;
		if (n < 0 || n > ID_HTMLZOOM_050 - ID_HTMLZOOM_400)
			return;
		SetBodyStyleZoom(0, scls[n], false);
		}
		break;
	}
}


/// 選択範囲のリンクを開く
void 	CChildFrame::Impl::OnEditOpenSelectedRef(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	class GetSelTextDOMVisitor : public CefDOMVisitor
	{
	public:
		GetSelTextDOMVisitor() {}

		virtual void Visit(CefRefPtr<CefDOMDocument> document)
		{
			if (document->HasSelection() == false)
				return ;

			std::wstring strText = document->GetSelectionAsMarkup().c_str();
			if (strText.empty())
				return;

			CSimpleArray<CString> arrUrls;
			CString strBaseURL = document->GetBaseURL().c_str();
			MtlCreateHrefUrlArray(arrUrls, strText.c_str(), strBaseURL);

			int size = arrUrls.GetSize();
			for (int i = 0; i < size; ++i) {
				DWORD dwOpenFlags = D_OPENFILE_CREATETAB;
				if (  i == (size - 1) 
					&& !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE_NEWWIN))
					dwOpenFlags |= D_OPENFILE_ACTIVATE;	// リンクを開くときアクティブにしないに従う
				DonutOpenFile(arrUrls[i], dwOpenFlags);
			}
		}

		IMPLEMENT_REFCOUNTING(GetSelTextDOMVisitor);
	};
	CefRefPtr<CefDOMVisitor> getSelDomVisitor(new GetSelTextDOMVisitor);
	m_Browser->GetMainFrame()->VisitDOM(getSelDomVisitor);

#if 0
	CSimpleArray<CString> arrUrls;
	bool bNoAddFromMenu = false;
	MtlForEachHTMLDocument2( m_spBrowser, [&arrUrls, &bNoAddFromMenu, this] (IHTMLDocument2 *pDocument) {
		CComPtr<IHTMLSelectionObject>	spSelection;
		HRESULT 	hr	= pDocument->get_selection(&spSelection);
		if ( FAILED(hr) )
			return;

		CComPtr<IDispatch>				spDisp;
		hr = spSelection->createRange(&spDisp);
		if ( FAILED(hr) )
			return;

		CComQIPtr<IHTMLTxtRange>		spTxtRange = spDisp;
		if (!spTxtRange)
			return;

		CComBSTR						bstrLocationUrl;
		CComBSTR						bstrText;
		hr = spTxtRange->get_htmlText(&bstrText);
		if (FAILED(hr) || !bstrText) {	//+++
			if (bNoAddFromMenu == false) {	// 右クリックメニューから取ってくる
				bNoAddFromMenu = true;
				CString strURL = m_view.GetAnchorURL();
				if (strURL.IsEmpty() == FALSE)
					arrUrls.Add(strURL);
			}
		} else {
			hr = pDocument->get_URL(&bstrLocationUrl);
			if ( FAILED(hr) )
				return;

			//BASEタグに対処する minit
			CComPtr<IHTMLElementCollection> spAllClct;
			hr = pDocument->get_all(&spAllClct);
			if ( SUCCEEDED(hr) ) {
				CComQIPtr<IHTMLElementCollection> spBaseClct;
				CComVariant 		val = _T("BASE");
				CComPtr<IDispatch>	spDisp;
				hr = spAllClct->tags(val, &spDisp);
				spBaseClct	= spDisp;
				if ( SUCCEEDED(hr) && spBaseClct ) {
					long	length;
					hr = spBaseClct->get_length(&length);
					if (length > 0) {
						CComPtr<IHTMLElement> spElem;
						CComVariant 		  val1( (int) 0 ), val2( (int) 0 );
						hr = spBaseClct->item(val1, val2, (IDispatch **) &spElem);
						if ( SUCCEEDED(hr) ) {
							CComPtr<IHTMLBaseElement> spBase;
							hr = spElem->QueryInterface(&spBase);
							if ( SUCCEEDED(hr) ) {
								CComBSTR bstrBaseUrl;
								hr = spBase->get_href(&bstrBaseUrl);
								if ( SUCCEEDED(hr) && !(!bstrBaseUrl) )
									bstrLocationUrl = bstrBaseUrl;
							}
						}
					}
				}
			}

			MtlCreateHrefUrlArray( arrUrls, WTL::CString(bstrText), WTL::CString(bstrLocationUrl) );
			if (arrUrls.GetSize() > 0)
				bNoAddFromMenu = true;	// 選択範囲からリンクが見つかったので
		}
	});
#if 1	//:::
	//m_MDITab.SetLinkState(LINKSTATE_B_ON);
	int size = arrUrls.GetSize();
	for (int i = 0; i < size; ++i) {
		DWORD dwOpenFlags = D_OPENFILE_CREATETAB;
		if (  i == (size - 1) 
			&& !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE_NEWWIN))
			dwOpenFlags |= D_OPENFILE_ACTIVATE;	// リンクを開くときアクティブにしないに従う
		DonutOpenFile(arrUrls[i], dwOpenFlags);
	}
	//m_MDITab.SetLinkState(LINKSTATE_OFF);
#endif
#endif
}

/// URLテキストを開く
void 	CChildFrame::Impl::OnEditOpenSelectedText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	class GetSelTextDOMVisitor : public CefDOMVisitor
	{
	public:
		GetSelTextDOMVisitor() {}

		virtual void Visit(CefRefPtr<CefDOMDocument> document)
		{
			if (document->HasSelection() == false)
				return ;

			std::wstring strText = document->GetSelectionAsText().c_str();
			if (strText.empty())
				return;

			std::vector<CString>	vecUrls;
			std::wregex rx(L"(http(?:s|)://|)((?:[a-zA-Z0-9_\\-]+\\.)+\\w+(?::\\d+|)(?:/[a-zA-Z0-9./_\\-?#%&=+あ-んア-ンｱ-ﾝ一-龠]*|))");
			std::wsmatch result;
			auto itbegin = strText.cbegin();
			auto itend	 = strText.cend();
			while (std::regex_search(itbegin, itend, result, rx)) {
				CString strUrl;
				if (result[1].str().empty())
					strUrl = _T("http://");
				strUrl += result[2].str().c_str();
				vecUrls.push_back(strUrl);
				itbegin = result[0].second;
			}

			size_t size = vecUrls.size();
			for (unsigned i = 0; i < size; ++i) {
				CString& strUrl = vecUrls[i];
				//Misc::StrToNormalUrl(strUrl);		//+++ 関数化
				DWORD dwOpenFlags = D_OPENFILE_CREATETAB;
				if (  i == (size - 1) 
					&& !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE_NEWWIN))
					dwOpenFlags |= D_OPENFILE_ACTIVATE;	// リンクを開くときアクティブにしないに従う
				DonutOpenFile(strUrl, dwOpenFlags);
			}
		}

		IMPLEMENT_REFCOUNTING(GetSelTextDOMVisitor);
	};
	CefRefPtr<CefDOMVisitor> getSelDomVisitor(new GetSelTextDOMVisitor);
	m_Browser->GetMainFrame()->VisitDOM(getSelDomVisitor);


#if 0
	std::vector<CString> lines;
	lines.reserve(20);
	Misc::SeptTextToLines(lines, strText);
	size_t	size = lines.size();
	//+++ 選択文字列中にurlぽいものがなかったら、文字列検索にしてみる.
	bool	f	= 0;
	for (unsigned i = 0; i < size; ++i) {
		CString& strUrl = lines[i];
		f |= (strUrl.Find(_T(':')) >= 0) || (strUrl.Find(_T('/')) >= 0) || (strUrl.Find(_T('.')) >= 0) ||  (strUrl.Find(_T('\\')) >= 0);
		if (f)
			break;
	}
	if (f == 0) {	// urlぽい文字列がなかった...
		CDonutSearchBar*	pSearchBar = CDonutSearchBar::GetInstance();
		if (pSearchBar) {
			CString str	= lines[0];
			LPCTSTR	strExcept  = _T(" \t\"\r\n　");
			str.TrimLeft(strExcept);
			str.TrimRight(strExcept);

			pSearchBar->SearchWeb(str);
			return;
		}
	}
#endif
}

/// 印刷
void	CChildFrame::Impl::OnFilePrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	m_Browser->GetMainFrame()->Print();
}

/// ポップアップ抑止に追加して閉じます。
void 	CChildFrame::Impl::OnAddClosePopupUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
#if 0
	CIgnoredURLsOption::Add( GetLocationURL() );
	//m_bClosing = true;
	PostMessage(WM_CLOSE);
#endif
}

/// タイトル抑止に追加
void 	CChildFrame::Impl::OnAddClosePopupTitle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
#if 0
	CCloseTitlesOption::Add( MtlGetWindowText(m_hWnd) );
	//m_bClosing = true;
	PostMessage(WM_CLOSE);
#endif
}

/// nページ戻る
void	CChildFrame::Impl::OnViewBackX(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	int 		nStep = (wID - ID_VIEW_BACK1) + 1;
	CLockRedraw lock(m_hWnd);

	for (int i = 0; i < nStep; ++i)
		GoBack(i == (nStep - 1));
}

/// nページ進む
void	CChildFrame::Impl::OnViewForwardX(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	int 		nStep = (wID - ID_VIEW_FORWARD1) + 1;
	CLockRedraw lock(m_hWnd);

	for (int i = 0; i < nStep; ++i)
		GoForward(i == (nStep - 1));
}

// 編集

void	CChildFrame::Impl::OnEditCut(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	m_Browser->GetMainFrame()->Copy();
}

void	CChildFrame::Impl::OnEditCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	m_Browser->GetMainFrame()->Cut();
}

void	CChildFrame::Impl::OnEditPaste(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	m_Browser->GetMainFrame()->Paste();
}

void	CChildFrame::Impl::OnEditSelectAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	m_Browser->GetMainFrame()->SelectAll();
}

void	CChildFrame::Impl::OnEditFind(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if (CMainOption::s_bUseCustomFindBar) {
		CString strText/* = GetSelectedText()*/;
		GetTopLevelWindow().SendMessage(WM_OPENFINDBARWITHTEXT, (WPARAM)(LPCTSTR)strText);
	} else {
//		__super::OnEditFind(0, 0, NULL);
	}
}

/// タイトルをクリップボードにコピーします。
void	CChildFrame::Impl::OnTitleCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	MtlSetClipboardText(MtlGetWindowText(m_hWnd), m_hWnd);
}

/// アドレスをクリップボードにコピーします。
void	CChildFrame::Impl::OnUrlCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	MtlSetClipboardText(GetLocationURL(), m_hWnd);
}

/// タイトルとアドレスをクリップボードにコピーします。
void	CChildFrame::Impl::OnTitleAndUrlCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	CString strText;
	strText.Format(_T("%s\r\n%s"), MtlGetWindowText(m_hWnd), GetLocationURL());
	MtlSetClipboardText(strText, m_hWnd);
}

// 表示

void	CChildFrame::Impl::OnViewBack(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	GoBack();
}

void	CChildFrame::Impl::OnViewForward(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	GoForward();
}

void	CChildFrame::Impl::OnViewHome(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	m_Browser->GetMainFrame()->LoadURL(L"http://www.google.co.jp/");
}


/// 自動ログイン編集ダイアログ表示
void	CChildFrame::Impl::OnShowAutoLoginEditDialog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl)
{
	LoginInfomation	info;
	info.strLoginUrl	= GetLocationURL();
	CLoginInfoEditDialog	dlg(info);
	dlg.SetAutoLoginfunc(boost::bind(&CefBrowser::Reload, m_Browser.get()));
	dlg.DoModal(m_hWnd);
}

void	CChildFrame::Impl::OnViewStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	m_Browser->StopLoad();

//	m_nDownloadCounter = 0;
//	OnStateCompleted();
}

/// 一つ上の階層へ
void	CChildFrame::Impl::OnViewUp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	CString strURL = GetLocationURL();

	if ( strURL.ReverseFind(_T('/')) == (strURL.GetLength() - 1) )
		strURL = strURL.Left( strURL.ReverseFind(_T('/')) );

	if (strURL.ReverseFind(_T('/')) != -1) {
		strURL = strURL.Left(strURL.ReverseFind(_T('/')) + 1);
	} else
		return;

	if (strURL[strURL.GetLength() - 2] == _T('/'))
		return;

	m_Browser->GetMainFrame()->LoadURL((LPCTSTR)strURL);
}

/// ルート階層へ
void	CChildFrame::Impl::OnViewGoToRoot(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	CString strURL = GetLocationURL();
	int nPos = strURL.Find(_T('/'), 9);
	if (nPos == -1)
		return ;

	strURL = strURL.Left(nPos + 1);
	m_Browser->GetMainFrame()->LoadURL((LPCTSTR)strURL);
}

/// 更新
void	CChildFrame::Impl::OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;

	m_bReload = true;
	if (::GetAsyncKeyState(VK_CONTROL) < 0) 	// Inspired by DOGSTORE, Thanks
		m_Browser->ReloadIgnoreCache();
	else
		m_Browser->Reload();
}

/// ソース
void	CChildFrame::Impl::OnViewSource(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	m_Browser->GetMainFrame()->ViewSource();
}

/// 開発者ツール
void	CChildFrame::Impl::OnViewDevTool(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (!m_Browser)
		return ;
	m_Browser->ShowDevTools();
}

// WebViewHost

void CChildFrame::Impl::OnViewMButtonUp(UINT nFlags, CPoint point)
{
	m_bMClick = true;
	m_wndWebViewHost.DefWindowProc();
	m_bMClick = false;
}

void CChildFrame::Impl::OnViewXButtonUp(int fwButton, int dwKeys, CPoint ptPos)
{
	switch (fwButton) {
	case XBUTTON1: GoBack(); break;
	case XBUTTON2: GoForward(); break;
	}
}

void CChildFrame::Impl::OnViewRButtonDown(UINT nFlags, CPoint point)
{
	if (OnRButtonHook(point))
		return ;
	SetMsgHandled(FALSE);
}

void CChildFrame::Impl::OnViewRButtonUp(UINT nFlags, CPoint point)
{
	if (s_bRButtonUpCancel) {
		s_bRButtonUpCancel = false;
		return ;
	}
	SetMsgHandled(FALSE);
}

void CChildFrame::Impl::OnViewClose()
{
	PostMessage(WM_CLOSE);
}


/// 継承するDLコントロールを得る
DWORD	CChildFrame::Impl::_GetInheritedDLCtrlFlags()
{
	DWORD dwDLFlags = CDLControlOption::s_dwDLControlFlags;
	if ( _check_flag(MAIN_EX_INHERIT_OPTIONS, CMainOption::s_dwMainExtendedStyle) )
		dwDLFlags = m_view.GetDLControlFlags();
	return dwDLFlags;
}

/// 継承するExStyleを得る
DWORD	CChildFrame::Impl::_GetInheritedExStyleFlags()
{
	DWORD dwExFlags = CDLControlOption::s_dwExtendedStyleFlags;
	if ( _check_flag(MAIN_EX_INHERIT_OPTIONS, CMainOption::s_dwMainExtendedStyle) ) {
		dwExFlags = m_view.GetExStyle();
	  #if 1	//+++ ナビゲートロックに関しては継承しない....
		dwExFlags &= ~DVS_EX_OPENNEWWIN;											//+++ off
		dwExFlags |= CDLControlOption::s_dwExtendedStyleFlags & DVS_EX_OPENNEWWIN;	//+++ けど、デフォルト設定があれば、それを反映.
	  #endif
	}
	return dwExFlags;
}

bool	CChildFrame::Impl::_CursorOnSelectedText()
{
	REQUIRE_UI_THREAD();

	CefRefPtr<CefFrame> frame = m_Browser->GetMainFrame();
	CefRefPtr<CefV8Context> v8Context = frame->GetV8Context();
	CefString url = frame->GetURL();

	if (!v8Context.get())
		return false;

	if (!v8Context->Enter())
		return false;

	CefRefPtr<CefV8Value> globalObj = v8Context->GetGlobal();
	CefRefPtr<CefV8Value> evalFunc = globalObj->GetValue("eval");

	CefV8ValueList args;
	CefRefPtr<CefV8Value> retVal;
	CefRefPtr<CefV8Exception> exception;
	args.push_back(CefV8Value::CreateString("getSelection().getRangeAt().getBoundingClientRect()"));
	if (evalFunc->ExecuteFunctionWithContext(v8Context, globalObj, args, retVal, exception, false)) {
		if (retVal) {
			CRect rc;
			rc.top	= retVal->GetValue(L"top")->GetIntValue();
			rc.left	= retVal->GetValue(L"left")->GetIntValue();
			rc.right= retVal->GetValue(L"right")->GetIntValue();
			rc.bottom = retVal->GetValue(L"bottom")->GetIntValue();
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			if (rc.PtInRect(pt)) {
				v8Context->Exit();
				return true;
			}
		}
	}

	v8Context->Exit();

	return false;
#if 0
	try {
		HRESULT hr = S_OK;

		CPoint	pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);

		CComPtr<IDispatch>	spDisp;
		hr = m_spBrowser->get_Document(&spDisp);
		CComQIPtr<IHTMLDocument2>	spDocument = spDisp;
		if (spDocument == NULL)
			AtlThrow(hr);


		auto funcGetHTMLWindowOnCursorPos = [](CPoint& pt, IHTMLDocument3* pDoc) -> CComPtr<IHTMLWindow2> {
			auto funcGetIFrameAbsolutePosition = [](CComQIPtr<IHTMLElement>	spIFrame) -> CRect {
				CRect rc;
				spIFrame->get_offsetHeight(&rc.bottom);
				spIFrame->get_offsetWidth(&rc.right);
				CComPtr<IHTMLElement>	spCurElement = spIFrame;
				do {
					CPoint temp;
					spCurElement->get_offsetTop(&temp.y);
					spCurElement->get_offsetLeft(&temp.x);
					rc += temp;
					CComPtr<IHTMLElement>	spTemp;
					spCurElement->get_offsetParent(&spTemp);
					spCurElement.Release();
					spCurElement = spTemp;
				} while (spCurElement.p);
				
				return rc;
			};
			auto funcGetScrollPosition = [](CComQIPtr<IHTMLDocument2> spDoc2) -> CPoint {
				CPoint ptScroll;
				CComPtr<IHTMLElement>	spBody;
				spDoc2->get_body(&spBody);
				CComQIPtr<IHTMLElement2>	spBody2 = spBody;
				spBody2->get_scrollTop(&ptScroll.y);
				spBody2->get_scrollLeft(&ptScroll.x);
				if (ptScroll == CPoint(0, 0)) {
					CComQIPtr<IHTMLDocument3>	spDoc3 = spDoc2;
					CComPtr<IHTMLElement>	spDocumentElement;
					spDoc3->get_documentElement(&spDocumentElement);
					CComQIPtr<IHTMLElement2>	spDocumentElement2 = spDocumentElement;
					spDocumentElement2->get_scrollTop(&ptScroll.y);
					spDocumentElement2->get_scrollLeft(&ptScroll.x);
				}
				return ptScroll;
			};

			HRESULT hr = S_OK;
			CComQIPtr<IHTMLDocument2>	spDoc2 = pDoc;

			CComPtr<IHTMLFramesCollection2>	spFrames;
			spDoc2->get_frames(&spFrames);
			CComPtr<IHTMLElementCollection>	spIFrameCol;
			pDoc->getElementsByTagName(CComBSTR(L"iframe"), &spIFrameCol);
			CComPtr<IHTMLElementCollection>	spFrameCol;
			pDoc->getElementsByTagName(CComBSTR(L"frame"), &spFrameCol);
			
			long frameslength = 0, iframelength = 0, framelength = 0;
			spFrames->get_length(&frameslength);
			spIFrameCol->get_length(&iframelength);
			spFrameCol->get_length(&framelength);
			ATLASSERT(frameslength == iframelength || frameslength == framelength);

			if (frameslength == iframelength && spIFrameCol.p && spFrames.p) {	// インラインフレーム
				for (long i = 0; i < iframelength; ++i) {
					CComVariant vIndex(i);
					CComPtr<IDispatch>	spDisp2;
					spIFrameCol->item(vIndex, vIndex, &spDisp2);
					CRect rcAbsolute = funcGetIFrameAbsolutePosition(spDisp2.p);
					CPoint ptScroll = funcGetScrollPosition(spDoc2);
					CRect rc = rcAbsolute - ptScroll;
					if (rc.PtInRect(pt)) {
						pt.x	-= rc.left;
						pt.y	-= rc.top;
						CComVariant vResult;
						spFrames->item(&vIndex, &vResult);
						CComQIPtr<IHTMLWindow2> spWindow = vResult.pdispVal;
						return spWindow;
					}
				}
			}

			if (frameslength == framelength && spFrameCol.p && spFrames.p) {	// 普通のフレーム
				for (long i = 0; i < framelength; ++i) {
					CComVariant vIndex(i);
					CComPtr<IDispatch>	spDisp2;
					spFrameCol->item(vIndex, vIndex, &spDisp2);
					CComQIPtr<IHTMLElement>	spFrame = spDisp2;
					if (spFrame.p) {
						CRect rc = funcGetIFrameAbsolutePosition(spFrame);
						//spFrame->get_offsetLeft(&rc.left);
						//spFrame->get_offsetTop(&rc.top);
						//long temp;
						//spFrame->get_offsetWidth(&temp);
						//rc.right += rc.left + temp;
						//spFrame->get_offsetHeight(&temp);
						//rc.bottom+= rc.top + temp;
						if (rc.PtInRect(pt)) {
							pt.x	-= rc.left;
							pt.y	-= rc.top;
							CComVariant vResult;
							spFrames->item(&vIndex, &vResult);
							CComQIPtr<IHTMLWindow2> spWindow = vResult.pdispVal;
							return spWindow;
						}
					}
				}
			}
			return nullptr;
		};	// funcGetHTMLWindowOnCursorPos

		CComQIPtr<IHTMLDocument3>	spDocument3 = spDocument;
		CComPtr<IHTMLWindow2> spWindow = funcGetHTMLWindowOnCursorPos(pt, spDocument3);
		if (spWindow) {
			spDocument.Release();
			CComQIPtr<IHTMLDocument2>	spFrameDocument;
			hr = spWindow->get_document(&spFrameDocument);
			if ( FAILED(hr) ) {	// 別ドメイン
				CComQIPtr<IServiceProvider>  spServiceProvider = spWindow;
				ATLASSERT(spServiceProvider);
				CComPtr<IWebBrowser2>	spBrowser;
				hr = spServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&spBrowser);
				if (!spBrowser)
					AtlThrow(hr);
				CComPtr<IDispatch>	spDisp;
				hr = spBrowser->get_Document(&spDisp);
				if (!spDisp)
					AtlThrow(hr);
				spDocument = spDisp;
			} else {
				spDocument = spFrameDocument;
			}
		}

		CComPtr<IDispatch>	spTargetDisp;
		auto funcGetRangeDisp = [&spTargetDisp](CPoint pt, IHTMLDocument2 *pDocument) {
			CComPtr<IHTMLSelectionObject>		spSelection;
			HRESULT 	hr	= pDocument->get_selection(&spSelection);
			if ( SUCCEEDED(hr) ) {
				CComPtr<IDispatch>				spDisp;
				hr	   = spSelection->createRange(&spDisp);
				if ( SUCCEEDED(hr) ) {
					CComQIPtr<IHTMLTxtRange>	spTxtRange = spDisp;
					if (spTxtRange != NULL) {
						CComBSTR				bstrText;
						hr	   = spTxtRange->get_text(&bstrText);
						if (SUCCEEDED(hr) && !!bstrText) {
							spTargetDisp = spDisp;
						}
					}
				}
			}
		};

		funcGetRangeDisp(pt, spDocument);
		if (spTargetDisp == NULL)
			return false;

		CComQIPtr<IHTMLTextRangeMetrics2>	spMetrics = spTargetDisp;
		ATLASSERT(spMetrics);
		CComPtr<IHTMLRect>	spRect;
		hr = spMetrics->getBoundingClientRect(&spRect);
		if (FAILED(hr))
			AtlThrow(hr);

		CRect rc;
		spRect->get_top(&rc.top);
		spRect->get_left(&rc.left);
		spRect->get_right(&rc.right);
		spRect->get_bottom(&rc.bottom);
		if (rc.PtInRect(pt)) {
			return true;
		}


#if 0
		CComPtr<IHTMLRectCollection>	spRcCollection;
		hr = spMetrics->getClientRects(&spRcCollection);
		if (FAILED(hr))
			AtlThrow(hr);

		long l;
		spRcCollection->get_length(&l);
		for (long i = 0; i < l; ++i) {
			VARIANT vIndex;
			vIndex.vt	= VT_I4;
			vIndex.lVal	= i;
			VARIANT vResult;
			if (spRcCollection->item(&vIndex, &vResult) == S_OK) {
				CComQIPtr<IHTMLRect>	spRect = vResult.pdispVal;
				ATLASSERT(spRect);
				CRect rc;
				spRect->get_top(&rc.top);
				spRect->get_left(&rc.left);
				spRect->get_right(&rc.right);
				spRect->get_bottom(&rc.bottom);
				if (rc.PtInRect(pt)) {
					return true;
				}
			}
		}
#endif

	}
	catch (const CAtlException& e) {
		e;
	}
	return false;
#endif
}

/// トラベルログを初期化する
void	CChildFrame::Impl::_InitTravelLog()
{
#if 0
	if (m_bInitTravelLog == false) {
		m_bInitTravelLog = true;
		if (m_TravelLogFore.empty() && m_TravelLogBack.empty())
			return ;
		auto LoadTravelLog = [this](vector<std::pair<CString, CString> >& arrLog, bool bFore) -> BOOL {
			HRESULT 					 hr;
			CComPtr<IEnumTravelLogEntry> pTLEnum;
			CComPtr<ITravelLogEntry>	 pTLEntryBase;
			int		nDir = bFore ? TLEF_RELATIVE_FORE : TLEF_RELATIVE_BACK;

			CComQIPtr<IServiceProvider>	 pISP = m_spBrowser;
			if (pISP == NULL)
				return FALSE;

			CComPtr<ITravelLogStg>		 pTLStg;
			hr	   = pISP->QueryService(SID_STravelLogCursor, IID_ITravelLogStg, (void **) &pTLStg);
			if (FAILED(hr) || pTLStg == NULL)
				return FALSE;

			hr	   = pTLStg->EnumEntries(nDir, &pTLEnum);
			if (FAILED(hr) || pTLEnum == NULL)
				return FALSE;

			int		nLast	= ( 10 > arrLog.size() ) ? (int) arrLog.size() : 10;
			for (int i = 0; i < nLast; i++) {
				CComPtr<ITravelLogEntry> pTLEntry;

			  #if 1	//+++ UNICODE 対応.
				std::vector<wchar_t>	title = Misc::tcs_to_wcs( LPCTSTR( arrLog[i].first ) );
				std::vector<wchar_t>	url   = Misc::tcs_to_wcs( LPCTSTR( arrLog[i].second) );
				// CreateEntry の第四引数がTRUEだと前に追加される
				hr			 = pTLStg->CreateEntry(&url[0], &title[0], pTLEntryBase, !bFore, &pTLEntry);
			  #else
				CString 	strTitle	= arrLog[i].first;
				CString 	strURL		= arrLog[i].second;

				LPOLESTR				 pszwURL   = _ConvertString(strURL, strURL.GetLength() + 1);		//new
				LPOLESTR				 pszwTitle = _ConvertString(strTitle, strTitle.GetLength() + 1);	//new

				hr			 = pTLStg->CreateEntry(pszwURL, pszwTitle, pTLEntryBase, !bFore, &pTLEntry);
				delete[] pszwURL;																			//Don't forget!
				delete[] pszwTitle;
			  #endif

				if (FAILED(hr) || pTLEntry == NULL)
					return FALSE;

				if (pTLEntryBase.p)
					pTLEntryBase.Release();

				pTLEntryBase = pTLEntry;
			}

			return TRUE;
		};	// lamda

		LoadTravelLog(m_TravelLogFore, true);
		LoadTravelLog(m_TravelLogBack, false);
		m_TravelLogFore.clear();
		m_TravelLogBack.clear();
	}
#endif
}

/// 最近閉じたタブ用のデータを集める
void	CChildFrame::Impl::_CollectDataOnClose(ChildFrameDataOnClose& data)
{
	data.strTitle	= m_UIChange.GetUIData().strTitle;
	data.strTitle.Replace(_T('\"'), _T('_'));
	data.strURL		= m_UIChange.GetUIData().strLocationURL;
	data.dwDLCtrl	= m_view.GetDLControlFlags();
	data.dwExStyle	= m_view.GetExStyle();
	data.dwAutoRefreshStyle	= m_view.GetAutoRefreshStyle();

	data.TravelLogBack	= m_TravelLogBack;
	data.TravelLogFore	= m_TravelLogFore;
#if 0
	HRESULT hr;
	CComQIPtr<IServiceProvider>	 pISP = m_spBrowser;
	if (pISP == NULL)
		return ;

	CComPtr<ITravelLogStg>		 pTLStg;
	hr	= pISP->QueryService(SID_STravelLogCursor, IID_ITravelLogStg, (void **) &pTLStg);
	if (FAILED(hr) || pTLStg == NULL)
		return ;

	auto GetTravelLog	= [&](vector<std::pair<CString, CString> >& vecLog, bool bFore) {
		DWORD	dwCount = 0;
		hr	= pTLStg->GetCount(bFore ? TLEF_RELATIVE_FORE : TLEF_RELATIVE_BACK, &dwCount);
		if (FAILED(hr) || dwCount == 0)	// 単に履歴がないだけの状態ならtrue.
			return ;

		CComPtr<IEnumTravelLogEntry> pTLEnum;
		hr	= pTLStg->EnumEntries(bFore ? TLEF_RELATIVE_FORE : TLEF_RELATIVE_BACK, &pTLEnum);
		if (FAILED(hr) || pTLEnum == NULL)
			return ;

		vecLog.reserve(10);

		int 	count = 0;
		for (int j = 0; j < (int)dwCount; ++j) {
			CComPtr<ITravelLogEntry>  pTLEntry	= NULL;
			LPOLESTR				  szURL 	= NULL;
			LPOLESTR				  szTitle	= NULL;
			DWORD	dummy = 0;
			hr = pTLEnum->Next(1, &pTLEntry, &dummy);
			if (pTLEntry == NULL || FAILED(hr))
				break;

			if (   SUCCEEDED( pTLEntry->GetTitle(&szTitle) ) && szTitle
				&& SUCCEEDED( pTLEntry->GetURL  (&szURL  ) ) && szURL  )
			{
				CString strTitle = szTitle;
				strTitle.Replace(_T('\"'), _T('_'));
				vecLog.push_back(std::make_pair<CString, CString>(szTitle, szURL));
				++count;
			}
			if (szTitle) ::CoTaskMemFree( szTitle );
			if (szURL)	 ::CoTaskMemFree( szURL );

			if (count >= 10)
				break;
		}
	};	// lamda

	GetTravelLog(data.TravelLogBack, false);
	GetTravelLog(data.TravelLogFore, true);
#endif
}


/// タブなどにFaviconを設定
void	CChildFrame::Impl::_SetFavicon(CefRefPtr<CefFrame> frame)
{
	class GetFaviconUrlDOMVisitor : public CefDOMVisitor
	{
	public:
		GetFaviconUrlDOMVisitor(HWND hWndChildFrame) : m_hWndChildFrame(hWndChildFrame) {}

		virtual void Visit(CefRefPtr<CefDOMDocument> document)
		{
			CString strFaviconURL;
			CString strURL = document->GetBaseURL().c_str();
			auto headNode = document->GetHead();
			if (headNode == nullptr)
				return ;

			for (auto it = headNode->GetFirstChild(); it; it = it->GetNextSibling()) {
				auto tagName = it->GetElementTagName();
				if (tagName == L"LINK") {
					CString relContent = it->GetElementAttribute(L"rel").c_str();
					relContent.MakeLower();
					if (relContent == L"shortcut icon" || relContent == L"icon") {
						auto hrefContent = it->GetElementAttribute(L"href");
						CString strhref = hrefContent.c_str();
						DWORD	dwSize = INTERNET_MAX_URL_LENGTH;
						ATLVERIFY(SUCCEEDED(::UrlCombine(strURL, strhref, strFaviconURL.GetBuffer(INTERNET_MAX_URL_LENGTH), &dwSize, 0)));
						strFaviconURL.ReleaseBuffer();
						break;
					}
				}
			}

			if (strFaviconURL.IsEmpty()) {	// ルートにあるFaviconのアドレスを得る
				DWORD cchResult = INTERNET_MAX_URL_LENGTH;
				if (::CoInternetParseUrl(strURL, PARSE_ROOTDOCUMENT, 0, strFaviconURL.GetBuffer(INTERNET_MAX_URL_LENGTH), INTERNET_MAX_URL_LENGTH, &cchResult, 0) == S_OK) {
					strFaviconURL.ReleaseBuffer();
					strFaviconURL += _T("/favicon.ico");
				}
			}
			::SendMessage(m_hWndChildFrame, WM_FAVICONCHANGE, (WPARAM)(LPCTSTR)strFaviconURL, 0);
			CFaviconManager::SetFavicon(m_hWndChildFrame, strFaviconURL);
		}

	private:
		HWND	m_hWndChildFrame;

		IMPLEMENT_REFCOUNTING(GetFaviconUrlDOMVisitor);
	};
	CefRefPtr<CefDOMVisitor> getFaviconDomVisitor(new GetFaviconUrlDOMVisitor(m_hWnd));
	frame->VisitDOM(getFaviconDomVisitor);

#if 0
	CString strFaviconURL;
	HRESULT hr = S_OK;
	CComPtr<IDispatch>	spDisp;
	m_spBrowser->get_Document(&spDisp);
	CComQIPtr<IHTMLDocument3>	spDocument = spDisp;
	if (spDocument) {
		CComPtr<IHTMLElementCollection>	spCol;
		spDocument->getElementsByTagName(CComBSTR(L"link"), &spCol);
		if (spCol) {
			ForEachHtmlElement(spCol, [&](IDispatch* pDisp) -> bool {
				CComQIPtr<IHTMLLinkElement>	spLink = pDisp;
				if (spLink.p) {
					CComBSTR strrel;
					spLink->get_rel(&strrel);
					CComBSTR strhref;
					spLink->get_href(&strhref);
					strrel.ToLower();
					if (strrel == _T("shortcut icon") || strrel == _T("icon")) {
						DWORD	dwSize = INTERNET_MAX_URL_LENGTH;
						hr = ::UrlCombine(strURL, strhref, strFaviconURL.GetBuffer(INTERNET_MAX_URL_LENGTH), &dwSize, 0);
						strFaviconURL.ReleaseBuffer();
						if (SUCCEEDED(hr))
							return false;
					}
				}
				return true;
			});
		}
	}
#endif
}

void	CChildFrame::Impl::_HilightOnce(IDispatch *pDisp, LPCTSTR lpszKeyWord)
{
	CComQIPtr<IWebBrowser2> 	pWebBrowser = pDisp;
	if (pWebBrowser) {
		MtlForEachHTMLDocument2g(pWebBrowser, _Function_SelectEmpt());
		MtlForEachHTMLDocument2g(pWebBrowser, _Function_Hilight2(lpszKeyWord, TRUE));
	}
}


	
BOOL CChildFrame::Impl::_FindKeyWordOne(IHTMLDocument2* pDocument, const CString& strKeyword, BOOL bFindDown, long Flags /*= 0*/)
{
	return FALSE;
#if 0
	// ドキュメントがNULLなら終了
	if (!pDocument)
		return FALSE;

	HRESULT	hr = S_OK;
	// キーワードを一語取得
	//x strKeyWord = strtok( (LPSTR) strKeyWord.GetBuffer(0), " " );
	CString 	strKeyWord = strKeyword;//\\Misc::GetStrWord( strKeyword );
	LPCTSTR		strExcept  = _T(" \t\"\r\n　");
	strKeyWord.TrimLeft(strExcept);
	strKeyWord.TrimRight(strExcept);

	// <body>を取得
	CComPtr<IHTMLElement>		spHTMLElement;
	pDocument->get_body(&spHTMLElement);
	CComQIPtr<IHTMLBodyElement> spHTMLBody = spHTMLElement;
	if (!spHTMLBody)
		return FALSE;

	// テキストレンジを取得
	CComPtr<IHTMLTxtRange>		spTxtRange;
	spHTMLBody->createTextRange(&spTxtRange);
	if (!spTxtRange)
		return FALSE;

	if (m_strBookmark && m_strOldKeyword == strKeyword) {
		VARIANT_BOOL vMoveBookmark = VARIANT_FALSE;
		spTxtRange->moveToBookmark(m_strBookmark, &vMoveBookmark);
		if (vMoveBookmark == TRUE) {
			long lActual;
			if (bFindDown) {
				spTxtRange->collapse(false);	// Caretの位置を選択したテキストの一番下に
				if (Misc::getIEMejourVersion() < 9)
					spTxtRange->moveStart(CComBSTR(L"character"), 1, &lActual);
				//spTxtRange->moveEnd(CComBSTR("Textedit"), 1, &lActual);
			} else {
				spTxtRange->collapse(true);	// Caretの位置を選択したテキストの一番上に
				//if (Misc::getIEMejourVersion() < 9) 
				//	spTxtRange->moveEnd  (CComBSTR(L"character"), -10, &lActual);			
			}
		}
	} else {	// 検索範囲を全体にする
		long lActual;
		spTxtRange->moveStart(CComBSTR("Textedit"), -1, &lActual);
		spTxtRange->moveEnd(CComBSTR("Textedit"), 1, &lActual);
	}
	m_strOldKeyword = strKeyword;

	CComBSTR		bstrText(strKeyWord);
	BOOL			bSts  = FALSE;
	VARIANT_BOOL	vBool = VARIANT_FALSE;
	int	nSearchCount = 0;
	while (spTxtRange->findText(bstrText, (bFindDown) ? 1 : -1, Flags, &vBool), vBool == VARIANT_TRUE) {

		auto funcMove = [&spTxtRange, bFindDown] () {	// 検索範囲を変更する関数
			long lActual = 0;
			if (bFindDown)
				spTxtRange->collapse(false);
				//spTxtRange->moveStart(CComBSTR(L"character"), 1, &lActual);
			else
				spTxtRange->collapse(true);
				//spTxtRange->moveEnd  (CComBSTR(L"character"), -10, &lActual);
		};
		
		CComPtr<IHTMLElement>	spFirstParentElement;

		bool	bVisible = true;
		CComQIPtr<IHTMLElement> spParentElement;
		spTxtRange->parentElement(&spParentElement);
		spFirstParentElement = spParentElement;
		while (spParentElement) {
			CComQIPtr<IHTMLElement2>	spParentElm2 = spParentElement;
			CComPtr<IHTMLCurrentStyle>	spStyle;
			spParentElm2->get_currentStyle(&spStyle);
			if (spStyle) {
				CComBSTR	strdisplay;
				spStyle->get_display(&strdisplay);
				if (strdisplay && strdisplay == _T("none")) {	// 表示されていない場合はスキップ
					funcMove();
					bVisible = false;
					break;
				}
			}
			CComPtr<IHTMLElement>	spPPElm;
			spParentElement->get_parentElement(&spPPElm);
			spParentElement = spPPElm;
		}
		if (bVisible == false)
			continue;

		CComBSTR	bstrParentTag;
		spFirstParentElement->get_tagName(&bstrParentTag);
		if (   bstrParentTag != _T("SCRIPT")
			&& bstrParentTag != _T("NOSCRIPT")
			&& bstrParentTag != _T("TEXTAREA")) 
			break;	/* 終わり */

		//++nSearchCount;
		//if (nSearchCount > 5)	// 5以上で打ち止め
		//	break;

		funcMove();
	}

	auto funcScrollBy = [](IHTMLDocument2 *pDoc2) {
		CComPtr<IHTMLDocument3> 	   pDoc3;
		HRESULT 	hr = pDoc2->QueryInterface(&pDoc3);
		if ( FAILED(hr) )
			return;

		CComPtr<IHTMLElement>		   pElem;
		hr	= pDoc3->get_documentElement(&pElem);
		if ( FAILED(hr) )
			return;

		long		height = 0;
		hr	= pElem->get_offsetHeight(&height); 	// HTML表示領域の高さ
		if ( FAILED(hr) )
			return;

		CComPtr<IHTMLSelectionObject>  pSel;
		hr	= pDoc2->get_selection(&pSel);
		if ( FAILED(hr) )
			return;

		CComPtr<IDispatch>			   pDisp;
		hr	= pSel->createRange(&pDisp);

		if ( FAILED(hr) )
			return;

		CComPtr<IHTMLTextRangeMetrics> pTxtRM;
		hr	= pDisp->QueryInterface(&pTxtRM);
		if ( FAILED(hr) )
			return;

		long		y = 0;
		hr	= pTxtRM->get_offsetTop(&y);		// 選択部分の画面上からのy座標
		if ( FAILED(hr) )
			return;

		long scy = y - height / 2;				// 画面中央までの距離

		// 距離が表示部分の1/4より大きければスクロールさせる
		if ( (scy > height / 4) || (scy < -height / 4) ) {
			CComPtr<IHTMLWindow2> pWnd;
			hr = pDoc2->get_parentWindow(&pWnd);

			if ( FAILED(hr) )
				return;

			pWnd->scrollBy(0, scy);
		}
	};

	if (vBool == VARIANT_FALSE) {
		CComPtr<IHTMLSelectionObject> spSelection;
		pDocument->get_selection(&spSelection);
		if (spSelection)
			spSelection->empty();
	} else {
		if (spTxtRange->getBookmark(&m_strBookmark) != S_OK)
			m_strBookmark = LPCOLESTR(NULL);

		spTxtRange->select();
		spTxtRange->scrollIntoView(VARIANT_TRUE);

		bSts = TRUE;

		if (CSearchBarOption::s_bScrollCenter)
			funcScrollBy(pDocument);
	}

	return bSts;
#endif
}




void CChildFrame::Impl::AddDOMVisitor(const std::string& path,
                                  CefRefPtr<CefDOMVisitor> visitor)
{
  AutoLock lock_scope(this);
  DOMVisitorMap::iterator it = m_DOMVisitors.find(path);
  if (it == m_DOMVisitors.end())
    m_DOMVisitors.insert(std::make_pair(path, visitor));
  else
    it->second = visitor;
}

CefRefPtr<CefDOMVisitor> CChildFrame::Impl::GetDOMVisitor(const std::string& path)
{
  AutoLock lock_scope(this);
  DOMVisitorMap::iterator it = m_DOMVisitors.find(path);
  if (it != m_DOMVisitors.end())
    return it->second;
  return NULL;
}


