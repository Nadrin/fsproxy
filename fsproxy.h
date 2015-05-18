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

#ifndef __FSPROXY_H
#define __FSPROXY_H

#include "setup.h"

#define WM_STATUSCHANGE	WM_USER+4
#define WM_SHELLNOTIFY	WM_USER+5

#define ID_TRAY			0
#define ID_CLOSE		2000
#define ID_RESTART		2001
#define ID_STOP			2002
#define ID_SETUP		2003
#define ID_ABOUT		2004
#define ID_REMOUNT		2005
#define ID_SYNC			2006

typedef void (*ActionCallback)(HWND, HWND, UINT);
typedef struct _CallbackEntry
{
	int key;
	ActionCallback action;
} CallbackEntry;

typedef struct 
{
	wchar_t name[256];
	wchar_t fname[256];
	wchar_t desc[256];
} NET_IFACE;

typedef struct
{
	wchar_t path[256];
	wchar_t desc[512];
} DRIVE;

typedef enum
{
	ServerStopped,
	ServerStarting,
	ServerRunning,
	ServerUnknown,
} SERVER_STATUS;

wchar_t* GetString(const int id);
BOOL CallAction(const int id, HWND hWnd, HWND hCaller, UINT uMsg);

int ConfigGetString(const wchar_t *key, wchar_t *out);
int ConfigPutString(const wchar_t *key, const wchar_t *string);
int ConfigStartupAdd(const wchar_t* name, const wchar_t *path);
int ConfigStartupRemove(const wchar_t* name);
int ConfigStartupQuery(const wchar_t* name);
void ServerStart(HWND hWnd, HWND hCaller, UINT uMsg);
void ServerStop(HWND hWnd, HWND hCaller, UINT uMsg);

int EnumerateDrives(DRIVE* drives, const size_t max);
int EnumerateInterfaces(NET_IFACE* iface, const size_t max);
NET_IFACE* ResolveInterface(const wchar_t* name, int* index, NET_IFACE* iface, const int count);

void UpdateDrives(HWND hWnd);
void UpdateInterfaces(HWND hWnd);
wchar_t* TranslateServerStatus();

int SendCtl(const char *host, const char *ctl, const size_t timeout);
int SendCtlLoop(const char *host, const char *ctl, const size_t timeout, const int repeat);
void SetServerStatus(const SERVER_STATUS newStatus);
SERVER_STATUS GetServerStatus(void);

void ShowSetupDialog(HWND hWnd, HWND hCaller, UINT uMsg);

#endif