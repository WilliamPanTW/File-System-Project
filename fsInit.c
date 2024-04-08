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



int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	//**************************VCB**************************//


    // Check if the signature matches
	if (vcb.signature == PART_SIGNATURE) {
        printf("Volume already initialized.\n");
		return 0; // Volume already initialized, return success
	}
	
	//initialize value in Volume control block 
	vcb.signature = PART_SIGNATURE; 
	vcb.block_index = numberOfBlocks; //19531
	vcb.block_size = numberOfBlocks*MINBLOCKSIZE; // capacity or size of the storage 9,999,872 byte
	//**************************FreeSpace**************************//

	if (initFreeSpace(numberOfBlocks) == -1) {

        return -1; //fail to inital free space 
    }
	

	//**************************Root**************************//


	if (initRootDir(MIN_DE) == -1) {
        return -1; //fail to inital Root directory  
    }


	//**************************Write to Disk**************************//
	// write 1 blocks starting from block 0
	LBAwrite(&vcb, 1, 0);
	if (LBAread(&vcb, 1, 0) != 1) {
        printf("Failed to read first block.\n");
        return -1;
    }
	// int bitmap_state ;
	// for(int i=0;i<=36;i++){
	// 	bitmap_state = get_bit(fsmap, i);
	// 	printf("bitmap index %d is %d\n",i,bitmap_state); //free as 0 and used as 1 
	// } 

	return 0;
	}
	
	
void exitFileSystem ()
	{

	//Free pinter prevent memory leak  
    free(fsmap); //bitmap
    fsmap = NULL;
	
	free(rootDir); // root pointer 
    rootDir = NULL;

	printf ("System exiting\n");
	}

	//**************************Helper function**************************//
int initFreeSpace(uint64_t numberOfBlocks) {

	int startBlock = 1; //VCB take up block 0,thus start it at index 1
	int bytesNeeded = numberOfBlocks / 8; //1 bit per block (smallest addresable Byte)
	int bitmap_needed_block = (bytesNeeded + (MINBLOCKSIZE - 1)) / MINBLOCKSIZE; // floor operation 
	// printf("\ncechking : %d \n",bitmap_needed_block); //5 

    fsmap = malloc(bitmap_needed_block * MINBLOCKSIZE); //bitmap space (5*512=2560 bytes)
	if (!fsmap) { 
    	printf("fail to malloc free space map");
		return -1; 
    }
	// Initialize free space bitmap to all zeros
    memset(fsmap, 0, bitmap_needed_block * MINBLOCKSIZE);

	set_bit(fsmap, 0);//set block 0 in bitmap(fsmap) to 1(used)
	//iterate to set the needed block for bitmap to allocate free space 
    for (int i = 1; i <= bitmap_needed_block; i++) {
        set_bit(fsmap, i); 
    }

    // write 5 blocks starting from block 1
    LBAwrite(fsmap, bitmap_needed_block, startBlock);

	//inital vcb 
    vcb.free_block_index = startBlock;
	vcb.free_block_size = bitmap_needed_block;

	printf("VCB return%d\n",startBlock);
	// Return number of the free space to the VCB init that called
	return startBlock;
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
	vcb.root_dir_size = block_num; // 0x1d 29 in block 1 (VCB)
	// printf("\ndirEntryAmount: %d",dirEntryAmount);
	// printf("\nnew update byte of dir: %d",dirEntry_bytes);


	int startBlock = 6; //VCB and bitmap take up 0-5 blocks ,thus start it at index 6
	vcb.root_dir_index = startBlock; 
	
	// pointer to an array of directory entries
	struct dirEntry* dir = malloc(block_byte); //ask free space system for 6 blocks
    if (!dir) {
        printf("Directory failed to allocate memory.\n");
        return -1;
    }

    // Initialize directory entries
    for (int i = 0; i < dirEntryAmount; i++) {
        dir[i].fileName[0] = '\0';
    }
	// Actual directory entry of root 
 	rootDir = malloc(dirEntry_bytes);
    if (!rootDir) {
        return -1;
    }

   	// Get current time
    time_t current_time;
    time(&current_time);
	// printf("current time is %ld second \n",current_time);
    dir[0].createDate = current_time;
    dir[0].modifyDate = current_time;
	dir[0].dirSize = dirEntryAmount; 

	// durectiry entry zero , cd dot indicate current directory 
	strcpy(dir[0].fileName, ".");
	dir[0].isDirectory = 1; //Root is a directory


	//Root Directory entry one, cd dot dot should point itself
	strcpy(dir[1].fileName, "..");
	dir[1].isDirectory = 1; //Root is a directory

	// printf("init root dir\n");
	// printf("write %d blocks\n",block_num);
	// printf("starting from %d\n",startBlock);

    // Write ROOT directory in number of block starting from index 
    LBAwrite(dir, block_num, startBlock);
	//Mark the block that is used 
	for (int i = 0; i <= block_num; i++) {
        set_bit(fsmap, (startBlock+i)); //from where it start with block is allocated 
    }

	free(dir);
    return 0;
}


// Set the bit at a specific position in the bitmap
void set_bit(char* fsmap, int block_number) {
    int byte_number= block_number / 8; // get the specific block byte(8 bits) index
    int bit_number = block_number % 8; // remainder of the block number
    fsmap[byte_number] |= (1 << bit_number); //set according to 1 as used
	//Initial at block 0 fsmap will get 0x0001 and at block 3 fsmap 0x1111
}


// Clear the bit at a specific position in the bitmap
void clear_bit(char* fsmap, int block_number) {
     int byte_number= block_number / 8; // get the specific block byte(8 bits) index
    int bit_number = block_number % 8; // remainder of the block number
    int result = fsmap[byte_number] &= ~(1 << bit_number); 	//more efficent using bit operator 
    //for example 0x1111 1111 by clear position 4 it bcome 0x1110 1111 
}

// Get the bit at a specific position in the bitmap
int get_bit(char* fsmap, int block_number) {
    int byte_number= block_number / 8; // get the specific block byte(8 bits) index
    int bit_number = block_number % 8; // remainder of the block number
	return (fsmap[byte_number] >> bit_number) & 1;//retrieved used(1) or free(0) 
}



