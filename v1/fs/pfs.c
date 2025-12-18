#include <string.h>
#include <stdlib.h>
#include "../include/pfs.h"
#include <errno.h>
#include "../include/disk_emulator.h"


int disk;

static void* shared_cache = NULL;

static uint_8* fs_bitmap_cache = NULL;

static uint_8* inode_bitmap_cache = NULL;

static SuperBlock* super_cache = NULL;

static Inode* inode_cache = NULL;



void create_caches() {
    shared_cache = calloc(PAGE_SIZE, sizeof(char));

    fs_bitmap_cache = calloc(PAGE_SIZE, sizeof(char));

    inode_bitmap_cache = calloc(2 * PAGE_SIZE, sizeof(char));

    super_cache = calloc(256, sizeof(SuperBlock));

    inode_cache = calloc(128, sizeof(Inode));
}


void fs_bitmap_init() {
    read_block(disk, super_cache->fs_bitmap_block, fs_bitmap_cache);

    memset(fs_bitmap_cache, 0xFF, 512);
}


void inode_bitmap_init() {
    read_block(disk, super_cache->inode_bitmap_block1, inode_bitmap_cache);

    read_block(disk, super_cache->inode_bitmap_block2, inode_bitmap_cache + PAGE_SIZE);

    memset(inode_bitmap_cache, 0xFF, 6560);

}


void clear_shared_cache(int no_of_bytes) {
    memset(shared_cache, 0, no_of_bytes);
}


ssize_t get_correct_entity(uint_8* bitmap_t, int entity_no) {
    int check_entity_no = 0;

    if (entity_no < 0) return check_entity_no;

    if (bitmap_t == fs_bitmap_cache) {
        check_entity_no = entity_no < 4096;
    }

    else if (bitmap_t == inode_bitmap_cache) {
        check_entity_no = entity_no < 52480;
    }

    return check_entity_no;

}


int search_bitmap(uint_8* bitmap, int entity_no) {
    int index, bit_blk;

    if (!get_correct_entity(bitmap, entity_no)) {
        return -1;
    }

    index = entity_no >> 3, bit_blk = 7 - (entity_no & 7);

    return ((*(bitmap + index) >> bit_blk) & 1) == 1;
}


int mark_bitmap(uint_8* bitmap, int op, int entity_no) {
    int index, bit_blk;

    if (!get_correct_entity(bitmap, entity_no)) {
        return -1;
    }

    index = entity_no >> 3, bit_blk = 7 - (entity_no & 7);

    if (op) {
        *(bitmap + index) |= 1 << bit_blk ;
    }
    else {
        *(bitmap + index) &= ~(1 << bit_blk);
    }

    return entity_no;

}


void sync_fs_bitmap(int disk) {
    write_block(disk, super_cache->fs_bitmap_block, fs_bitmap_cache);

}


void sync_inode_bitmap(int disk) {
    write_block(disk, super_cache->inode_bitmap_block1, inode_bitmap_cache);

    write_block(disk, super_cache->inode_bitmap_block2, inode_bitmap_cache + PAGE_SIZE);
}


int find_free_inode() {
    int inode_idx;

    for (inode_idx = 0; inode_idx < super_cache -> inodes; inode_idx++) {
        if (search_bitmap(inode_bitmap_cache, inode_idx)) {
            return inode_idx;
        }
    }

    return -1;
}


int allocate_free_blocks(uint_32* direct_ptr) {
    int free_block_id, j = 0;

    for (free_block_id = 0; free_block_id < super_cache -> blocks; free_block_id++) {

        if (j >= DIRECT_BLOCKS_PER_INODE) {
            sync_fs_bitmap(disk);
            return 1;
        }

        if (search_bitmap(fs_bitmap_cache, free_block_id)) {
            *(direct_ptr + j) = free_block_id;

            mark_bitmap(fs_bitmap_cache, MARK_ALLOCATED, free_block_id);

            j++;
        }
    }

    fprintf(stderr, "Error: allocate_free_blocks() --> no available free blocks, cannot create file\n");

    return -1;

}


