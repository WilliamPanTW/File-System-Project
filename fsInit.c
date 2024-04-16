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
#define vcbSIG 0x7760602795671593

void set_Dir(struct dirEntry *dirEntries , int index,char *name,int dirEntryAmount);
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

int initFreeSpace(uint64_t numberOfBlocks) {
	int bytesNeeded = numberOfBlocks / 8; //1 bit per block (smallest addresable Byte)
	int bitmap_needed_block = (bytesNeeded + (MINBLOCKSIZE - 1)) / MINBLOCKSIZE; // floor operation 
	// printf("\ncechking : %d \n",bitmap_needed_block); //5 

    fsmap = malloc(bitmap_needed_block * MINBLOCKSIZE); //bitmap space (5*512=2560 bytes)
	if (!fsmap) { 
    	printf("fail to malloc free space map");
		free(fsmap);
		fsmap=NULL;
		return -1; 
    }
	// Initialize free space to all zeros as free in bitmap
    memset(fsmap, 0, bitmap_needed_block * MINBLOCKSIZE);

	set_bit(fsmap, 0);//set block 0 in bitmap(fsmap) to 1(used) for VCB
	//iterate to set the needed block for bitmap to allocate free space 
    for (int i = 1; i <= bitmap_needed_block; i++) {
        set_bit(fsmap, i); 
    }

	//inital vcb 
	VCB->free_block_index = BITMAP_POSITION;
	VCB->free_block_size=bitmap_needed_block;
    trackAndSetBit(fsmap, numberOfBlocks); //update free space index
	if (VCB->free_block_size == -1) {
        printf("Failed to find a free block.\n");
        return -1;
    }

    // write 5 blocks starting from block 1 
	// printf("Fsmap write %d blocks in position %d\n",bitmap_needed_block,BITMAP_POSITION);
    LBAwrite(fsmap, bitmap_needed_block, BITMAP_POSITION); 

	// printf("VCB return%d\n",startBlock);
	// Return number of the free space to the VCB init that called
	return BITMAP_POSITION;
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
	VCB->root_dir_size = block_num; // amount of blocks of Root Dir 
	VCB->root_dir_index = VCB->free_block_index; //set root index only when inital 
	// printf("\ndirEntryAmount: %d",dirEntryAmount);
	// printf("\nnew update byte of dir: %d",dirEntry_bytes);

	// Set the bits for the blocks allocated for the root directory
    for (int i = 0; i < block_num; i++) {
        set_bit(fsmap, (VCB->free_block_index + i));
    }
	LBAwrite(fsmap, 6, 1); // Big error affecting block 7
    // Update the free block index in VCB
    VCB->free_block_index += block_num;	// Update block that used. 

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

int trackAndSetBit(char* fsmap, int numberOfBlocks) {
    for (int i = 0; i < numberOfBlocks; i++) {
        if (!get_bit(fsmap, i)) { // If the block is free
            set_bit(fsmap, i); // Set the bit to indicate it's used
            VCB->free_block_index = i; // Update the free block index in VCB
            return i; // Return the index which is updated
        }
    }
    return -1; // If no free block is found
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


