/**
*	@file FaviconManager.h
*/

#pragma once

#include <unordered_map>
using std::unordered_map;



#define WM_SETFAVICONIMAGE		(WM_APP + 1)
#define USER_MSG_WM_SETFAVICONIMAGE( func )	\
	if (uMsg == WM_SETFAVICONIMAGE) {		\
		SetMsgHandled(TRUE);				\
		OnSetFaviconImage((HWND)wParam, (HICON)lParam);  \
		lResult = 0;						\
		if ( IsMsgHandled() )				\
			return TRUE; 					\
	}


/////////////////////////////////////////////////
/// Favicon���Ǘ����� 

class CFaviconManager
{
public:
	static void	Init(HWND hWndTabBar);

	static void		SetFavicon(HWND hWndChild, LPCTSTR strFaviconURL);
	static HICON	GetFavicon(LPCTSTR strFaviconURL);

private:
	static void _DLIconAndRegister(CString strFaviconURL, HWND hWnd);

	// Data members
	static HWND	s_hWndTabBar;
	static unordered_map<std::wstring, CIcon>	s_mapIcon;	// key:favicon��URL �l:icon
};

























