/** @file
 * practicefs.cpp
 * A trial to mimic the legend EXT2 filesystem architecture in cpp
 * Copyright (C) 2020  Ming-Han Yang <mhy@mumi.rip>
*/

#include "practicefs.h"
#include <iostream>
#include <bitset>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>

// Global variables
struct superblock sb;
std::bitset<IMAP_SIZE> imap(0);
std::bitset<DMAP_SIZE> dmap(0);
struct datablock blocks[DMAP_SIZE];
struct inode inodes[IMAP_SIZE];

// Functions
static int op_getattr( const char *path, struct stat *st, struct fuse_file_info *info)
{
	std::cout<<"Geting attr"<<std::endl;
	memset(st, 0, sizeof(struct stat));
	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	if ( strcmp( path, "/" ) == 0)
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	return 0;
}

static int op_mknod( const char *path, mode_t mode, dev_t rdev )
{
	std::cout<<"Making inode"<<std::endl;
	return 0;
}

static int op_mkdir( const char *path, mode_t mode )
{
	std::cout<<"Making dir"<<std::endl;
	return 0;
}

static int op_unlink( const char *path)
{
	std::cout<<"Unlinking inode count by 1"<<std::endl;
	return 0;
}

static int op_rmdir( const char *path)
{
	std::cout<<"Removing dir"<<std::endl;
	return 0;
}

static int op_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	std::cout<<"Reading file"<<std::endl;
	return 0;
}

static int op_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	std::cout<<"Writing file"<<std::endl;
	return 0;
}

static int op_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flag)
{
	std::cout<<"Reading dir"<<std::endl;
	filler( buffer, ".", NULL, 0, FUSE_FILL_DIR_PLUS); // Current Directory
	filler( buffer, "..", NULL, 0, FUSE_FILL_DIR_PLUS); // Parent Directory
	return 0;
}

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
	return nullptr;
}

static int op_utimens( const char *path, const struct timespec tv[2], struct fuse_file_info *info)
{
	return 0;
}
// VFS Operations
static struct fuse_operations practicefs_op = {
	.getattr	= op_getattr,
	.mknod		= op_mknod,
	.mkdir		= op_mkdir,
	.unlink		= op_unlink,
	.rmdir		= op_rmdir,
	.read		= op_read,
    .write		= op_write,
    .readdir	= op_readdir,
	.init		= op_init,
	.utimens	= op_utimens,
};

int main( int argc, char *argv[] )
{
	return fuse_main( argc, argv, &practicefs_op, NULL );
}