/* FSproxy Agent v1.0 RC2
 * Copyright (C) 2009  Micha³ Siejak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "resource.h"
#include "fsproxy.h"

extern CallbackEntry callbacks[];
extern BOOL gSetupShown;
extern BOOL gMountRw;

BOOL		gCmdLine_Show;

PROCESS_INFORMATION	gServer;
CRITICAL_SECTION gThreadSection;
CRITICAL_SECTION gStatusSection;
volatile SERVER_STATUS gStatus;

void SetServerStatus(const SERVER_STATUS newStatus)
{
	EnterCriticalSection(&gStatusSection);
	gStatus = newStatus;
	LeaveCriticalSection(&gStatusSection);
}

SERVER_STATUS GetServerStatus(void)
{
	SERVER_STATUS result;
	EnterCriticalSection(&gStatusSection);
	result = gStatus;
	LeaveCriticalSection(&gStatusSection);
	return result;
}

void GetBasename(const wchar_t* string, wchar_t* out, const size_t max)
{
	size_t i;
	for(i=wcslen(string); i>0; i--)
	{
		if(string[i] == '\\')
		{
			wcscpy_s(out, max, &string[i+1]);
			return;
		}
	}
	wcscpy_s(out, max, string);
}

size_t GetInstanceCount(const wchar_t* name)
{
	DWORD dwProc[1024];
	DWORD dwSize, dwCount, i;
	wchar_t buffer[256];
	wchar_t basebuf[256];
	wchar_t basename[256];
	size_t instCount = 0;

	GetBasename(name, basename, 256);
	if(!EnumProcesses(dwProc, sizeof(dwProc), &dwSize))
		return 0;
	dwCount = dwSize / sizeof(DWORD);
	for(i=0; i<dwCount; i++)
	{
		HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE, dwProc[i]);
		if(!hProc) continue;
		if(!GetProcessImageFileName(hProc, buffer, 256))
			continue;
		GetBasename(buffer, basebuf, 256);
		if(wcscmp(basebuf, basename) == 0)
			instCount++;
	}
	return instCount;
}

BOOL CallAction(const int id, HWND hWnd, HWND hCaller, UINT uMsg)
{
	const CallbackEntry* ce = &callbacks[0];
	int index = 0;

	while(ce->action != NULL)
	{
		if(ce->key == id)
		{
			ce->action(hWnd, hCaller, uMsg);
			return TRUE;
		}
		ce = &callbacks[++index];
	}
	return FALSE;
}

wchar_t* GetString(const int id)
{
	static int     buf_id = 0;
	static wchar_t buffer[4][256];

	buf_id = (buf_id+1) % 4;
	if(!LoadString(GetModuleHandle(NULL), id, buffer[buf_id], 256))
		return NULL;
	return buffer[buf_id];
}

BOOL CALLBACK SetupProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HMENU hTrayPopup;
	static NOTIFYICONDATA notifyData;
	static HINSTANCE hInstance;

	wchar_t cfgAutostart[20];
	switch(uMsg)
	{
	case WM_INITDIALOG:
		hInstance  = GetModuleHandle(NULL);
		hTrayPopup = CreatePopupMenu();
		AppendMenu(hTrayPopup, MF_STRING, ID_SETUP, GetString(IDS_MENU_SETUP));
		AppendMenu(hTrayPopup, MF_SEPARATOR, 0, NULL);
		AppendMenu(hTrayPopup, MF_STRING, ID_RESTART, GetString(IDS_MENU_RESTART));
		AppendMenu(hTrayPopup, MF_STRING, ID_STOP, GetString(IDS_MENU_STOP));
		AppendMenu(hTrayPopup, MF_STRING | MF_DISABLED | MF_GRAYED, ID_REMOUNT, GetString(IDS_ENABLE_RW));
		AppendMenu(hTrayPopup, MF_STRING | MF_DISABLED | MF_GRAYED, ID_SYNC, GetString(IDS_SYNC));
		AppendMenu(hTrayPopup, MF_SEPARATOR, 0, NULL);
		AppendMenu(hTrayPopup, MF_STRING, ID_ABOUT, GetString(IDS_ABOUT));
		AppendMenu(hTrayPopup, MF_STRING, ID_CLOSE, GetString(IDS_MENU_CLOSE));

		memset(&notifyData, 0, sizeof(NOTIFYICONDATA));
		notifyData.cbSize	= sizeof(NOTIFYICONDATA);
		notifyData.hWnd		= hWnd;
		notifyData.uID		= ID_TRAY;
		notifyData.uFlags	= NIF_ICON | NIF_MESSAGE | NIF_TIP;
		notifyData.hIcon	= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NOTIFYICON));
		
		notifyData.uCallbackMessage = WM_SHELLNOTIFY;
		wcscpy_s(notifyData.szTip, 128, TranslateServerStatus());
		Shell_NotifyIcon(NIM_ADD, &notifyData);
		UpdateDrives(NULL);
		UpdateInterfaces(NULL);

		ConfigGetString(L"autostart", cfgAutostart);
		if(wcscmp(cfgAutostart, L"yes") == 0)
			ServerStart(hWnd, hWnd, 0);
		if(gCmdLine_Show == TRUE)
			ShowSetupDialog(hWnd, hWnd, 0);
		break;
	case WM_STATUSCHANGE:
		if(gSetupShown)
			SetDlgItemText(hWnd, IDC_STATUS, TranslateServerStatus());
		wcscpy_s(notifyData.szTip, 128, TranslateServerStatus());
		wcscpy_s(notifyData.szInfo, 128, TranslateServerStatus());

		if(!(notifyData.uFlags & NIF_INFO))
		{
			wcscpy_s(notifyData.szInfoTitle, 128, SHINFO_TITLE);
			notifyData.uFlags |= NIF_INFO;
			notifyData.uTimeout = SHINFO_TIMEOUT;
		}
		Shell_NotifyIcon(NIM_MODIFY, &notifyData);
		break;
	case WM_QUERYENDSESSION:
		ServerStop(hWnd, hWnd, WM_QUERYENDSESSION);
		return 1;
	case WM_ENDSESSION:
		break;
	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &notifyData);
		DestroyMenu(hTrayPopup);
		PostQuitMessage(0);
		break;
	case WM_SHELLNOTIFY:
		if(wParam != ID_TRAY) break;
		if(gSetupShown == TRUE) break;
		if(lParam == WM_RBUTTONDOWN)
		{
			POINT mouse;
			GetCursorPos(&mouse);
			if(gServer.hProcess)
			{
				ModifyMenu(hTrayPopup, ID_RESTART, MF_BYCOMMAND | MF_STRING,
					ID_RESTART, GetString(IDS_MENU_RESTART));
				ModifyMenu(hTrayPopup, ID_STOP, MF_BYCOMMAND | MF_STRING,
					ID_STOP, GetString(IDS_MENU_STOP));
				if(GetServerStatus() == ServerRunning)
				{
					ModifyMenu(hTrayPopup, ID_SYNC, MF_BYCOMMAND | MF_STRING,
						ID_SYNC, GetString(IDS_SYNC));
					if(gMountRw)
						ModifyMenu(hTrayPopup, ID_REMOUNT, MF_BYCOMMAND | MF_STRING,
						ID_REMOUNT, GetString(IDS_DISABLE_RW));
					else
						ModifyMenu(hTrayPopup, ID_REMOUNT, MF_BYCOMMAND | MF_STRING,
						ID_REMOUNT, GetString(IDS_ENABLE_RW));
				}
				else
				{
					ModifyMenu(hTrayPopup, ID_SYNC, MF_BYCOMMAND | MF_STRING | MF_GRAYED | MF_DISABLED,
						ID_SYNC, GetString(IDS_SYNC));
					ModifyMenu(hTrayPopup, ID_REMOUNT, MF_BYCOMMAND | MF_STRING | MF_GRAYED | MF_DISABLED,
						ID_REMOUNT, GetString(IDS_ENABLE_RW));
				}
			}
			else
			{
				ModifyMenu(hTrayPopup, ID_RESTART, MF_BYCOMMAND | MF_STRING,
					ID_RESTART, GetString(IDS_MENU_START));
				ModifyMenu(hTrayPopup, ID_STOP, MF_BYCOMMAND | MF_STRING | MF_GRAYED | MF_DISABLED,
					ID_STOP, GetString(IDS_MENU_STOP));
				ModifyMenu(hTrayPopup, ID_SYNC, MF_BYCOMMAND | MF_STRING | MF_GRAYED | MF_DISABLED,
					ID_SYNC, GetString(IDS_SYNC));
				ModifyMenu(hTrayPopup, ID_REMOUNT, MF_BYCOMMAND | MF_STRING | MF_GRAYED | MF_DISABLED,
					ID_REMOUNT, GetString(IDS_ENABLE_RW));
			}
			TrackPopupMenu(hTrayPopup, TPM_RIGHTALIGN, mouse.x, mouse.y, 0, hWnd, NULL);
		}
		else if(lParam == WM_LBUTTONDOWN && gSetupShown == FALSE)
			SendMessage(hWnd, WM_COMMAND, ID_SETUP, 0);
		break;
	case WM_COMMAND:
		CallAction(LOWORD(wParam), hWnd, (HWND)lParam, HIWORD(wParam));
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	wchar_t	self[256];
	HWND	hSetup;
	MSG		msg;

	InitCommonControls();
	InitializeCriticalSection(&gStatusSection);
	InitializeCriticalSection(&gThreadSection);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	
	GetModuleFileName(NULL, self, 256);
	if(GetInstanceCount(self) > 1)
	{
		MessageBox(NULL, GetString(IDS_ERROR_RUNNING), GetString(IDS_CAPTION),
			MB_ICONSTOP | MB_OK);
		return 1;
	}

	if(strstr(lpCmdLine, "-show")) gCmdLine_Show = TRUE;
	else gCmdLine_Show = FALSE;

	gStatus = ServerStopped;
	memset(&gServer, 0, sizeof(PROCESS_INFORMATION));
	hSetup = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SETUP), GetDesktopWindow(), SetupProc);
	if(hSetup == NULL)
	{
		MessageBox(NULL, GetString(IDS_ERROR_DIALOG), GetString(IDS_CAPTION), MB_ICONSTOP | MB_OK);
		return 1;
	}

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(msg.message == WM_QUIT)
			break;
		if(!IsDialogMessage(hSetup, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}