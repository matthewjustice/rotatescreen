/* -------------------------------------------------------------

rsconfigdlg.h
	Rotate Screen Configuration Dialog code


by: Matthew Justice

---------------------------------------------------------------*/


#ifndef _RSCONFIGDLG_H_
#define _RSCONFIGDLG_H_

#define CCH_DISPLAY_ADAPTER_NAME  32
#define CCH_DISPLAY_ADAPTER_DESC 128
#define CCH_DISPLAY_LIST_TEXT    180

void DisplayConfigDialog();
void MainDlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
INT_PTR CALLBACK ConfigDlgProc(HWND hdlg,	UINT msg, WPARAM wParam, LPARAM lParam);
BOOL ReadDlgSettings(HWND hdlg, PUINT pfsModifiers, PUINT pvk, PDWORD pdwDisplay);
BOOL MainDlg_OnOK(HWND hdlg);
BOOL MainDlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void WriteRegistrySettings(UINT fsModifiers, UINT vk, DWORD dwDisplay);
void ReadRegistrySettings(PUINT pfsModifiers, PUINT pvk, PDWORD pdwDisplay);
DWORD PopulateDisplayDevices(HWND hdlg);
DWORD GetSelectedDisplayDevice(HWND hdlg);

#endif // _RSCONFIGDLG_H_