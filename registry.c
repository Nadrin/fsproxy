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
#include "fsproxy.h"

int RegGetString(HKEY hBase, const wchar_t *path, const wchar_t *key, wchar_t *out)
{
	HKEY hKey;
	DWORD dwSize = 255;

	if(RegOpenKeyEx(hBase, path, 0L, KEY_READ, &hKey) != ERROR_SUCCESS)
		return 0;

	if(RegQueryValueEx(hKey, key, NULL, NULL, (LPBYTE)out, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return 0;
	}
	RegCloseKey(hKey);
	return dwSize;
}

int RegPutString(HKEY hBase, const wchar_t *path, const wchar_t *key, const wchar_t *string)
{
	HKEY hKey;
	if(RegOpenKeyEx(hBase, path, 0L, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
		return 0;
	RegSetValueEx(hKey, key, 0L, REG_SZ, (LPBYTE)string, 2*(wcslen(string)+1));
	return 1;
}

int ConfigStartupQuery(const wchar_t* name)
{
	HKEY hKey;
	DWORD dwType;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0L, KEY_READ, &hKey) != ERROR_SUCCESS)
		return 0;

	if(RegQueryValueEx(hKey, name, NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return 0;
	}
	return dwType == REG_SZ;
}

int ConfigStartupAdd(const wchar_t* name, const wchar_t *path)
{
	HKEY hKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0L, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
		return 0;
	RegSetValueEx(hKey, name, 0L, REG_SZ, (LPBYTE)path, 2*(wcslen(path)+1));
	return 1;
}

int ConfigStartupRemove(const wchar_t* name)
{
	HKEY hKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0L, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
		return 0;
	RegDeleteValue(hKey, name);
	return 1;
}

int ConfigGetString(const wchar_t *key, wchar_t *out)
{
	return RegGetString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\FSproxy", key, out);
}

int ConfigPutString(const wchar_t *key, const wchar_t *string)
{
	return RegPutString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\FSproxy", key, string);
}
