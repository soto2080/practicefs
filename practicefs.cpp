/** @file
 * practicefs.cpp
 * A trial to mimic the legend EXT2 filesystem architecture in cpp
 * Copyright (C) 2020  Ming-Han Yang <mhy@mumi.rip>
*/

#include "practicefs.h"
#include <iostream>


static struct fuse_operations practicefs_op = {
};

int main( int argc, char *argv[] )
{
	return fuse_main( argc, argv, &practicefs_op, NULL );
}