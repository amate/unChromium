/**
 *	@file	DonutLinkBarCtrl.inl
 *	@brief	リンクバーの実装
 */


CDonutLinkBarCtrl::Impl::Impl() : 
	m_nHotIndex(-1), 
	m_nPressedIndex(-1),
	m_pSubMenu(nullptr)
{	}


void	CDonutLinkBarCtrl::Impl::SetFont(HFONT hFont)
{
	m_font = hFont;
	WTL::CLogFont	lf;
	m_font.GetLogFont(&lf);
	LONG fontHeight = lf.GetHeight();
	fontHeight += kTopBottomPadding * 2;
	MoveWindow(0, 0, -1, fontHeight);

	_RefreshBandInfo();
	_UpdateItemPosition();
}


// Overrides
void CDonutLinkBarCtrl::Impl::DoPaint(CDCHandle dc)
{
	HWND	hWnd = GetParent();
	CPoint	pt;
	MapWindowPoints(hWnd, &pt, 1);
	::OffsetWindowOrgEx( (HDC)dc.m_hDC, pt.x, pt.y, NULL );
	LRESULT lResult = ::SendMessage(hWnd, WM_ERASEBKGND, (WPARAM)dc.m_hDC, 0L);
	::SetWindowOrgEx((HDC)dc.m_hDC, 0, 0, NULL);

	dc.SetBkMode(TRANSPARENT);
	HFONT hFontPrev = dc.SelectFont(m_font);
	
	CRect rcClient;
	GetClientRect(&rcClient);
	for (auto it = m_BookmarkList.cbegin(); it != m_BookmarkList.cend(); ++it) {
		LinkItem& item = *it->get();
		CRect rcItem = item.rcItem;
		if (IsThemeNull() == false) {
			int nState = TS_NORMAL;
			if (item.state == item.kItemHot)
				nState = TS_HOT;
			else if (item.state == item.kItemPressed)
				nState = TS_PRESSED;
			DrawThemeBackground(dc, TP_BUTTON, nState, &rcItem);
		}
		rcItem.left	+= kRightLeftPadding;
		rcItem.right-= kRightLeftPadding;
		dc.DrawText(item.strName, item.strName.GetLength(), &rcItem, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

	}
	//if (m_BookmarkList.empty())
	//	dc.DrawText(_T("none"), -1, CRect(0, kTopBottomPadding, 200, 20), DT_TOP);

	dc.SelectFont(hFontPrev);
}


void CDonutLinkBarCtrl::Impl::OnTrackMouseMove(UINT nFlags, CPoint pt)
{
	if (auto value = _HitTest(pt)) {
		if (m_pSubMenu) {
			LinkFolderPtr pFolder = m_BookmarkList[value.get()]->pFolder;
			if (pFolder) {
				_HotItem(-1);
				if (pFolder != m_pSubMenu->GetLinkFolderPtr()) {
					_DoPopupSubMenu(value.get());
				} else {
					_PressItem(value.get());
				}
				return ;
			}
		}
		_HotItem(value.get());
	} else {
		_HotItem(-1);
	}
}

void CDonutLinkBarCtrl::Impl::OnTrackMouseLeave()
{
	if (m_pSubMenu)
		return ;
	_HotItem(-1);
}

// IDropTargetImpl
DROPEFFECT CDonutLinkBarCtrl::Impl::OnDragOver(IDataObject *pDataObject, DWORD dwKeyState, CPoint point, DROPEFFECT dropOkEffect)
{
	CPoint	ptRightBottom;
	CRect	rcClient;
	GetClientRect(&rcClient);
	ptRightBottom.y	= rcClient.bottom;
	if (auto value = _HitTest(point)) {
		LinkItem& item = *m_BookmarkList[value.get()];
		CRect rcItem	= item.rcItem;
		if (item.pFolder) {
			rcItem.right	= rcItem.left + kDragInsertHitWidthOnFolder;
			if (rcItem.PtInRect(point)) {
				ptRightBottom.x	= m_BookmarkList[value.get()]->rcItem.left;
			} else {
				rcItem.right	= item.rcItem.right;
				rcItem.left		= rcItem.right - kDragInsertHitWidthOnFolder;
				if (rcItem.PtInRect(point)) {
					ptRightBottom.x	= m_BookmarkList[value.get()]->rcItem.right;
				} else {
					// フォルダ上
					_ClearInsertionEdge();

					if (m_pSubMenu == nullptr 
					|| m_pSubMenu->GetLinkFolderPtr() != item.pFolder)
						_DoPopupSubMenu(value.get());
					return DROPEFFECT_MOVE | DROPEFFECT_LINK;
				}
			}
		} else {
			int nItemHalfWidth = rcItem.Width() / 2;
			rcItem.right	= rcItem.left + nItemHalfWidth;
		
			if (rcItem.PtInRect(point)) {	// アイテムの左側にポインタがある
				ptRightBottom.x	= m_BookmarkList[value.get()]->rcItem.left;
			} else {
				ptRightBottom.x	= m_BookmarkList[value.get()]->rcItem.right;
			}
		}
	} else {
		if (m_BookmarkList.size() > 0) {
			ptRightBottom.x = m_BookmarkList[m_BookmarkList.size() - 1]->rcItem.right;
		} 
	}

	_DrawInsertEdge(ptRightBottom);

	if (m_ptInvalidateOnDrawingInsertionEdge != ptRightBottom) {
		_ClearInsertionEdge();
		m_ptInvalidateOnDrawingInsertionEdge = ptRightBottom;
	}

	_CloseSubMenu();

	return DROPEFFECT_MOVE | DROPEFFECT_LINK;
#if 0
	_hitTestFlag flag;
	int nIndex = _HitTestOnDragging(flag, point);

	_DrawInsertionEdge(flag, nIndex);

	if ( flag == htItem && _IsSameIndexDropped(nIndex) )
		return DROPEFFECT_NONE;

	if (!m_bDragFromItself)
		return _MtlStandardDropEffect(dwKeyState) | _MtlFollowDropEffect(dropOkEffect) | DROPEFFECT_COPY;
#endif
	//return _MtlStandardDropEffect(dwKeyState);
}

DROPEFFECT CDonutLinkBarCtrl::Impl::OnDrop(IDataObject *pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropEffectList, CPoint point)
{
	_ClearInsertionEdge();

	return DROPEFFECT_MOVE | DROPEFFECT_LINK;
}

void	CDonutLinkBarCtrl::Impl::OnDragLeave()
{
	_ClearInsertionEdge();
}


 int CDonutLinkBarCtrl::Impl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	OpenThemeData(VSCLASS_TOOLBAR);

	ATLVERIFY(RegisterDragDrop());

	CString LinkBookmarkFilePath = Misc::GetExeDirectory() + _T("LinkBookmark.xml");
	try {
		std::wifstream	filestream(LinkBookmarkFilePath);
		if (!filestream)
			return 0;

		filestream.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>));
		wptree	pt;
		read_xml(filestream, pt);

		if (auto optChild = pt.get_child_optional(L"LinkBookmark")) {
			wptree ptChild = optChild.get();
			_AddLinkItem(&m_BookmarkList, ptChild);
		}
	} catch (const boost::property_tree::ptree_error& error) {
		CString strError = _T("LinkBookmark.xmlの読み込みに失敗\n");
		strError += error.what();
		MessageBox(strError, NULL, MB_ICONERROR);
	}

	_UpdateItemPosition();

	g_hWndLinkBar = m_hWnd;

	return 0;
}

 void CDonutLinkBarCtrl::Impl::OnDestroy()
 {
	 RevokeDragDrop();
 }

 void CDonutLinkBarCtrl::Impl::OnSize(UINT nType, CSize size)
 {
	 DefWindowProc();
	 _RefreshBandInfo();
 }

