// DownloadingListView.cpp

#include "stdafx.h"
#include "DownloadingListView.h"
#include "DownloadedListView.h"
#include "DownloadOptionDialog.h"
#include "../MtlWin.h"
#include "../MtlFile.h"
#include "../MtlCom.h"
#include "../HlinkDataObject.h"
#include "../DonutPFunc.h"
#include "DLListWindow.h"
#include "../MainFrame.h"

namespace {

/////////////////////////////////////////////////////////////////
/// �t�@�C�����ύX�_�C�A���O

class CRenameDialog : public CDialogImpl<CRenameDialog>
{
public:
	enum { IDD = IDD_RENAMEDIALOG };
	
	// Constructor
	CRenameDialog(LPCTSTR strOldFileName, LPCTSTR strFilePath) : m_strOldFileName(strOldFileName)
	{
		m_strFolder = Misc::GetDirName(CString(strFilePath)) + _T("\\");
	}
	
	CString	GetNewFileName() const { return m_strNewFileName; }
	CString GetNewFilePath() const { return m_strFolder + m_strNewFileName; }

	BEGIN_MSG_MAP( CRenameDialog )
		MSG_WM_INITDIALOG( OnInitDialog )
		COMMAND_ID_HANDLER_EX( IDOK, OnOk)
		COMMAND_ID_HANDLER_EX( IDCANCEL, OnCancel )
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		GetDlgItem(IDC_EDIT).SetWindowText(m_strOldFileName);
		WTL::CLogFont	lf;
		lf.SetMenuFont();
		GetDlgItem(IDC_EDIT).SetFont(lf.CreateFontIndirect());
		return 0;
	}

	void OnOk(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		m_strNewFileName = MtlGetWindowText(GetDlgItem(IDC_EDIT));
		if (m_strNewFileName.IsEmpty()) {
			MessageBox(_T("�t�@�C��������͂��Ă��������B"), NULL, MB_ICONERROR);
			return ;
		}
		if (MtlIsValidateFileName(m_strNewFileName) == false) {
			MessageBox(_T("�L���ȃt�@�C�����ł͂���܂���B\n�u\\/:*?\"<>|�v�̓t�@�C�����Ɋ܂߂邱�Ƃ͂ł��܂���B"), NULL, MB_ICONERROR);
			return ;
		}
		if (::PathFileExists(m_strFolder + m_strNewFileName)) {
			if (MessageBox(_T("�������Ƀt�@�C�������݂��܂��B\n�㏑�����܂����H"), NULL, MB_ICONQUESTION) == IDCANCEL)
				return ;
		}
		EndDialog(nID);
	}

	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog(nID);
	}


private:
	CString	m_strOldFileName;
	CString m_strFolder;
	CString m_strNewFileName;
};

};	// namespace


////////////////////////////////////////////////////////////////
// CDownloadingListView

// Constructor/Destructor
CDownloadingListView::CDownloadingListView() : 
	m_bTimer(false), 
	m_pItemPopup(nullptr),
	m_dwLastTime(0)
{ }

CDownloadingListView::~CDownloadingListView()
{
	m_ImageList.Destroy();
	m_ImgStop.Destroy();
}


