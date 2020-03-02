/* -------------------------------------------------------------

rotatescreen.cpp
   Rotate Screen

by: Matthew Justice

---------------------------------------------------------------*/

#include <windows.h>
#include <tchar.h>
#include <Windowsx.h>
#include <strsafe.h>
#include "resource.h"
#include "rotatescreen.h"
#include "rsconfigdlg.h"

//
// Globals
//
HINSTANCE g_hinst;
HWND      g_hwndHidden;
TCHAR     g_tszClassHidden[] = TEXT("HiddenWinClass");
TCHAR     g_tszShortAppName[] = TEXT("Rotate Screen");
UINT      g_fsModifiers = 0;
UINT      g_vk = 0;
DWORD     g_dwDisplay = DISPLAY_UNDEFINED;

//
// WinMain
// program entry point
//
int WINAPI WinMain(
   HINSTANCE hinst,     // instance handle
   HINSTANCE hinstPrev, // always NULL
   LPSTR pszCmd,        // pointer to ANSI command line arguments
   int nCmdShow)        // initial app window state
{

   // save the value of the main instance handle
   g_hinst = hinst;
   
   // check to see if another instance of this app is running
   if(AppIsAlreadyRunning())
   {
      return -1;
   }
   
   // read the global settings from the registry
   ReadRegistrySettings(&g_fsModifiers, &g_vk, &g_dwDisplay);

   // Initialize the window data and register the window class
   if (!InitApp())
   {
      MessageBox(NULL, TEXT("Error during app init"), 
      TEXT("notify"), MB_ICONERROR);
      return -1;
   }

   // Create the window but do not display it
   if(!InitWindow(nCmdShow))
   {
      MessageBox(NULL, TEXT("Error during window init"), 
      TEXT("notify"), MB_ICONERROR);
      return -1;
   }
   
   // Register the hotkey if one was specified in the registry
   if(g_fsModifiers && g_vk)
   {
      if(!RegisterHotKey(g_hwndHidden, RS_HOTKEY_ID, g_fsModifiers, g_vk))
      {
         DbgPrintf(L"Rotate Screen: RegisterHotKey failed with 0x%08x\n", GetLastError());
         MessageBox(NULL, TEXT("Unable to register requested hotkey."), TEXT("Rotate Screen"), MB_ICONERROR); 
      }
   }

   // run the message loop
   return MsgLoop();	
}

//
// InitApp
// Initialize the window data and register the window class
//
BOOL InitApp()
{
   // a window class structure describes the window
   WNDCLASSEX wc;	
   ZeroMemory(&wc, sizeof(wc));

   // required parameters
   wc.cbSize = sizeof(wc);	               // the size in bytes of the structure
   wc.lpfnWndProc = HiddenWndProc;         // the window procedure for the class
   wc.hInstance = g_hinst;	               // the instance handle of the program 
   wc.lpszClassName = g_tszClassHidden;    // the name of the class

   // additional parameters
   wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
   wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

   // register the class so it can be used to create windows
   return RegisterClassEx(&wc);
}

//
// InitWindow
// Create the window and display it
// Save the window instance in a global
//
BOOL InitWindow(int nCmdShow)	
{
   // save the app title in tszAppTitle		
   TCHAR tszAppTitle[CCH_APP_TITLE];
   LoadString(g_hinst, IDS_APPTITLE, tszAppTitle, CCH_APP_TITLE);

   // create the main window; store its handle in g_hwndHidden
   // this is a dummy window - it won't be shown
   // I'll use its message loop for the notification icon
   g_hwndHidden = CreateWindowEx(0, g_tszClassHidden, tszAppTitle,
      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,	1, 1,
      NULL, NULL,	g_hinst, 0);

   if (!g_hwndHidden)
   {
      return FALSE;
   }

   // don't make the window visible
   // ShowWindow(g_hwndHidden, nCmdShow);

   return TRUE;
}

