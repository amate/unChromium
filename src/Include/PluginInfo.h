/**
 *	@file	PluginInfo.h
 *  @brief	unDonut�̃v���O�C���Ɋւ���e���`.
 *	@note
 *	unDonut�̃v���O�C���Ɋւ���e���`���L�q�����w�b�_�ł��B
 *	�v���O�C�����쐬����ۂɂ͂��̃w�b�_���C���N���[�h���Ă��������B
 */
#ifndef _x_PLUGININFO_H_x_
#define _x_PLUGININFO_H_x_

#pragma once

#include <windows.h>



// PluginInfo : �v���O�C���̏��E�ݒ��ۗL����\����
//			  : �{�̂�GetPluginInfo()�֐��ɂ���ăv���O�C���ɂ��̍\���̂ɏ���ݒ肷�邱�Ƃ�v�����܂��B
typedef struct _PLUGININFO {
	int  	type;								// �v���O�C���̎�ނƃX�^�C����g�ݍ��킹�Đݒ肵�܂��B
												// �d�v�Ȃ̂͂��̒l�����Ȃ̂ŁA��������Ɛݒ肵�Ă��������B
	char 	name[100]; 							// �v���O�C���̖��̂�ݒ肵�܂��B
	char 	version[100];						// �o�[�W�����ł��B�����͎��R�ł��B
	char 	versionDate[100];					// �X�V���������邱�Ƃ���������܂����A���Ɏg���Ă��\���܂���B
	char 	authorName[100];					// ��Җ���ݒ肵�܂��B
	char 	authorEmail[100];					// ��҂̘A�����ݒ肵�܂��B(�����Ă��\���܂���)
	char 	authorUrl[100];						// �K�v�ł���΍�҂�Web�T�C�g��URL��ݒ肵�Ă��������B
	char 	comment[1024 * 10];					// �v���O�C���̐�����֘A����ݒ肵�Ă��������B
} PLUGININFO;


// PluginType : PLUGININFO�\���̂�type�����o�ɐݒ肷��v���O�C���^�C�v�̒萔�ł��B
//			  : ���̒������I��Őݒ肵�܂��B
typedef enum _PLUGIN_TYPE {
	PLT_TOOLBAR 	= 1,						// ���o�[�Ɋi�[�����c�[���o�[�v���O�C��
	PLT_EXPLORERBAR = 2,						// �v���O�C���o�[�Ɋi�[�����G�N�X�v���[���o�[�v���O�C��
	PLT_STATUSBAR	= 3,						// �X�e�[�^�X�o�[�Ɋi�[�����X�e�[�^�X�o�[�v���O�C��(�������ł�)
	PLT_OPERATION	= 4,						// �\�ɂ͕\������Ȃ��o�b�N�O���E���h�v���O�C��(�������ł�)
	PLT_DOCKINGBAR	= 5, 						// �h�b�L���O�E�B���h�E�Ƃ��Ĕz�u�����h�b�L���O�o�[�v���O�C��
} PLUGIN_TYPE;

#define PLUGIN_TYPECNT	5						// �v���O�C���̎�ސ�(�b��)



// PluginStyle : PLUGININFO�\���̂�type�����o�ɐݒ肷��v���O�C���̃X�^�C���萔�ł��B
//			   : PLUGIN_TYPE�̒萔�Ƒg�ݍ��킹�Đݒ肵�Ă��������B
#define PLUGIN_STYLE_DERAYLOADING	0			//�{�͕̂K�v�ɂȂ����Ƃ��Ƀv���O�C���ɃE�B���h�E������v�����܂��B
#define PLUGIN_STYLE_EARLYLOADING	16			//�{�̂̏������������Ƀv���O�C���ɃE�B���h�E������v�����܂��B

//PLUGIN_STYLE_DERAYLOADING��PLUGIN_STYLE_EARLYLOADING�͓����Ɏw�肷�邱�Ƃ͏o���܂���B
//�����̃t���O�̓G�N�X�v���[���o�[�v���O�C���ł̂ݗL���ł��B
//����ȊO�͖{�̂̏��������܂��͏�������ɃE�B���h�E�������s���܂��B


//PluginType :�v���O�C���֐����g���^���̒�`�ł��B����Ȃɑ厖�Ȃ��̂ł�����܂��񂪁B
#define FPARAM LONG_PTR
#define SPARAM LONG_PTR


// Plugin Event ID : �v���O�C���ɑ΂��đ��M�����{�̑��C�x���g�̎���ID�ł��B
//				   : �{�̂�PluginEvent()�֐���ʂ��ăv���O�C���Ɋe��C�x���g�̔�����ʒm���܂��B
#define DEVT_INITIALIZE_COMPLETE	1			//�{�̑��̏��������������܂����B
#define DEVT_TAB_OPENED 			2			//�V�����^�u����������܂����B
#define DEVT_TAB_CHANGEACTIVE		3			//�A�N�e�B�u�ȃ^�u���ύX����܂����B
#define DEVT_TAB_BEFORENAVIGATE 	4			//�V�����y�[�W�Ƀi�r�Q�[�g����悤�Ƃ��Ă��܂��B
#define DEVT_TAB_NAVIGATE			5			//�i�r�Q�[�g���J�n����܂����B
#define DEVT_TAB_DOCUMENTCOMPLETE	6			//�i�r�Q�[�g���������܂����B
#define DEVT_TAB_CLOSE				7			//�^�u�������܂����B
#define DEVT_TAB_MOVE				8			//�^�u���ړ�����܂����B(�������ł��B���߂�Ȃ���)
#define DEVT_CHANGESIZE 			10			//�{�̃E�B���h�E�̃T�C�Y���ύX����܂����B
#define DEVT_CHANGESTATUSTEXT		11			//�X�e�[�^�X�o�[�̃e�L�X�g���ύX����܂����B
#define DEVT_BLOCKTITLE 			12			//�^�C�g���ɂ���ă^�u�̃I�[�v�����L�����Z������܂����B
#define DEVT_BLOCKPOPUP 			13			//URL�ɂ���ă^�u�̃I�[�v�����L�����Z������܂����B


// Plugin Functions' Parameters : �C�x���g�̕����Ƃ��đ����Ă���f�[�^���i�[����\���̂ł��B
typedef struct _DEVTS_TAB_NAVIGATE {			//DEVT_TAG_NAVIGATE�C�x���g�̃p�����[�^�f�[�^���i�[���܂��B
	int 		nIndex; 						//�i�r�Q�[�g�����^�u�̃C���f�b�N�X
	LPCTSTR 	lpstrURL;						//�i�r�Q�[�g���URL
	LPCTSTR 	lpstrTargetFrame;				//�i�r�Q�[�g��̃t���[����
} DEVTS_TAB_NAVIGATE;


typedef struct _DEVTS_TAB_DOCUMENTCOMPLETE {	//DEVT_TAB_DOCUMENTCOMPLETE�C�x���g�̃p�����[�^�f�[�^���i�[���܂��B
	int 		nIndex; 						//�i�r�Q�[�g���������^�u�̃C���f�b�N�X
	LPCTSTR 	lpstrURL;						//�^�u�̐V����URL
	LPCTSTR 	lpstrTitle; 					//�^�u�̐V�����^�C�g��
	BOOL		bMainDoc;						//�ŏ�ʂ̃h�L�������g(�t���[��)���ǂ���
	IDispatch*	pDispBrowser;					//QueryInterface��IWebBrowser2*�����o����
} DEVTS_TAB_DOCUMENTCOMPLETE;



#endif
