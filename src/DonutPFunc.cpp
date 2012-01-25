/**
 *	@file	DonutPFunc.cpp
 *	@brief	DonutP �̔ėp�֐�.
 */

#include "stdafx.h"
#include "DonutPFunc.h"
#include "MtlBase.h"
#include "MtlWin.h"
#include "ToolTipManager.h"
#include "resource.h"
#include <Shlobj.h>


#include "MtlMisc.h"
#include "IniFile.h"

using namespace ATL;
using namespace WTL;
using namespace MTL;


extern TCHAR		g_szIniFileName[MAX_PATH];

extern const TCHAR	g_cSeparater[]	= _T("��������������������������������������������������������");



//---------------------------------------------------
/// �A�C�e���h�c���X�g����A�C�R�������
HICON	CreateIconFromIDList(PCIDLIST_ABSOLUTE pidl)
{
	HRESULT	hr;
	CComPtr<IShellItem>	pShellItem;
	//hr = ::SHCreateItemFromIDList(pidl, IID_IShellItem, (LPVOID*)&pShellItem);
	hr = ::SHCreateShellItem(NULL, NULL, pidl, &pShellItem);
	if (FAILED(hr)) 
		return NULL;

	CComPtr<IExtractIcon>	pExtractIcon;
	hr = pShellItem->BindToHandler(NULL, BHID_SFUIObject, IID_IExtractIcon, (LPVOID*)&pExtractIcon);
	if (FAILED(hr)) 
		return NULL;

	WCHAR	szPath[MAX_PATH];
	int		iIndex;
	UINT	uFlags;
	hr = pExtractIcon->GetIconLocation(GIL_FORSHELL, szPath, MAX_PATH, &iIndex, &uFlags);
	if (FAILED(hr)) 
		return NULL;

	HICON	hIcon;
	hr = pExtractIcon->Extract(szPath, iIndex, NULL, &hIcon, MAKELONG(32, 16));
	if (FAILED(hr)) 
		return NULL;
	ATLASSERT(hIcon);

	//CDC dc = ::GetDC(NULL);
	//dc.DrawIconEx(CPoint(2000, 250), hIcon, CSize(16,16));
	return hIcon;
}

//-----------------------------------------
/// �A�C�R������r�b�g�}�b�v�����
HBITMAP	CreateBitmapFromHICON(HICON hIcon)
{
	static UINT	uWidth	= GetSystemMetrics(SM_CXSMICON);
	static UINT	uHeight	= GetSystemMetrics(SM_CYSMICON);

	LPVOID	lpBits = NULL;
	CBitmapHandle bmp;
	bmp.CreateBitmap(uWidth, uHeight, 1, 32, lpBits);	// Bitmap���쐬����

	CDC memDC = CreateCompatibleDC(NULL);
	HBITMAP	hbmpPrev = memDC.SelectBitmap(bmp);
	HBRUSH	hbr = CreateSolidBrush(GetSysColor(COLOR_MENU));	// �w�i
	memDC.DrawIconEx(0, 0, hIcon, uWidth, uHeight, 0, hbr, DI_NORMAL);
	memDC.SelectBitmap(hbmpPrev);

	return bmp;
}



static __inline double RADIAN_TO_ANGLE(double a) {
	const double PI	= 3.141592653589793238462643383279;
	return (a) / PI * 180.0;
}



double _GetAngle(CPoint pt1, CPoint pt2)
{
	double xlen  = pt2.x - pt1.x;
	double ylen  = pt1.y - pt2.y;

	if (fabs( xlen ) < 1.0)
		xlen = 0.0;

	if (fabs( ylen ) < 1.0)
		ylen = 0.0;

	if ( (xlen == 0.0) && (ylen == 0.0) )
		return ( 0.0 );

	if (xlen == 0.0) {
		if (pt1.y < pt2.y)
			return ( 270.0 );
		else
			return (  90.0 );
	} else if (ylen == 0.0) {
		if (pt1.x < pt2.x)
			return (  0.0);
		else
			return (180.0);
	}

	double angle = RADIAN_TO_ANGLE( atan2(ylen, xlen) );

	if (angle < 0.0)
		angle += 360.0;

	return ( angle );
}



BOOL _CheckOsVersion_XPLater()
{
	OSVERSIONINFO osi = { sizeof (OSVERSIONINFO) };

	//BOOL bRet =
	::GetVersionEx(&osi);

	//windowsXP 5.1
	return ( (osi.dwMajorVersion == 5) && (osi.dwMinorVersion >= 1) || (osi.dwMajorVersion > 5) );
}

BOOL _CheckOsVersion_VistaLater()
{
	OSVERSIONINFO osi  = { sizeof (OSVERSIONINFO) };

	//BOOL		  bRet =
	::GetVersionEx(&osi);

	//windowsVista 6.0
	return (osi.dwMajorVersion >= 6);
}


