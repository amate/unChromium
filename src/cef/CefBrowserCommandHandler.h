/**
*	@file	CefBrowserCommandHandler.h
*	@brief	CChildFrameからのコマンドを処理します
*/

#pragma once


///////////////////////////////////////////////////////////////////
// CCefBrowserCommandHandler

template <class T>
class CCefBrowserCommandHandler 
{
	enum EHtmlId {
		HTMLID_FIND 		= 1,
		HTMLID_VIEWSOURCE	= 2,
		HTMLID_OPTIONS		= 3,
	};

public:
	// Message map and handlers
	BEGIN_MSG_MAP_EX(CCefBrowserCommandHandler)
		COMMAND_ID_HANDLER_EX( ID_FILE_SAVE_AS		, OnFileSaveAs		)
		COMMAND_ID_HANDLER_EX( ID_FILE_PAGE_SETUP	, OnFilePageSetup	)
		COMMAND_ID_HANDLER_EX( ID_FILE_PRINT		, OnFilePrint		)
		COMMAND_ID_HANDLER_EX( ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
		COMMAND_ID_HANDLER_EX( ID_FILE_PROPERTIES	, OnFileProperties	)
		COMMAND_ID_HANDLER_EX( ID_EDIT_FIND 		, OnEditFind		)
		
		COMMAND_ID_HANDLER_EX( ID_VIEW_FONT_LARGEST , OnViewFontLargest )
		COMMAND_ID_HANDLER_EX( ID_VIEW_FONT_LARGER	, OnViewFontLarger	)
		COMMAND_ID_HANDLER_EX( ID_VIEW_FONT_MEDIUM	, OnViewFontMedium	)
		COMMAND_ID_HANDLER_EX( ID_VIEW_FONT_SMALLER , OnViewFontSmaller )
		COMMAND_ID_HANDLER_EX( ID_VIEW_FONT_SMALLEST, OnViewFontSmallest)
		COMMAND_ID_HANDLER_EX( ID_VIEW_SOURCE		, OnViewSource		)
		COMMAND_ID_HANDLER_EX( ID_VIEW_OPTION		, OnViewOption		)

		COMMAND_ID_HANDLER_EX( ID_VIEW_SOURCE_SELECTED , OnViewSourceSelected )
		//COMMAND_ID_HANDLER_EX( ID_EDIT_FIND_MAX	, OnEditFindMax 	)
		COMMAND_ID_HANDLER_EX( ID_VIEW_UP			, OnViewUp			)
	END_MSG_MAP()


public:
	void OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_Browser->ExecWB(OLECMDID_SAVEAS, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
	}


	void OnFilePageSetup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->ExecWB(OLECMDID_PAGESETUP, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
	}


	void OnFilePrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->ExecWB(OLECMDID_PRINT, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
	}


private:
	void OnFilePrintPreview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->ExecWB(OLECMDID_PRINTPREVIEW, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
	}


public:
	void OnFileProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->ExecWB(OLECMDID_PROPERTIES, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
	}


private:
	void OnEditCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
	//	T *pT = static_cast<T *>(this);
	//	pT->m_spBrowser->ExecWB(OLECMDID_CUT, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
		T *pT = static_cast<T *>(this);
		CComQIPtr<IOleCommandTarget> spCmdTarget = pT->m_spBrowser;
		spCmdTarget->Exec(&CGID_MSHTML, IDM_CUT, 0, NULL, NULL);
	}


	void OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		CComQIPtr<IOleCommandTarget> spCmdTarget = pT->m_spBrowser;
		spCmdTarget->Exec(&CGID_MSHTML, IDM_COPY, 0, NULL, NULL);
	}


	void OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
	//	T *pT = static_cast<T *>(this);
	//	pT->m_spBrowser->ExecWB(OLECMDID_PASTE, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
		T *pT = static_cast<T *>(this);
		CComQIPtr<IOleCommandTarget> spCmdTarget = pT->m_spBrowser;
		spCmdTarget->Exec(&CGID_MSHTML, IDM_PASTE, 0, NULL, NULL);
	}


public:
	void OnEditFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T * 	pT = static_cast<T *>(this);
		CComQIPtr<IOleCommandTarget> spCmdTarget = pT->m_spBrowser;
		spCmdTarget->Exec(&CGID_IWebBrowser, HTMLID_FIND, 0, NULL, NULL);
		// this is just file search
		//	m_spBrowser->ExecWB(OLECMDID_FIND, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
	}