//
// HiddenWndProc
// The window proc for the hidden window
//
LRESULT CALLBACK HiddenWndProc(	
   HWND hwnd,
   UINT msg,
   WPARAM wParam, 
   LPARAM lParam)
{
   
    switch (msg)
    {
      HANDLE_MSG(hwnd, WM_COMMAND, HiddenWnd_OnCommand);
      HANDLE_MSG(hwnd, WM_CREATE, HiddenWnd_OnCreate);
      HANDLE_MSG(hwnd, WM_CLOSE, HiddenWnd_OnClose);
      HANDLE_MSG(hwnd, WM_HOTKEY, HiddenWnd_OnHotKey);
      case WM_NOTIFYICON:
         HiddenWnd_OnNotifyIcon(hwnd, wParam, lParam);
         break;
      case WM_DESTROY:
         PostQuitMessage(0);   //  quit the program by posting a WM_QUIT
         break;
      default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}


//
// MsgLoop
// message loop
//
int MsgLoop(void)
{
   MSG msg;

   while (GetMessage(&msg, NULL, 0, 0))
   {
      TranslateMessage(&msg);		// translate WM_KEYDOWN to WM_CHAR
      DispatchMessage(&msg);
   }

   return (int) msg.wParam;
}

BOOL AppIsAlreadyRunning()
{
   // save the app title in tszAppTitle		
   TCHAR tszAppTitle[CCH_APP_TITLE];
   LoadString(g_hinst, IDS_APPTITLE, tszAppTitle, CCH_APP_TITLE);
   
   if(FindWindow(NULL, tszAppTitle))
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

//
// HiddenWnd_OnCreate handles WM_CREATE
//
BOOL HiddenWnd_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
   NOTIFYICONDATA nid;
   HICON hIcon;

   //
   // Add the notification icon
   //
   ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
   hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_NOTIFIER));
   nid.cbSize = sizeof(NOTIFYICONDATA);
   nid.hWnd = hwnd;
   nid.uID = NOTIFYICONID;
   nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
   nid.uCallbackMessage = WM_NOTIFYICON;
   nid.hIcon = hIcon;
   StringCchCopy(nid.szTip, 64, g_tszShortAppName);
   Shell_NotifyIcon(NIM_ADD, &nid);

   return TRUE;
}

//
// HiddenWnd_OnClose handles WM_CLOSE
//
void HiddenWnd_OnClose(HWND hwnd)
{
   NOTIFYICONDATA nid;

   //
   // Delete the notification icon
   //
   ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
   nid.cbSize = sizeof(NOTIFYICONDATA);
   nid.hWnd = hwnd;
   nid.uID = 0;
   Shell_NotifyIcon(NIM_DELETE, &nid);
   
   //
   // Unregister the hotkey
   //
   if(!UnregisterHotKey(hwnd, RS_HOTKEY_ID))
   {
      DbgPrintf(L"Rotate Screen: UnregisterHotKey failed with 0x%08x\n", GetLastError());
   }

   //
   // When the main window is closed, destroy it
   //
   DestroyWindow(hwnd);

   return;
}
//
// HiddenWnd_OnCommand
// handle WM_COMMAND messages
//
void HiddenWnd_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   switch(id)
   {
   case IDM_SHOW_CONFIG_DIALOG:
      DisplayConfigDialog();
      break;
   case IDM_EXIT:
      SendMessage(hwnd, WM_CLOSE, 0, 0);
      break;
   }

   // FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, DefWindowProc);

   return;
}

//
// HiddenWnd_OnNotifyIcon
// Process messages send to the notification icon
//
void HiddenWnd_OnNotifyIcon(HWND hwnd, UINT nTaskbarIconId, UINT nMsg)
{
   if(nTaskbarIconId == NOTIFYICONID && nMsg == WM_LBUTTONDBLCLK)
   {
      DisplayConfigDialog();
   }
   else if(nTaskbarIconId == NOTIFYICONID && nMsg == WM_RBUTTONUP)
   {
      HMENU hMenuPopup;
      POINT pt = {0};
      
      if(!GetCursorPos(&pt))
      {
         pt.x = 0;
         pt.y = 0;
      }

      hMenuPopup = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_NOTIFY_POPUP));

      if(hMenuPopup)
      {
         // SetForegroundWindow / PostMessage calls present to fix focus issues with popups from notification area
         SetForegroundWindow(g_hwndHidden);
         TrackPopupMenu(GetSubMenu(hMenuPopup, 0), TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
            pt.x, pt.y, 0, g_hwndHidden, NULL);
         PostMessage(g_hwndHidden, WM_NULL, 0, 0);
      }
   }
}

