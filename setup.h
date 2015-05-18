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

#ifndef __SETUP_H
#define __SETUP_H

#define GUI_INFORMATION \
	L"FSproxy v1.0 RC2\n(C)Copyright 2009 Micha³ Siejak\nLicensed under GPL version 3.\n\nhttp://fsproxy.masterm.org"

#define DRIVE_MAX		32
#define IFACE_MAX		32

//#define CONFIG_NO_ROCTL

#define TAP_ADDRESS		L"192.168.255.2"
#define TAP_NETMASK		L"255.255.255.0"
#define QEMU_MEM		16
#define QEMU_NIC		L"e1000"
#define QEMU_SERVERIMG	L"fsproxy.img"
#define QEMU_ARGS		L" -vga none -serial null -monitor null -no-acpi -nographic"
#define QEMU_TIMEOUT	1000
#define QEMU_IP			L"192.168.255.1"
#define QEMU_IP_ANSI	"192.168.255.1"
#define NET_TIMEOUT		4
#define BOOT_WAIT		30
#define SHINFO_TITLE	L"FSproxy Agent"
#define SHINFO_TIMEOUT	1000


#endif