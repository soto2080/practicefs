#ifndef PRACTICEFS

#define PRACTICEFS
#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdint.h>


/*
Bitmaps:
*/

const int IMAP_SIZE = 256;
const int DMAP_SIZE = 512;

/*
DataBlock:
*/

const int BLK_SIZE = 512;
const int PG_SIZE = 8 * BLK_SIZE;

struct datablock {
    char data[BLK_SIZE];
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
 * Special inode numbers
 */
#define	EXT2_BAD_INO		 1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2	/* Root inode */
#define EXT2_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 6	/* Undelete directory inode */

/* First non-reserved inode for old ext2 filesystems */
#define EXT2_GOOD_OLD_FIRST_INO	11

/*
 * Constants relative to the data blocks
 */
#define	EXT2_NDIR_BLOCKS		12
#define	EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define	EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define	EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1)

struct inode {
    enum INODE_TYPE i_type; /* What kind of file this inode point to */
    uint32_t		i_nlink;	/* File link count. */
	uint32_t	    i_uid;		/* File owner. */
	uint32_t	    i_gid;		/* File group. */
	uint64_t	    i_size;		/* File byte count. */
	uint64_t	    i_blocks;	/* Blocks actually held. */
    uint32_t        i_number;   /* The identity of the inode. */
    struct datablock  *i_block; /* pointer to datablock */
    // Time Related
};

/*
SuperBlock:
*/
#define NAME_LENGTH 256
struct superblock {
    uint32_t imap_size;
    uint32_t dmap_size;
    uint32_t blk_size;
    uint32_t num_blks;
    uint64_t fs_size;
    uint32_t name_size;
    uint32_t cur_inode;
    uint32_t num_free_inode;
    uint32_t num_free_dblk;
};

/*
Others:
*/


struct directory_entry {
  int inode_num;
  char file_name[NAME_LENGTH];
};

#endif