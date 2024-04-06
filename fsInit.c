/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: fsInit.c
*
* Description:: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	typedef struct vcb{
    uint64_t block_size;		//Total Number of blocks
	uint64_t block_index;		//number of free blocks in volume
    
	uint64_t free_block_size;	//Total number of free blocks(bitmap length)
	uint64_t free_block_index;	//location of the free space in bitmap
	
	uint64_t root_dir_size;		//Total number  of the root directory 
	uint64_t root_dir_index;	//Location of the root directory 
	
	uint64_t signature;			//Long type unique identify (8bytes) generate by magic number 
	} vcb;

	char * freespace;
	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}