int free_allocated_blocks(uint_16 size, uint_32* direct_ptr) {
    if (size == 0) {
        return -1;
    }

    uint_16 direct_block_index = (size - 1) >> 12;

    for (int i = 0; i <= direct_block_index; i++) {

        mark_bitmap(fs_bitmap_cache, MARK_FREE, (int) *(direct_ptr + i));

        *(direct_ptr + i) = 0;

    }

    sync_fs_bitmap(disk);

    return 1;

}


bool fs_format() {
    create_caches();

    SuperBlock sup = {
        MAGIC_NUMBER,
        BLOCKS,
        INODE_BLOCKS,
        INODES,
        SUP_BLK_NO,
        FS_BIT_MAP_BLK_NO,
        INODE_BIT_MAP_BLK_NO1,
        INODE_BIT_MAP_BLK_NO2
    };

    disk = open_disk(DISK_PATH, DISK_SIZE);

    read_block(disk, sup.super_block, super_cache);

    *super_cache = sup;

    write_block(disk, super_cache->super_block, super_cache);

    fs_bitmap_init();

    inode_bitmap_init();

    for (int i = 0; i <= INDIRECT_BLK; i++) mark_bitmap(fs_bitmap_cache, MARK_ALLOCATED, i);

    sync_fs_bitmap(disk);

    sync_inode_bitmap(disk);

    return true;

}

bool fs_mount() {
    if (disk > 0) {
        read_block(disk, SUP_BLK_NO, super_cache);

        read_block(disk, super_cache->fs_bitmap_block, fs_bitmap_cache);

        read_block(disk, super_cache->inode_bitmap_block1, inode_bitmap_cache);

        read_block(disk, super_cache->inode_bitmap_block2, inode_bitmap_cache + PAGE_SIZE);

        SuperBlock* sup_blk_ptr = super_cache;

        if (sup_blk_ptr -> magic_number == MAGIC_NUMBER) {
            mount();

            return is_mounted();
        }
    }

    return false;
}


ssize_t fs_create() {
    int inode_idx = find_free_inode(), block, inode_pos;

    if (inode_idx == -1) {
        perror("Error: find_free_inode() --> no available inode, cannot create file");
        return -1;
    }

    block = (inode_idx >> 7) + 1, inode_pos = inode_idx & 127;

    read_block(disk, block, inode_cache);

    (inode_cache + inode_pos) -> type = IS_A_FILE;

    (inode_cache + inode_pos) -> size = 0;

    if (allocate_free_blocks((inode_cache + inode_pos) -> direct) == -1) {
        return -1;
    }

    (inode_cache + inode_pos) -> indirect = INDIRECT_BLK;

    mark_bitmap(inode_bitmap_cache, MARK_ALLOCATED, inode_idx);

    sync_inode_bitmap(disk);

    write_block(disk, block, inode_cache);

    printf("Successful created inode %d\n", inode_idx);

    return 0;
}


ssize_t fs_remove(int inode_id) {
    int block, inode_pos, store_bitmap_search;

    block = (inode_id >> 7) + 1; inode_pos = inode_id & 127;

    store_bitmap_search = search_bitmap(inode_bitmap_cache, inode_id);

    if (store_bitmap_search == -1) {
        fprintf(stderr, "Error: fs_remove() --> inode %d does not exist\n", inode_id);

        return -1;
    }

    if (!store_bitmap_search) {
        read_block(disk, block, inode_cache);

        free_allocated_blocks((inode_cache + inode_pos) -> size, (inode_cache + inode_pos) -> direct);

        (inode_cache + inode_pos) -> size = 0;

        (inode_cache + inode_pos) -> type = 0;

        (inode_cache + inode_pos) -> indirect = 0;

        mark_bitmap(inode_bitmap_cache, MARK_FREE, inode_id);

        sync_inode_bitmap(disk);

        write_block(disk, block, inode_cache);

        printf("Successful removed inode %d\n", inode_id);

        return inode_id;

    }

    fprintf(stderr, "Error: fs_remove() --> inode %d already free\n", inode_id);

    return -1;
}