//---------------------------
/// �ݒ肳�ꂽ�X�L���t�H���_�̃p�X��Ԃ�(�Ō��'\\'����)
CString _GetSkinDir()
{
	CString 	strPath    = Misc::GetExeDirectory() + _T("Skin\\");
	CString 	strDefPath = strPath + _T("default");

	CIniFileI	pr( g_szIniFileName, _T("Skin") );
	CString 	strSkinPath = pr.GetStringUW( _T("SkinFolder") );
	pr.Close();

	if ( strSkinPath.IsEmpty() )
		strSkinPath = strDefPath;
	else
		strSkinPath = strPath + strSkinPath;

	if (::PathFileExists(strSkinPath) == FALSE) {
		if (PathFileExists(strDefPath) == FALSE) {
			strSkinPath = strPath;
		} else {
			strSkinPath = strDefPath;
		}
	}

	if ( !strSkinPath.IsEmpty() && strSkinPath.Right(1) != _T("\\") )
		strSkinPath += _T("\\");

	return strSkinPath;
}


//----------------------------------------
/// �R�}���h�Ɋ֘A�t����ꂽ��������擾����
bool _LoadToolTipText(int nCmdID, CString &strText)
{
	enum { SIZE = 1024/*256*/ };
	TCHAR	szBuff[SIZE+2];
	szBuff[0] = 0;
	int   nRet = ::LoadString(_Module.GetResourceInstance(), nCmdID, szBuff, SIZE);

	for (int i = 0; i < nRet; i++) {
		if ( szBuff[i] == _T('\n') ) {
			TCHAR	szText[SIZE];
			lstrcpyn(szText, &szBuff[i + 1], SIZE);
			strText = szText;
			return true;
		}
	}

	return false;
}



#if 1	//+++ DonutRAPT ���p�N�炵�Ă���������[�`��.

// WinNT3.5+: �������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ���
void RtlSetMinProcWorkingSetSize()
{
  #if 1
	HINSTANCE hLib = ::LoadLibrary( _T( "kernel32.dll" ) );
	if ( hLib ) {
		typedef BOOL ( WINAPI *LPFN )( HANDLE, SIZE_T, SIZE_T );
		LPFN fnSetProcessWorkingSetSize = reinterpret_cast< LPFN >( ::GetProcAddress( hLib, "SetProcessWorkingSetSize" ) );
		if ( fnSetProcessWorkingSetSize ) {
			HANDLE hCurrentProcess = ::GetCurrentProcess();
		  #ifdef WIN64
			MTLVERIFY( fnSetProcessWorkingSetSize(hCurrentProcess, 0xFFFFFFFFFFFFFFFFll, 0xFFFFFFFFFFFFFFFFll ) );
		  #else
			MTLVERIFY( fnSetProcessWorkingSetSize(hCurrentProcess, 0xFFFFFFFF, 0xFFFFFFFF ) );
		  #endif
		}
		::FreeLibrary( hLib );
	}

	Misc::mallocHeapCompact();	//+++
  #endif
}

#endif


/*+++ ����:
	Xp(�r�W���A���X�^�C��)��Manifest�t�@�C�����g�p�����ꍇ�AANSI�ł�Edit�R���g���[���֌W
	EM_GETSEL,EM_SETSEL,EM_LIMITTEXT �ŁA����ȑO��WinOS�ł̓o�C�g���Ŏw�肵�Ă������̂��A
	�������P�ʂɂ�����Ă��܂��炵���B(���̓���� Unicode �� API �Ɠ��l�̓���炵��)
		http://support.microsoft.com/default.aspx?scid=kb;ja;418099
			WindowsXP �� Manifest �t�@�C�����g�p�����A�v���P�[�V������ Common Control
			�o�[�W���� 6 ���g�p�����ꍇ�AEdit �R���g���[���ւ� EM_LIMITTEXT ���b�Z�[�W
			�ɂ����͗ʐ�����ݒ肷��ۂ̒P�ʂ��A�o�C�g���P�ʂł͂Ȃ��������P�ʂɕύX
			����܂����B���̓���͎d�l�ł��B

	�c�c��Manifest�t�@�C�����g�p���Ă����̂́A�g�ݍ���manifest�̏ꍇ�͓��Ă͂܂�Ȃ��A��
	�������ƂȂ̂�? vista��xp�ł��ʏ�̓o�C�g�����Ԃ����. ���L���[�`���ł́A.manifest
	�t�@�C���̑��݃`�F�b�N�ŉ��߂�؂�ւ��Ă���̂�...
 */



// 1�����i�߁A�i�߂��o�C�g��Ԃ��B�A��NULL�̂Ƃ��̓|�C���^��i�߂��A0��Ԃ�
int RtlMbbNext(LPTSTR &r_lp)
{
	LPCTSTR lpBegin = r_lp;

	if (r_lp) {
		++r_lp;
	  #ifndef UNICODE
		if ( _ismbblead( *(r_lp - 1) ) && *r_lp /*_ismbbtrail(*r_lp)*/ ) {
			// 2�o�C�g�����̎��͂���1�o�C�g�i�߂�
			++r_lp;
		}
	  #endif
	}

	return int (r_lp - lpBegin);
}


