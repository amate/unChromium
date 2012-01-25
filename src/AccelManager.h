/**
 *	@file	AccelManager.h
 *	@brief	�L�[�A�N�Z�����[�^(�V���[�g�J�b�g�L�[)�Ɋւ��鏈�����󂯎��N���X�Q
 *	@note
 *	CKeyHelper
 *	CAccelerManager
 *	CEditAccel
 *	��L����
 */


#ifndef __ACCELMANAGER_H__
#define __ACCELMANAGER_H__

#if _MSC_VER > 1000
 #pragma once
#endif	// _MSC_VER > 1000

#include <windows.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <atlctrls.h>



/*
	CKeyHelper
	�A�N�Z�����[�^�L�[�̏�񂩂�V���[�g�J�b�g�L�[������𐶐�����w���p�N���X

	CKeyHelper��1�V���[�g�J�b�g�L�[�̏�Ԃ���"Ctrl+Shift+R","F8"�̂悤�ȕ������
	��������V���[�g�J�b�g�L�[���������߂̕⏕�N���X�ł��B

	CKeyHelper(lpAccel);�܂���SetAccelerator(lpAccel)���Ăяo����
	��������擾�������A�N�Z�����[�^��ݒ肵�܂��B
	������Format�֐����Ăяo���ƈ����̎Q�Ƃ���ĕ�����𓾂邱�Ƃ��o���܂��B
 */
class CKeyHelper {
protected:
	//�����o�ϐ�
	LPACCEL 	m_lpAccel;									//�A�N�Z�����[�^�̃|�C���^

	//�����֐�
	//void		AddVirtKeyStr(CString &str, UINT uiVirtKey, BOOL bLast = FALSE) const;
	bool		AddVirtKeyStr(CString &str, UINT uiVirtKey, LPCTSTR xname=NULL, BOOL bLast = FALSE) const;

public:
	//�������E�j��
	CKeyHelper();											//�f�t�H���g�R���X�g���N�^
	CKeyHelper(LPACCEL lpAccel);							//�p�����[�^�t���R���X�g���N�^
	virtual ~CKeyHelper();									//�f�X�g���N�^

	//����
	bool		Format(CString &str) const; 				//�A�N�Z�����[�^�̐ݒ�ɂ�鏑����

	//����
	void		SetAccelerator(LPACCEL lpAccel);			//�N���X�Ŏ�舵���A�N�Z�����[�^�̎w��
};



/*
	CAccelerManager
	�A�N�Z�����[�^�e�[�u���ƌʂ̃A�N�Z�����[�^�Ɋւ��郁�\�b�h��L����
	�A�N�Z�����[�^�Ǘ��N���X

	�A�N�Z�����[�^�e�[�u���ɑ΂��ăR�}���hID�ɂ�錟���A�d�������A
	�A�N�Z�����[�^�̒ǉ���폜�Ƃ������@�\��L���܂��B�܂��������t�@�C����
	���[�h�E�Z�[�u�@�\�ɂ���ăL�[�A�N�Z�����[�^�̏���ۑ��E�Đ��ł��܂��B

	�R���X�g���N�^���Ăяo���ăL�[�A�N�Z�����[�^�̃n���h����ݒ肵�܂��B
	���Ƃ͊e��֐����Ăяo���܂��B
 */
class CAccelerManager {
private:
	//�����o�ϐ�
	HACCEL		m_hAccel;									//�A�N�Z�����[�^�e�[�u���̃n���h��
	LPACCEL 	m_lpAccel;									//�A�N�Z�����[�^�W���̃|�C���^
	int 		m_nAccelSize;								//�A�N�Z�����[�^�̐�

public:
	CAccelerManager(HACCEL hAccel); 						//�R���X�g���N�^
	virtual ~CAccelerManager(); 							//�f�X�g���N�^

	int 		GetCount() const;							//�e�[�u�����̃A�N�Z�����[�^�̐����擾
	LPACCEL 	GetAt(int index);							//�w�肵���ԍ��̃A�N�Z�����[�^���擾(0origin)
	BOOL		FindAccelerator(UINT uiCmd, CString &str);	//�w�肵���R�}���hID�����A�N�Z�����[�^������

	//�߂�l�͌����������ǂ����B���������ꍇ�ɂ�
	//����str�ɃL�[�̑g�ݍ��킹�����������񂪕Ԃ����
	UINT		FindCommandID(ACCEL *pAccel);				//�w�肵���A�N�Z�����[�^�ƃL�[����v������̂̃R�}���hID��Ԃ�

	//������Ȃ��ꍇ��0��Ԃ�
	HACCEL		DeleteAccelerator(UINT uCmd);				//�w�肵���R�}���hID�����A�N�Z�����[�^���폜����
	HACCEL		AddAccelerator(ACCEL *lpAccel); 			//�w�肵���A�N�Z�����[�^���e�[�u���ɒǉ�����
	HACCEL		LoadAccelaratorState(HACCEL hAccel);		//�t�@�C������A�N�Z�����[�^�̏������[�h����

	//(minit)�����̓t�@�C�����ǂ߂Ȃ��ꍇ�̑�փA�N�Z�����[�^�e�[�u��?
	BOOL		SaveAccelaratorState(); 					//�t�@�C���ɃA�N�Z�����[�^�e�[�u���̏�Ԃ��L�^����
};



/*
	CEditAccel
	�z�b�g�L�[�R���g���[���̂悤�ȃe�L�X�g�{�b�N�X����������N���X

	�L�[�{�[�h���b�Z�[�W�����߂��ē��͂ɉ����đΉ������L�[�������\������
	�e�L�X�g�{�b�N�X���쐬���܂��B

 ##�R�����g�ǉ��r��##

 */
class CEditAccel : public CContainedWindowT < CEdit > {
private:
	BOOL	   m_bKeyDefined;
	CKeyHelper m_Helper;
	ACCEL	   m_Accel;

	void		SetAccelFlag(BYTE bFlag, BOOL bOn);

public:
	CEditAccel();

	ACCEL const *GetAccel() const;
	void		ResetKey();
	BOOL		OnTranslateAccelerator(MSG *pMsg);
};



#endif
