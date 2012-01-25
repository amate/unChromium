/**
 *	@file	IniFile.h
 *	@brief	ini�t�@�C���̓ǂݏ���.
 *	@note
 *		MtlPrivateProfile.h �����ɍ��ς������m
 */

#ifndef INIFILE_H
#define INIFILE_H

#pragma once

#include <windows.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>

#if _ATL_VER >= 0x700
#include <atlsync.h>
#define INISECTION_USE_MUTEX		1		// 0 1 2
#else	//+++ atl3�Ŗ������R���p�C�����Ă݂�e�X�g�p.
#define INISECTION_USE_MUTEX		0		// 0 1 2
#endif



///+++ �������t�@�C��(*.ini)�̓ǂ݂��݂��s�����߂̃N���X.  ���������݂� ������p������ CIniFileIO �ŉ\�ɂ���.
/// @note
///   ���ӓ_(minit) �t�@�C�����̓t���p�X�ŗ^���Ȃ��ƃt�@�C����Windows�f�B���N�g���ӂ�ɐ��������.
class CIniFileI {
public:
	CIniFileI(const CString &strFileName, const CString &strSectionName);
	//x ~CIniFileI() {;}

	virtual void Close();

	LONG	QueryValue(DWORD& dwValue, LPCTSTR lpszValueName);
	LONG	QueryValue(int&  nValue  , LPCTSTR lpszValueName) { return QueryValue(*(DWORD*)&nValue, lpszValueName); }
	//LONG	QueryValue(LPTSTR szValue, LPCTSTR lpszValueName, DWORD *pdwCount);
	LONG	QueryString(LPTSTR szValue, LPCTSTR lpszValueName, DWORD *pdwCount);

	DWORD	GetValue(LPCTSTR lpszValueName, DWORD defaultValue = 0);
	int		GetValuei(LPCTSTR lpszValueName, int defaultValue = 0);
	const CString GetString(LPCTSTR lpszValueName, const TCHAR* pszDefult=0, DWORD dwBufSize = 0);
	const CString GetStringUW(LPCTSTR lpszValueName, const TCHAR* pszDefult=0, DWORD dwBufSize = 0);

	///+++ �Z�N�V�������̕ύX (�����s�׍H�����A����A�d�v�Ȃ̂̓t�@�C����(�̕ύX)�����Ȃ̂ŁA����͂���ɂ��Ă���)
	void	ChangeSectionName(LPCTSTR sectionName) { ATLASSERT(!m_strFileName.IsEmpty()); m_strSectionName = sectionName; }

protected:
	enum { cnt_nDefault = 0xABCD0123 }; // magic number
	BOOL	IsOpen() {	return !m_strFileName.IsEmpty(); }
	void	Open(const CString &strFileName, const CString &strSectionName);
	const CString ConvPathNameToAlNumName(const TCHAR* src);


protected:
	CString 	m_strFileName;
	CString 	m_strSectionName;

  #if INISECTION_USE_MUTEX == 2 //+++	API��PrivateProfile���g���Ă�̂Ŗ��Ӗ�����?�����A�O�̂��� mutex �Ŕr������������Ă݂�.
	CString 	m_mutexName;
	CMutex		m_mutex;
	CMutexLock* m_mutexLock;
  #endif
};



///+++ �������t�@�C��(*.ini)�̓ǂݏ������s�����߂̃N���X.
class CIniFileIO : public CIniFileI {
public:
	CIniFileIO(const CString &strFileName, const CString &strSectionName = _T(""));
	~CIniFileIO() { Close();}

	virtual void Close();

	LONG	SetValue(DWORD dwValue, LPCTSTR lpszValueName);
//	LONG	SetValue(LPCTSTR lpszValue, LPCTSTR lpszValueName) { return SetString(lpszValue, lpszValueName); }
	LONG	SetString(LPCTSTR lpszValue, LPCTSTR lpszValueName);
	LONG	SetStringUW(LPCTSTR lpszValue, LPCTSTR lpszValueName);
	LONG	DeleteValue(LPCTSTR lpszValueName);

	//+++ �Z�N�V�����������Ă��珑���ꍇ�p.
	bool	DeleteSection();

	//+++ ini�t�@�C����.bak�ɂ��č폜�������Ƃɂ���.
	void	RemoveFileToBak();

	//x void ForceWriteFile(const CString str); 	//+++ ���|��

private:
  #if INISECTION_USE_MUTEX == 1 //+++	API��PrivateProfile���g���Ă�̂Ŗ��Ӗ�����?�����A�O�̂��� mutex �Ŕr������������Ă݂�.
	CString 	m_mutexName;
	CMutex		m_mutex;
	CMutexLock* m_mutexLock;
  #endif
};



#ifdef NDEBUG
typedef CIniFileIO	CIniFileO;
#else
///+++ �������t�@�C��(*.ini)�ւ̏������݂��s���N���X.
class CIniFileO : protected CIniFileIO {
public:
	CIniFileO(const CString &strFileName, const CString &strSectionName = _T("")) : CIniFileIO(strFileName, strSectionName) {;}
	~CIniFileO() {;}

	virtual void Close() { CIniFileIO::Close(); }

	LONG	SetValue(DWORD dwValue, LPCTSTR lpszValueName) { return CIniFileIO::SetValue(dwValue, lpszValueName); }
	//LONG	SetValue(LPCTSTR lpszValue, LPCTSTR lpszValueName) { return CIniFileIO::SetValue(lpszValue, lpszValueName); }
	LONG	SetString(LPCTSTR lpszValue, LPCTSTR lpszValueName) { return CIniFileIO::SetString(lpszValue, lpszValueName); }
	LONG	SetStringUW(LPCTSTR lpszValue, LPCTSTR lpszValueName) { return CIniFileIO::SetStringUW(lpszValue, lpszValueName); }
	LONG	DeleteValue(LPCTSTR lpszValueName) { return CIniFileIO::DeleteValue(lpszValueName); }
	void	ChangeSectionName(LPCTSTR sectionName) { CIniFileI::ChangeSectionName(sectionName); }

	//+++ �Z�N�V�����������Ă��珑���ꍇ�p.
	bool	DeleteSection() { return CIniFileIO::DeleteSection(); }

	//+++ ini�t�@�C����.bak�ɂ��č폜�������Ƃɂ���.
	void	RemoveFileToBak() { CIniFileIO::RemoveFileToBak(); }
};
#endif



#endif