void	CDownloadingListView::DoPaint(CDCHandle dc)
{
	CPoint ptOffset;
	GetScrollOffset(ptOffset);

	CRect rcClient;
	GetClientRect(rcClient);
	rcClient.bottom	+= ptOffset.y;

	CMemoryDC	memDC(dc, rcClient);
	HFONT hOldFont = memDC.SelectFont(m_Font);

	// �w�i��`��
	memDC.FillSolidRect(rcClient, RGB(255, 255, 255));

	std::size_t	size = m_vecpDLItem.size();
	for (auto it = m_vecpDLItem.begin(); it != m_vecpDLItem.end(); ++it) {
		CRect rcItem = (*it)->rcItem;
		rcItem.right = rcClient.right;

		memDC.SetBkMode(TRANSPARENT);
		if ((*it)->dwState & DLITEMSTATE_SELECTED) {
			static COLORREF SelectColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
			memDC.SetTextColor(SelectColor);
		} else {
			memDC.SetTextColor(0);
		}

		// �I���`��
		if ((*it)->dwState & DLITEMSTATE_SELECTED){
			memDC.FillRect(&rcItem, COLOR_HIGHLIGHT);
		}

		// �A�C�R���`��
		CRect rcImage = rcItem;
		CPoint ptImg(cxImageMargin, rcImage.top + cyImageMargin);
		m_ImageList.Draw(memDC, (*it)->nImgIndex, ptImg, ILD_NORMAL);

		// �t�@�C������`��
		CRect rcFileName = rcItem;
		rcFileName.left = cxFileNameMargin;
		rcFileName.top += cyFileNameMargin;
		memDC.DrawText((*it)->strFileName, -1, rcFileName, DT_SINGLELINE);

		// progress
		CRect rcProgress(cxFileNameMargin, rcItem.top + cyProgressMargin, rcItem.right - cleftProgressMargin, rcItem.top + cyProgressMargin + ProgressHeight);
		if (IsThemeNull() == false) {
			static int PROGRESSBAR  = _CheckOsVersion_VistaLater() ? PP_TRANSPARENTBAR : PP_BAR;
			static int PROGRESSBODY = _CheckOsVersion_VistaLater() ? PP_FILL		   : PP_CHUNK;
			if (IsThemeBackgroundPartiallyTransparent(PROGRESSBAR, 0))
				DrawThemeParentBackground(memDC, rcProgress);
			DrawThemeBackground(memDC, PROGRESSBAR, 0, rcProgress);
			CRect rcContent;
			GetThemeBackgroundContentRect(memDC, PROGRESSBAR, 0, rcProgress, rcContent);

			double Propotion = double((*it)->nProgress) / double((*it)->nProgressMax);
			int nPos = (int)((double)rcContent.Width() * Propotion);
			rcContent.right = rcContent.left + nPos;
			DrawThemeBackground(memDC, PROGRESSBODY, 0, rcContent);
		} else {
			memDC.DrawEdge(rcProgress, EDGE_RAISED, BF_ADJUST | BF_MONO | BF_RECT | BF_MIDDLE);
			double Propotion = double((*it)->nProgress) / double((*it)->nProgressMax);
			int nPos = (int)((double)rcProgress.Width() * Propotion);
			rcProgress.right = rcProgress.left + nPos;
			memDC.FillSolidRect(rcProgress, RGB(0, 255, 0));
		}

		// �����`��
		CRect rcDiscribe = rcItem;
		rcDiscribe.top += cyProgressMargin + ProgressHeight;
		rcDiscribe.left = cxFileNameMargin;
		memDC.DrawText((*it)->strText, -1, rcDiscribe, DT_SINGLELINE);

		// ��~�A�C�R���`��
		CPoint ptStop(rcClient.right - cxStopLeftSpace, rcItem.top + cyStopTopMargin);
		m_ImgStop.Draw(memDC, 0, ptStop, ILD_NORMAL);

		// ���Ƀ��C��������
		static COLORREF BorderColor = ::GetSysColor(COLOR_3DLIGHT);
		HPEN hPen = ::CreatePen(PS_SOLID, 1, BorderColor);
		HPEN hOldPen = memDC.SelectPen(hPen);
		memDC.MoveTo(CPoint(rcItem.left, rcItem.bottom));
		memDC.LineTo(rcItem.right, rcItem.bottom);
		memDC.SelectPen(hOldPen);
		::DeleteObject(hPen);
	}

	dc.SelectFont(hOldFont);
}


DROPEFFECT CDownloadingListView::OnDragEnter(IDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
	if (_MtlIsHlinkDataObject(pDataObject))
		return DROPEFFECT_LINK | DROPEFFECT_COPY;
	return DROPEFFECT_NONE;
}


DROPEFFECT CDownloadingListView::OnDragOver(IDataObject *pDataObject, DWORD dwKeyState, CPoint point, DROPEFFECT dropOkEffect)
{
	if (_MtlIsHlinkDataObject(pDataObject))
		return DROPEFFECT_LINK | DROPEFFECT_COPY;
	return DROPEFFECT_NONE;
}


