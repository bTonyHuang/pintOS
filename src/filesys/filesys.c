#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"

/* Partition that contains the file system. */
struct block* fs_device;

static void do_format(void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void filesys_init(bool format) {
  fs_device = block_get_role(BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC("No file system device found, can't initialize file system.");

  inode_init();
  free_map_init();

  if (format)
    do_format();

  free_map_open();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void filesys_done(void) { free_map_close(); }

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool filesys_create(const char* name, off_t initial_size, bool is_dir) {
  block_sector_t inode_sector = 0;
  char filename[NAME_MAX + 1];
  struct dir* container_dir = resolve(name, filename);
  bool success = (container_dir != NULL && free_map_allocate(1, &inode_sector);

  if (!success && inode_sector != 0) {
    free_map_release(inode_sector, 1);
    dir_close(container_dir);
    return false;
  }
  
  /* Two cases for "file" creation, create and mkdir. */
  if (is_dir) {
    success = dir_create(inode_sector, initial_size + 2) && dir_add(dir, name, inode_sector));
  } else {
    success = inode_create(inode_sector, initial_size, is_dir) && dir_add(dir, name, inode_sector));
  }
  dir_close(container_dir);

  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file* filesys_open(const char* name) {
  char filename[NAME_MAX + 1];
  struct dir* container_dir = resolve(name, filename);
  if (container_dir == NULL) {
    return NULL;
  }
  struct inode* inode;
  bool inode_exists = dir_lookup(container_dir, filename, &inode);
  dir_close(container_dir);
  if (!inode_exists) {
    return NULL;
  }

  return file_open(inode);
}

struct dir* filesys_dir_open(const char* name) {
  char filename[NAME_MAX + 1];
  struct dir* container_dir = resolve(name, filename);
  if (container_dir == NULL) {
    return NULL;
  }
  struct inode* inode;
  bool inode_exists = dir_lookup(container_dir, filename, &inode);
  dir_close(container_dir);
  if (!inode_exists) {
    return NULL;
  }

  return dir_reopen(inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool filesys_remove(const char* name) {
  char filename[NAME_MAX + 1];
  struct dir* container_dir = resolve(name, filename);
  if (container_dir == NULL) {
    return false;
  }
  bool remove_success = dir_remove(container_dir, filename);
  /* Resolve incremented open_cnt for container_dir, but now we're done with it. */
  dir_close(container_dir);

  return remove_success;
}

/* Changes the current working directory of the current thread to the directory located at PATH.
  Returns true is successful, false otherwise. */
bool filesys_chdir(const char *path) {
  struct thread *t = thread_current();
  char filename[NAME_MAX + 1];

  struct dir* container_dir = resolve(path, filename);
  if (container_dir == NULL) {
    return false;
  }
  
  struct inode* inode;
  bool inode_exists = dir_lookup(container_dir, filename, &inode);
  if (!inode_exists) {
    return false;
  }
  dir_close(container_dir);

  if (!inode_is_dir(inode)) {
    return false;
  }

  inode_close(t->pcb->cwd);
  t->pcb->cwd = dir_open(inode);
  return true;
}

/* Returns the inumber aka sector id corresponding to a file. */
uint32_t file_inumber(struct file *file) {
  return file->inode->sector;
}

/* Returns the inumber aka sector id corresponding to a directory. */
uint32_t dir_inumber(struct dir *dir) {
  return dir->inode->sector;
}

/* User syscall for checking if a file refers to a directory. */
bool file_is_dir(struct file *file) {
  return inode_is_dir(file->inode);
}

/* Formats the file system. */
static void do_format(void) {
  printf("Formatting file system...");
  free_map_create();
  if (!dir_create(ROOT_DIR_SECTOR, 16))
    PANIC("root directory creation failed");
  free_map_close();
  printf("done.\n");
}
