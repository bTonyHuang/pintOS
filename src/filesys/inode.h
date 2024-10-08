#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include <stdbool.h>
#include <list.h>
#include "filesys/off_t.h"
#include "devices/block.h"
#include "threads/synch.h"

struct bitmap;

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

#define DIRECTS_SIZE 123

#define INDIRECT_SIZE 128

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk {
  block_sector_t directs[DIRECTS_SIZE]; /* Array of 123 direct pointers. */
  block_sector_t indirect;     /* Singly indirect pointer. */
  block_sector_t dbl_indirect; /* Doublely-indirect pointer. */
  off_t length;                /* File size in bytes. */
  unsigned magic;              /* Magic number. */
  uint8_t unused1;           /* Padding. */
  uint8_t unused2;
  uint8_t unused3;
  bool is_dir;                 /* Whether the inode represenets a directory or a file. */
};

/* In-memory inode. */
struct inode {
  struct list_elem elem;  /* Element in inode list. */
  block_sector_t sector;  /* Sector number of disk location. */
  int open_cnt;           /* Number of openers. */
  bool removed;           /* True if deleted, false otherwise. */
  int deny_write_cnt;     /* 0: writes ok, >0: deny writes. */
  bool is_cwd;         /* Directory is serving as some process's cwd. */
  struct lock inode_lock; /* Synnchronization for concurrent inode resize and inode read, as well as deny_write concurrency. */
};

void inode_init(void);
bool inode_create(block_sector_t, off_t, bool);
struct inode* inode_open(block_sector_t);
struct inode* inode_reopen(struct inode*);
block_sector_t inode_get_inumber(const struct inode*);
void inode_close(struct inode*);
void inode_remove(struct inode*);
off_t inode_read_at(struct inode*, void*, off_t size, off_t offset);
off_t inode_write_at(struct inode*, const void*, off_t size, off_t offset);
void inode_deny_write(struct inode*);
void inode_allow_write(struct inode*);
off_t inode_length(const struct inode*);
bool inode_is_dir (const struct inode *inode);

bool inode_resize(struct inode_disk* id, off_t size);
bool indirect_block_check(block_sector_t* indirect, off_t size);
bool dbl_indirect_block_check(block_sector_t* dbl_indirect, off_t size);

#endif /* filesys/inode.h */
