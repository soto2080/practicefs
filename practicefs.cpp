/** @file
 * practicefs.cpp
 * A trial to mimic the legend EXT2 filesystem architecture in cpp
 * Copyright (C) 2020  Ming-Han Yang <mhy@mumi.rip>
 */

#include "practicefs.h"
#include <bits/stdint-uintn.h>
#include <bitset>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <queue>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

// Global variables
struct superblock sb;
std::bitset<IMAP_SIZE> imap(0);
std::bitset<DMAP_SIZE> dmap(0);
struct datablock blocks[DMAP_SIZE];
struct inode inodes[IMAP_SIZE];

// Helpers

// Split the full string into small pieces
std::vector<std::string> split_path(const char *path) {
  std::string str(path);
  std::vector<std::string> ancestor;
  ancestor.push_back("/");

  size_t pos = 0;
  std::string name;
  auto end = str.find('/');
  while (end != std::string::npos) {
    
	if (str.substr(pos, end - pos).length() > 0) {
  		ancestor.push_back(str.substr(pos, end - pos));
    }
    pos = end + 1;
    end = str.find('/', pos);
  }

  if (str.substr(pos, end).length() > 0) {
  	ancestor.push_back(str.substr(pos, end));
  }
  
  // Debug logging
  for (auto x : ancestor) {
    std::cout << x << std::endl;
  }
  return ancestor;
}

// Find inode number by name
size_t find_inum(std::string name)
{
  // It's broken
  size_t idx = 0;
	while(idx < IMAP_SIZE){
    if( imap.test(idx) && *inodes[idx].i_name == name)
      return idx;
  }
	return -1;
}

// Allocate an inode number for new inode
size_t alloc_inum(){
  size_t idx = 0;
  while(idx < imap.size() && imap.test(idx))
  {
    ++idx;
  }
  imap.set(idx);
  return idx;
}

int init_inode(std::string& name ,size_t inum, size_t parent, INODE_TYPE type){
  memset(&inodes[inum], 0, sizeof(struct inode));
  inodes[inum].i_type = type;
  inodes[inum].i_blocks = 0;
  inodes[inum].i_size = 0;
  inodes[inum].i_number = inum;
  inodes[inum].i_parent = parent;
  inodes[inum].i_nlink = 1;
  inodes[inum].i_uid = getuid();
  inodes[inum].i_gid = getgid();
  inodes[inum].i_name = &name;

  return 0;
}

void print_inode(size_t inum){
  std::cout<<"i_name: "     << *inodes[inum].i_name   << std::endl
           <<"i_uid: "        << inodes[inum].i_uid    << std::endl
           <<"i_gid: "        << inodes[inum].i_gid    << std::endl
           <<"i_nlink: "      << inodes[inum].i_nlink  << std::endl
           <<"i_number: "     << inodes[inum].i_number << std::endl
           <<"i_parent: "     << inodes[inum].i_parent << std::endl
           <<"i_type: "       << inodes[inum].i_type   << std::endl
           <<"i_size: "       << inodes[inum].i_size   << std::endl
           <<"i_blocks: "     << inodes[inum].i_blocks << std::endl;
}

// Functions
static int op_getattr(const char *path, struct stat *st,
                      struct fuse_file_info *info) {
  std::cout << "Geting attr: " << path << std::endl;
  memset(st, 0, sizeof(struct stat));
  st->st_uid = getuid(); // The owner of the file/directory is the user who
                         // mounted the filesystem
  st->st_gid = getgid(); // The group of the file/directory is the same as the
                         // group of the user who mounted the filesystem
  if (strcmp(path, "/") == 0) {
    st->st_uid = inodes[root_inode_num].i_uid;
    st->st_gid = inodes[root_inode_num].i_gid;
    st->st_mode = S_IFDIR | 0755;
    st->st_nlink =
        inodes[root_inode_num]
            .i_nlink; // Why "two" hardlinks instead of "one"? The answer is
                      // here: http://unix.stackexchange.com/a/101536
  } else {
    return -ENOENT;
  }

  return 0;
}

