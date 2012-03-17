/**
 *	@file	DonutLinkBarCtrl.h
 *	@brief	�����N�o�[
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
	void	Refresh();

	void	LinkImportFromFolder(LPCTSTR folder);
	void	LinkExportToFolder(LPCTSTR folder);

private:
	class Impl;
	Impl* pImpl;
};