private:
	void OnEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->ExecWB(OLECMDID_SELECTALL, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
	}


	void OnViewBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->GoBack();
	}


	void OnViewForward(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->GoForward();
	}


	void OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->Refresh();
	}


	void OnViewStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->Stop();
	}


	void OnViewHome(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T *pT = static_cast<T *>(this);
		pT->m_spBrowser->GoHome();
	}


	void OnViewUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T * 	pT = static_cast<T *>(this);
		CString strURL;
		strURL = pT->GetLocationURL();

		if ( strURL.ReverseFind(_T('/')) == (strURL.GetLength() - 1) )
			strURL = strURL.Left( strURL.ReverseFind(_T('/')) );

		if (strURL.ReverseFind(_T('/')) != -1) {
			strURL = strURL.Left(strURL.ReverseFind(_T('/')) + 1);
		} else
			return;

		if (strURL[strURL.GetLength() - 2] == _T('/'))
			return;

		DonutOpenFile(strURL, D_OPENFILE_NOCREATE);
	}


public:
	void OnViewOption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T * 	pT		 = static_cast<T *>(this);

		CComQIPtr<IOleCommandTarget> spCmdTarget = pT->m_spBrowser;
		spCmdTarget->Exec(&CGID_IWebBrowser, HTMLID_OPTIONS, 0, NULL, NULL);
		// this is modeless
		// ::ShellExecute(0, "open", "control.exe", "inetcpl.cpl", ".", SW_SHOW);
	}