void CDonutLinkBarCtrl::Impl::OnLButtonDown(UINT nFlags, CPoint point)
{
	int nIndex = -1;
	if (auto value = _HitTest(point))
		nIndex = value.get();
	_DoDragDrop(point, nFlags, nIndex);
#if 0
	if (auto value = _HitTest(point)) {
		LinkFolderPtr pFolder = m_BookmarkList[value.get()]->pFolder;
		if (pFolder) {
			if (m_pSubMenu) {
				// 既に開いてるサブメニューを再び開こうとしたら閉じる
				if (m_pSubMenu->GetLinkFolderPtr() == pFolder) {
					_CloseSubMenu();
					_PressItem(value.get());
					return ;
				}
			}
			_DoPopupSubMenu(value.get());
			return ;
		}
		_HotItem(-1);
		_PressItem(value.get());
		_CloseSubMenu();
		// URLを開く
		DonutOpenFile(m_BookmarkList[value.get()]->strUrl);
	} else {
		_CloseSubMenu();
	}
#endif
}

void CDonutLinkBarCtrl::Impl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_pSubMenu)
		return ;

	_PressItem(-1);
	if (auto value = _HitTest(point))
		_HotItem(value.get());
}

LRESULT CDonutLinkBarCtrl::Impl::OnCloseBaseSubMenu(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CRect rc;
	GetWindowRect(&rc);
	CPoint	pt;
	GetCursorPos(&pt);
	if (rc.PtInRect(pt)) {
		//ScreenToClient(&pt);
		//OnLButtonDown(0, pt);
	} else {
		_CloseSubMenu();
	}
	return 0;
}

 //-----------------------------------
