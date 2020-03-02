/* -------------------------------------------------------------

rsconfigdlg.cpp
	Rotate Screen Configuration Dialog code


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
extern HWND g_hwndHidden;
extern HINSTANCE g_hinst;
extern UINT g_fsModifiers;
extern UINT g_vk;
extern DWORD g_dwDisplay;

//
// ConfigDlgProc
// Dialog box window procedure
//
INT_PTR CALLBACK ConfigDlgProc(
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
   switch (msg)
   {
   HANDLE_MSG(hdlg, WM_INITDIALOG, MainDlg_OnInitDialog);
   HANDLE_MSG(hdlg, WM_COMMAND, MainDlg_OnCommand);
   }

   return FALSE;
}

//
// DisplayConfigDialog
// Open the main dialog if it isn't already open
//
void DisplayConfigDialog()
{
   HWND hwndConfigDialog;

   hwndConfigDialog = FindWindow(NULL, TEXT("Configure Rotate Screen"));

   if(hwndConfigDialog)
   {
      // Already open.  Bring to foreground
      SetForegroundWindow(hwndConfigDialog);
   }
   else
   {
      // Not open.  So open it.
      DialogBox(
         g_hinst, 
         MAKEINTRESOURCE(IDD_CONFIGURE),
         NULL, 
         ConfigDlgProc);
   }

   return;
}

//
// MainDlg_OnInitDialog
// handles WM_INITDIALOG
//
BOOL MainDlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
   WCHAR wszVk[2];
   DWORD cDisplayDevices;
   HWND hwndComboDisplayList;
   LRESULT result;
   DWORD dwDisplayToSelect;
   
   //
   // Set the UI elements based on the globals
   //
   if(g_fsModifiers & MOD_CONTROL)
   {
      CheckDlgButton(hwnd, IDC_CHECK_CTRL, BST_CHECKED);
   }
   
   if(g_fsModifiers & MOD_SHIFT)
   {
      CheckDlgButton(hwnd, IDC_CHECK_SHIFT, BST_CHECKED);
   }
   
   if(g_fsModifiers & MOD_WIN)
   {
      CheckDlgButton(hwnd, IDC_CHECK_WINDOWS, BST_CHECKED);
   }
   
   if(g_vk != 0)
   {
      wszVk[0] = (WCHAR) MapVirtualKey(g_vk, MAPVK_VK_TO_CHAR);
      wszVk[1] = 0;
      SetDlgItemText(hwnd, IDC_EDIT_VK, wszVk);
   }

   // Populate the specific display drop down
   cDisplayDevices = PopulateDisplayDevices(hwnd);
   if (g_dwDisplay < cDisplayDevices)
   {
       dwDisplayToSelect = g_dwDisplay;
   }
   else
   {
       dwDisplayToSelect = 0;
   }

   // Select the active item in the display drop down
   hwndComboDisplayList = GetDlgItem(hwnd, IDC_COMBO_DISPLAY_LIST);
   result = SendMessage(hwndComboDisplayList, CB_SETCURSEL, (WPARAM)dwDisplayToSelect, 0);

   // Select the correct radio button for primary / specific display
   if (g_dwDisplay == DISPLAY_UNDEFINED)
   {
       CheckDlgButton(hwnd, IDC_RADIO_PRIMARY_DISPLAY, BST_CHECKED);
       EnableWindow(hwndComboDisplayList, false);
   }
   else
   {
       CheckDlgButton(hwnd, IDC_RADIO_SPECIFIC_DISPLAY, BST_CHECKED);
       EnableWindow(hwndComboDisplayList, true);
   }

   return TRUE;
}

//
// MainDlg_OnCommand
// handle WM_COMMAND messages
//
void MainDlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    HWND hwndComboDisplayList;

    switch(id)
    {
    case IDC_RADIO_PRIMARY_DISPLAY:
        hwndComboDisplayList = GetDlgItem(hwnd, IDC_COMBO_DISPLAY_LIST);
        EnableWindow(hwndComboDisplayList, false);
        break;
    case IDC_RADIO_SPECIFIC_DISPLAY:
        hwndComboDisplayList = GetDlgItem(hwnd, IDC_COMBO_DISPLAY_LIST);
        EnableWindow(hwndComboDisplayList, true);
        break;
    case IDOK:
        if(MainDlg_OnOK(hwnd))
        {
            // Close the dialog if there were no errors processing the OK
            EndDialog(hwnd, IDOK);
        }
        break;
    case IDCANCEL:
        EndDialog(hwnd, IDCANCEL);
        break;
    }

    return;
}

//
// MainDlg_OnOK
// Handle the OK button click
//
BOOL MainDlg_OnOK(HWND hdlg)
{
   BOOL fRet = FALSE;
   
   if(ReadDlgSettings(hdlg, &g_fsModifiers, &g_vk, &g_dwDisplay))
   {
      // DbgPrintf(L"Rotate Screen: ReadDlgSettings success.  fsModifiers=0x%x vk=0x%x\n", 
      //   g_fsModifiers, g_vk);
         
       WriteRegistrySettings(g_fsModifiers, g_vk, g_dwDisplay);
      
      //
      // Unregister any existing hotkey and then re-register
      //
      UnregisterHotKey(g_hwndHidden, RS_HOTKEY_ID);
      
      if(RegisterHotKey(g_hwndHidden, RS_HOTKEY_ID, g_fsModifiers, g_vk))
      {
         fRet = TRUE;
      }
      else
      {
         DbgPrintf(L"Rotate Screen: RegisterHotKey failed with 0x%08x\n", GetLastError());
         MessageBox(hdlg, TEXT("Unable to register requested hotkey."), TEXT("Rotate Screen"), MB_ICONERROR); 
      }
   }
   else
   {
      DbgPrintf(L"Rotate Screen: ReadDlgSettings failed\n");
      MessageBox(hdlg, TEXT("The settings specified are invalid or incomplete."), TEXT("Rotate Screen"), MB_ICONERROR); 
   }
   return fRet;
} 

//
// ReadDlgSettings
// Read the settings that user typed into the dialog
//
BOOL ReadDlgSettings(HWND hdlg, PUINT pfsModifiers, PUINT pvk, PDWORD pdwDisplay)
{
   TCHAR tszVk[2];
   
   // 
   // Read the virtual key code
   //
   if(GetDlgItemText(hdlg, IDC_EDIT_VK, tszVk, 2) != 0)
   {
      *pvk = (UINT) VkKeyScan(tszVk[0]);
      *pvk = 0xFF & *pvk; // ignore the shift state
   }
   else
   {
      return FALSE;
   }
   
   //
   // Read the modifier check boxes
   //
   *pfsModifiers = 0;
   
   if(BST_CHECKED == IsDlgButtonChecked(hdlg, IDC_CHECK_CTRL))
   {
      *pfsModifiers |= MOD_CONTROL;
   }
   
   if(BST_CHECKED == IsDlgButtonChecked(hdlg, IDC_CHECK_SHIFT))
   {
      *pfsModifiers |= MOD_SHIFT;
   }
   
   if(BST_CHECKED == IsDlgButtonChecked(hdlg, IDC_CHECK_WINDOWS))
   {
      *pfsModifiers |= MOD_WIN;
   }

   //
   // Read the display device
   //
   *pdwDisplay = DISPLAY_UNDEFINED;

   if (BST_CHECKED == IsDlgButtonChecked(hdlg, IDC_RADIO_SPECIFIC_DISPLAY))
   {
       *pdwDisplay = GetSelectedDisplayDevice(hdlg);
   }
   
   return TRUE;
}

//
// ReadRegistrySettings
// Read the registry settings from HKCU\Software\mattjustice\rotatescreen
// If the key doesn't exist create it and exit
// 
// This function does return a value.
//
void ReadRegistrySettings(PUINT pfsModifiers, PUINT pvk, PDWORD pdwDisplay)
{
    HKEY hKey = NULL;
	BOOL fSuccess = TRUE;
	DWORD dwDisposition;
	LONG lResult;
	DWORD cbData;
   
   //
   // Open or create HKCU\Software\mattjustice\rotatescreen
   //
   lResult = RegCreateKeyEx(
      HKEY_CURRENT_USER,
      TEXT("SOFTWARE\\mattjustice\\rotatescreen"),
      0,	
      NULL,	
      REG_OPTION_NON_VOLATILE, 
      KEY_READ,
      NULL,	
      &hKey,
      &dwDisposition);

   if(lResult != ERROR_SUCCESS )
   {
      DbgPrintf(TEXT("Rotate Screen: RegCreateKeyEx failed on rotatescreen key.\n"));
      return;
   }
   else 
   {
      if (dwDisposition != REG_OPENED_EXISTING_KEY)
      {
         // If the key doesn't already exist, just exit.
         // The key should be created now, and the values
         // will be updated on exit.
         RegCloseKey(hKey);
         return;
      }
      
      // An existing key was openened and no error was returned

      // Query the value and store the result
      // if the query failed just keep going and try the other
      // values

      // Read modifiers
      cbData = sizeof(UINT);
      lResult = RegQueryValueEx(hKey, TEXT("modifiers"), NULL, NULL,
         (LPBYTE) pfsModifiers, &cbData);

      if(lResult != ERROR_SUCCESS)
      {
         *pfsModifiers = 0;
      }
         
      // Read vk
      cbData = sizeof(UINT);
      lResult = RegQueryValueEx(hKey, TEXT("vk"), NULL, NULL,
         (LPBYTE) pvk, &cbData);

      if(lResult != ERROR_SUCCESS)
      {
         *pvk = 0;
      }

      // Read display
      cbData = sizeof(DWORD);
      lResult = RegQueryValueEx(hKey, TEXT("display"), NULL, NULL,
          (LPBYTE)pdwDisplay, &cbData);

      if (lResult != ERROR_SUCCESS)
      {
          // Set the invalid value
          *pdwDisplay = DISPLAY_UNDEFINED;
      }
      
      RegCloseKey(hKey);
   }
   
   return;
}

//
// WriteRegistrySettings
// Saves the registry settings to HKCU\Software\mattjustice\rotatescreen
//
void WriteRegistrySettings(UINT fsModifiers, UINT vk, DWORD dwDisplay)
{
   HKEY   hKey;
   LONG   lResult; 
   DWORD  cbData;
   
   //
   // open the key
   //
   lResult = RegOpenKeyEx(HKEY_CURRENT_USER, 
      TEXT("SOFTWARE\\mattjustice\\rotatescreen"),
      0, KEY_WRITE, &hKey);
      
   if(lResult != ERROR_SUCCESS )
   {
      DbgPrintf(TEXT("Rotate Screen: RegOpenKeyEx failed on rotatescreen key.\n"));
      return;
   }
   
   //
   // Write the values we want to save to the registry
   //
   cbData = sizeof(UINT);
   RegSetValueEx(hKey, TEXT("modifiers"), 0, REG_DWORD,
      (const BYTE*) &fsModifiers, cbData);
      
   RegSetValueEx(hKey, TEXT("vk"), 0, REG_DWORD,
      (const BYTE*) &vk, cbData);

   cbData = sizeof(DWORD);
   RegSetValueEx(hKey, TEXT("display"), 0, REG_DWORD,
       (const BYTE*)&dwDisplay, cbData);
   
   RegCloseKey(hKey);
   
   return;
}

//
// PopulateDisplayDevices
// Returns the count of items added
//
DWORD PopulateDisplayDevices(HWND hdlg)
{
    DISPLAY_DEVICE dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);

    WCHAR wszDisplayApapterName[CCH_DISPLAY_ADAPTER_NAME] = { 0 };
    WCHAR wszDisplayApapterDescription[CCH_DISPLAY_ADAPTER_DESC] = { 0 };
    WCHAR wszDisplayListText[CCH_DISPLAY_LIST_TEXT] = { 0 };

    DWORD dwDisplayNumber = 0;

    HWND hwndComboDisplayList;
    hwndComboDisplayList = GetDlgItem(hdlg, IDC_COMBO_DISPLAY_LIST);

    // Get the display adapter info
    while (EnumDisplayDevices(NULL, dwDisplayNumber, &dd, 0))
    {
        DbgPrintf(TEXT("Adapter %d | %s | %s | 0x%08x\n"), dwDisplayNumber, dd.DeviceName, dd.DeviceString, dd.StateFlags);
        if (SUCCEEDED(StringCchCopy(wszDisplayApapterName, CCH_DISPLAY_ADAPTER_NAME, dd.DeviceName))
            && 
            SUCCEEDED(StringCchCopy(wszDisplayApapterDescription, CCH_DISPLAY_ADAPTER_DESC, dd.DeviceString)))
        {
            // Get the monitor info for this adapter
            EnumDisplayDevices(wszDisplayApapterName, 0, &dd, 0);
            DbgPrintf(TEXT("    Monitor %s | %s | 0x%08x\n"), dd.DeviceName, dd.DeviceString, dd.StateFlags);

            if (SUCCEEDED(StringCchPrintf(
                wszDisplayListText, 
                CCH_DISPLAY_LIST_TEXT, 
                TEXT("%d %s (%s)"),
                dwDisplayNumber + 1, 
                wszDisplayApapterDescription, 
                dd.DeviceString)))
            {
                SendMessage(hwndComboDisplayList, CB_ADDSTRING, 0, (LPARAM)wszDisplayListText);
            }
        }
        dwDisplayNumber++;
    }

    return dwDisplayNumber;
}

//
// GetSelectedDisplayDevice
// Get the index of the selected display device from the drop down list
//
DWORD GetSelectedDisplayDevice(HWND hdlg)
{
    HWND hwndComboDisplayList;
    LRESULT result;

    hwndComboDisplayList = GetDlgItem(hdlg, IDC_COMBO_DISPLAY_LIST);
    result = SendMessage(hwndComboDisplayList, CB_GETCURSEL, 0, 0);

    if (CB_ERR == result)
    {
        return DISPLAY_UNDEFINED;
    }
    else
    {
        return (DWORD)result;
    }
}


