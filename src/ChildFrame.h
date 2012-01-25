/**
 *	@file	ChildFrame.h
 *	@brief	�^�u�y�[�W�P�̏���.
 */

#pragma once

class CChildFrame;

struct NewChildFrameData {
	HWND	hWndParent;
	CString	strURL;
	CString strReferer;
	DWORD	dwDLCtrl;
	DWORD	dwExStyle;
	DWORD	dwAutoRefresh;
	bool	bActive;
	bool	bLink;
	function<void (CChildFrame*)>	funcCallAfterCreated;
	NewChildFrameData*	pNext;

	NewChildFrameData(HWND Parent) : 
		hWndParent(Parent), dwDLCtrl(-1), dwExStyle(-1), dwAutoRefresh(0), bActive(false), bLink(false), pNext(nullptr)
	{	}
};

struct ChildFrameDataOnClose {
	CString strURL;
	CString strTitle;
	vector<std::pair<CString, CString> > TravelLogFore;
	vector<std::pair<CString, CString> > TravelLogBack;
	DWORD	dwDLCtrl;
	DWORD	dwExStyle;			// TabList.xml�p
	DWORD	dwAutoRefreshStyle;	// 

	ChildFrameDataOnClose() : dwDLCtrl(0), dwExStyle(0), dwAutoRefreshStyle(0)
	{	}
};


/////////////////////////////////////////////////////////////
// CChildFrame

class CChildFrame
{
public:
	/// �X���b�h�𗧂Ă�CChildFrame�̃C���X�^���X�����
	static void	AsyncCreate(NewChildFrameData& data);

	void	SetThreadRefCount(int* pCount);
	HWND	CreateEx(HWND hWndParent, LPCTSTR strUrl);
	void	Navigate2(LPCTSTR lpszURL);

	HWND	GetHwnd() const;
	DWORD	GetExStyle() const;
	void	SetExStyle(DWORD dwStyle);
	void	SetDLCtrl(DWORD dwDLCtrl);
	void	SetMarshalDLCtrl(DWORD dwDLCtrl);
	void	SetAutoRefreshStyle(DWORD dwAutoRefresh);
	void	SaveSearchWordflg(bool bSave);
	void	SetSearchWordAutoHilight(const CString& str, bool bAutoHilight);
	void	SetTravelLog(const vector<std::pair<CString, CString> >& fore, const vector<std::pair<CString, CString> >& back);

	CComPtr<IWebBrowser2>	GetIWebBrowser();
	CComPtr<IWebBrowser2>	GetMarshalIWebBrowser();
	CString	GetLocationURL();

private:
	CChildFrame();
	~CChildFrame();

	class Impl;
	Impl*	pImpl;
};











