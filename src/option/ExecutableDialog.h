/**
 *	@file	ExecuteableDialog.h
 *	@brief	donut�̃I�v�V���� : "�֘A�Â�"
 */

#pragma once

#include "../resource.h"

class CDonutExecutablePropertyPage : public CPropertyPageImpl<CDonutExecutablePropertyPage> {
protected:
	// Data members
	CCheckListViewCtrl		m_listview;
	CSimpleArray<CString>	m_arrType;
	CSimpleArray<int>		m_arrChecked;

public:
	// Constants
	enum { IDD = IDD_PROPPAGE_EXECUTABLE };

	// Ctor
	CDonutExecutablePropertyPage();

	// Overrides

	BOOL OnSetActive();
	BOOL OnKillActive();
	BOOL OnApply();

	// Implementation
private:
	BOOL _DoTypeDataExchange(BOOL bSaveAndValidate);
	void _GetData();
	void _SetData();
};