//-------------------------------------------
/// strText �� nFirst �����ڂ���AnCount ���������擾���ĕԂ��B�}���`�o�C�g�����Ή���
static CString RtlMbbStrMid(CString strText, int nFirst, int nCount)
{
	ATLASSERT( nFirst >= 0 );

	if (nCount < 1 || strText.GetLength() <= nFirst)
		return CString();

	LPTSTR lp = strText.GetBuffer(0);
	int    i, p1 = 0, p2 = 0;

	for (i = 0; *lp && p1 < nFirst; ++i) {
		p1 += RtlMbbNext(lp);
	}

	for (i = 0; *lp && p2 < nCount; ++i) {
		p2 += RtlMbbNext(lp);
	}

	strText.ReleaseBuffer();
	return strText.Mid(p1, p2);
}

//-------------------------------------------
/// �G�f�B�b�g�R���g���[���őI������Ă��镶�����Ԃ�
static CString RtlGetSelectedText(const CEdit &edit)
{
	CString str    = MtlGetWindowText(edit);

	if ( str.IsEmpty() )
		return CString();

	int 	nStart = -1;
	int 	nEnd   = -1;
	edit.GetSel(nStart, nEnd);									// �I��͈͂��u�������v�ŕԂ�

	if ( nStart != nEnd && str.GetLength() != (nEnd - nStart) && !(nStart == 0 && nEnd == -1) ) {
		str = RtlMbbStrMid(str, nStart, nEnd - nStart); 		// ���������o�C�g���ɕϊ�����K�v������
	}

	return str;
}


#ifndef UNICODE		//+++ UNICODE �R���p�C�����́A�]�v�Ȏ�Ԃ͕s�v

/// EM_GETSEL��VisualStyle�K�p����UNICODE�I���������(UNICODE���`���Ȃ��ꍇ�ł�)�̂łȂ�Ƃ�����֐�
CString _GetSelectTextWtoA(CEdit &edit)
{
	int 	nStart = -1;
	int 	nEnd   = -1;
	edit.GetSel(nStart, nEnd);									// �I��͈͂��u�������v�ŕԂ�

	//�����UNICODE�ł̒l(������)
	if (nStart == nEnd)
		return MtlGetWindowText(edit);							// +++ ����:�͈͎w�肪�Ȃ��ꍇ�́A�S�̂�Ԃ�.

	int 	nTextLen  = ::GetWindowTextLength(edit.m_hWnd);
	int 	nSelCount = nEnd - nStart;

	LPSTR	lpAll	  = new CHAR [nTextLen	+ 1];
	LPWSTR	lpwAll	  = new WCHAR[nTextLen	+ 1];
	LPWSTR	lpwSel	  = new WCHAR[nSelCount + 1];

	::GetWindowText(edit.m_hWnd, (LPTSTR) lpAll, nTextLen + 1); 		//�����ANSI�ȃe�L�X�g��Ԃ�
	::MultiByteToWideChar(CP_ACP, 0, lpAll, -1, lpwAll, nTextLen + 1);	// +++ ����: ��U UTF16�ɂ���

	memset( lpwSel, 0, (nSelCount + 1) * sizeof (WCHAR) );
	CopyMemory( lpwSel, lpwAll + nStart, nSelCount * sizeof (WCHAR) );	// +++ ����:

	CString str 	  = lpwSel; 		// +++ ����:���̃R�s�[�R���X�g���N�^�� UTF16���� MBC �ɕϊ�.

	delete[] lpAll;
	delete[] lpwAll;
	delete[] lpwSel;
	return str;
}

#endif

//-------------------------------------------
/// �G�f�B�b�g�R���g���[���őI������Ă��镶�����Ԃ�
CString _GetSelectText(const CEdit &edit)
{
	//VisualStyle ���K�p����Ă���ꍇ�� EM_GETSEL �� UNICODE �ŕԂ�
  #if defined(UNICODE)		//+++ UNICODE �R���p�C�����́A�]�v�Ȏ�Ԃ͕s�v
	CString str    = MtlGetWindowText(edit);

	if ( str.IsEmpty() )
		return CString();

	int 	nStart = -1;
	int 	nEnd   = -1;
	edit.GetSel(nStart, nEnd);									// �I��͈͂��u�������v�ŕԂ�
	if (nStart == nEnd || (nStart == 0 && nEnd == -1) || str.GetLength() == (nEnd - nStart))
		return str;
	return str.Mid(nStart, nEnd - nStart);
  #else
	// +++ ����:Xp�ȍ~��WinOS �ŁAexe�Ɠ����t�H���_�� .manifest�t�@�C��������ꍇ.
	//			�� exe�t�@�C���Ɠ����Ƃ���ɂ���.manifest�t�@�C�����āA��Q���΍�悤�Ȃ̂���.
	//x if ( CThemeDLLLoader::IsExistManifest() && _CheckOsVersion_XPLater() )	//+++ CThemeDLLLoader ���g��Ȃ��悤�ɂ���.
	if ( IsExistManifestFile() && _CheckOsVersion_XPLater() ) {
		//UNICODE����ANSI�֕ϊ�
		return _GetSelectTextWtoA(edit);
	} else
	{
		//ANSI�̏ꍇ��RAPT���̃R�[�h�𗬗p
		return RtlGetSelectedText(edit);
	}
  #endif
}



