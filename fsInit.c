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
	vcb.block_size = numberOfBlocks; //19531

	//**************************FreeSpace**************************//

	if (initFreeSpace(numberOfBlocks) == -1) {
        return -1; //fail to inital free space 
    }


	// //check empty 
	// int result = get_bit(fsmap, 5);
	// printf("myresult%d\n",result); //free as 0 and used as 1 


	//**************************Root**************************//


	if (initRootDir(MIN_DE) == -1) {
        return -1; //fail to inital Root directory  
    }

	// Write the root directory to disk
    if (writeRootDirectory() == -1) {
        return -1; // Failed to write root directory
    }


	//**************************Write to Disk**************************//
	// write 1 blocks starting from block 0
	LBAwrite(&vcb, 1, 0);
	if (LBAread(&vcb, 1, 0) != 1) {
        printf("Failed to read first block.\n");
        return -1;
    }
	
	return 0;
	}
	
	
void exitFileSystem ()
	{

	//Free bitmap
    free(fsmap);
    fsmap = NULL;

	printf ("System exiting\n");
	}

	//**************************Helper function**************************//
int initFreeSpace(uint64_t numberOfBlocks) {

	int startBlock = 1; //VCB take up block 0 we start it at index 1
	int bytesNeeded = numberOfBlocks / 8; //1 bit per block (smallest addresable Byte)
	int bitmap_needed_block = (bytesNeeded + (MINBLOCKSIZE - 1)) / MINBLOCKSIZE; // floor operation 
	// printf("\ncechking : %d \n",bitmap_needed_block); //5 

    fsmap = malloc(bitmap_needed_block * MINBLOCKSIZE); //bitmap space (512*5=2560 bytes)
	if (!fsmap) { 
    	printf("fail to malloc free space map");
		return -1; 
    }

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

	// Return number of the free space to the VCB init that called
	return startBlock;
}

int initRootDir(uint64_t entries_number) {
	int dirEntrySize = sizeof(struct dirEntry); // directory entry size
    int bytesNeeded =  dirEntrySize * entries_number ; // byte needed for Root directory
    int blocksNeeded = (bytesNeeded + (MINBLOCKSIZE - 1)) / MINBLOCKSIZE; //floor operator
	bytesNeeded = blocksNeeded * MINBLOCKSIZE; //update the actual size we allocated
	int dirEntryAmount = bytesNeeded / dirEntrySize; // result in less waste  
	vcb.root_dir_size = blocksNeeded;
	vcb.root_dir_index = 6; //??


	
	// pointer to an array of directory entries
	struct dirEntry* dir = malloc(bytesNeeded);
    if (!dir) {
        printf("Directory failed to allocate memory.\n");
        return -1;
    }

    // Initialize directory entries
    for (int i = 0; i < dirEntryAmount; i++) {
        dir[i].fileName[0] = '\0';
    }


    //If everything's successful, read in the necessary data
    LBAread(rootDir, vcb.root_dir_size , vcb.root_dir_index);
    
    return 0;
}

int writeRootDirectory() {
	//Allocate enough memory before proceeding
    rootDir = malloc(vcb.root_dir_size * MINBLOCKSIZE);
    if (!rootDir) {
        return -1;
    }

    // Write the root directory to disk
    return LBAwrite(rootDir, vcb.root_dir_size, vcb.root_dir_index);
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



