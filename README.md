# FSproxy v1.0RC2
(c) 2009 Michał Siejak (http://www.siejak.pl)

### Development of FSproxy has been discontinued. I will not accept any pull requests.

#### For full documentation & download links refer to: http://nadrin.github.io/fsproxy

FSproxy is a small application that runs in the system tray and lets you access GNU/Linux partitions from within Microsoft Windows. The list of detected partitions is always availible at \\fsproxy and a single one can easily be mapped to a network drive using address like `\\fsproxy\<partition>`. By default partitions are mounted as read-only but experimental write support can be enabled. FSproxy currently supports: `ext2`, `ext3`, `ext4`, `reiserfs`, `jfs`, `xfs` and few other filesystems. For many of them there is no known Windows driver available (although note that FSproxy is not a driver itself).

This code is very ugly, quickly written and contains large amounts of C and Win32 API.
May not be suitable for liberal art majors and Java programmers. :-)

Solution is in Visual Studio 2008 format.

* FSproxy agent uses an icon from [Tango Icontheme](http://tango.freedesktop.org)
* FSproxy agent uses [libcurl 7.19.5](http://curl.haxx.se)
* Network TAP driver by [OpenVPN](http://openvpn.net).