//+++ ���Ƃō�蒼��.
static CString GetCurPosWord(const TCHAR* buf, unsigned len, unsigned pos);
static CString GetCurPosWordB(const TCHAR* buf, unsigned len, unsigned pos);

///*+++ �����i: GetSel�̕Ԃ�l�̈������K���Ȃ̂ŁA�듮��(�Ⴄ�P�������)�����肦��.
///+++ �G�f�B�b�g�́A�I���e�L�X�g�̎擾. ���I���̏ꍇ�́A�J�[�\���ʒu(�L�����b�g�̂��ƁH)�̒P����擾.(�y�[�W�������p)
CString _GetSelectText_OnCursor(const CEdit &edit)
{
	int 	len = ::GetWindowTextLength( edit.m_hWnd ); 	// �o�C�g��. (�L������)
	int 	nStart = -1, nEnd = -1;
	edit.GetSel(nStart, nEnd);								// �I��͈͂�Ԃ�(���������o�C�g�����󋵂������̂悤)

	if (len == 0 || nStart != nEnd || nStart < 0)			// �͈͎w�肾������A�������Ȃ�������w���ȏ�ԂȂ猳�̏����܂���.
		return _GetSelectText( edit );

	std::vector<TCHAR>	buf( len + 2 );
	::GetWindowText(edit.m_hWnd, (LPTSTR)&buf[0], len + 1); // ������擾.

  #ifdef UNICODE	//+++ UNICODE�R���p�C���Ȃ�A�]�v�Ȃ��Ƃ�����K�v�Ȃ�
	return GetCurPosWord( &buf[0], len, nStart );			// �P��擾. nStart����������.
  #else
	// +++ ����:Xp�ȍ~��WinOS �ŁAexe�Ɠ����t�H���_�� .manifest�t�@�C��������ꍇ.
	if ( IsExistManifestFile() && _CheckOsVersion_XPLater() ) {
		return GetCurPosWord( &buf[0], len, nStart );			// �P��擾. nStart����������.
	} else {
		for (int i = 0; i < len; ++i) { 						// �S�p�󔒂͔��p�Q�ɒu��������.
			unsigned c = *(unsigned char*)&buf[i];
			if (c == 0x81 && buf[i+1] == 0x40) {
				buf[i  ]  = ' ';
				buf[++i]  = ' ';
			}
		}
		return GetCurPosWordB( &buf[0], len, nStart );			// �P��擾. nStart���o�C�g����.
	}
  #endif
}



///+++ pos�̈ʒu(������)�ɂ���󔒂ŋ�؂�ꂽ�P��� CString�ɂ��ĕԂ�. '"'���͖��l��.
static CString GetCurPosWord(const TCHAR* src, unsigned len, unsigned pos)
{
	#define IS_SPACE_W(c)		((c) == L' ' || (c) == L'\t' || (c) == L'\n' || (c) == L'\r' || (c) == L'�@')
	ATLASSERT(src != 0 && len > 0 && pos <= len);
	std::vector<WCHAR>	wbuf( len + 2 );
  #ifdef UNICODE
	::lstrcpyn(&wbuf[0], src, len+1);
  #else
	::MultiByteToWideChar(CP_ACP, 0, &src[0], -1, &wbuf[0], len + 1);	// ��Uwchar��.
  #endif
	unsigned	top = 0;
	unsigned	end = len;

  #if 0 	// �܂��A�J�[�\���ʒu�ɕ��������邩�`�F�b�N. �󔒂Ȃ�A�O�̒P��̍Ō�ֈړ�.
	unsigned c = wbuf[ pos ];
	if (c == 0)
		return CString( src );
	if (IS_SPACE_W(c)) {
		if (pos > 0) {
			do {
				c = wbuf[--pos];
			} while (pos > top && IS_SPACE_W(c));
		}
		if (IS_SPACE_W(c)) {	// �J�[�\�����O�ɒP�ꂪ�Ȃ��Ȃ�A���Ɉړ�.
			do {
				c = wbuf[++pos];
			} while (pos < end && IS_SPACE_W(c));
		}
	}
  #else 	// �܂��A�J�[�\���ʒu�ɕ��������邩�`�F�b�N. �󔒂Ȃ�A���̒P��̐擪�ֈړ�.
	unsigned c = wbuf[ pos ];
	if (IS_SPACE_W(c)) {
		do {
			c = wbuf[++pos];
		} while (pos < end && IS_SPACE_W(c));
	}
	if (c == 0)
		return CString( src );
  #endif

	// �J�[�\���ʒu����P��̐擪��T��.
	for (int i = int(pos); --i >= 0;) {
		c = wbuf[i];
		if (IS_SPACE_W(c) || c == 0) {
			top = i;
			break;
		}
	}

	// �J�[�\���ʒu����P��̍Ō��T��.
	for (unsigned i = pos; i < end; ++i) {
		c = wbuf[i];
		if (IS_SPACE_W(c) || c == 0) {
			end = i;
			break;
		}
	}

	// �I�[��ݒ�.
	wbuf[end] = 0;

	if (top >= end) { // ����������������S�̂�Ԃ�.
		return CString( src );
	} else {
		return CString ( &wbuf[top] );	// top����end�܂ł�wchar������CString��(�R�s�[�R���X�g���N�^�ɔC��)
	}

	#undef IS_SPACE_W
}


