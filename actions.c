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

BOOL gSetupShown = FALSE;
BOOL gMountRw	 = FALSE;

int	 gDriveCount = 0;
int  gInterfaceCount = 0;

DRIVE		gDrives[DRIVE_MAX];
NET_IFACE	gInterfaces[IFACE_MAX];

extern CRITICAL_SECTION gThreadSection;
extern PROCESS_INFORMATION	gServer;

void UpdateDrives(HWND hWnd)
{
	int i, hdbi, hdci, hddi;
	wchar_t hdb[256], hdc[256], hdd[256];

	hdbi = hdci = hddi = -1;
	if(hWnd)
	{
		SendMessage(GetDlgItem(hWnd, IDC_HDB), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hWnd, IDC_HDB), CB_ADDSTRING, 0, (LPARAM)GetString(IDS_LISTNONE));
		SendMessage(GetDlgItem(hWnd, IDC_HDB), CB_SETCURSEL, 0, 0);
		SendMessage(GetDlgItem(hWnd, IDC_HDC), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hWnd, IDC_HDC), CB_ADDSTRING, 0, (LPARAM)GetString(IDS_LISTNONE));
		SendMessage(GetDlgItem(hWnd, IDC_HDC), CB_SETCURSEL, 0, 0);
		SendMessage(GetDlgItem(hWnd, IDC_HDD), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hWnd, IDC_HDD), CB_ADDSTRING, 0, (LPARAM)GetString(IDS_LISTNONE));
		SendMessage(GetDlgItem(hWnd, IDC_HDD), CB_SETCURSEL, 0, 0);
	}

	gDriveCount = EnumerateDrives(gDrives, DRIVE_MAX);
	ConfigGetString(L"hdb", hdb);
	ConfigGetString(L"hdc", hdc);
	ConfigGetString(L"hdd", hdd);

	for(i=0; i<gDriveCount; i++)
	{
		if(wcscmp(hdb, gDrives[i].path) == 0) hdbi = i;
		else if(wcscmp(hdc, gDrives[i].path) == 0) hdci = i;
		else if(wcscmp(hdd, gDrives[i].path) == 0) hddi = i;

		if(hWnd)
		{
			SendMessage(GetDlgItem(hWnd, IDC_HDB), CB_ADDSTRING, 0, (LPARAM)gDrives[i].desc);
			SendMessage(GetDlgItem(hWnd, IDC_HDC), CB_ADDSTRING, 0, (LPARAM)gDrives[i].desc);
			SendMessage(GetDlgItem(hWnd, IDC_HDD), CB_ADDSTRING, 0, (LPARAM)gDrives[i].desc);
		}
		
	}
	if(hdbi == -1) 
		ConfigPutString(L"hdb", L"");
	else if(hWnd)
		SendMessage(GetDlgItem(hWnd, IDC_HDB), CB_SETCURSEL, (WPARAM)hdbi+1, 0);

	if(hdci == -1)
		ConfigPutString(L"hdc", L"");
	else if(hWnd)
		SendMessage(GetDlgItem(hWnd, IDC_HDC), CB_SETCURSEL, (WPARAM)hdci+1, 0);

	if(hddi == -1)
		ConfigPutString(L"hdd", L"");
	else if(hWnd)
		SendMessage(GetDlgItem(hWnd, IDC_HDD), CB_SETCURSEL, (WPARAM)hddi+1, 0);
}

void UpdateInterfaces(HWND hWnd)
{
	wchar_t iface_uid[256];
	int iface_index;
	NET_IFACE *iface;

	ConfigGetString(L"net_uid", iface_uid);
	gInterfaceCount = EnumerateInterfaces(gInterfaces, IFACE_MAX);
	iface = ResolveInterface(iface_uid, &iface_index, gInterfaces, gInterfaceCount);

	if(hWnd)
	{
		int i, list_index=0;
		SendMessage(GetDlgItem(hWnd, IDC_IFNAME), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hWnd, IDC_IFNAME), CB_ADDSTRING, 0, (LPARAM)GetString(IDS_LISTNONE));
		SetDlgItemText(hWnd, IDC_TAP, GetString(IDS_LISTNONE));
		for(i=0; i<gInterfaceCount; i++)
			SendMessage(GetDlgItem(hWnd, IDC_IFNAME), CB_ADDSTRING, 0, (LPARAM)gInterfaces[i].fname);
		if(iface)
			list_index = iface_index+1;
		SendMessage(GetDlgItem(hWnd, IDC_IFNAME), CB_SETCURSEL, (WPARAM)list_index, 0);
	}

	if(!iface)
	{
		ConfigPutString(L"net_uid", L"");
		ConfigPutString(L"net_name", L"");
	}
	else
	{
		ConfigPutString(L"net_name", iface->fname);
		if(hWnd)
			SetDlgItemText(hWnd, IDC_TAP, iface->fname);
	}
}

