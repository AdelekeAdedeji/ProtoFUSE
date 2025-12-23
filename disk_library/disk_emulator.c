#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../include/disk_emulator.h"

static int fd = -1;
static int mounts;


void mount() { mounts++; }

void unmount() { if (mounts > 0) mounts--; }

bool is_mounted() { return mounts > 0; }


int open_disk(const char* file_name, const int n_bytes) {
    fd = open(file_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd == FILE_ERROR) {
        perror("Error opening file.....");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, n_bytes) == FILE_ERROR) {
        perror("size-set error");
        exit(EXIT_FAILURE);
    }

    return fd;
}

int read_block(int disk, int block_nr, void* block) {
    lseek(disk, block_nr << 12, SEEK_SET);

    ssize_t bytes_read = read(disk, block, PAGE_SIZE);

    if (bytes_read == FILE_ERROR) {
        perror("Error reading block....");
        return FILE_ERROR;
    }
    return (int) bytes_read;
}

int write_block(int disk, int block_nr, void* block) {
    lseek(disk, block_nr << 12, SEEK_SET);

    ssize_t bytes_written = write(disk, block, PAGE_SIZE);

    if (bytes_written == FILE_ERROR) {
        perror("Error writing block.....");
        return FILE_ERROR;
    }

    return (int) bytes_written;
}



void sync_disk() { fsync(fd); }

void close_disk() { close(fd); }