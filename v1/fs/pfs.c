#include <string.h>
#include <stdlib.h>
#include "../include/pfs.h"
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


ssize_t get_correct_entity(uint_8* bitmap, int entity_no) {
    int check_entity_no = 0;

    if (bitmap == fs_bitmap_cache) {
        check_entity_no = entity_no < 4096;
    }

    else if (bitmap == inode_bitmap_cache) {
        check_entity_no = entity_no < 52480;
    }

    return check_entity_no;

}


bool search_bitmap(uint_8* bitmap, int entity_no) {
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

int find_free_inode() {
    int inode_idx;

    for (inode_idx = 0; inode_idx < super_cache -> inodes; inode_idx++) {
        if (search_bitmap(inode_bitmap_cache, inode_idx)) {
            return inode_idx;
        }
    }

    return -1;
}


ssize_t fs_create() {
    int inode_idx = find_free_inode(), block, inode_pos;

    if (inode_idx == -1)
        perror("Cannot and will not create more files, disc space full");

    block = (inode_idx >> 16) + 1, inode_pos = inode_idx & 15;

    read_block(disk, block, inode_cache);

    (inode_cache + inode_pos) -> size = 0;

    (inode_cache + inode_pos) -> direct;

    (inode_cache + inode_pos) -> indirect;

    return 0;
}