/// リバーを更新する
void	CDonutLinkBarCtrl::Impl::_RefreshBandInfo()
{
	HWND		  hWndReBar = GetParent();
	CReBarCtrl	  rebar(hWndReBar);

	static UINT wID = 0;
	if (wID == 0) {
		REBARBANDINFO rb = { sizeof (REBARBANDINFO) };
		rb.fMask	= RBBIM_CHILD | RBBIM_ID;
		int nCount	= rebar.GetBandCount();
		for (int i = 0; i < nCount; ++i) {
			rebar.GetBandInfo(i, &rb);
			if (rb.hwndChild == m_hWnd) {
				wID = rb.wID;
				break;
			}
		}
		return;
	}

	int	nIndex = rebar.IdToIndex( wID );
	if ( nIndex == -1 ) 
		return;				// 設定できないので帰る

	REBARBANDINFO rbBand = { sizeof (REBARBANDINFO) };
	rbBand.fMask  = RBBIM_CHILDSIZE;
	rebar.GetBandInfo(nIndex, &rbBand);
	{
		// Calculate the size of the band
		rbBand.cxMinChild = 0;

		WTL::CLogFont	lf;
		m_font.GetLogFont(&lf);
		LONG fontHeight = lf.GetHeight();
		fontHeight += kTopBottomPadding * 2;
		rbBand.cyMinChild = fontHeight;

		rebar.SetBandInfo(nIndex, &rbBand);
	}
}

void	CDonutLinkBarCtrl::Impl::_AddLinkItem(LinkFolderPtr pFolder, wptree pt)
{
	for (auto it = pt.begin(); it != pt.end(); ++it) {
		if (it->first == L"<xmlattr>")
			continue;
		wptree& ptItem = it->second;
		unique_ptr<LinkItem>	pItem(new LinkItem);
		pItem->strName	= ptItem.get<std::wstring>(L"<xmlattr>.name").c_str();
		if (it->first == L"item") {
			pItem->strUrl	= ptItem.get<std::wstring>(L"<xmlattr>.url").c_str();
		} else if (it->first == L"folder") {
			pItem->pFolder = new LinkFolder;
			_AddLinkItem(pItem->pFolder, ptItem);
		}
		pFolder->push_back(std::move(pItem));
	}
}

void	CDonutLinkBarCtrl::Impl::_UpdateItemPosition()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int nLeftPos = kLeftMargin;
	for (auto it = m_BookmarkList.begin(); it != m_BookmarkList.end(); ++it) {
		int nTextWidth = MTL::MtlComputeWidthOfText(it->get()->strName, m_font);
		if (kMaxItemTextWidth < nTextWidth) {
			nTextWidth = kMaxItemTextWidth;			
		}
		CRect&	rcItem = it->get()->rcItem;
		rcItem.left	= nLeftPos;
		rcItem.bottom	= rcClient.bottom;
		rcItem.right	= nLeftPos + nTextWidth + (kRightLeftPadding * 2);
		nLeftPos = rcItem.right;
	}
	Invalidate(FALSE);
}

boost::optional<int>	CDonutLinkBarCtrl::Impl::_HitTest(const CPoint& point)
{
	int nCount = (int)m_BookmarkList.size();
	for (int i = 0; i < nCount; ++i) {
		if (m_BookmarkList[i]->rcItem.PtInRect(point))
			return i;
	}
	return boost::none;
}

void	CDonutLinkBarCtrl::Impl::_HotItem(int nNewHotIndex)
{
	// clean up
	if ( _IsValidIndex(m_nHotIndex) ) {
		LinkItem& item = *m_BookmarkList[m_nHotIndex];
		if ( item.ModifyState(item.kItemNormal) )
			InvalidateRect(item.rcItem);
	}

	m_nHotIndex = nNewHotIndex;

	if ( _IsValidIndex(m_nHotIndex) ) {
		LinkItem& item = *m_BookmarkList[m_nHotIndex];
		//if (item.state != item.kItemPressed) {
			if ( item.ModifyState(item.kItemHot) )
				InvalidateRect(item.rcItem);
		//}
	}
}

