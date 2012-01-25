// MainFrm.h : CMainFrame �N���X�̃C���^�[�t�F�C�X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../resource.h"
#include "DownloadingListView.h"
#include "DownloadedListView.h"
#include "CustomBindStatusCallBack.h"


class CDownloadManager;

//////////////////////////////////////////////////////////
/// �_�E�����[�h�}�l�[�W���[�̃��C���t���[��

class CDownloadFrame
	: public CFrameWindowImpl<CDownloadFrame>
{
	friend class CDownloadManager;
public:
	DECLARE_FRAME_WND_CLASS(_T("DonutDownloadManager"), NULL)
	
	// Constructor
	CDownloadFrame();

	void	SetParentWindow(HWND hWnd) { m_hWndParent = hWnd; }
	void	EnableVisible() { m_bVisible = true; }

	int		GetDownloadingCount() const { 
		return m_wndDownloadingListView.GetDownloadingCount();
	}

	// Message map and Handler
	BEGIN_MSG_MAP(CDownloadFrame)
		MSG_WM_CREATE	( OnCreate	)
		MSG_WM_DESTROY	( OnDestroy	)
		MSG_WM_CLOSE	( OnClose	)
		COMMAND_ID_HANDLER(ID_DLOPENOPTION	, OnOpenOption )
		COMMAND_ID_HANDLER(ID_SET_DLFOLDER	, OnSetDLFolder )
		COMMAND_ID_HANDLER(ID_OPEN_DLFOLDER	, OnOpenDLFolder )
		COMMAND_ID_HANDLER(ID_DLAPP_ABOUT	, OnAppAbout )
		CHAIN_MSG_MAP(CFrameWindowImpl<CDownloadFrame>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()


	int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	void	OnDestroy();
	void	OnClose();
	LRESULT OnOpenOption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSetDLFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOpenDLFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	

private:
	CHorSplitterWindow		m_wndSplitter;
	CDownloadingListView	m_wndDownloadingListView;
	CDownloadedListView		m_wndDownloadedListView;
	HWND					m_hWndParent;
	bool					m_bVisible;
	CRect					m_rcWindowPos;
	//std::vector<std::unique_ptr<CCustomBindStatusCallBack> >	m_vecpCSCB;

};
