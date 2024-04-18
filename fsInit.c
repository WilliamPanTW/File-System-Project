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
#include <time.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsInit.h"
#include "bitmap.c"

#define vcbSIG 0x7760602795671593 //magic number signature for VCB

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	//**************************VCB**************************//

	if (initVolumeControlBlock(numberOfBlocks) == -1) {
        return -1; //fail to init Volume Control Block
    }

	//**************************FreeSpace**************************//

	if (initFreeSpace(numberOfBlocks) == -1) {
        return -1; //fail to inital free space 
    }
	

	//**************************Root**************************//


	if (initRootDir(MIN_DE) == -1) {
        return -1; //fail to inital Root directory  
    }


	//**************************Check**************************//
	// int bitmap_status ;
	// for(int i=0;i<=36;i++){
	// 	bitmap_status = get_bit(fsmap, i);
	// 	printf("bitmap index %d is %d\n",i,bitmap_status); //free as 0 and used as 1 
	// } 
	// write 1 blocks starting from block 0 after All initialize 
	if (LBAwrite(VCB, 1, 0) != 1) {
        printf("Failed to write Volume contorl block.\n");
        return -1;
    }
	return 0;
	}
	
	
void exitFileSystem ()
	{
	//Free pinter prevent memory leak  
    free(fsmap); //bitmap
    fsmap = NULL;

	free(VCB);
    VCB = NULL;

	printf ("System exiting\n");
	}

	//**************************Helper function**************************//
int initVolumeControlBlock(uint64_t numberOfBlocks){
	VCB= malloc(MINBLOCKSIZE * sizeof(struct vcb));
    // Check if the signature matches
	if (LBAread(VCB, 1, 0) != 1) {
		printf("Failed to read first block.\n");
		free(VCB);
    	VCB = NULL;
		return -1;
	}

	if (VCB->signature == vcbSIG) {
        printf("Volume already initialized.\n");
		return 0; // Volume already initialized, return success
	}

	//initialize value in Volume control block 
	VCB->signature = vcbSIG; 
	VCB->block_index = 0; //location of the VCB
	VCB->block_size = numberOfBlocks; //amount of block size
}


int initRootDir(uint64_t entries_number) {
	int dirEntrySize = sizeof(struct dirEntry); // directory entry size (ex.60 bytes)
    int dirEntry_bytes =  dirEntrySize * entries_number ; // byte needed for Root directory (ex.3000 bytes)
    int block_num = (dirEntry_bytes + (MINBLOCKSIZE - 1)) / MINBLOCKSIZE; //floor operator (ex.6 blocks)
	int block_byte  = block_num * MINBLOCKSIZE; // The actual size we can allocated by block (ex.3072 bytes)
	// printf("\n block byte : %d",block_byte);
	// printf("\ndir entry size: %d",dirEntrySize);
	int dirEntryAmount = block_byte / dirEntrySize; // result in less waste  (ex.3072/60 = 51 entries)
	dirEntry_bytes = dirEntrySize * dirEntryAmount; // update the actual byte dirtory could allocate  (ex.60*51= 3060)

	//encapsulate the functionality for other freespace system  
	int min_block_count=block_num;
	//allocate free space with the minimum and maximum(block size) limit  
    struct extent* extents = allocateSpace(block_num, min_block_count);
	if (extents == NULL) {
		// int bitmap_status ;
		// for(int i=0;i<=36;i++){
		// 	bitmap_status = get_bit(fsmap, i);
		// 	printf("bitmap index %d is %d\n",i,bitmap_status); //free as 0 and used as 1 
		// } 
		printf("There is not enough space to allocate, please check butmap status");
	    return -1;
	}

	VCB->root_dir_index = extents->start; //set root index only when inital 
	VCB->root_dir_size = extents->count; // amount of blocks of Root Dir 


	// pointer to an array of directory entries
	struct dirEntry *dirEntries = malloc(dirEntry_bytes);
    if (!dirEntries) {
        printf("Directory failed to allocate memory.\n");
        return -1;
    }

    // Initialize Root directory entries
	for (int i = 0; i < dirEntryAmount; i++) {
		dirEntries[i].fileName[0] = '\0';// Null terminated  
		dirEntries[i].isDirectory = 0;
	}

	//Directory entry zero, cd dot should point current
	set_Dir(dirEntries,0,".",dirEntryAmount);


	//Root Directory entry one, cd dot dot should point itself
	set_Dir(dirEntries,1,"..",dirEntryAmount);

    // Write ROOT directory in number of block starting after bitmap block
    LBAwrite(dirEntries, VCB->root_dir_size, VCB->root_dir_index);
	free(dirEntries);
	dirEntries= NULL;

    return 0;
}

void set_Dir(struct dirEntry *dirEntries , int index,char *name,int dirEntryAmount){
	time_t current_time;
	time(&current_time);
	strcpy(dirEntries[index].fileName, name);
	// printf("what is my %d root index? %ld \n",index,VCB->root_dir_index);
	dirEntries[index].location = VCB->root_dir_index;
	dirEntries[index].isDirectory = 1; //Root is a directory
	dirEntries[index].dirSize = dirEntryAmount;
	dirEntries[index].createDate = current_time;
	dirEntries[index].modifyDate = current_time;
}