DROPEFFECT CDownloadingListView::OnDrop(IDataObject *pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropEffectList, CPoint point)
{
	vector<CString>	vecUrl;
	if (GetDonutURLList(pDataObject, vecUrl) == false) {
		CString strText;
		if (   MtlGetHGlobalText(pDataObject, strText)
			|| MtlGetHGlobalText(pDataObject, strText, CF_SHELLURLW))
		{
			vecUrl.push_back(strText);
		}
	}

	if (vecUrl.empty() == false) {
		CDLListWindow* pwndDL = new CDLListWindow;
		pwndDL->Create(m_hWnd);
		pwndDL->SetDLList(vecUrl);
	}
	return DROPEFFECT_LINK | DROPEFFECT_COPY;
}



// Message map

int CDownloadingListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// �����X�N���[���o�[���폜
	ModifyStyle(WS_HSCROLL, 0);

	// ��ʍX�V�p�̃^�C�}�[���N��
	//SetTimer(1, 1000);

	static bool bInit = false;
	if (bInit)
		return 0;
	bInit = true;

	SetScrollSize(10, 10);
    SetScrollLine(10, 10);
    //SetScrollPage(100, 100);

	// �C���[�W���X�g�쐬
	m_ImageList.Create(32, 32, ILC_COLOR32 | ILC_MASK, 20, 1);

	// �f�t�H���g�̃A�C�R����ǂݍ���
	SHFILEINFO sfinfo;
	::SHGetFileInfo(_T("*.*"), FILE_ATTRIBUTE_NORMAL, &sfinfo, sizeof(SHFILEINFO)
		, SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
	int nImgIndex = m_ImageList.AddIcon(sfinfo.hIcon);	/* 0 */
	::DestroyIcon(sfinfo.hIcon);

	// ��~�A�C�R����ǂݍ���
	m_ImgStop.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	HICON hStopIcon = AtlLoadIcon(IDI_DLSTOP);
	m_ImgStop.AddIcon(hStopIcon);
	::DestroyIcon(hStopIcon);

	// �t�H���g��ݒ�
	WTL::CLogFont	logfont;
	logfont.SetMenuFont();
	m_Font = logfont.CreateFontIndirectW();

	// �c�[���`�b�v��ݒ�
	m_ToolTip.Create(m_hWnd);
	m_ToolTip.ModifyStyle(0, TTS_ALWAYSTIP);
	CToolInfo ti(TTF_SUBCLASS, m_hWnd);
	ti.hwnd = m_hWnd;
	m_ToolTip.AddTool(ti);
	m_ToolTip.Activate(TRUE);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 30 * 1000);
	m_ToolTip.SetMaxTipWidth(600);

	OpenThemeData(L"PROGRESS");

	RegisterDragDrop();

    return 0;
}

void CDownloadingListView::OnDestroy()
{
	if (m_bTimer)
		KillTimer(1);

	RevokeDragDrop();
}


void CDownloadingListView::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);

	if (size != CSize(0, 0)) {
		std::for_each(m_vecpDLItem.begin(), m_vecpDLItem.end(), [size](std::unique_ptr<DLItem>&	pItem) {
			pItem->rcItem.right = size.cx;
		});
		CSize	scrollsize;
		GetScrollSize(scrollsize);
		scrollsize.cx	= size.cx ? size.cx : 1;
		m_sizeClient.cx = scrollsize.cx;
		SetScrollSize(scrollsize, FALSE, false);
		SetScrollLine(10, 10);
	}
}

void CDownloadingListView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcClient;
	GetClientRect(rcClient);

	int nIndex = _HitTest(point);

	int nCount = (int)m_vecpDLItem.size();
	for (int i = 0; i < nCount; ++i) {
		DLItem& item = *m_vecpDLItem[i];
		if (item.dwState & DLITEMSTATE_SELECTED) 
			item.dwState &= ~DLITEMSTATE_SELECTED;
		if (i == nIndex) {
			CRect rcItem = _GetItemClientRect(i);
			CRect rcStop(CPoint(rcClient.right - cxStopLeftSpace, rcItem.top + cyStopTopMargin), CSize(cxStop, cyStop));
			if (rcStop.PtInRect(point)) {
				item.bAbort = true;	// DL���~����
				TRACEIN(_T("DL���L�����Z�����܂����I: %s"), m_vecpDLItem[i]->strFileName);
			} else {
				item.dwState |= DLITEMSTATE_SELECTED;
			}
		}
	}
	Invalidate(FALSE);
}

