/**
*	@file	MultiThreadManager.h
*	@brief	CChildFrame�p�̃X���b�h�쐬�Ǘ�
*/

#pragma once

// �O���錾
class CChildFrame;
struct NewChildFrameData;


namespace MultiThreadManager {

int Run(LPTSTR lpstrCmdLine, int nCmdShow, bool bTray);

void	ExecuteChildFrameThread(CChildFrame* pChild, NewChildFrameData* pData);


};	// namespace MultiThreadManager





