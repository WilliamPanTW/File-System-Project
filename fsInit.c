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

char * fsmap; //global unsign char fsmap pointer 


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


	//check empty 
	int result = get_bit(fsmap, 5);
	printf("myresult%d\n",result); //free as 0 and used as 1 

	
	//**************************Root**************************//




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
	int bitmap_needed_block = (bytesNeeded + (BLOCK_SIZE - 1)) / BLOCK_SIZE; // floor operation 
	// printf("\ncechking : %d \n",bitmap_needed_block); //5 

    fsmap = malloc(bitmap_needed_block * BLOCK_SIZE); // malloc the space needed ( 2560 bytes)

	set_bit(fsmap, 0);
    for (int i = 1; i <= bitmap_needed_block; i++) {
        set_bit(fsmap, i);
    }


    // write 5 blocks starting from block 1
    LBAread(fsmap, vcb.free_block_size, vcb.free_block_index);

	//Assign location and size to Volume Control Block
    vcb.free_block_size = bitmap_needed_block;
 	vcb.free_block_index = startBlock;


	return startBlock;
}


// Set the bit at a specific position in the bitmap
void set_bit(char* fsmap, int block_number) {
	printf("block:%d\n",block_number);
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
	return (fsmap[byte_number] >> bit_number) & 1;//indicate on zero right shift
}



