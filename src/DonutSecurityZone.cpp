/**
 *	@file	DonutSecurityZone.cpp
 *	@brief	�N�b�L�[�̈����Ɋւ���R�}���h���b�Z�[�W���n���h�����O����N���X
 */
#include "stdafx.h"
#include "DonutSecurityZone.h"


#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




void CDonutSecurityZone::OnUrlActionCookies(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	//�N�b�L�[�̎g�p��Ԕ��]
	MtlSetInternetZoneActionPolicy(URLACTION_COOKIES,
			MtlIsInternetZoneActionPolicyDisallow(URLACTION_COOKIES) ? URLPOLICY_ALLOW : URLPOLICY_DISALLOW);
}



void CDonutSecurityZone::OnUrlActionCookiesSession(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	//�Z�b�V�������̃N�b�L�[�g�p��Ԕ��]
	MtlSetInternetZoneActionPolicy(URLACTION_COOKIES_SESSION,
			MtlIsInternetZoneActionPolicyDisallow(URLACTION_COOKIES_SESSION) ? URLPOLICY_ALLOW : URLPOLICY_DISALLOW);
}



// UH
void CDonutSecurityZone::OnUrlActionCookiesChg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	//�������]
	OnUrlActionCookies(0,0,0);
	OnUrlActionCookiesSession(0,0,0);
}

