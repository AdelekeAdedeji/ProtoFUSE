**ProtoFUSE**

**Status**: ongoing

ProtoFUSE is a simple user space file system. It is aimed at emulating how a file system works at a very foundational level (at least for now)

This is currently v1, with improvements to be made, however the current version works pretty well and does 
expected operations.

The filesystem contains a disk emulator ([disk_emulator.c](https://github.com/AdelekeAdedeji/ProtoFUSE/blob/main/disk_library/disk_emulator.c)) whose job is to emulate a physical disk and provides 
interfaces like open_disk which opens an existing disk (or create the disk if it doesn't exist), 
read_block, write_block to perform Disk I/O read-write operations, sync_disk to immediately write disk caches to disk and 
close_disk which closes the disk (the emulated disk is a bin file and must be passed as an argument to the executable)

We then have the implementation of the filesystem ([pfs.c](https://github.com/AdelekeAdedeji/ProtoFUSE/blob/main/fs/pfs.c)) which implements several interfaces, the major ones including
fs_format, fs_mount, fs_create, fs_remove, fs_stat, fs_read, fs_write and fs_destroy. 

Other interfaces also worthy of note
are the read_inode_disk_block and write_inode_disk_block interfaces. 

pfs also implements many useful helper functions such as 
search_bitmap, mark_bitmap, sync_fs_bitmap, sync_inode_bitmap, find_free_inode, allocate_free_blocks,
free_allocated_blocks, get_correct_entity, create_caches, fs_bitmap_init, inode_bitmap_init 
and fs_mounted

**An overview of major interfaces**

`fs_format` formats the filesystem and initializes super block

`fs_mount` mounts the filesystem

`fs_create` creates an inode in the filesystem

`fs_remove` deletes an inode from the filesystem

`fs_stat` shows the size of a file 

`fs_write` validates inode and writes to inode's data blocks

`fs_read` also validates inode but reads from inode's data_blocks

`fs_destroy` unmounts filesystem, frees filesystem caches (superblock cache, inodes cache, bitmap caches and shared cache/data cache) 
and closes the disk

Then we have the custom shell ([pfssh.c](https://github.com/AdelekeAdedeji/ProtoFUSE/blob/main/pfssh.c)) and while not a fully POSIX-compliant interpreter still works with the filesystem at least for this stage

Currently directories are yet to be implemented, so for now you'll generally see files represented as inode numbers (their internal representation) 
and not with explicit names or file paths. 

Also file permissions have not been implemented in this current version (normally inode struct should contain rwx permission bits or fields) 

`fs_mount` in the proper sense doesn't truly mount, usually mount will initialize root inode and maintains a mount table, 
the current version of mount could be seen as just a helper to a guard that prevents calling `fs_read`, `fs_write` and other filesystem interfaces before `fs_mount`   

The indirect block is not used, only direct blocks are used, will become useful if we allocate more disk space or reduce the number of direct blocks, the latter might not happen.

All these issues and more will be addressed in v2. If you happen to find any additional flaw or bug, feel free to raise an [issue](https://github.com/AdelekeAdedeji/ProtoFUSE/issues)