//------------------------------------
/// �E�N���b�N���j���[��\������
void CDownloadingListView::OnRButtonUp(UINT nFlags, CPoint point)
{
	int nIndex = _HitTest(point);
	if (nIndex == -1)
		return;

	int nCount = (int)m_vecpDLItem.size();
	for (int i = 0; i < nCount; ++i) {
		DLItem& item = *m_vecpDLItem[i];
		if (item.dwState & DLITEMSTATE_SELECTED) 
			item.dwState &= ~DLITEMSTATE_SELECTED;
		if (i == nIndex) 
			item.dwState |= DLITEMSTATE_SELECTED;
	}
	Invalidate(FALSE);

	CMenu	menu;
	menu.LoadMenu(IDM_DOWNLOADINGLISTVIEW);
	CMenu	submenu = menu.GetSubMenu(0);

	m_pItemPopup = m_vecpDLItem[nIndex].get();
	CPoint pt;
	::GetCursorPos(&pt);
	submenu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
}


void CDownloadingListView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) {
		DWORD dwNowTime = ::timeGetTime();
		DWORD dwTimeMargin = dwNowTime - m_dwLastTime;
		if (dwTimeMargin <= 0) {
			return;
		} else {
			m_dwLastTime = dwNowTime;
		}

		int	nMaxTotalSecondTime = 0;
		for (auto it = m_vecpDLItem.begin(); it != m_vecpDLItem.end(); ++it) {
			DLItem& item = *(*it);

			// (1.2 MB/sec)
			CString strTransferRate;
			int nProgressMargin = item.nProgress - item.nOldProgress;
			item.nOldProgress = item.nProgress;

			if (item.deq.size() >= 10)
				item.deq.pop_front();
			item.deq.push_back(make_pair(nProgressMargin, (int)dwTimeMargin));
			nProgressMargin = 0;
			int nTotalTime = 0;
			for (auto itq = item.deq.cbegin(); itq != item.deq.cend(); ++itq) {
				nProgressMargin += itq->first;
				nTotalTime		+= itq->second;
			}

			double dKbTransferRate = (double)nProgressMargin / (double)nTotalTime;	// kbyte / second
			double MbTransferRate = dKbTransferRate / 1000.0;
			if (MbTransferRate > 1) {
				::swprintf(strTransferRate.GetBuffer(30), _T(" (%.1lf MB/sec)"), MbTransferRate);
				strTransferRate.ReleaseBuffer();
			} else {
				::swprintf(strTransferRate.GetBuffer(30), _T(" (%.1lf KB/sec)"), dKbTransferRate);
				strTransferRate.ReleaseBuffer();
			}
			

			// �c�� 4 ��
			int nRestByte = item.nProgressMax - item.nProgress;
			if (nRestByte <= 0) {
				CString strDownloaded;
				double dMByte = (double)item.nProgress / (1000.0 * 1000.0);
				if (dMByte >= 1) {
					::swprintf(strDownloaded.GetBuffer(30), _T("%.1lf MB �_�E�����[�h"), dMByte);
					strDownloaded.ReleaseBuffer();
				} else {
					double dKByte = (double)item.nProgress / 1000.0;
					::swprintf(strDownloaded.GetBuffer(30), _T("%.1lf KB �_�E�����[�h"), dKByte);
					strDownloaded.ReleaseBuffer();
				}
				item.strText.Format(_T("�c�� ??? ���� �\ %s%s �\ %s"), strDownloaded, strTransferRate, item.strDomain);
			} else {
				if (dKbTransferRate > 0) {
					int nTotalSecondTime = static_cast<int>((nRestByte / 1000) / dKbTransferRate);	// �c�莞��(�b)
					if (nMaxTotalSecondTime < nTotalSecondTime)
						nMaxTotalSecondTime = nTotalSecondTime;
					int nhourTime	= nTotalSecondTime / (60 * 60);									// ����
					int nMinTime	= (nTotalSecondTime - (nhourTime * (60 * 60))) / 60;			// ��
					int nSecondTime = nTotalSecondTime - (nhourTime * (60 * 60)) - (nMinTime * 60);	// �b
					item.strText = _T("�c�� ");
					if (nhourTime > 0) {
						item.strText.Append(nhourTime) += _T(" ���� ");
					}
					if (nMinTime > 0) {
						item.strText.Append(nMinTime) += _T(" �� ");
					}
					item.strText.Append(nSecondTime) += _T(" �b �\ ");


					// 10.5 / 288 MB
					CString strDownloaded;
					double dMByte = (double)item.nProgressMax / (1000.0 * 1000.0);
					if (dMByte >= 1) {
						double DownloadMByte = (double)item.nProgress / (1000.0 * 1000.0);
						::swprintf(strDownloaded.GetBuffer(30), _T("%.1lf / %.1lf MB"), DownloadMByte, dMByte);
						strDownloaded.ReleaseBuffer();
					} else {
						double dKByte = (double)(*it)->nProgressMax / 1000.0;
						double DownloadKByte = (double)item.nProgress / 1000.0;
						::swprintf(strDownloaded.GetBuffer(30), _T("%.1lf / %.1lf KB"), DownloadKByte, dKByte);
						strDownloaded.ReleaseBuffer();
					}
					item.strText += strDownloaded + strTransferRate + _T(" �\ ") + item.strDomain;
				}
			}
		}
		/* �^�C�g���o�[�ɏ���\������ */
		if (nMaxTotalSecondTime > 0) {
			CString strTitle;
			strTitle.Format(_T("DL�A�C�e���� %d - �S�t�@�C����DL�I���܂Ŏc�� "), (int)m_vecpDLItem.size());
			int nhourTime	= nMaxTotalSecondTime / (60 * 60);									// ����
			int nMinTime	= (nMaxTotalSecondTime - (nhourTime * (60 * 60))) / 60;			// ��
			int nSecondTime = nMaxTotalSecondTime - (nhourTime * (60 * 60)) - (nMinTime * 60);	// �b
			if (nhourTime > 0) {
				strTitle.Append(nhourTime) += _T(" ���� ");
			}
			if (nMinTime > 0) {
				strTitle.Append(nMinTime) += _T(" �� ");
			}
			strTitle.Append(nSecondTime) += _T(" �b");
			GetTopLevelWindow().SetWindowText(strTitle);
		} else {
			GetTopLevelWindow().SetWindowText(_T("\0"));
		}
		Invalidate(FALSE);
	}
}

