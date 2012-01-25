/**
 *	@file	DonutLinkBarCtrl.h
 *	@brief	リンクバー
 */
#pragma once

///////////////////////////////////////////////////////////////////
// CDonutLinkBarCtrl

class CDonutLinkBarCtrl
{
public:
	CDonutLinkBarCtrl();
	~CDonutLinkBarCtrl();

	HWND	Create(HWND hWndParent);

	void	SetFont(HFONT hFont);

private:
	class Impl;
	Impl* pImpl;
};
