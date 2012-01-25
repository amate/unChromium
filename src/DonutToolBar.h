/**
 *	@file	DonutToolBar.h
 *	@brief	�c�[���o�[
 */
#pragma once


///////////////////////////////////////////////////////////////////
// CDonutToolBar

class CDonutToolBar
{
public:
	// Constructor/Destructor
	CDonutToolBar();
	~CDonutToolBar();

	HWND	Create(HWND hWndParent);
	void	SetFont(HFONT hFont);
	void	SetDropDownMenu(HMENU hMenu, HMENU hMenuUser, HMENU hMenuCSS);
	void	ReloadSkin();
	void	Customize();

private:
	class Impl;
	Impl*	pImpl;
};
