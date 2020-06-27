/** @file
 * practicefs.cpp
 * A trial to use the Linux virtual filesystem architecture in cpp
 * Copyright (C) 2020  Ming-Han Yang <mhy@mumi.rip>
 */

#include "practicefs.h"
#include <bits/stdint-uintn.h>
#include <bitset>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ostream>
#include <queue>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vector>

// Global variables
struct superblock sb;
std::bitset<IMAP_SIZE> imap(0);
std::bitset<DMAP_SIZE> dmap(0);
struct datablock blocks[DMAP_SIZE];
struct inode inodes[IMAP_SIZE];

// Helpers

// Split the full string into small pieces till last /
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
  //for (auto x : ancestor) {
  //  std::cout << x << std::endl;
  //}
  return ancestor;
}

std::string split_filename(const char *path){
  std::string tmp(path);
  std::size_t found = tmp.find_last_of("/");
  return tmp.substr(found+1);
}

// find specify name's inode in splted path from specift parent inode
size_t find_inum_helper(std::vector<std::string> splited_path, size_t pnum, size_t idx){
  size_t inum = 0;
  if(!inodes[pnum].entries.size())
    return 0;
  //std::cout<< pnum <<"before loop"<< inodes[pnum].entries.size()<< " depth: " <<idx<< std::endl;

  for(auto entry : inodes[pnum].entries){
    if(entry->name == splited_path[idx]){
      inum = entry->inode_num;
      //std::cout<<"Gotcha: "<<inum<<std::endl;
    }
  }
  return inum;
}

// Find inode number by path
size_t find_inum(std::vector<std::string> splited_path)
{
  // finding inode from root inum, so bypass "/"
  size_t idx = 1;
  size_t inum = root_inode_num;

  if(splited_path.size() == 1)      // only "/", it's root , so inum == 2
    return root_inode_num;

  if(!inodes[inum].entries.size()){
    return 0;
  }
  
  while(idx < splited_path.size() && inodes[inum].entries.size()){
    inum = find_inum_helper(splited_path, inum, idx);
    //std::cout <<" tmp inum: "<< inum << std::endl;
    if(!inum)
      return 0;
    ++idx;
  }
  //std::cout <<"inum: "<< inum << std::endl;
  if(idx == splited_path.size())
    return inum;
  return 0;
}

// Allocate an inode number for new inode
size_t alloc_inum(){
  // Reserving the first two inum
  size_t idx = root_inode_num;
  while(idx < imap.size() && imap.test(idx))
  {
    ++idx;
  }
  imap.set(idx);
  return idx;
}

int init_inode(std::string name ,size_t inum, size_t parent, INODE_TYPE type){
  struct timespec now;
	timespec_get(&now, TIME_UTC);

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
  inodes[inum].ATIME = now;
  inodes[inum].CTIME = now;
  inodes[inum].MTIME = now;

  // inject the inode into its parent's dir list
  if(inodes[parent].i_type == IFDIR){
    // a new child dir add a link to its parent to itself
    ++inodes[parent].i_nlink;
    inodes[parent].entries.push_back(new directory_entry(inum,name));
    inodes[parent].CTIME = now;
    return 0;
  }
   //TODO: deallocate the inode
  return -1;
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

  // get the relative inode
  std::vector<std::string> splited_path = split_path(path);
  size_t inum = find_inum(splited_path);

  memset(st, 0, sizeof(struct stat));

  if (inum && imap.test(inum)) {
    st->st_ino = inodes[inum].i_number;
    st->st_uid = inodes[inum].i_uid;
    st->st_gid = inodes[inum].i_gid;
    if(inodes[inum].i_type == IFDIR){
      st->st_mode = S_IFDIR | 0755;
    }else{
      st->st_mode = S_IFREG | 0644;
    }
    st->st_nlink =inodes[inum].i_nlink;
    st->st_size = inodes[inum].i_size;
    st->st_atim = inodes[inum].ATIME;
    st->st_mtim = inodes[inum].MTIME;
    st->st_ctim = inodes[inum].CTIME;
  } else {
    return -ENOENT;
  }

  return 0;
}

static int op_mknod(const char *path, mode_t mode, dev_t rdev) {
  std::cout << "Making inode: " << path << std::endl;
  // Find parent dir first
  std::vector<std::string> splited_path = split_path(path);
  splited_path.erase(splited_path.end());
  size_t parent = find_inum(splited_path);

  // Init a new inode as a regular file
  // Get a inode num first
  size_t i_num = alloc_inum();

  // Init the new inode to the inum
  init_inode(split_filename(path), i_num, parent, IFREG);
  // print_inode(i_num);
  return 0;
}

static int op_mkdir(const char *path, mode_t mode) {
  std::cout << "Making dir: " << path << std::endl;
  // Find parent dir first
  std::vector<std::string> splited_path = split_path(path);
  splited_path.erase(splited_path.end());
  size_t parent = find_inum(splited_path);

  // Init a new inode as a dir

  // Get a inode num first
  size_t i_num = alloc_inum();

  // Init the new inode to the inum
  init_inode(split_filename(path), i_num, parent, IFDIR);
  //print_inode(i_num);
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
  // Todo: add read dir support for any path
  //if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
	//{ 
  //  if(inodes[root_inode_num].entries.size())
  //    for(auto entry: inodes[root_inode_num].entries){
  //      std::cout<<entry->name<<std::endl;
  //      filler( buffer, entry->name.c_str(), NULL, 0 ,FUSE_FILL_DIR_PLUS);
  //    }
	//}

  std::vector<std::string> splited_path = split_path(path);
  size_t inum = find_inum(splited_path);
  
  if(inodes[inum].i_type == IFDIR){
  
    for(auto entry : inodes[inum].entries){
      //std::cout<<entry->name<<std::endl;
      filler(buffer, entry->name.c_str(), NULL, 0, FUSE_FILL_DIR_PLUS);
    }
    return 0;
  }

  // Not a dir
  return -1;
  
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

  struct timespec now;
	timespec_get(&now, TIME_UTC);
  inodes[root_inode_num].ATIME = now;
  inodes[root_inode_num].CTIME = now;
  inodes[root_inode_num].MTIME = now;

  inodes[root_inode_num].i_block[0] = &blocks[0];
  std::cout << "Init Root Inode:" << *inodes[root_inode_num].i_name << std::endl;

  // memset(blocks, 0, sizeof(blocks) * sb.dmap_size);
  // memset(inodes, 0, sizeof(inodes) * sb.imap_size);
  return nullptr;
}

static int op_utimens(const char *path, const struct timespec tv[2],
                      struct fuse_file_info *info) {
  std::cout << "Setting time: " << path << std::endl;

  // Find target inum
  std::vector<std::string> splited_path = split_path(path);
  size_t inum = find_inum(splited_path);

  // Get the 
  struct timespec now;
	timespec_get(&now, TIME_UTC);

  // Workaround, the timespec give wrong time
  inodes[inum].ATIME = now;
  inodes[inum].CTIME = now;

  //inodes[inum].ATIME = tv[0];
  //inodes[inum].CTIME = tv[1];
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