#if defined(UNICODE) == 0	//+++ MultiByte�ł̎�

///+++ pos�̈ʒu(�o�C�g��)�ɂ���󔒂ŋ�؂�ꂽ�P��� CString�ɂ��ĕԂ�. '"'���͖��l��.
/// �Ƃ肠�����A�ʓ|�Ȃ�ŁA�S�p�󔒖��Ή�.���\�����ł��炩���ߒu������悤�ɂ���.(�w����xp�͂܂�...)
static CString GetCurPosWordB(const TCHAR* src, unsigned len, unsigned pos)
{
	#define IS_SPACE(c) 	((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')
	ATLASSERT(src != 0 && len > 0 && pos <= len);
	std::vector<char>		bbuf(len + 4);
	memcpy(&bbuf[0], src, len+1);
	unsigned	top = 0;
	unsigned	end = len;

  #if 0 	// �܂��A�J�[�\���ʒu�ɕ��������邩�`�F�b�N. �󔒂Ȃ�A�O�̒P��̍Ō�ֈړ�.
	unsigned c = bbuf[ pos ];
	if (c == 0)
		return CString( src );
	if (IS_SPACE(c) || c == 0) {
		if (pos > 0) {
			do {
				c = bbuf[--pos];
			} while (pos > top && IS_SPACE(c));
		}
		if (IS_SPACE(c)) {	// �J�[�\�����O�ɒP�ꂪ�Ȃ��Ȃ�A���Ɉړ�.
			do {
				c = bbuf[++pos];
			} while (pos < end && IS_SPACE(c));
		}
	}
  #else 	// �܂��A�J�[�\���ʒu�ɕ��������邩�`�F�b�N. �󔒂Ȃ�A���̒P��̐擪�ֈړ�.
	unsigned c = bbuf[ pos ];
	if (IS_SPACE(c)) {
		do {
			c = bbuf[++pos];
		} while (pos < end && IS_SPACE(c));
	}
	if (c == 0)
		return CString( src );
  #endif

	// �J�[�\���ʒu����P��̐擪��T��.
	for (int i = int(pos); --i >= 0;) {
		c = bbuf[i];
		if (IS_SPACE(c) || c == 0) {
			top = i;
			break;
		}
	}

	// �J�[�\���ʒu����P��̍Ō��T��.
	for (unsigned i = pos; i < end; ++i) {
		c = bbuf[i];
		if (IS_SPACE(c) || c == 0) {
			end = i;
			break;
		}
	}

	// �I�[��ݒ�.
	bbuf[end] = 0;

	if (top >= end) { // ����������������S�̂�Ԃ�.
		return CString( src );
	} else {
		return CString ( &bbuf[top] );	// top����end�܂ł�wchar������CString��(�R�s�[�R���X�g���N�^�ɔC��)
	}

	#undef IS_SPACE
}

#endif




