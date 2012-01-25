/**
 *	@file	ToolTipManager.h
 *	@brief	�c�[���`�b�v�E�}�l�[�W��.
 */
#pragma once


#include "ExMenu.h"



class CToolTipManager 
{
public:
	// nCmdID�Ɋ֘A�t����ꂽ�������strText�ɐݒ肷��
	static bool LoadToolTipText(UINT nCmdID, CString &strText)
	{
		enum { SIZE = 1000/*256*/ };
		TCHAR	szBuff[SIZE+2] = _T("\0");
		TCHAR	szText[SIZE+2] = _T("\0");

		int 	nRet = ::LoadString(_Module.GetResourceInstance(), nCmdID, szBuff, SIZE);

		for (int i = 0; i < nRet; i++) {
			if ( szBuff[i] == _T('\n') ) {	// ���s�������炻���܂ł̕������Ԃ�
				lstrcpyn(szText, &szBuff[i + 1], SIZE);
				strText = szText;
				return true;
			}
		}

		CString 	strTip = CExMenuManager::GetToolTip(nCmdID);

		if ( strTip.IsEmpty() ) {
			return false;
		} else {
			strText = strTip;
			return true;
		}
	}
};

