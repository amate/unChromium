/**
 *	@file	ParseInternetShortcutFile.h
 *	@brief	�C���^�[�l�b�g�V���[�g�J�b�g(.url)����url���擾. (�����g������)
 *	@note
 *		MtlWeb.cpp �ɂ����� MdlParseInternetShortcutFile ���A�����֌W�̊g����
 *		��������.
 *		�����o�[��A�h���X�o�[���Ăяo���ĂāA���ł� MTL �̔��e�O�����A
 *		Mtl::MDITabCtrl ������Ă΂��̂ŁA�Ƃ肠���� MTL �̃t��������.
 *		(MDITabCtrl����̌Ăѕ����P�N�b�V���������悤�ɂ��ׂ�...)
 */

#pragma once

namespace MTL {

///+++ unDonut���̏������Ăт܂���悤�ȉ��ς������̂ŁAMtlWeb.cpp ����ړ�.
bool	ParseInternetShortcutFile(CString &strFilePath);

///+++ �����N�o�[�Ńh���b�v���ꂽ���������������ꍇ�p.
int 	ParseInternetShortcutFile_SearchMode(CString &strFilePath, const CString& strSearch);

///+++ �A�h���X�o�[�̕������Ԃ�.
///    �� CSearchBar�����ɗp��. �{���� CDonutAddressBar::�̓����֐����Ăׂ΂������������A
/// 	  DonutAddressBar.h��SearchBar.h��include����ƌオ����ǂ��̂�...
CString GetAddressBarText();

}	// namespace MTL