private:
	// zoom font
	void OnViewFontLargest(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		CComVariant 	vaZoomFactor(4L);
		T * 			pT = static_cast<T *>(this);
		pT->m_spBrowser->ExecWB(OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER, &vaZoomFactor, NULL);
	}


	void OnViewFontLarger(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		CComVariant 	vaZoomFactor(3L);
		T * 			pT = static_cast<T *>(this);
		pT->m_spBrowser->ExecWB(OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER, &vaZoomFactor, NULL);
	}


	void OnViewFontMedium(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		CComVariant 	vaZoomFactor(2L);
		T * 			pT = static_cast<T *>(this);

		pT->m_spBrowser->ExecWB(OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER, &vaZoomFactor, NULL);
	}


	void OnViewFontSmaller(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		CComVariant vaZoomFactor(1L);
		T * 		pT = static_cast<T *>(this);
		pT->m_spBrowser->ExecWB(OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER, &vaZoomFactor, NULL);
	}


	void OnViewFontSmallest(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		CComVariant vaZoomFactor(0L);
		T * 		pT = static_cast<T *>(this);
		pT->m_spBrowser->ExecWB(OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER, &vaZoomFactor, NULL);
	}


	void OnViewSource(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		T * 		pT		 = static_cast<T *>(this);
		CComBSTR	bstrUrl;
		CString		strUrl;
		pT->m_spBrowser->get_LocationURL(&bstrUrl);
		strUrl = bstrUrl;
		if ( strUrl.Left(8) == _T("file:///") ) {
			TCHAR	strpath[MAX_PATH];
			ULONG	pnChars = MAX_PATH;
			CRegKey	rk;
			if ( rk.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer\\View Source Editor\\Editor Name"), KEY_READ) == ERROR_SUCCESS ) {
				if ( rk.QueryStringValue(NULL, strpath, &pnChars) == ERROR_SUCCESS ) {
					strUrl.Replace(_T("%20"), _T(" "));
					::ShellExecute(NULL, NULL, strpath, _T("\"") + strUrl + _T("\""), NULL, SW_SHOWNORMAL);
					return;
				}
			}
		}
		CComQIPtr<IOleCommandTarget>	spCmdTarget = pT->m_spBrowser;
		spCmdTarget->Exec(&CGID_IWebBrowser, HTMLID_VIEWSOURCE, 0, NULL, NULL);
	}


	void OnViewSourceSelected(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
	{
		CComBSTR/*BSTR*/											bstrHTML;			//+++ CComBSTRに変更
		
		CComPtr<IDispatch>											pDisp;
		CComPtr<IDispatch>											pDisp2;
		CComQIPtr<IHTMLSelectionObject, &IID_IHTMLSelectionObject>	spSelection;
		CComQIPtr<IHTMLTxtRange, &IID_IHTMLTxtRange>				spTextRange;
		T *		pT	= static_cast<T *>(this);
		HRESULT	hr	= pT->m_spBrowser->get_Document(&pDisp);
		if (FAILED(hr)) {
			ATLASSERT(FALSE);
			return;
		}
		CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2>				spHTML;
		spHTML = pDisp;
		ATLASSERT(spHTML);

		if (spHTML) {
			spHTML->get_selection(&spSelection);
			spSelection->createRange(&pDisp2);
			spTextRange = pDisp2;

			if (spTextRange) {	// Non Frame && GetSelection
				spTextRange->get_htmlText(&bstrHTML);

				if (bstrHTML) {
					viewSelectedSource(LPCOLESTR(bstrHTML));
				} else {
					parseFramesCollection(pDisp , wID);
				}
			} else {
				parseFramesCollection(pDisp , wID);
			}
		}
	}

	/// 選択範囲のソースをエディタで表示する
	void viewSelectedSource(const CString &strHTMLText)
	{
		if ( strHTMLText.IsEmpty() ) 
			return;

		T* pT = static_cast<T*>(this);

		// get tmp path & create tmp file
		CString strTempPath;
		if (GetDonutTempPath(strTempPath) == false)
			return ;

		CString	tmpFileName;
		::GetTempFileName(strTempPath , _T("dnr") , 0 , tmpFileName.GetBuffer(MAX_PATH));
		tmpFileName.ReleaseBuffer();
		tmpFileName.Replace(_T(".tmp"), _T(".htm"));

		HANDLE	hFile	= ::CreateFile(tmpFileName ,
								GENERIC_WRITE,
								0,
								0,
								CREATE_ALWAYS,
								FILE_ATTRIBUTE_TEMPORARY,
								NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			CString strError;
			strError.Format(_T("ファイルが作成できません。\nパス:%s"), tmpFileName);
			MessageBox(NULL, strError, NULL, MB_ICONERROR);
			return ;
		}

		DWORD	dwAccBytes = 0;
	  #ifdef UNICODE
		//CIniFileI pr( g_szIniFileName, _T("etc") );
		//bool bSelectedSource = pr.GetValue(_T("SelectedSourceTextMode"));
		//pr.Close();
		if (Misc::IsEnableUnicodeToSJIS(strHTMLText)) {	// SJIS に変換できるなら、SJISにして出力
			std::vector<char>	sjis = Misc::wcs_to_sjis( LPCTSTR( strHTMLText ) );
			::WriteFile(hFile , &sjis[0], (DWORD)strlen(&sjis[0]), &dwAccBytes , NULL);
		} else {			// SJISにできないUnicode文字なら Utf8にして出力
			std::vector<char>	utf8 = Misc::wcs_to_utf8( LPCTSTR( strHTMLText ) );
			::WriteFile(hFile , &utf8[0], (DWORD)strlen(&utf8[0]), &dwAccBytes , NULL);
		}
	  #else
		::WriteFile(hFile , strHTMLText , strHTMLText.GetLength() , &dwAccBytes , NULL);
	  #endif
		::CloseHandle(hFile);

		// get editor path
		TCHAR	strpath[MAX_PATH];
		ULONG	pnChars = MAX_PATH;
		CRegKey	rk;
		if ( rk.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer\\View Source Editor\\Editor Name"), KEY_READ) == ERROR_SUCCESS ) {
			if ( rk.QueryStringValue(NULL, strpath, &pnChars) == ERROR_SUCCESS ) {
				ShellExecute(pT->m_hWnd , _T("open") , strpath , tmpFileName , NULL , SW_SHOWNORMAL);
				return;
			}
		}

		ShellExecute(pT->m_hWnd , _T("open") , _T("notepad.exe") , tmpFileName , NULL , SW_SHOWNORMAL);
		// i cannot use CreateProcess for checking Exit code, coz i cannnot check it ,in case using src Handling tool.
	}


	void parseFramesCollection(CComPtr<IDispatch> pDisp , WORD wID)
	{
		CComBSTR/*BSTR*/												bstrHTML;					//+++ CComBSTR に変更
		// BSTR bstrLocation;
		CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2>					spHTML;
		CComQIPtr<IHTMLFramesCollection2, &IID_IHTMLFramesCollection2>	spFrameCollection;

		spHTML		= pDisp;
		HRESULT hr	= spHTML->get_frames(&spFrameCollection);
		if (hr != S_OK) return;

		LONG	lFrameLength;
		hr = spFrameCollection->get_length(&lFrameLength);
		ATLASSERT(SUCCEEDED(hr));

		for (LONG i = 0; i < lFrameLength; i++) {
			CComQIPtr<IHTMLWindow2, &IID_IHTMLWindow2>				   spHTMLWindow;
			CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2>			   spNextHTML;
			CComQIPtr<IHTMLSelectionObject, &IID_IHTMLSelectionObject> spNextSelection;
			CComQIPtr<IHTMLTxtRange, &IID_IHTMLTxtRange>			   spNextTextRange;
			CComQIPtr<IHTMLLocation, &IID_IHTMLLocation>			   spHTMLLocation;
			CComPtr<IDispatch> pDispNext, pDispBack;

			VARIANT 	varResult;
			VARIANT 	varItem;
			varItem.vt	 = VT_I4;
			varItem.lVal = i;
			spFrameCollection->item(&varItem, &varResult);
			spHTMLWindow = varResult.pdispVal;

			if (spHTMLWindow) {
				spHTMLWindow->get_document(&spNextHTML);

				if (spNextHTML) {
					spNextHTML->get_selection(&spNextSelection);
					spNextSelection->createRange(&pDispNext);
					spNextTextRange = pDispNext;

					if (spNextTextRange) {
						if (wID == ID_EDIT_FIND_MAX) {
							spNextTextRange->get_text(&bstrHTML);
							T *pT = static_cast<T *>(this);

							if (bstrHTML)
								pT->searchEngines(LPCOLESTR(bstrHTML));
						} else if (wID == ID_VIEW_SOURCE_SELECTED) {
							spNextTextRange->get_htmlText(&bstrHTML);
							if (bstrHTML) {
								viewSelectedSource(LPCOLESTR(bstrHTML));
							}
						}
					} else {
						pDispBack = spNextHTML;
						parseFramesCollection(pDispBack , wID);
					}
				}
			}
		}
	}


public:
	void _OnEditFindMax(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
	{
		CComBSTR/*BSTR*/	bstrHTML;								//+++ CComBSTRに変更
		T * 				pT		= static_cast<T *>(this);

		CComPtr<IDispatch>										   pDisp;
		CComPtr<IDispatch>										   pDisp2;
		CComQIPtr<IHTMLSelectionObject, &IID_IHTMLSelectionObject> spSelection;
		CComQIPtr<IHTMLTxtRange, &IID_IHTMLTxtRange>			   spTextRange;
		HRESULT 												   hr = pT->m_spBrowser->get_Document(&pDisp);
		CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2>			   spHTML;
		spHTML = pDisp;

		if (spHTML) {
			spHTML->get_selection(&spSelection);
			spSelection->createRange(&pDisp2);
			spTextRange = pDisp2;

			if (spTextRange) { // Non Frame && GetSelection
				spTextRange->get_text(&bstrHTML);

				if (bstrHTML)
					pT->searchEngines(LPCOLESTR(bstrHTML));

				else
					parseFramesCollection(pDisp , wID);
			} else {
				parseFramesCollection(pDisp , wID);
			}
		}
	}


private:
	// Overriable
	void searchEngines(const CString &strKeyWord )
	{
		T * 		pT		  = static_cast<T *>(this);
		CString 	strURL;
		int 		nMaxCount = 15;
		TCHAR		szGoogleCode[INTERNET_MAX_PATH_LENGTH];

		memset(szGoogleCode, 0, INTERNET_MAX_PATH_LENGTH);
		DWORD		dwCount   = INTERNET_MAX_PATH_LENGTH;
		DWORD		dwFlags	  = 0;

		{
			CIniFileI pr0( g_szIniFileName, _T("AddressBar") );
			pr0.QueryString(szGoogleCode, _T("EnterCtrl"), &dwCount);
			pr0.QueryValue( dwFlags, _T("") );
		}

		CIniFileI	pr( g_szIniFileName, _T("INUYA") );
		TCHAR		szRetString[INTERNET_MAX_PATH_LENGTH + 1];
		TCHAR		szBuff[32];

		for (int nItem = 0; nItem < nMaxCount; nItem++) {
			::_stprintf_s(szBuff, _T("Engine%i"), nItem);
			szRetString[0]= 0;
			DWORD dwCount = INTERNET_MAX_PATH_LENGTH;	//+++ * sizeof (TCHAR);

			if (pr.QueryString(szRetString, szBuff, &dwCount) == ERROR_SUCCESS) {
				strURL = szRetString + strKeyWord;
				DonutOpenFile( strURL, DonutGetStdOpenCreateFlag() );
			} else {
				if (nItem == 0) {
					strURL = szGoogleCode + strKeyWord;
					DonutOpenFile( strURL, DonutGetStdOpenCreateFlag() );
				}

				break;
			}
		}
	}
};

