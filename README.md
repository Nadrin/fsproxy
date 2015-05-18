# FSproxy v1.0RC2
(c) 2009 Michał Siejak (http://www.siejak.pl)

### Please note that FSproxy is considered legacy, and is currently unmaintained. I will not accept any pull requests.

FSproxy is a small application that runs in the system tray and lets you access GNU/Linux partitions from within Microsoft Windows. The list of detected partitions is always availible at \\fsproxy and a single one can easily be mapped to a network drive using address like `\\fsproxy\<partition>`. By default partitions are mounted as read-only but experimental write support can be enabled. FSproxy currently supports: `ext2`, `ext3`, `ext4`, `reiserfs`, `jfs`, `xfs` and few other filesystems. For many of them there is no known Windows driver available (although note that FSproxy is not a driver itself).
