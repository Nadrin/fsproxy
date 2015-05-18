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

int EnumerateDrives(DRIVE* drives, const size_t max)
{
	wchar_t name[100];
	wchar_t info[512];
	size_t index = 0;
	size_t dcount = 0;

	HANDLE hFile;
	STORAGE_PROPERTY_QUERY query;
	PSTORAGE_DEVICE_DESCRIPTOR pdesc;
	DISK_GEOMETRY_EX dg;
	UCHAR buffer[512];
	UCHAR product_id[256];
	PUCHAR p;
	DWORD dwBufferSize;

	while(index < max )
	{
		wsprintf(name, L"\\\\.\\PhysicalDrive%d", index++);
		hFile = CreateFile(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
			continue;

		if(!DeviceIoControl(hFile, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
			NULL, 0, &dg, sizeof(DISK_GEOMETRY_EX), &dwBufferSize, NULL))
		{
			CloseHandle(hFile);
			continue;
		}

		if(dg.Geometry.MediaType != FixedMedia)
		{
			CloseHandle(hFile);
			continue;
		}
		
		memset(&query, 0, sizeof(STORAGE_PROPERTY_QUERY));
		query.PropertyId	= StorageDeviceProperty;
		query.QueryType		= PropertyStandardQuery;

		if(!DeviceIoControl(hFile, IOCTL_STORAGE_QUERY_PROPERTY,
			&query, sizeof(STORAGE_PROPERTY_QUERY),
			&buffer, 512, &dwBufferSize, NULL))
		{
			CloseHandle(hFile);
			continue;
		}

		p = (PUCHAR)buffer;
		pdesc = (PSTORAGE_DEVICE_DESCRIPTOR)buffer;
		if(pdesc->ProductIdOffset && p[pdesc->ProductIdOffset])
		{
			ULONG i=0, j=0;
			size_t converted;
			for(i=pdesc->ProductIdOffset; p[i] != 0 && i<dwBufferSize; i++)
				product_id[j++] = p[i];
			product_id[j] = 0;
			mbstowcs_s(&converted, info, strlen(product_id)+1, product_id, _TRUNCATE);
		}
		else wcscpy_s(info, 512, L"");

		wsprintf(info, L"%s (%d GB)", info, dg.DiskSize.QuadPart / (1000 * 1000 * 1000));
		wcscpy_s(drives[dcount].path, 256, name);
		wcscpy_s(drives[dcount].desc, 512, info);

		dcount++;
		CloseHandle(hFile);
	}
	return dcount;
}

int EnumerateInterfaces(NET_IFACE* iface, const size_t max)
{
	size_t i=0;
	ULONG ulSize = 0;
	PIP_ADAPTER_ADDRESSES pinfo;
	PIP_ADAPTER_ADDRESSES pbase;

	GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &ulSize);
	pbase = (PIP_ADAPTER_ADDRESSES)HeapAlloc(GetProcessHeap(), 0, ulSize);
	GetAdaptersAddresses(AF_INET, 0, NULL, pbase, &ulSize);

	pinfo = pbase;
	while(pinfo && i < max)
	{
		size_t converted;
		if(wcsstr(pinfo->Description, L"TAP") == NULL)
		{
			pinfo = pinfo->Next;
			continue;
		}
		mbstowcs_s(&converted, iface[i].name, strlen(pinfo->AdapterName)+1, pinfo->AdapterName, _TRUNCATE);
		wcscpy_s(iface[i].fname, 256, pinfo->FriendlyName);
		wcscpy_s(iface[i].desc, 256, pinfo->Description);
		pinfo = pinfo->Next;
		i++;
	}
	HeapFree(GetProcessHeap(), 0, pbase);
	return i;
}

NET_IFACE* ResolveInterface(const wchar_t* name, int* index, NET_IFACE* iface, const int count)
{
	int i;
	for(i=0; i<count; i++)
	{
		if(wcscmp(name, iface[i].name) == 0)
		{
			if(index) *index = i;
			return &iface[i];
		}
	}
	return NULL;
}