wchar_t* TranslateServerStatus()
{
	SERVER_STATUS status = GetServerStatus();
	switch(status)
	{
	case ServerStopped:
		return GetString(IDS_STATUS_STOPPED);
	case ServerStarting:
		return GetString(IDS_STATUS_STARTING);
	case ServerRunning:
		return GetString(IDS_STATUS_RUNNING);
	default:
		return GetString(IDS_STATUS_UNKNOWN);
	}
}

void ShowSetupDialog(HWND hWnd, HWND hCaller, UINT uMsg)
{
	wchar_t serverIP[256];
	wchar_t checkAutoStart[256];
	wchar_t checkWrite[256];

	RECT desktopRect, dialogRect;
	POINT showPos;

	if(gSetupShown)
	{
		ShowWindow(hWnd, SW_HIDE);
		gSetupShown = FALSE;
		return;
	}

	UpdateDrives(hWnd);
	UpdateInterfaces(hWnd);
	SetDlgItemText(hWnd, IDC_STATUS, TranslateServerStatus());

	wcscpy_s(serverIP, 256, GetString(IDS_SERVER_ETH0));
	wcscat_s(serverIP, 256, QEMU_IP);
	SetDlgItemText(hWnd, IDC_ETH0, serverIP);

	ConfigGetString(L"autostart", checkAutoStart);
	ConfigGetString(L"rw", checkWrite);

	if(wcscmp(checkAutoStart, L"yes") == 0)
		SendMessage(GetDlgItem(hWnd, IDC_AUTOSERVER), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	else
		SendMessage(GetDlgItem(hWnd, IDC_AUTOSERVER), BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

	if(wcscmp(checkWrite, L"yes") == 0)
		SendMessage(GetDlgItem(hWnd, IDC_MOUNTRW), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	else
		SendMessage(GetDlgItem(hWnd, IDC_MOUNTRW), BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

	if(ConfigStartupQuery(GetString(IDS_NAME)))
		SendMessage(GetDlgItem(hWnd, IDC_AUTOSTART), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	else
		SendMessage(GetDlgItem(hWnd, IDC_AUTOSTART), BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

	GetClientRect(GetDesktopWindow(), &desktopRect);
	GetWindowRect(hWnd, &dialogRect);
	
	showPos.x = (desktopRect.right - (dialogRect.right - dialogRect.left)) / 2;
	showPos.y = (desktopRect.bottom - (dialogRect.bottom - dialogRect.top)) / 2;
	SetWindowPos(hWnd, HWND_TOPMOST, showPos.x, showPos.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	gSetupShown = TRUE;
}

void DriveSelChanged(HWND hWnd, HWND hCaller, UINT uMsg)
{
	int hdb_sel = SendMessage(GetDlgItem(hWnd, IDC_HDB), CB_GETCURSEL, 0, 0);
	int hdc_sel = SendMessage(GetDlgItem(hWnd, IDC_HDC), CB_GETCURSEL, 0, 0);
	int hdd_sel = SendMessage(GetDlgItem(hWnd, IDC_HDD), CB_GETCURSEL, 0, 0);

	if(hCaller == GetDlgItem(hWnd, IDC_HDB))
	{
		if((hdb_sel == hdc_sel || hdb_sel == hdd_sel) && hdb_sel > 0)
		{
			SendMessage(hCaller, CB_SETCURSEL, 0, 0);
			MessageBox(hWnd, GetString(IDS_SELDISKERROR), GetString(IDS_CAPTION), MB_ICONEXCLAMATION | MB_OK);
			return;
		}
	}
	else if(hCaller == GetDlgItem(hWnd, IDC_HDC))
	{
		if((hdc_sel == hdb_sel || hdc_sel == hdd_sel) && hdc_sel > 0)
		{
			SendMessage(hCaller, CB_SETCURSEL, 0, 0);
			MessageBox(hWnd, GetString(IDS_SELDISKERROR), GetString(IDS_CAPTION), MB_ICONEXCLAMATION | MB_OK);
			return;
		}
	}
	else if(hCaller == GetDlgItem(hWnd, IDC_HDD))
	{
		if((hdd_sel == hdb_sel || hdd_sel == hdc_sel) && hdd_sel > 0)
		{
			SendMessage(hCaller, CB_SETCURSEL, 0, 0);
			MessageBox(hWnd, GetString(IDS_SELDISKERROR), GetString(IDS_CAPTION), MB_ICONEXCLAMATION | MB_OK);
			return;
		}
	}
}

void ConnectClicked(HWND hWnd, HWND hCaller, UINT uMsg)
{
	int net_sel, index;
	net_sel = SendMessage(GetDlgItem(hWnd, IDC_IFNAME), CB_GETCURSEL, 0, 0);
	if(net_sel == 0)
	{
		ConfigPutString(L"net_uid", L"");
		ConfigPutString(L"net_name", L"");
		SetDlgItemText(hWnd, IDC_TAP, GetString(IDS_LISTNONE));
	}
	else
	{
		wchar_t netsh_args[256];
		index = net_sel - 1;

		wsprintf(netsh_args,
			L"interface ip set address name=\"%s\" static %s %s",
			gInterfaces[index].fname, TAP_ADDRESS, TAP_NETMASK);

		ConfigPutString(L"net_uid", gInterfaces[index].name);
		ConfigPutString(L"net_name", gInterfaces[index].fname);
		ShellExecute(NULL, L"open", L"netsh", netsh_args, NULL, SW_HIDE);
		SetDlgItemText(hWnd, IDC_TAP, gInterfaces[index].fname);
	}
}

void ServerRestart(HWND, HWND, UINT);
void SetupDialogOK(HWND hWnd, HWND hCaller, UINT uMsg)
{
	int hdbi, hdci, hddi;
	int autostartCheck, onlogonCheck;
	int writeCheck;

	hdbi = SendMessage(GetDlgItem(hWnd, IDC_HDB), CB_GETCURSEL, 0, 0);
	hdci = SendMessage(GetDlgItem(hWnd, IDC_HDC), CB_GETCURSEL, 0, 0);
	hddi = SendMessage(GetDlgItem(hWnd, IDC_HDD), CB_GETCURSEL, 0, 0);
	autostartCheck = SendMessage(GetDlgItem(hWnd, IDC_AUTOSERVER), BM_GETCHECK, 0, 0);
	onlogonCheck = SendMessage(GetDlgItem(hWnd, IDC_AUTOSTART), BM_GETCHECK, 0, 0);
	writeCheck = SendMessage(GetDlgItem(hWnd, IDC_MOUNTRW), BM_GETCHECK, 0, 0);

	if(autostartCheck == BST_CHECKED)
		ConfigPutString(L"autostart", L"yes");
	else
		ConfigPutString(L"autostart", L"no");

	if(writeCheck == BST_CHECKED)
		ConfigPutString(L"rw", L"yes");
	else
		ConfigPutString(L"rw", L"no");

	if(onlogonCheck == BST_CHECKED)
	{
		wchar_t fullPath[MAX_PATH];
		ConfigGetString(L"path", fullPath);
		wcscat_s(fullPath, MAX_PATH, L"\\");
		wcscat_s(fullPath, MAX_PATH, GetString(IDS_SELF));
		ConfigStartupAdd(GetString(IDS_NAME), fullPath);
	}
	else ConfigStartupRemove(GetString(IDS_NAME));

	if(hdbi == 0) ConfigPutString(L"hdb", L"");
	else ConfigPutString(L"hdb", gDrives[hdbi-1].path);
	if(hdci == 0) ConfigPutString(L"hdc", L"");
	else ConfigPutString(L"hdc", gDrives[hdci-1].path);
	if(hddi == 0) ConfigPutString(L"hdd", L"");
	else ConfigPutString(L"hdd", gDrives[hddi-1].path);

	ShowWindow(hWnd, SW_HIDE);
	if(gServer.hProcess)
	{
		if(MessageBox(NULL, GetString(IDS_MSG_RESTART),
			GetString(IDS_CAPTION), MB_ICONQUESTION | MB_YESNO) == IDYES)
			ServerRestart(hWnd, hCaller, uMsg);
	}
	gSetupShown = FALSE;
}

void SetupDialogCancel(HWND hWnd, HWND hCaller, UINT uMsg)
{
	ShowWindow(hWnd, SW_HIDE);
	gSetupShown = FALSE;
}

void ServerStop(HWND, HWND, UINT);
void AgentClose(HWND hWnd, HWND hCaller, UINT uMsg)
{
	ServerStop(hWnd, hCaller, uMsg);
	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
}

DWORD WINAPI StatusThread(LPVOID lpParameter)
{
	int i, ctl_ret;
	wchar_t mount_rw[256];

	HWND hConfigDialog = (HWND)lpParameter;

	EnterCriticalSection(&gThreadSection);
	ConfigGetString(L"rw", mount_rw);
	SetServerStatus(ServerStarting);
	PostMessage(hConfigDialog, WM_STATUSCHANGE, 0, 0);
	
	for(i=0; i<BOOT_WAIT; i++)
	{
		ctl_ret = SendCtl(QEMU_IP_ANSI, "default", NET_TIMEOUT);
		if(GetServerStatus() != ServerStarting)
		{
			LeaveCriticalSection(&gThreadSection);
			ExitThread(1);
		}
		if(ctl_ret == 0)
			break;
	}

	if(ctl_ret == 0)
	{
		if(GetServerStatus() != ServerStarting)
		{
			LeaveCriticalSection(&gThreadSection);
			ExitThread(1);
		}
		if(wcscmp(mount_rw, L"yes") == 0)
		{
			if(SendCtl(QEMU_IP_ANSI, "rw", NET_TIMEOUT) != 0)
				MessageBox(NULL, GetString(IDS_CTL_FAILED), GetString(IDS_CAPTION), MB_ICONEXCLAMATION | MB_OK);
			gMountRw = TRUE;
		}
		if(GetServerStatus() == ServerStarting)
			SetServerStatus(ServerRunning);
	}
	else if(GetServerStatus() != ServerStopped)
		SetServerStatus(ServerUnknown);

	PostMessage(hConfigDialog, WM_STATUSCHANGE, 0, 0);
	LeaveCriticalSection(&gThreadSection);
	ExitThread(0);
	return 2L;
}

void ServerStart(HWND hWnd, HWND hCaller, UINT uMsg)
{
	wchar_t qemu[256];
	wchar_t path[256];

	wchar_t hdb[256], hdc[256], hdd[256];
	wchar_t net_name[256];
	wchar_t qemu_args[2048];

	STARTUPINFO si;
	HANDLE hStatusThread;
	memset(&si, 0, sizeof(STARTUPINFO));

	ConfigGetString(L"qemu", qemu);
	ConfigGetString(L"path", path);
	if(wcslen(qemu) == 0 || wcslen(path) == 0)
		return;

	UpdateDrives(NULL);
	UpdateInterfaces(NULL);

	ConfigGetString(L"hdb", hdb);
	ConfigGetString(L"hdc", hdc);
	ConfigGetString(L"hdd", hdd);
	ConfigGetString(L"net_name", net_name);

	if(wcslen(net_name) == 0)
	{
		MessageBox(NULL, GetString(IDS_ERROR_NONET), GetString(IDS_CAPTION), MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	if(wcslen(hdb) == 0 && wcslen(hdc) == 0 && wcslen(hdd) == 0)
	{
		MessageBox(NULL, GetString(IDS_ERROR_NODRIVE), GetString(IDS_CAPTION), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	wsprintf(qemu_args, L"\"%s\" -L . -m %d -net nic,model=%s -net tap,ifname=\"%s\" -hda %s",
		qemu, QEMU_MEM, QEMU_NIC, net_name, QEMU_SERVERIMG);
	if(wcslen(hdb) > 0)
		wsprintf(qemu_args, L"%s -hdb %s", qemu_args, hdb);
	if(wcslen(hdc) > 0)
		wsprintf(qemu_args, L"%s -hdc %s", qemu_args, hdc);
	if(wcslen(hdd) > 0)
		wsprintf(qemu_args, L"%s -hdd %s", qemu_args, hdd);
	wcscat_s(qemu_args, 2048, QEMU_ARGS);

	GetStartupInfo(&si);
	si.dwFlags	   = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	if(!CreateProcess(qemu, qemu_args, NULL, NULL, FALSE, 0, NULL, path, &si, &gServer))
	{
		MessageBox(NULL, GetString(IDS_ERROR_START), GetString(IDS_CAPTION), MB_ICONSTOP | MB_OK);
		return;
	}

	hStatusThread = CreateThread(NULL, 0, StatusThread, (LPVOID)hWnd, 0, NULL);
	CloseHandle(hStatusThread);
}

BOOL CALLBACK CloseServerCallback(HWND hWnd, LPARAM lParam)
{
	DWORD dwID;
	PROCESS_INFORMATION* pi = (PROCESS_INFORMATION*)lParam;
	GetWindowThreadProcessId(hWnd, &dwID);

	if(dwID == pi->dwProcessId)
	{
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		return FALSE;
	}
	return TRUE;
}

void ServerStop(HWND hWnd, HWND hCaller, UINT uMsg)
{
	SERVER_STATUS status;
	if(!gServer.hProcess)
		return;

	status = GetServerStatus();
	SetServerStatus(ServerStopped);
#ifndef CONFIG_NO_ROCTL
	if(status == ServerRunning || status == ServerUnknown)
	{
		if(uMsg == WM_QUERYENDSESSION)
			SendCtl(QEMU_IP_ANSI, "ro", 1);
		else
		{
		//if(SendCtlLoop(QEMU_IP_ANSI, "ro", NET_TIMEOUT, NET_RETRY) != 0)
			if(SendCtl(QEMU_IP_ANSI, "ro", NET_TIMEOUT) != 0)
				MessageBox(NULL, GetString(IDS_ERROR_CTLRO), GetString(IDS_CAPTION), MB_ICONSTOP | MB_OK);
		}
	}
#endif
	EnumWindows(CloseServerCallback, (LPARAM)&gServer);
	if(WaitForSingleObject(gServer.hProcess, QEMU_TIMEOUT) == WAIT_TIMEOUT)
		TerminateProcess(gServer.hProcess, 0);
	CloseHandle(gServer.hThread);
	CloseHandle(gServer.hProcess);
	memset(&gServer, 0, sizeof(PROCESS_INFORMATION));
	PostMessage(hWnd, WM_STATUSCHANGE, 0, 0);
}

void ServerRestart(HWND hWnd, HWND hCaller, UINT uMsg)
{
	if(!gServer.hProcess)
		ServerStart(hWnd, hCaller, uMsg);
	else
	{
		ServerStop(hWnd, hCaller, uMsg);
		ServerStart(hWnd, hCaller, uMsg);
	}
}

void ServerRemount(HWND hWnd, HWND hCaller, UINT uMsg)
{
	if(GetServerStatus() != ServerRunning)
		return;

	if(gMountRw)
	{
		if(SendCtl(QEMU_IP_ANSI, "ro", NET_TIMEOUT) != 0)
			MessageBox(NULL, GetString(IDS_CTL_FAILED), GetString(IDS_CAPTION), MB_ICONEXCLAMATION | MB_OK);
		else
			gMountRw = FALSE;
	}	
	else
	{
		if(SendCtl(QEMU_IP_ANSI, "rw", NET_TIMEOUT) != 0)
			MessageBox(NULL, GetString(IDS_CTL_FAILED), GetString(IDS_CAPTION), MB_ICONEXCLAMATION | MB_OK);
		else
			gMountRw = TRUE;
	}
}

void ServerSync(HWND hWnd, HWND hCaller, UINT uMsg)
{
	if(GetServerStatus() != ServerRunning)
		return;
	if(SendCtl(QEMU_IP_ANSI, "sync", NET_TIMEOUT) != 0)
		MessageBox(NULL, GetString(IDS_CTL_FAILED), GetString(IDS_CAPTION), MB_ICONEXCLAMATION | MB_OK);
}

void AboutBox(HWND hWNd, HWND hCaller, UINT uMsg)
{
	MessageBox(NULL, GUI_INFORMATION, GetString(IDS_ABOUT_CAPTION),
		MB_ICONINFORMATION | MB_OK);
}

const CallbackEntry callbacks[] = {
	{ ID_CLOSE, AgentClose },
	{ ID_SETUP, ShowSetupDialog },
	{ ID_RESTART, ServerRestart },
	{ ID_STOP, ServerStop },
	{ ID_ABOUT, AboutBox },
	{ ID_REMOUNT, ServerRemount },
	{ ID_SYNC, ServerSync },
	{ IDC_RESTART, ServerRestart },
	{ IDC_HDB, DriveSelChanged },
	{ IDC_HDC, DriveSelChanged },
	{ IDC_HDD, DriveSelChanged },
	{ IDC_CONNECT, ConnectClicked },
	{ IDOK, SetupDialogOK },
	{ IDCANCEL, SetupDialogCancel },
	{ 0, NULL },
};