BOOL _AddSimpleReBarBandCtrl(
		HWND	hWndReBar,
		HWND	hWndBand,
		int 	nID,
		LPTSTR	lpstrTitle,
		BOOL	bNewRow,
		int 	cxWidth,
		BOOL	bFullWidthAlways)
{
	ATLASSERT( ::IsWindow(hWndReBar) ); 	// must be already created
  #ifdef _DEBUG
	// block - check if this is really a rebar
	{
		TCHAR lpszClassName[sizeof (REBARCLASSNAME)];
		lpszClassName[0] = 0;	//+++
		::GetClassName( hWndReBar, lpszClassName, sizeof (REBARCLASSNAME) );
		ATLASSERT(lstrcmp(lpszClassName, REBARCLASSNAME) == 0);
	}
  #endif	//_DEBUG
	ATLASSERT( ::IsWindow(hWndBand) );		// must be already created

	// Get number of buttons on the toolbar
	int 		  nBtnCount = (int) ::SendMessage(hWndBand, TB_BUTTONCOUNT, 0, 0L);
	// Set band info structure
	REBARBANDINFO rbBand;
	rbBand.cbSize	 = sizeof (REBARBANDINFO);
  #if (_WIN32_IE >= 0x0400)
	rbBand.fMask	 = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE | RBBIM_IDEALSIZE;
  #else
	rbBand.fMask	 = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE;
  #endif	//!(_WIN32_IE >= 0x0400)
	if (lpstrTitle != NULL)
		rbBand.fMask |= RBBIM_TEXT;

	rbBand.fStyle	 = 0;
  #if (_WIN32_IE >= 0x0500)
	if (nBtnCount > 0)											// add chevron style for toolbar with buttons
		rbBand.fStyle |= RBBS_USECHEVRON;
  #endif	//(_WIN32_IE >= 0x0500)

	if (bNewRow)
		rbBand.fStyle |= RBBS_BREAK;

	rbBand.lpText	 = lpstrTitle;
	rbBand.hwndChild = hWndBand;

	if (nID == 0)	// calc band ID
		nID = ATL_IDW_BAND_FIRST + (int) ::SendMessage(hWndReBar, RB_GETBANDCOUNT, 0, 0L);

	rbBand.wID		 = nID;

	// Calculate the size of the band
	BOOL	bRet;
	RECT	rcTmp;

	if (nBtnCount > 0) {
		bRet			  = ::SendMessage(hWndBand, TB_GETITEMRECT, nBtnCount - 1, (LPARAM) &rcTmp) != 0;
		ATLASSERT(bRet);
		rbBand.cx		  = (cxWidth != 0) ? cxWidth : rcTmp.right;
		rbBand.cyMinChild = rcTmp.bottom - rcTmp.top;
		rbBand.cxMinChild = 0;
	} else {	// no buttons, either not a toolbar or really has no buttons
		bRet			  = ::GetWindowRect(hWndBand, &rcTmp) != 0;
		ATLASSERT(bRet);
		rbBand.cx		  = (cxWidth != 0) ? cxWidth : (rcTmp.right - rcTmp.left);
		rbBand.cxMinChild = (bFullWidthAlways) ? rbBand.cx : 0;
		rbBand.cyMinChild = rcTmp.bottom - rcTmp.top;
	}

  #if (_WIN32_IE >= 0x0400)
	rbBand.cxIdeal = rbBand.cx;
  #endif	//(_WIN32_IE >= 0x0400)

	// Add the band
	LRESULT lRes = ::SendMessage(hWndReBar, RB_INSERTBAND, (WPARAM) -1, (LPARAM) &rbBand);

	if (lRes == 0) {
		ATLTRACE2( atlTraceUI, 0, _T("Failed to add a band to the rebar.\n") );
		return FALSE;
	}

	return TRUE;
}

//----------------------------------------------
/// strFile �� Strings�����s����������
bool FileWriteString(const CString& strFile, const std::list<CString>& Strings)
{
	FILE*	fp = _tfopen(strFile, _T("w"));
	if (fp == NULL)
		return false;

  #if 1	//+++ UNICODE �Ή��ŏ�������
	for (auto it = Strings.cbegin(); it != Strings.cend(); ++it) {
	  #ifdef UNICODE
		fprintf(fp, "%s\n", Misc::tcs_to_sjis(*it).data());
	  #else
		fprintf(fp, "%s\n", LPCTSTR(*str));
	  #endif
	}
  #else
	std::list<CString>::iterator str;
	CString 					 strOut;

	for (str = pString->begin(); str != pString->end(); ++str) {
		fprintf(fp, "%s\n", *str);
	}
  #endif

	fclose(fp);

	return true;
}

//---------------------------------------------
/// strFile ���� 1�s���Ƃ�Strings�ɓ���Ă���(��1�s�ɂ�4096�����ȏ�ǂݍ��ނƕ��������)
bool FileReadString(const CString& strFile, std::list<CString>& Strings)
{
	FILE*	fp = _tfopen(strFile, _T("r"));
	if (fp == NULL) 
		return false;

	enum { L = 4096 };
	char	cBuff[L];

	while (fgets(cBuff, L, fp)) {
		unsigned	nLen = unsigned( strlen(cBuff) );
		if (nLen != 0 && cBuff[nLen - 1] == '\n')
			cBuff[nLen - 1] = '\0';

		Strings.push_back(CString(cBuff));
	}

	fclose(fp);

	return true;
}

//-----------------------------------------
/// ���j���[�̕�����������R���{�{�b�N�X�ɒǉ�����
BOOL _SetCombboxCategory(CComboBox &cmb, HMENU hMenu)
{
	ATLASSERT(cmb.IsWindow());

	if (cmb.GetCount() != 0)
		return FALSE;

	int nCount = ::GetMenuItemCount(hMenu);
	for (int ii = 0; ii < nCount; ++ii) {
		TCHAR	cBuff[MAX_PATH] = _T("\0");
		if (::GetMenuString(hMenu, ii, cBuff, MAX_PATH, MF_BYPOSITION) == 0)
			continue;

		CString strMenu(cBuff);

		if (strMenu.Find(_T("(")) != -1)
			strMenu = strMenu.Left( strMenu.Find(_T("(")) );
		else if (strMenu.Find(_T("\t")) != -1)
			strMenu = strMenu.Left( strMenu.Find(_T("\t")) );

		cmb.AddString(strMenu);
	}

	cmb.SetCurSel(0);
	return TRUE;
}



