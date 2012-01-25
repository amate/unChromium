/**
 *	@file	ParseInternetShortcutFile.cpp
 *	@brief	�C���^�[�l�b�g�V���[�g�J�b�g(.url)����url���擾. (�����g������)
 *	@note
 *	+++ MtlWeb.cpp �ɂ����� MdlParseInternetShortcutFile ���A�����֌W�̊g������������.
 *		�����o�[��A�h���X�o�[���Ăяo���ĂāA���ł� MTL �̔��e�O�����A
 *		Mtl::MDITabCtrl ������Ă΂��̂ŁA�Ƃ肠���� MTL �̃t��������.
 *		(MDITabCtrl����̌Ăѕ����P�N�b�V���������悤�ɂ��ׂ�...)
 */

#include "stdafx.h"
#include "DonutPFunc.h"
#include "DonutViewOption.h"
#include "FavoritesMenu.h"
#include "MtlDragDrop.h"
#include "HlinkDataObject.h"
#include "MenuDropTargetWindow.h"
#include "FlatComboBox.h"
#include "DonutFavoritesMenu.h"
#include "ExStyle.h"
#include "DonutSearchBar.h"
#include "MtlBrowser.h"
#include "DonutAddressBar.h"
#include "Donut.h"		// CString Donut_GetActiveSelectedText() �̂���
#include "ParseInternetShortcutFile.h"


#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



namespace MTL {


///+++ �A�h���X�o�[�̕������Ԃ�.
///    �� CSearchBar�����ɗp��. �{���� CDonutAddressBar::�̓����֐����Ăׂ΂������������A
/// 	  DonutAddressBar.h��SearchBar.h��include����ƌオ����ǂ��̂�...
CString GetAddressBarText()
{
	return	CDonutAddressBar::GetAddressBarText();
}



///+++ strFilePath ���C���^�[�l�b�g�V���[�g�J�b�g��������΁A���ۂ�url�ɂ��ĕԂ�.
///    ���̂Ƃ��A�g���v���p�e�B([unDonut])�Ō����w�肪�ݒ肳��Ă����炻��𔽉f.
/// @return 0:�C���^�[�l�b�g�V���[�g�J�b�g����Ȃ�	1:�L����url������. 2:�����w�肪���f���ꂽurl�ɂȂ���.
int ParseInternetShortcutFile_SearchMode(CString &strFilePath, const CString& strSearch)
{
	CString 	strExt = strFilePath.Right(4);
	if (strExt.CompareNoCase( _T(".url") ) != 0)
		return 0;

	CIniFileI	pr0(strFilePath, _T("InternetShortcut"));
	CString 	strUrl = pr0.GetString(_T("URL"), NULL, INTERNET_MAX_PATH_LENGTH );
	pr0.Close();
	if (strUrl.IsEmpty())
		return 0;

  #if 0 //+++ �g���v���p�e�B��on����Ă��邩�`�F�b�N.
	CIniFileI	pr(strFilePath, DONUT_SECTION);
	if (pr.GetValue(EXPROP_KEY_ENABLED, 0) == 0) {
		strFilePath = strUrl;
		return 1;
	}
  #endif

	//+++ ���ɂ΂�����...���A�Ƃɂ����������ƗD��...
	CDonutSearchBar* pSearch = CDonutSearchBar::GetInstance();
  #if 1 //ndef NDEBUG
	if (pSearch) {
	//	CDonutSearchBar *pSearch0 = pSearch;
	//	pSearch = (CDonutSearchBar *) ::SendMessage(pSearch->GetTopLevelParent(), WM_GET_SEARCHBAR, 0, 0);
	//	ATLASSERT( pSearch == pSearch0 );
	}
  #endif
	if (pSearch) {
		bool		bUrlSearch = false;

		CString 	strIniFile	= strFilePath;
		CString 	strWord 	= strSearch;		// ����������.

		CIniFileI	pr(strIniFile, DONUT_SECTION/*_T("unDonut")*/);
		bool		bAdrBar = pr.GetValue(_T("ExPropOpt"), 0) & 1;
		pr.Close();

	   #if 1	//�����Ȃ̂ŁA�ՂŁA�d�g�݂𒼂�
		// �I��͈͂�����΁A�����D�悷��.
		CString 		strSel = Donut_GetActiveSelectedText();
		if (strSel.IsEmpty() == 0) {
			strWord = strSel;
			if (bAdrBar)	// �A�h���X�o�[�������Ă���w�肪����Ƃ�.
				bUrlSearch = true;
		}
	   #endif
		if (strWord.IsEmpty()) {
			if (bAdrBar) {	// �A�h���X�o�[�������Ă���w�肪����Ƃ�.
				strWord    = CDonutAddressBar::GetAddressBarText();
				bUrlSearch = true;
			}
			if (strWord.IsEmpty()) {	// �܂����������񂪐ݒ肳��Ă��Ȃ��ꍇ�́A�����o�[�̃e�L�X�g���擾.
				strWord    = MtlGetWindowText(pSearch->GetEditCtrl());
				             //pSearch->RemoveShortcutWord( MtlGetWindowText( pSearch->GetEditCtrl() ) );
				bUrlSearch = false;
			}
		  #if 0
			if (strWord.IsEmpty()) {	// �܂������񂪋󂾂�����΁A�e�L�X�g�I��������΂�����̗p.
				strWord    = Donut_GetActiveSelectedText();
			}
		  #endif
		}
		strFilePath   = strUrl;
		return 2;	//\\test
#if 0
		if (strWord.IsEmpty() == FALSE) {	// ���������񂪐ݒ肳��Ă�����A���̂悤��Url���쐬.
			CIniFileI	pr(strIniFile, DONUT_SECTION/*_T("unDonut")*/);
			bool rc = pSearch->GetOpenURLstr(strFilePath, strWord, pr, CString()/*strUrl*/);
			pr.Close();
			if (rc && bUrlSearch == 0) {	// url�����̎��ȊO�͗����ɓ����.
				pSearch->_AddToSearchBoxUnique(strWord);
			}
			return 2;
		}
#endif
	} else {
		strFilePath = strUrl;
	}
	return 1;
}



///+++ strFilePath ���C���^�[�l�b�g�V���[�g�J�b�g��������΁A���ۂ�url�ɂ��ĕԂ�.
///    ���̂Ƃ��A�g���v���p�e�B�Ō����w��(unDonutSearch)���ݒ肳��Ă����炻��𔽉f.
bool ParseInternetShortcutFile(CString &strFilePath)
{
	return ParseInternetShortcutFile_SearchMode(strFilePath, CString()) != 0;
}


}	// MTL
