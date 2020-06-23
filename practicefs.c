/** @file
 * practicefs.c
 * mimic the legend EXT2 filesystem
 * Copyright (C) 2020  Ming-Han Yang <mhy@mumi.rip>
*/

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

static struct fuse_operations practicefs_op = {
};

int main( int argc, char *argv[] )
{
	return fuse_main( argc, argv, &practicefs_op, NULL );
}