BOOL _DontUseID(UINT uID)
{
	// UINT uDontUseID[] = {ID_FILE_MRU_FILE1, ID_INSERTPOINT_GROUPMENU, ID_INSERTPOINT_FAVORITEMENU};
	static const UINT uDontUseID[] = { ID_INSERTPOINT_GROUPMENU, ID_INSERTPOINT_FAVORITEMENU };

	for (int ii = 0; ii < _countof(uDontUseID); ++ii) {
		if (uID == uDontUseID[ii])
			return TRUE;
	}

	return FALSE;
}

//----------------------------------------------
/// ���j���[�̃R�}���h�Ɋ��蓖�Ă�ꂽ��������R���{�{�b�N�X�ɒǉ�����
void _PickUpCommandSub(HMENU hMenuSub, CComboBox &cmbCmd)
{
	CMenuHandle	menuSub(hMenuSub);
	int nPopStartIndex = cmbCmd.AddString(g_cSeparater);
	int nAddCnt 	   = 0;

	int nCount = menuSub.GetMenuItemCount();
	for (int ii = 0; ii < nCount; ++ii) {
		HMENU	hMenuSub2 = menuSub.GetSubMenu(ii);
		if (hMenuSub2)
			_PickUpCommandSub(hMenuSub2, cmbCmd);

		UINT	nCmdID	  = menuSub.GetMenuItemID(ii);
		if ( _DontUseID(nCmdID) )
			break;

		CString strMenu;
		CToolTipManager::LoadToolTipText(nCmdID, strMenu);

		if ( strMenu.IsEmpty() )
			continue;

		int 	nIndex	  = cmbCmd.AddString(strMenu);
		cmbCmd.SetItemData(nIndex, nCmdID);
		nAddCnt++;
	}

	if (nAddCnt != 0)
		cmbCmd.AddString(g_cSeparater);
	else
		cmbCmd.DeleteString(nPopStartIndex);
}

//-------------------------------------------------
/// hMenu��nPopup�Ԗڂ̃T�u���j���[�̃R�}���h�Ɋ��蓖�Ă�ꂽ��������R���{�{�b�N�X�ɒǉ�����
void _PickUpCommand(HMENU hMenu, int nPopup, CComboBox &cmbCmd)
{
	HMENU	hMenuSub = ::GetSubMenu(hMenu, nPopup);
	CMenu	menu(hMenuSub);
	cmbCmd.ResetContent();

	int nCount = ::GetMenuItemCount(hMenuSub);
	for (int ii = 0; ii < nCount; ++ii) {
		HMENU	hMenuSub = menu.GetSubMenu(ii);
		if (hMenuSub)
			_PickUpCommandSub(hMenuSub, cmbCmd);

		UINT	nCmdID	 = menu.GetMenuItemID(ii);

		if ( _DontUseID(nCmdID) )
			break;

		CString strMenu;
		CToolTipManager::LoadToolTipText(nCmdID, strMenu);

		if ( strMenu.IsEmpty() )
			continue;

		int 	nIndex	 = cmbCmd.AddString(strMenu);
		cmbCmd.SetItemData(nIndex, nCmdID);
	}

	menu.Detach();

	//�s�v�ȃZ�p���[�^�̍폜
	int   nCountSep = 0;
	int   nCountCmb = cmbCmd.GetCount();

	for (int i = 0; i < nCountCmb - 1; i++) {
		if (cmbCmd.GetItemData(i) == 0) {
			nCountSep++;

			if (cmbCmd.GetItemData(i + 1) == 0) {
				cmbCmd.DeleteString(i);
				nCountCmb--;
				i--;
			}
		}
	}

	if (nCountSep > 2) {
		if (cmbCmd.GetItemData(0) == 0)
			cmbCmd.DeleteString(0);

		int nIndexLast = cmbCmd.GetCount() - 1;

		if (cmbCmd.GetItemData(nIndexLast) == 0)
			cmbCmd.DeleteString(nIndexLast);
	}
}