ssize_t fs_stat(int inode_id) {
    int inode_is_free = search_bitmap(inode_bitmap_cache, inode_id), inode_pos, block;

    inode_pos = inode_id & 127; block = (inode_id >> 7) + 1;

    if (inode_is_free == -1) {
        fprintf(stderr, "Error: fs_stat() --> inode %d does not exist\n", inode_id);
        return -1;
    }

    if (!inode_is_free) {
        read_block(disk, block, inode_cache);

        return (inode_cache + inode_pos) -> size;
    }

    fprintf(stderr, "Error: inode %d is not allocated, can't check size\n", inode_id);

    return -1;
}


ssize_t read_inode_disk_block(Inode* inode, char* user_buff, int bytes_read, off_t offset) {
    int file_size = DIRECT_BLOCKS_PER_INODE * PAGE_SIZE, requested_bytes = 0;

    if (bytes_read <= 0 || offset < 0) {
        return 0;
    }

    if (bytes_read > inode -> size  || bytes_read + offset > inode -> size) {
        fprintf(stderr, "EOF: Cannot read past file end (%.2fKB)\n", inode -> size / 1024.0);
        return 0;
    }

    if (bytes_read > file_size || bytes_read + offset >= file_size) {
        fprintf(stderr, "Error: Cannot read past max file size (%dKB)\n", file_size >> 10);
        return 0;
    }

    uint_16 direct_block_start = offset >> 12, offset_at_block = offset & 4095,

    no_of_blocks = (bytes_read + offset_at_block - 1) >> 12;

    uint_16 direct_block_end = direct_block_start + no_of_blocks;

    while (bytes_read + offset_at_block > PAGE_SIZE && direct_block_start <= direct_block_end) {
        read_block(disk, (int) *(inode -> direct + direct_block_start), shared_cache);

        memcpy(user_buff, shared_cache + offset_at_block, PAGE_SIZE - offset_at_block);

        bytes_read -= PAGE_SIZE - offset_at_block;

        user_buff += PAGE_SIZE - offset_at_block;

        requested_bytes += PAGE_SIZE - offset_at_block;

        offset_at_block = 0;

        direct_block_start++;
    }

    read_block(disk, (int) *(inode -> direct + direct_block_start), shared_cache);

    memcpy(user_buff, shared_cache + offset_at_block, bytes_read);

    requested_bytes += bytes_read;

    return requested_bytes;

}


ssize_t fs_read(int inode_id, char* data, int length, off_t offset) {
    int inode_is_free = search_bitmap(inode_bitmap_cache, inode_id), inode_pos, block;

    inode_pos = inode_id & 127; block = (inode_id >> 7) + 1;

    if (inode_is_free == -1) {
        fprintf(stderr, "Error: fs_read() --> inode %d does not exist\n", inode_id);

        return -1;
    }

    if (inode_is_free) {
        fprintf(stderr, "Error: fs_read() --> Cannot read, inode %d not allocated\n", inode_id);

        return -1;
    }

    read_block(disk, block, inode_cache);

    (inode_cache + inode_pos) -> size = 4110;

    if ((inode_cache + inode_pos) -> size <= 0) {

        fprintf(stderr, "Error: Cannot read empty file\n");

        return 0;
    }

    if ((inode_cache + inode_pos) -> type != IS_A_FILE) {

        fprintf(stderr, "Error: fs_read() --> Cannot read, inode %d is not a file\n", inode_id);

        return -1;

    }

    return read_inode_disk_block(inode_cache + inode_pos, data, length, offset);

}

ssize_t fs_write() {

}