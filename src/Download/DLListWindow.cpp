/**
*	@file	DLListWindow.cpp
*/

#include "stdafx.h"
#include "DLListWindow.h"
#include "../MtlWin.h"
#include "../IniFile.h"
#include "../HlinkDataObject.h"
#include "DownloadOptionDialog.h"
#include "DownloadManager.h"

/////////////////////////////////////////////////////
// CDLListWindow

/// DL����URL���X�g��ݒ�
void CDLListWindow::SetDLList(const std::vector<CString>& vecURL)
{
	int nCount = (int)vecURL.size();
	for (int i = 0; i < nCount; ++i) {
		m_DLList.AddItem(i, 0, vecURL[i]);
	}
	_SetTitle();
}

// Overrides

/// View�E�B���h�E�ɃL�[�������̂Ŏ�����TranslateMessage��DispatchMessage���Ă�
BOOL CDLListWindow::PreTranslateMessage(MSG* pMsg)
{
	UINT msg = pMsg->message;
	if (m_editDLFolder.m_hWnd == pMsg->hwnd) {
		if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP || msg == WM_KEYDOWN) {
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
	} else if (m_DLList.m_hWnd == pMsg->hwnd) {
		if (msg == WM_KEYDOWN && pMsg->wParam == VK_DELETE) {	// ���X�g�r���[��delete�L�[�����ăA�C�e���폜����
			int nIndex = -1;
			vector<int> vecIndex;
			int nCount = m_DLList.GetItemCount();
			for (int i = 0; i < nCount; ++i) {
				nIndex = m_DLList.GetNextItem(nIndex, LVNI_SELECTED);
				if (nIndex == -1)
					break;

				vecIndex.push_back(nIndex);
			}
			for (auto rit = vecIndex.rbegin(); rit != vecIndex.rend(); ++rit) {
				m_DLList.DeleteItem(*rit);
			}
			_SetTitle();
		}
	}
	return FALSE;
}


DROPEFFECT CDLListWindow::OnDragEnter(IDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
	if (_MtlIsHlinkDataObject(pDataObject))
		return DROPEFFECT_LINK | DROPEFFECT_COPY;
	return DROPEFFECT_NONE;
}


DROPEFFECT CDLListWindow::OnDragOver(IDataObject *pDataObject, DWORD dwKeyState, CPoint point, DROPEFFECT dropOkEffect)
{
	if (_MtlIsHlinkDataObject(pDataObject))
		return DROPEFFECT_LINK | DROPEFFECT_COPY;
	return DROPEFFECT_NONE;
}


DROPEFFECT CDLListWindow::OnDrop(IDataObject *pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropEffectList, CPoint point)
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
		SetDLList(vecUrl);
	}
	return DROPEFFECT_LINK | DROPEFFECT_COPY;
}





BOOL	CDLListWindow::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_nDownloading	= 0;
	WM_DLCOMPLETE	= ::RegisterWindowMessage(REGISTERMESSAGE_DLCOMPLETE);

	DoDataExchange(DDX_LOAD);

	m_editDLFolder = m_cmbDLFolder.GetDlgItem(1001);

	// �_�C�A���O���T�C�Y������
    DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);

	for (int i = 1; i <= 5; ++i) {
		CString str;
		str.Append(i);
		m_cmbParallelDL.AddString(str);
	}

	/* �ݒ�𕜌����� */
	CIniFileI pr(CDLOptions::s_DLIniFilePath, _T("DLList"));
	m_cmbParallelDL.SetCurSel(pr.GetValue(_T("ParallelDL"), 0));

	CRect rcWindow;
	rcWindow.top	= pr.GetValue(_T("top"), -1);
	rcWindow.left	= pr.GetValue(_T("left"), -1);
	rcWindow.right	= pr.GetValue(_T("right"), -1);
	rcWindow.bottom	= pr.GetValue(_T("bottom"), -1);
	if (rcWindow != CRect(-1, -1, -1, -1))
		MoveWindow(rcWindow);	// �ʒu�𕜌�

	// ���X�g�r���[������
	m_DLList.InsertColumn(0, _T("URL"));
	m_DLList.SetColumnWidth(0, pr.GetValue(_T("ColumnWidthURL"), 300));


	int nCount = (int)CDLOptions::s_vecImageDLFolderHistory.size();
	for (int i = 0; i < nCount; ++i) {
		m_cmbDLFolder.AddString(CDLOptions::s_vecImageDLFolderHistory[i]);
	}

	// ImageDLFolder�̃p�X�ɐݒ�
	m_cmbDLFolder.SetWindowText(CDLOptions::strImgDLFolderPath);


	m_cmbDLOption.AddString(_T("�㏑���̊m�F������"));
	m_cmbDLOption.AddString(_T("�㏑���̊m�F�����Ȃ�"));
	m_cmbDLOption.AddString(_T("�A�Ԃ�t����"));
	m_cmbDLOption.SetCurSel(pr.GetValue(_T("DLOption"), 0));

	RegisterDragDrop();

	CMessageLoop *pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	return FALSE;
}