//
// HiddenWnd_OnHotKey
// Process WM_HOTKEY
//
void HiddenWnd_OnHotKey(HWND hwnd, int idHotKey, UINT fsModifiers, UINT vk)
{
   DbgPrintf(L"Rotate Screen: Hotkey=0x%x, fsModifiers=0x%x, vk=0x%x\n", idHotKey, fsModifiers, vk);
   
   Rotate(g_dwDisplay);
}

//
// DbgPrintf
// printf-style function for writing debugger output
//
void __cdecl DbgPrintf(PTSTR tszFormat, ...)
{
   PTSTR ptszOutputBuffer;
   va_list vaArgs;
   
   ptszOutputBuffer = (PTSTR) HeapAlloc(
      GetProcessHeap(), 
      HEAP_ZERO_MEMORY, 
      CCH_DGB_PRINT_BUFFER * sizeof(TCHAR));
   
   if(ptszOutputBuffer)
   {
      // get the address of the arguments
      va_start(vaArgs, tszFormat);
      
      if(StringCchVPrintf(ptszOutputBuffer, CCH_DGB_PRINT_BUFFER, tszFormat, vaArgs) == S_OK)
      {
         OutputDebugString(ptszOutputBuffer);
      }
      
      va_end(vaArgs);
      
      HeapFree(GetProcessHeap(), 0, ptszOutputBuffer);
   }
   
   return;
}

//
// Make the notification icon something different
// 
void ChangeNotifyIcon(HWND hwnd, WORD wIdIcon)
{
   NOTIFYICONDATA nid;
   HICON hIcon;

   //
   // Add the notification icon
   //
   ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
   hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(wIdIcon));
   nid.cbSize = sizeof(NOTIFYICONDATA);
   nid.hWnd = hwnd;
   nid.uID = NOTIFYICONID;
   nid.uFlags = NIF_ICON;
   nid.hIcon = hIcon;
   Shell_NotifyIcon(NIM_MODIFY, &nid);
   
   return;
}

void Rotate(DWORD dwDisplayIndex)
{
   DEVMODE dm;
   ZeroMemory(&dm, sizeof(dm));
   dm.dmSize = sizeof(dm);

   DISPLAY_DEVICE dd;
   ZeroMemory(&dd, sizeof(dd));
   dd.cb = sizeof(dd);

   // Try to use the specified display index, but if 
   // that index fails falls back on a NULL (default)
   // device name.
   LPTSTR ptszDeviceName = NULL;
   if (EnumDisplayDevices(NULL, dwDisplayIndex, &dd, 0))
   {
       ptszDeviceName = dd.DeviceName;
   }

   if (0 != EnumDisplaySettings(ptszDeviceName, ENUM_CURRENT_SETTINGS, &dm))
   {
      // swap height and width
      DWORD dwTemp = dm.dmPelsHeight;
      dm.dmPelsHeight= dm.dmPelsWidth;
      dm.dmPelsWidth = dwTemp;

      // determine new orientaion
      switch (dm.dmDisplayOrientation)
      {
      case DMDO_DEFAULT:
         dm.dmDisplayOrientation = DMDO_270;
         break;
      case DMDO_270:
         dm.dmDisplayOrientation = DMDO_180;
         break;
      case DMDO_180:
         dm.dmDisplayOrientation = DMDO_90;
         break;
      case DMDO_90:
         dm.dmDisplayOrientation = DMDO_DEFAULT;
         break;
      default:
         // unknown orientation value
         break;
      }

      LONG result = ChangeDisplaySettingsEx(ptszDeviceName, &dm, NULL, 0, NULL);
      if (DISP_CHANGE_SUCCESSFUL != result)
      {
          DbgPrintf(TEXT("ChangeDisplaySettingsEx failed with 0x%08x\n"), result);
      }
   }
}
