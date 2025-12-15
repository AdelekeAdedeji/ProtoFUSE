#ifndef DISK_LIBRARY_H
#define DISK_LIBRARY_H

#define DISK_SIZE 4096  * 4096

#define PAGE_SIZE 4096


#define DISK_PATH  "/home/adedeji/CLion/CLionProjects/ssd/data.bin"

#define FILE_ERROR -1

int open_disk(const char* file_name, const int n_bytes);

int read_block(int disk, int block_nr, void* block);

int write_block(int disk, int block_nr, void* block);

void sync_disk();

void mount();

void unmount();

bool is_mounted();

#endif