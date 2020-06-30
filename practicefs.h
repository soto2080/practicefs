#ifndef PRACTICEFS

#define PRACTICEFS
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>
#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdint.h>


#define BLOCK_DEV_PATH  "/dev/sdb"

// hardcoded block offset
// the UNIT is """BLK_SIZE"""
#define SUPERBLK_OFFSET 0;
#define IMAP_OFFSET 1;
#define DMAP_OFFSET 10;
// 256Bytes per * IMAP_SIZE / BLK_SIZE => 256*1024/4096 = 64BLK
#define INODE_OFFSET 100;
// DMAP_SIZE
#define DATABLK_OFFSET 200;


/*
Bitmaps:
*/

const int IMAP_SIZE = 1024;
const int DMAP_SIZE = 1048576;

/*
DataBlock:
*/
const int BLK_SIZE = 4096;
const int PG_SIZE = 1 * BLK_SIZE;

struct datablock {
    char data[BLK_SIZE];
};


/*
Others:
*/

//
// In ram filename cache struct
//
class directory_entry {
public:
  directory_entry(size_t inum, std::string name){
      this->inode_num = inum;
      this->name = name;
  }
  directory_entry(){
  }
  size_t inode_num;
  std::string name;
};

//
// filename and inum, 256Bytes
//
class file_name{
public:
    size_t inode_num;
    char name[BLK_SIZE-8];

    file_name(){
    };
    file_name(size_t inum, const char* input){
        this->inode_num = inum;
        size_t len = strlen(input);
        if(len < BLK_SIZE-8)
            strcpy(this->name, input);    
    }
    std::string get_name(){
        std::string tmp(name);
        return tmp;
    }
};

//
// indirect offset table, matching the blk size
//
class indirect_offset{
public:
    size_t table [ BLK_SIZE / sizeof(size_t) ];
};

/*
SuperBlock: 64Bytes
*/
const int root_inode_num = 2;
struct superblock {
    size_t imap_size;
    size_t dmap_size;
    size_t blk_size;
    size_t num_blks;
    size_t fs_size;
    size_t cur_inode;
    size_t num_free_inode;
    size_t num_free_dblk;
};

/*
Inode:
*/

/* File types. */
enum INODE_TYPE {
    IFIFO,
    IFCHR,
    IFDIR,
    IFBLK,
    IFREG,
    IFLNK,
    IFSOCK
};

/*
 * Constants relative to the data blocks
 */
#define	EXT2_NDIR_BLOCKS		12
#define	EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define	EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define	EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1)

// 256Bytes per
struct inode {  
    enum INODE_TYPE i_type; /* What kind of file this inode point to */
    //std::string     *i_name;    /* File readable name*/
    size_t		i_nlink;	/* File link count. */
	size_t	    i_uid;		/* File owner. */
	size_t	    i_gid;		/* File group. */
	size_t	    i_size;		/* File byte count. */
	size_t	    i_blocks;	/* Blocks actually held. */
    size_t        i_number;   /* The identity of the inode. */
    size_t        i_parent;   /* The parent of the inode. */

    size_t i_block[EXT2_N_BLOCKS]; /* array of blk offsets in storage */

    // Time related
    struct timespec i_atim;
    struct timespec i_ctim;
    struct timespec i_mtim;
    
    // replace this with pointer
    //std::vector<directory_entry *> entries; /* pointer to dir content list contructed from filelist in blocks */
    size_t		i_file;
};



#endif