//----------------------------------------------------
/// ���X�g����폜����
LRESULT CDownloadingListView::OnRemoveFromList(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DLItem* pItem = (DLItem*)wParam;
	//CCustomBindStatusCallBack* pbscb = (CCustomBindStatusCallBack*)lParam;
	//DWORD dwThreadId = pbscb->GetThreadId();
	//if (dwThreadId) {
	//	::PostThreadMessage(dwThreadId, WM_DECREMENTTHREADREFCOUNT, 0, 0);
	//}

	//if (pItem->bAbort == false)
	//	delete pbscb;
	//pbscb->Release();
	
	// vector����폜����
	int nCount = (int)m_vecpDLItem.size();
	for (int i = 0; i < nCount; ++i) {
		if (m_vecpDLItem[i].get() == pItem) {
			m_vecpDLItem[i].release();
			m_vecpDLItem.erase(m_vecpDLItem.begin() + i);
			break;
		}
	}

	_RefreshList();

	// DLedView�ɒǉ�
	m_funcAddDownloadedItem(pItem);

	/* �S�t�@�C����DL�I�� */
	if (m_vecpDLItem.size() == 0) {
		KillTimer(1);		// ��ʍX�V�^�C�}�[���~
		m_bTimer = false;
		GetTopLevelWindow().SetWindowText(_T("\0"));
		if (CDLOptions::bCloseAfterAllDL) {	
			::PostMessage(GetTopLevelParent(), WM_CLOSE, 0, 0);	// �S�Ă�DL���I������̂ŕ���
		}
	}

	if (pItem->bAbort == false)
		CDLOptions::PlaySE();

	return 0;
}

LRESULT CDownloadingListView::OnAddToList(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bTimer == false) {	
		SetTimer(1, 1000);		// ��ʍX�V�^�C�}�[���J�n
		m_dwLastTime = ::timeGetTime();
		m_bTimer = true;
	}
	_AddItemToList((DLItem*)wParam);
	return 0;
}

