#pragma once
#include <stdbool.h>
#include <stdio.h>

#define IS_A_FILE                        1
#define FILE_MAX_SIZE                    7 << 12
#define MAGIC_NUMBER                     0xFEED       // 65261
#define BLOCKS                           4096
#define INODE_BLOCKS                     410
#define INODES                           52480
#define DIRECT_BLOCKS_PER_INODE        6
#define SUP_BLK_NO                       0
#define FS_BIT_MAP_BLK_NO                411
#define INODE_BIT_MAP_BLK_NO1            412
#define INODE_BIT_MAP_BLK_NO2            413
#define INDIRECT_BLK                     414
#define MARK_FREE                        1
#define MARK_ALLOCATED                   0



typedef unsigned char uint_8;

typedef unsigned short uint_16;

typedef unsigned int uint_32;

typedef unsigned long uint_64;


typedef struct superblock {

    uint_16 magic_number;
    uint_16 blocks;
    uint_16 inode_blocks;
    uint_16 inodes;
    uint_16 super_block;
    uint_16 fs_bitmap_block;
    uint_16 inode_bitmap_block1;
    uint_16 inode_bitmap_block2;

} SuperBlock;

typedef struct inode {

    uint_16 type;
    uint_16 size;
    uint_32 direct[DIRECT_BLOCKS_PER_INODE];
    uint_32 indirect;

} Inode;

typedef struct directory_entry {

    char* file_name;
    int i_fd;

} D_Entry;


static void create_caches();

static void fs_bitmap_init();

static void inode_bitmap_init();

static void clear_shared_cache(int no_of_bytes);

static ssize_t get_correct_entity(uint_8* bitmap_t, int entity_no);

static int search_bitmap(uint_8* bitmap, int entity_no);

static int mark_bitmap(uint_8* bitmap, int op, int entity_no);

static void sync_fs_bitmap(int disk);

static void sync_inode_bitmap(int disk);

static int find_free_inode();

static int allocate_free_blocks(uint_32* direct_ptr);

static int free_allocated_blocks(uint_16 size, uint_32* direct_ptr);

static ssize_t read_inode_disk_block(Inode* inode, char* user_buff, int bytes_read, off_t offset);

ssize_t write_inode_disk_block(Inode* inode, char* user_buff, int bytes_write, off_t offset);

bool fs_format();

bool fs_mount();

ssize_t fs_create();

ssize_t fs_remove(int inode_id);

ssize_t fs_stat(int inode_id);

ssize_t fs_read(int inode_id, char* data, int length, off_t offset);

ssize_t fs_write(int inode_id, char* data, int length, off_t offset);