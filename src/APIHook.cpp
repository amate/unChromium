/**
*	@file APIHook.cpp
*/

#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
//#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <Dbghelp.h>

// ImageDirectoryEntryToData
#pragma comment(lib, "Dbghelp.lib")

// �ЂƂ̃��W���[���ɑ΂���API�t�b�N���s���֐�
void ReplaceIATEntryInOneMod(
                             PCSTR pszModuleName,
                             PROC pfnCurrent,
                             PROC pfnNew,
                             HMODULE hmodCaller) 
{
    ULONG ulSize;
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
    pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
        hmodCaller, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);

    if (pImportDesc == NULL)
        return;

    while(pImportDesc->Name) {
        PSTR pszModName = (PSTR) ((PBYTE) hmodCaller + pImportDesc->Name);
        if (lstrcmpiA(pszModName, pszModuleName) == 0) 
            break;
        pImportDesc++;
    }

    if (pImportDesc->Name == 0)
        return;

    PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA) 
        ((PBYTE) hmodCaller + pImportDesc->FirstThunk);

    while(pThunk->u1.Function) {
        PROC *ppfn = (PROC*) &pThunk->u1.Function;
        BOOL fFound = (*ppfn == pfnCurrent);
        if (fFound) {
            DWORD dwDummy;
            VirtualProtect(ppfn, sizeof(ppfn), PAGE_EXECUTE_READWRITE, &dwDummy);
            WriteProcessMemory(
                GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), NULL);
            return;
        }
        pThunk++;
    }
    return;
}

// ���ׂẴ��W���[���ɑ΂���API�t�b�N���s���֐�
void ReplaceIATEntryInAllMods(
                              PCSTR pszModuleName, 
                              PROC pfnCurrent, 
                              PROC pfnNew) 
{
    // ���W���[�����X�g���擾
    HANDLE hModuleSnap = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE, GetCurrentProcessId());
    if(hModuleSnap == INVALID_HANDLE_VALUE)
        return;

    MODULEENTRY32 me;
    me.dwSize = sizeof(me);
    BOOL bModuleResult = Module32First(hModuleSnap, &me);
    // ���ꂼ��̃��W���[���ɑ΂���ReplaceIATEntryInOneMod�����s
    while(bModuleResult) {        
        ReplaceIATEntryInOneMod(pszModuleName, pfnCurrent, pfnNew, me.hModule);
        bModuleResult = Module32Next(hModuleSnap, &me);
    }
    CloseHandle(hModuleSnap);
}

#if 0
// �t�b�N����֐��̃v���g�^�C�v���`
typedef int (WINAPI *PFNMESSAGEBOXA)(HWND, PCSTR, PCSTR, UINT);
typedef int (WINAPI *PFNMESSAGEBOXW)(HWND, PCSTR, PCSTR, UINT);

int WINAPI Hook_MessageBoxA(
                            HWND hWnd, 
                            PCSTR pszText, 
                            PCSTR pszCaption, 
                            UINT uType)
{
    // �I���W�i��MessageBoxA���Ăяo��
    PROC pfnOrig = GetProcAddress(
        GetModuleHandleA("user32.dll"), "MessageBoxA");
    int nResult = ((PFNMESSAGEBOXA) pfnOrig)
        (hWnd, pszText, _T("I am Hook_MessageBoxA"), uType);
    return nResult;
}

int WINAPI Hook_MessageBoxW(
                            HWND hWnd, 
                            PCSTR pszText, 
                            PCSTR pszCaption, 
                            UINT uType)
{
    // �I���W�i��MessageBoxA���Ăяo��
    PROC pfnOrig = GetProcAddress(
        GetModuleHandleA("user32.dll"), "MessageBoxW");
    int nResult = ((PFNMESSAGEBOXA) pfnOrig)
        (hWnd, pszText, _T("I am Hook_MessageBoxW"), uType);
    return nResult;
}
#endif

#if 0
int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    PROC pfnOrig;
    pfnOrig = ::GetProcAddress(
        GetModuleHandleA("user32.dll"), "MessageBoxA");
    ReplaceIATEntryInAllMods(
        "user32.dll", pfnOrig, (PROC)Hook_MessageBoxA);

    pfnOrig = ::GetProcAddress(
        GetModuleHandleA("user32.dll"), "MessageBoxW");
    ReplaceIATEntryInAllMods(
        "user32.dll", pfnOrig, (PROC)Hook_MessageBoxW);

    MessageBox(GetActiveWindow(), 
        _T("���b�Z�[�W�{�b�N�X�\���̃e�X�g"), _T("�e�X�g"), MB_OK);
    return 0;
}

#endif

#if 0
typedef UINT_PTR (WINAPI *PFNSETTIMER)(HWND, UINT_PTR, UINT, TIMERPROC);

PFNSETTIMER pfnOrg;

UINT_PTR WINAPI Hook_SetTimer(
  HWND hWnd,              // �E�B���h�E�̃n���h��
  UINT_PTR nIDEvent,      // �^�C�}�̎��ʎq
  UINT uElapse,           // �^�C���A�E�g�l
  TIMERPROC lpTimerFunc)   // �^�C�}�̃v���V�[�W��
{
	return pfnOrg(hWnd, nIDEvent, uElapse, lpTimerFunc);
}

APIHook("user32.dll", "SetTimer", (PROC)Hook_SetTimer, (PROC*)&pfnOrg);

#endif

void	APIHook(PCSTR pszModuleName, PCSTR pszFuncName, PROC pfnReplace, PROC* ppfnOrig)
{
    PROC pfnOrig = ::GetProcAddress(GetModuleHandleA(pszModuleName), pszFuncName);
	if (pfnOrig == NULL)
		return;

    ReplaceIATEntryInAllMods(
        pszModuleName, pfnOrig, pfnReplace);
	*ppfnOrig = pfnOrig;
}