/**
*	@ file	FindBar.h
*	@brief	Firefox���C�N�ȃy�[�W�������o�[
*/

#pragma once


class CFindBar
{
public:
	CFindBar();
	~CFindBar();

	HWND	Create(HWND hWndParent);
	void	SetUpdateLayoutFunc(function<void (BOOL)> func);

	HWND	GetHWND();
	void	ShowFindBar(const CString& strKeyword);

private:
	class Impl;
	Impl*	pImpl;
};





