LRESULT CDownloadingListView::OnTooltipGetDispInfo(LPNMHDR pnmh)
{
    LPNMTTDISPINFO pntdi = (LPNMTTDISPINFO)pnmh;
    if (pntdi->uFlags & TTF_IDISHWND) {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		int nIndex = _HitTest(pt);
		if (nIndex != -1) {
			CRect rcItem = _GetItemClientRect(nIndex);
			CRect rcFileName = rcItem;
			rcFileName.left = cxFileNameMargin;
			rcFileName.top += cyFileNameMargin;
			rcFileName.bottom	-= (ItemHeight + UnderLineHeight) - cyProgressMargin;
			if (rcFileName.PtInRect(pt)) {
				pntdi->lpszText = m_vecpDLItem[nIndex]->strFileName.GetBuffer(0);
			} else {
				CRect	rcStop(CPoint(rcItem.right - cxStopLeftSpace, rcItem.top + cyStopTopMargin), CSize(cxStop, cyStop));
				if (rcStop.PtInRect(pt)) {
					::wcscpy_s(pntdi->szText, L"�L�����Z��");
				} else {
					pntdi->lpszText = m_vecpDLItem[nIndex]->strURL.GetBuffer(0);
				}
			}
		} else {
			pntdi->lpszText = NULL;
		}
    }

    return 0;
}

void CDownloadingListView::OnMouseMove(UINT nFlags, CPoint pt)
{
	SetMsgHandled(FALSE);

	enum HoldPosition {
		HOLD_NONE = 0,
		HOLD_NAME = 1,
		HOLD_STOP = 2,
	};
	static HoldPosition OldPosition = HOLD_NONE;
	static int nOldIndex = -1;
	HoldPosition	Position = HOLD_NONE;
	int nIndex = _HitTest(pt);
	if (nIndex != -1) {
		CRect rcItem = _GetItemClientRect(nIndex);
		CRect rcFileName = rcItem;
		rcFileName.left = cxFileNameMargin;
		rcFileName.top += cyFileNameMargin;
		rcFileName.bottom	-= (ItemHeight + UnderLineHeight) - cyProgressMargin;
		if (rcFileName.PtInRect(pt)) {
			Position = HOLD_NAME;
		} else {
			CRect	rcStop(CPoint(rcItem.right - cxStopLeftSpace, rcItem.top + cyStopTopMargin), CSize(cxStop, cyStop));
			if (rcStop.PtInRect(pt)) {
				Position = HOLD_STOP;
			} else {
				
			}
		}
	}

	if (nIndex != nOldIndex || Position != OldPosition) {
		nOldIndex	= nIndex;
		OldPosition	= Position;
		m_ToolTip.Activate(FALSE);
		m_ToolTip.Activate(TRUE);
	}
}

//---------------------------------------------
/// DL���̃A�C�e���̖��O��ύX����
void	CDownloadingListView::OnRenameDLItem(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_pItemPopup == nullptr)
		return;

	CRenameDialog	dlg(m_pItemPopup->strFileName, m_pItemPopup->strFilePath);
	if (dlg.DoModal(m_hWnd) != IDOK)
		return;

	CString strOldFilePath = m_pItemPopup->strFilePath;
	m_pItemPopup->strFileName = dlg.GetNewFileName();
	m_pItemPopup->strFilePath = dlg.GetNewFilePath();

	int nCount = (int)m_vecpDLItem.size();
	for (int i = 0; i < nCount; ++i) {
		if (m_pItemPopup == m_vecpDLItem[i].get()) {	// �܂�DL��
			InvalidateRect(_GetItemClientRect(i), FALSE);
			m_pItemPopup = nullptr;
			return ;
		}
	}
	
	// DL�͏I����Ă����̂ŕ��ʂɃ��l�[��
	::MoveFileEx(strOldFilePath, m_pItemPopup->strFilePath, MOVEFILE_REPLACE_EXISTING);

	/* �G�N�X�v���[���[�Ƀt�@�C���̕ύX�ʒm */
	::SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_PATH, static_cast<LPCTSTR>(strOldFilePath), static_cast<LPCTSTR>(m_pItemPopup->strFilePath));
	
	m_pItemPopup = nullptr;
}

//---------------------------------------------------------
/// �ۑ���̃t�H���_���J��
void	CDownloadingListView::OnOpenSaveFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_pItemPopup == nullptr)
		return ;

	OpenFolderAndSelectItem(m_pItemPopup->strIncompleteFilePath);

	m_pItemPopup = nullptr;
}

