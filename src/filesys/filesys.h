#ifndef FILESYS_FILESYS_H
#define FILESYS_FILESYS_H

#include <stdbool.h>
#include "devices/block.h"
#include "filesys/off_t.h"

/* Sectors of system file inodes. */
#define FREE_MAP_SECTOR 0 /* Free map file inode sector. */
#define ROOT_DIR_SECTOR 1 /* Root directory file inode sector. */

/* Block device that contains the file system. */
extern struct block* fs_device;

void filesys_init(bool format);
void filesys_done(void);
bool filesys_create(const char* name, off_t initial_size, bool is_dir);
bool sys_file_create(const char* name, off_t initial_size, bool is_dir);
struct file* filesys_open(const char* name);
bool filesys_remove(const char* name);

struct dir* filesys_dir_open(const char* name);
bool filesys_chdir(const char *path);

uint32_t file_inumber(struct file *file);

uint32_t dir_inumber(struct dir *dir);

bool file_is_dir(struct file *file);

off_t cache_read_at(block_sector_t sector, void* buffer_, off_t size, off_t offset);
off_t cache_write_at(block_sector_t sector, const void* buffer_, off_t size, off_t offset);

#endif /* filesys/filesys.h */
