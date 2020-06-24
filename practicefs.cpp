/** @file
 * practicefs.cpp
 * A trial to mimic the legend EXT2 filesystem architecture in cpp
 * Copyright (C) 2020  Ming-Han Yang <mhy@mumi.rip>
*/

#include "practicefs.h"
#include <iostream>
#include <bitset>
#include <string.h>

// Global variables
struct superblock sb;
std::bitset<IMAP_SIZE> imap(0);
std::bitset<DMAP_SIZE> dmap(0);
struct datablock blocks[DMAP_SIZE];
struct inode inodes[IMAP_SIZE];

// Functions
void * op_init(struct fuse_conn_info *conn, struct fuse_config *config){
	// Init superblock
	sb.blk_size = BLK_SIZE;
	sb.fs_size = 0;
	sb.imap_size = IMAP_SIZE;
	sb.num_free_inode = IMAP_SIZE;
	sb.dmap_size = DMAP_SIZE;
	sb.num_free_dblk = DMAP_SIZE;
	sb.name_size = NAME_LENGTH;
	sb.cur_inode = 0;
	std::cout<<"Init SuperBlock"<<std::endl;
	//memset(blocks, 0, sizeof(blocks) * sb.dmap_size);
	//memset(inodes, 0, sizeof(inodes) * sb.imap_size);
}

// VFS Operations
static struct fuse_operations practicefs_op = {
	.init		= op_init,
};

int main( int argc, char *argv[] )
{
	return fuse_main( argc, argv, &practicefs_op, NULL );
}