/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: bitmap.c
*
* Description:: This is a file that implement bitmap 
*
**************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "fsInit.h"
#include "fsLow.h"
#include "bitmap.h"
#define MAX_EXTENT_AMOUNT 29

////allocate free space with the minimum and maximum(block size) limit  
//encapsulate the functionality for other freespace system  
struct extent* allocateSpace(uint64_t numberOfBlocks, uint64_t min_block_count) {
    //Start from the root directory
    int rootDirLocation = VCB->root_dir_index ;
    int startBlock = -1;

    int extentsNeeded = 0;
    int blocksRemaining = numberOfBlocks;

    // Determine chunk size based on provided minimun block count
    int chunkSize = min_block_count;
    if (chunkSize > blocksRemaining) {
        chunkSize = blocksRemaining;
    }

    //Temporary storage for extents
    struct extent tempExtents[20];
    // Loop until all blocks are allocated or there's not enough space
    while (blocksRemaining > 0) {
        //Flag to break out of the loop if no suitable space found
        int forceBreak = 1;

        //Iterate through blocks to find free space
        for (int i = rootDirLocation; i < VCB->block_size; i++) {
            int x = get_bit(fsmap, i);
            if (x == 0) {
                int tempStartBlock = i;

                int remainingChunkSize = chunkSize - 1;
                int countedBlocks = 1;

                // Check consecutive blocks for availability
                while (countedBlocks < blocksRemaining) {
                    i++;
                    x = get_bit(fsmap, i);
                    if (x == 1) {
                        break;
                    }
                    
                    countedBlocks++;
                }

                //If enough consecutive blocks found, allocate space
                if (countedBlocks >= chunkSize) {
                    forceBreak = 0;

                    startBlock = tempStartBlock;
                    blocksRemaining -= countedBlocks;

                    //Set allocated blocks as used in the free space map
                    for (int b = 0; b < countedBlocks; b++) {
                        set_bit(fsmap, startBlock + b);
                    }

                    tempExtents[extentsNeeded].start = tempStartBlock;
                    tempExtents[extentsNeeded].count = countedBlocks;
                    extentsNeeded++;
                    break;
                }
            }
        }

        // If loop didn't find enough space, break out
        if (forceBreak) {
            break;
        }

    }
    // Check if allocation failed
    if (blocksRemaining > 0 || extentsNeeded == 0) {
        printf("Failed to allocate space from volume!\n");
        return NULL;
    }
    //Check if too many extents needed
    if (extentsNeeded > MAX_EXTENT_AMOUNT) {
        return NULL;
    }

    // Allocate memory for extents
    struct extent* extents = malloc(extentsNeeded * sizeof(struct extent));
    if (!extents) {
        return NULL;
    }

    // Update the free space map on disk
    LBAwrite(fsmap, VCB->bit_map_size, VCB->free_block_index);

    // Copy extent information to allocated memory
    for (int i = 0; i < extentsNeeded; i++) {
        extents[i].start = tempExtents[i].start;
        extents[i].count = tempExtents[i].count;
    }

    // int bitmap_status ;
	// for(int i=0;i<=36;i++){
	// 	bitmap_status = get_bit(fsmap, i);
	// 	printf("bitmap index %d is %d\n",i,bitmap_status); //free as 0 and used as 1 
	// } 

    return extents;
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
    VCB->bit_map_index=1; ///VCB take up block 0,thus start it at index 1
	VCB->free_block_index = VCB->bit_map_index;
	VCB->bit_map_size=bitmap_needed_block;
    trackAndSetBit(fsmap, numberOfBlocks); //update free space index
	if (VCB->bit_map_size == -1) {
        printf("Failed to find a free block.\n");
        return -1;
    }

    // write 5 blocks starting from block 1 
	// printf("Fsmap write %d blocks in position %d\n",bitmap_needed_block,BITMAP_POSITION);
    LBAwrite(fsmap, bitmap_needed_block, VCB->bit_map_index); 

	// printf("VCB return%d\n",startBlock);
	// Return number of the free space to the VCB init that called
	return VCB->bit_map_index;
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