void	CDLListWindow::OnClose()
{
	/* �ݒ��ۑ����� */
	CIniFileO	pr(CDLOptions::s_DLIniFilePath, _T("DLList"));
	
	CRect rc;
	GetWindowRect(&rc);
	pr.SetValue(rc.top, _T("top"));
	pr.SetValue(rc.left, _T("left"));
	pr.SetValue(rc.right, _T("right"));
	pr.SetValue(rc.bottom, _T("bottom"));

	pr.SetValue(m_cmbParallelDL.GetCurSel(), _T("ParallelDL"));

	pr.SetValue(m_DLList.GetColumnWidth(0), _T("ColumnWidthURL"));

	CString strPath = MtlGetWindowText(m_cmbDLFolder);
	CDLOptions::_SavePathHistory(strPath, CDLOptions::s_vecImageDLFolderHistory);

	pr.SetValue(m_cmbDLOption.GetCurSel(), _T("DLOption"));

	RevokeDragDrop();

	CMessageLoop *pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);

	DestroyWindow();
}

/// [...] �t�H���_�I���_�C�A���O��\������DL��̃t�H���_��ݒ肷��
void	CDLListWindow::OnGetDLFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CShellFileOpenDialog	dlg(NULL, FOS_PICKFOLDERS);
	if (dlg.IsNull()) {
		CFolderDialog olddlg;
		if (olddlg.DoModal(m_hWnd) == IDOK) {
			m_cmbDLFolder.SetWindowText(olddlg.GetFolderPath());
		}

	} else {
		if (dlg.DoModal(m_hWnd) == IDOK) {
			CString strFilePath;
			dlg.GetFilePath(strFilePath);
			m_cmbDLFolder.SetWindowText(strFilePath);
		}
	}
}

/// DL���J�n����
void	CDLListWindow::OnDLStart(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_strDLFolder = MtlGetWindowText(m_cmbDLFolder);
	MtlMakeSureTrailingBackSlash(m_strDLFolder);

	GetDlgItem(IDC_DLSTART).EnableWindow(FALSE);

	// DL�J�n
	_DLStart();
}

void	CDLListWindow::OnDLFinish(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	PostMessage(WM_CLOSE);
}

/// ����DL����ς����Ƃ�
void	CDLListWindow::OnSelChangeParallelDL(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (GetDlgItem(IDC_DLSTART).IsWindowEnabled() == FALSE)
		_DLStart();
}

/// DLManager�����̃t�@�C����DL�����ʒm
LRESULT CDLListWindow::OnDLComplete(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	--m_nDownloading;
	_DLStart();
	if (m_DLList.GetItemCount() == 0)
		PostMessage(WM_CLOSE);	// �I������

	return 0;
}

void	CDLListWindow::_DLStart()
{
	DWORD	dwOption = 0;
	int nOption = m_cmbDLOption.GetCurSel();
	switch (nOption) {
	case 0: dwOption = DLO_OVERWRITEPROMPT; break;
	case 2: dwOption = DLO_USEUNIQUENUMBER; break;
	}

	int nMaxDL	= m_cmbParallelDL.GetCurSel() + 1;
	for (int i = m_nDownloading; i < nMaxDL; ++i) {
		CString strURL;
		m_DLList.GetItemText(0, 0, strURL);

		CDownloadManager::GetInstance()->DownloadStart(strURL, m_strDLFolder, m_hWnd, dwOption);
		
		++m_nDownloading;
		m_DLList.DeleteItem(0);
	}
	_SetTitle();
}


void	CDLListWindow::_SetTitle()
{
	int nCount = m_DLList.GetItemCount();
	CString strTitle;
	strTitle.Format(_T("�A�C�e���� %d - DL���X�g"), nCount);
	SetWindowText(strTitle);
}




















