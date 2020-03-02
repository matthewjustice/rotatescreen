/* -------------------------------------------------------------

rotatescreen.h
   Rotate Screen

by: Matthew Justice

---------------------------------------------------------------*/

#ifndef _ROTATESCREEN_H_
#define _ROTATESCREEN_H_

//
// Constants
//
#define NOTIFYICONID              0
#define WM_NOTIFYICON             WM_APP+1
#define CCH_APP_TITLE             64	
#define CCH_DGB_PRINT_BUFFER      1024
#define RS_HOTKEY_ID              0
#define DISPLAY_UNDEFINED         0xFFFFFFFF

//
// Function prototypes
//
LRESULT CALLBACK HiddenWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL InitApp();
BOOL InitWindow(int);
int MsgLoop(void);
BOOL AppIsAlreadyRunning();
void HiddenWnd_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL HiddenWnd_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void HiddenWnd_OnClose(HWND hwnd);
void HiddenWnd_OnNotifyIcon(HWND hwnd, UINT nTaskbarIconId, UINT nMsg);
void HiddenWnd_OnHotKey(HWND hwnd, int idHotKey, UINT fsModifiers, UINT vk);
void __cdecl DbgPrintf(PTSTR tszFormat, ...);
void ChangeNotifyIcon(HWND hwnd, WORD wIdIcon);
void Rotate(DWORD dwDisplayIndex);

#endif // _ROTATESCREEN_H_