static int op_mknod(const char *path, mode_t mode, dev_t rdev) {
  std::cout << "Making inode: " << path << std::endl;
  // Find parent dir first
  std::vector<std::string> splited_path = split_path(path);
  uint32_t parent = 0;
  std::string test = "/";
  

  // Init a new inode as a regular file
  // Get a inode num first
  int i_num = alloc_inum();
  //std::cout<< imap.to_string()<<std::endl;

  // Init the relative inode to the inum
  init_inode(splited_path.back(), i_num, parent, IFREG);
  print_inode(i_num);
  // inject the inode into its parent
  return 0;
}

static int op_mkdir(const char *path, mode_t mode) {
  std::cout << "Making dir: " << path << std::endl;
  // Find parent dir first
  std::vector<std::string> splited_path = split_path(path);

  // Init a new inode as a dir
  // Get a inode num first
  int i_num = alloc_inum();
  std::cout<< imap.to_string()<<std::endl;
  // Init the relative inode to the inum
  //init_inode(splited_path.back().c_str(), i_num, IFDIR);
  print_inode(i_num);
  // Inject the inode into its parent
  return 0;
}

static int op_unlink(const char *path) {
  std::cout << "Unlinking inode" << path << " count by 1" << std::endl;
  return 0;
}

static int op_rmdir(const char *path) {
  std::cout << "Removing dir: " << path << std::endl;
  return 0;
}

static int op_read(const char *path, char *buffer, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
  std::cout << "Reading file: " << path << std::endl;
  return 0;
}

static int op_write(const char *path, const char *buffer, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
  std::cout << "Writing file: " << path << std::endl;
  return 0;
}

static int op_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi,
                      enum fuse_readdir_flags flag) {
  std::cout << "Reading dir: " << path << std::endl;

  filler(buffer, ".", NULL, 0, FUSE_FILL_DIR_PLUS);  // Current Directory
  filler(buffer, "..", NULL, 0, FUSE_FILL_DIR_PLUS); // Parent Directory
  return 0;
}

void *op_init(struct fuse_conn_info *conn, struct fuse_config *config) {
  // Init superblock
  sb.blk_size = BLK_SIZE;
  sb.fs_size = 0;
  sb.imap_size = IMAP_SIZE;
  sb.num_free_inode = IMAP_SIZE;
  sb.dmap_size = DMAP_SIZE;
  sb.num_free_dblk = DMAP_SIZE;
  sb.cur_inode = 1; // 0 is for root
  std::cout << "Init SuperBlock" << std::endl;

  imap.set(root_inode_num);
  dmap.set(0);

  inodes[root_inode_num].i_number = root_inode_num;
  inodes[root_inode_num].i_blocks = 1;
  inodes[root_inode_num].i_size = 4;
  inodes[root_inode_num].i_nlink = 2;
  inodes[root_inode_num].i_parent = 0;

  std::string s = "/";
  inodes[root_inode_num].i_name = &s;
  inodes[root_inode_num].i_uid = getuid();
  inodes[root_inode_num].i_gid = getgid();
  inodes[root_inode_num].i_type = IFDIR;

  inodes[root_inode_num].i_block[0] = &blocks[0];
  std::cout << "Init Root Inode:" << *inodes[root_inode_num].i_name << std::endl;

  // memset(blocks, 0, sizeof(blocks) * sb.dmap_size);
  // memset(inodes, 0, sizeof(inodes) * sb.imap_size);
  return nullptr;
}

static int op_utimens(const char *path, const struct timespec tv[2],
                      struct fuse_file_info *info) {
  std::cout << "Setting time: " << path << std::endl;
  return 0;
}
// VFS Operations
static struct fuse_operations practicefs_op = {
    .getattr = op_getattr,
    .mknod = op_mknod,
    .mkdir = op_mkdir,
    .unlink = op_unlink,
    .rmdir = op_rmdir,
    .read = op_read,
    .write = op_write,
    .readdir = op_readdir,
    .init = op_init,
    .utimens = op_utimens,
};

int main(int argc, char *argv[]) {
  return fuse_main(argc, argv, &practicefs_op, NULL);
}