void	CDonutLinkBarCtrl::Impl::_PressItem(int nPressedIndex)
{
	// clean up prev
	if ( _IsValidIndex(m_nPressedIndex) ) {
		LinkItem& item = *m_BookmarkList[m_nPressedIndex];
		if ( item.ModifyState(item.kItemNormal) )
			InvalidateRect(item.rcItem);
	}

	m_nPressedIndex = nPressedIndex;

	if ( _IsValidIndex(m_nPressedIndex) ) {
		LinkItem& item = *m_BookmarkList[m_nPressedIndex];
		if ( item.ModifyState(item.kItemPressed) )
			InvalidateRect(item.rcItem);
	}
}

void	CDonutLinkBarCtrl::Impl::_CloseSubMenu()
{
	if (m_pSubMenu) {
		m_pSubMenu->DestroyWindow();
		m_pSubMenu = nullptr;
	}
}

void	CDonutLinkBarCtrl::Impl::_DoPopupSubMenu(int nIndex)
{
	ATLASSERT(_IsValidIndex(nIndex));
	_CloseSubMenu();

	LinkFolderPtr	pFolder = m_BookmarkList[nIndex]->pFolder;
	ATLASSERT(pFolder);
	m_pSubMenu = new CMenuWindow(pFolder);
	CRect	rcWindow;
	rcWindow.right	= m_pSubMenu->ComputeWindowWidth();
	rcWindow.bottom	= m_pSubMenu->ComputeWindowHeight();
	CRect	rcButton = m_BookmarkList[nIndex]->rcItem;
	ClientToScreen(&rcButton);
	rcWindow.MoveToXY(rcButton.left, rcButton.bottom);
	m_pSubMenu->Create(GetDesktopWindow(), rcWindow, NULL, /*WS_VISIBLE | */WS_POPUP | WS_BORDER, WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
	m_pSubMenu->ShowWindow(SW_SHOWNOACTIVATE);

	_PressItem(nIndex);
}


void	CDonutLinkBarCtrl::Impl::_DoDragDrop(const CPoint& pt, UINT nFlags, int nIndex)
{
	_PressItem(nIndex);
	UpdateWindow();
	if ( PreDoDragDrop(m_hWnd) ) {
		_HotItem(-1); 								// clean up hot item		
		_PressItem(nIndex);

		CComPtr<IDataObject> spDataObject;
		auto funcGetDataObject = [&](CComPtr<IDataObject>& spDataObject) -> HRESULT {
			HRESULT hr	= CHlinkDataObject::_CreatorClass::CreateInstance(NULL,IID_IDataObject, (void **) &spDataObject);
			if ( FAILED(hr) ) {
				return E_NOTIMPL;
			}

			CHlinkDataObject *pHlinkDataObject = NULL;		// this is hack, no need to release

			hr = spDataObject->QueryInterface(IID_NULL, (void **) &pHlinkDataObject);
			if ( SUCCEEDED(hr) ) {
				LinkItem& item = *m_BookmarkList[nIndex];
				pHlinkDataObject->m_arrNameAndUrl.Add( std::make_pair(item.strName, item.strUrl) );
			}
			return S_OK;
		};
		HRESULT hr = funcGetDataObject(spDataObject);
		if ( SUCCEEDED(hr) ) {
			//m_bDragFromItself = true;
			DROPEFFECT dropEffect = DoDragDrop(spDataObject, DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK);
			//m_bDragFromItself = false;
		}
		_PressItem(-1);

	} else {	// Drag操作をしていなかったのでアイテムを開く
		//SetCurSel(nIndex, true);
	}
}


void	CDonutLinkBarCtrl::Impl::_DrawInsertEdge(const CPoint& ptRightBottom)
{
	const CPoint& pt = ptRightBottom;
	CClientDC	 dc(m_hWnd);
	CBrush		 hbr;
	hbr.CreateSolidBrush( ::GetSysColor(COLOR_3DDKSHADOW) );
	dc.SetBrushOrg(pt.x, 0);
	HBRUSH hbrOld = dc.SelectBrush(hbr);

	POINT pts[] = { 
		{ pt.x , 0 }, 
		{ pt.x , pt.y }, 
		{ pt.x + 1, pt.y },
		{ pt.x + 1, 0 },
	};
	dc.Polygon( pts, _countof(pts) );

	dc.SelectBrush(hbrOld);
}

void	CDonutLinkBarCtrl::Impl::_ClearInsertionEdge()
{
	if (m_ptInvalidateOnDrawingInsertionEdge != CPoint()) {
		CRect rc;
		rc.left	= m_ptInvalidateOnDrawingInsertionEdge.x;
		rc.right= m_ptInvalidateOnDrawingInsertionEdge.x + 2;
		rc.bottom= m_ptInvalidateOnDrawingInsertionEdge.y;
		InvalidateRect(&rc);
		UpdateWindow();
		m_ptInvalidateOnDrawingInsertionEdge.SetPoint(0, 0);
	}
}





