//-------------------------------------------------
/// �C���[�W���X�g�̃C���[�W��strBmpFile�̃r�b�g�}�b�v�Œu������
BOOL _ReplaceImageList(const CString& strBmpFile, CImageList& imgs, DWORD defaultResID/* = 0*/)
{
	CBitmap 	bmp;
	bmp.Attach( AtlLoadBitmapImage(const_cast<LPTSTR>((LPCTSTR)strBmpFile), LR_LOADFROMFILE) );
  #if 0 //+++
	return _ReplaceImageList(bmp.m_hBitmap, imgs);
  #else
	if (bmp.IsNull() && defaultResID)
		bmp.LoadBitmap(defaultResID);	//+++	�t�@�C�����Ȃ������Ƃ��A�f�t�H���g�̃��\�[�X��ǂݍ���ł݂�.
	
	if (bmp.m_hBitmap) {
		int nCount = imgs.GetImageCount();
		for (int i = 0; i < nCount; ++i) {
			if ( !imgs.Remove(0) )
				return FALSE;
		}
		imgs.Add( bmp, RGB(255, 0, 255) );
		return TRUE;
	}
	return FALSE;
  #endif
}







// ===========================================================================
//+++ +mod�ǉ�


//------------------------------------------
#if 1	//+++ �����I�ɁA���̏�Ń��b�Z�[�W�����΂�...
#include "dbg_wm.h"
int ForceMessageLoop(HWND hWnd/* = NULL*/)
{
#if 1
	MSG msg = {0};
	int ret;
	int n = 0;
wm_t wm;
	CMessageLoop *pLoop 	 = _Module.GetMessageLoop();
	while ((ret = ::PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE)) != 0) {
wm = (wm_t)msg.message;
		//if (ret < 0)
		//	break;
		if (!GetMessage (&msg,hWnd,0,0))	/* ���b�Z�[�W����. QUIT��������A���U������*/
			return 0;	//x return msg.wParam ;
		if (pLoop->PreTranslateMessage(&msg) == 0) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		++n;
	}
	pLoop->OnIdle(1);
#endif
	return 1;
}
#endif



#if 0
#include "option/SkinOption.h"

///+++ �L���v�V���������̎��A�c�[���o�[�̌��Ԃ����߂�悤�ɂ��邽�߂ɁA�L���v�V������͂񂾃t��������֐�.
bool  Donut_FakeCaptionLButtonDown(HWND hWnd, HWND hWndTopLevelParent, LPARAM lParam)
{
	if (CSkinOption::s_nMainFrameCaption == 0) {
	  #if 1 //+++ ���j���[�̂��C�ɓ�����o���āA���̍��ڂ��ړ����悤�Ƃ����瑋���ړ����Ă��܂��̂ŁA���̑΍�
		CRect	rc;
		::GetWindowRect(hWnd, rc);
		POINT	pt;
		::GetCursorPos(&pt);
		if (rc.PtInRect(pt))
	  #endif
		{
			//HWND	hWndTopLevelParent = GetTopLevelWindow();
			::PostMessage(hWndTopLevelParent, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
			return true;
		}
	}
	return false;
}
#endif

//------------------------------------------
/// �t�H���g�̍�����Ԃ�
int GetFontHeight(HFONT hFont)
{
	WTL::CLogFont	lf;
	CFontHandle(hFont).GetLogFont(&lf);
	CWindowDC	dc(NULL);
	int h = (int)lf.GetHeight(dc);
	return h;
	//if (h < 0)
	//	h = -h;
	//return h;
}


//------------------------------------------
/// �G���[�R�[�h�ɑΉ�����G���[���b�Z�[�W�������Ԃ�
CString	GetLastErrorString(HRESULT hr)
{
	if (hr == -1)
		hr = AtlHresultFromLastError();//GetLastError();
	LPVOID lpMsgBuf;
	ATLVERIFY(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL));
	CString strError;
	strError.Format(_T("ErrorCode : 0x%08x\n%s"), hr, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return strError;
}


//------------------------------------------
/// �ꎞ�t�@�C���u����̃p�X��Ԃ�
bool	GetDonutTempPath(CString& strTempPath)
{
	::GetTempPath(MAX_PATH, strTempPath.GetBuffer(MAX_PATH));
	strTempPath.ReleaseBuffer();
	if (::PathIsDirectory(strTempPath) == FALSE) {
		CString strError;
		strError.Format(_T("temp�t�H���_�����݂��܂���B\n�p�X:%s"), strTempPath);
		MessageBox(NULL, strError, NULL, MB_ICONERROR);
		return false;
	}
	strTempPath += _T("unDonut_Temp\\");
	if (::PathIsDirectory(strTempPath) == FALSE)
		::CreateDirectory(strTempPath, NULL);
	return true;
}



//------------------------------------------
/// �G�N�X�v���[���[���J���ăA�C�e����I������
void	OpenFolderAndSelectItem(const CString& strPath)
{
	LPITEMIDLIST	pidlFolder = ::ILCreateFromPath(strPath);
	if (pidlFolder == nullptr)
		return ;
	LPITEMIDLIST	pidlChild = ::ILClone(::ILFindLastID(pidlFolder));
	::ILRemoveLastID(pidlFolder);

	LPCITEMIDLIST	pidlchildarr[] = { pidlChild };
	HRESULT hr = ::SHOpenFolderAndSelectItems(pidlFolder, 1, pidlchildarr, 0);
	::ILFree(pidlFolder);
	::ILFree(pidlChild);
}