//-------------------------------------------------------------
/// �_�E�����[�h�����y�[�W��\������
void	CDownloadingListView::OnOpenReferer(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_pItemPopup == nullptr)
		return ;

	if (m_pItemPopup->strReferer.IsEmpty())
		return ;
	DonutOpenFile(m_pItemPopup->strReferer, D_OPENFILE_ACTIVATE);

	m_pItemPopup = nullptr;
}



//----------------------------------------------
/// ���X�g�r���[�ɒǉ�
void	CDownloadingListView::_AddItemToList(DLItem* pItem)
{
	// �擪�ɃA�C�e����ǉ�
	m_vecpDLItem.insert(m_vecpDLItem.begin(), std::unique_ptr<DLItem>(pItem));

	// �A�C�R����ǉ�
	_AddIcon(pItem);

	_RefreshList();
}

//----------------------------------------------
/// �A�C�e���̍��[�ɒu���A�C�R����ǂݍ���
void CDownloadingListView::_AddIcon(DLItem *pItem)
{
	CString strExt = Misc::GetFileExt(pItem->strFileName);
	if (strExt.IsEmpty()) {
		pItem->nImgIndex = 0;
	} else {
		auto it = m_mapImgIndex.find(strExt);
		if (it != m_mapImgIndex.end()) {	// ��������
			pItem->nImgIndex = it->second;
		} else {							// ������Ȃ�����
			SHFILEINFO sfinfo;
			CString strFind;
			strFind.Format(_T("*.%s"), strExt);
			// �A�C�R�����擾
			::SHGetFileInfo(strFind, FILE_ATTRIBUTE_NORMAL, &sfinfo, sizeof(SHFILEINFO)
				, SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
			ATLASSERT(sfinfo.hIcon);
			// �C���[�W���X�g�ɃA�C�R����ǉ�
			int nImgIndex = m_ImageList.AddIcon(sfinfo.hIcon);
			// �g���q�ƃC���[�W���X�g�̃C���f�b�N�X���֘A�t����
			m_mapImgIndex.insert(std::pair<CString, int>(strExt, nImgIndex));
			::DestroyIcon(sfinfo.hIcon);

			pItem->nImgIndex = nImgIndex;
		}
	}
}


//-----------------------------------------------
/// ���X�g���X�V����
void	CDownloadingListView::_RefreshList()
{
	CRect rcClient;
	GetClientRect(rcClient);

	int cySize = 0;
	for (auto it = m_vecpDLItem.begin(); it != m_vecpDLItem.end(); ++it) {
		// �A�C�e���̈ʒu��ݒ�
		(*it)->rcItem.top = cySize;
		cySize += ItemHeight;
		(*it)->rcItem.bottom = cySize;
		cySize += UnderLineHeight;

		(*it)->rcItem.right = rcClient.right;
	}

	CSize	size;
	size.cx	= rcClient.right;
	size.cy	= m_vecpDLItem.size() ? cySize : 1;
	SetScrollSize(size, TRUE, false);
	SetScrollLine(10, 10);

	Invalidate(FALSE);
}
//----------------------------------------------
/// pt�ɂ���DL�A�C�e���̃C���f�b�N�X��Ԃ�
int		CDownloadingListView::_HitTest(CPoint pt)
{
	int nCount = (int)m_vecpDLItem.size();
	for (int i = 0; i < nCount; ++i) {
		//if (m_vecpDLItem[i]->rcItem.PtInRect(pt)) 
		if (_GetItemClientRect(i).PtInRect(pt))
			return i;
	}
	return -1;
}

//----------------------------------------------
/// nIndex�̃A�C�e���̃N���C�A���g���W�ł͈̔͂�Ԃ�
CRect	CDownloadingListView::_GetItemClientRect(int nIndex)
{
	ATLASSERT(0 <= nIndex && nIndex < (int)m_vecpDLItem.size());

	CPoint	ptOffset;
	GetScrollOffset(ptOffset);

	CRect rcItem = m_vecpDLItem[nIndex]->rcItem;
	rcItem.top		-= ptOffset.y;
	rcItem.bottom	-= ptOffset.y;
	return rcItem;
}






