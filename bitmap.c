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
* Description:: This is a free space system that contains functions 
* for bitmap which tracks,set,clear,allocation,release of blocks.
*
**************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fsInit.h"
#include "fsLow.h"
#include "bitmap.h"
#include "global.h"
#define DEFAULT_AMOUNT 29

// Free range of blocks in free space map(bitmap)
void releaseBlock(uint64_t startBlock, uint64_t block_amount) {
    // Check if startBlock is within the valid range(amount of file system)
    if (startBlock < 0 || startBlock >= VCB->block_size) {
         // If it out of bounds, return without performing any action
        return;
    }
    //Loop through the range number of blocks and free
    for (int i = 0; i < block_amount; i++) {
        clear_bit(fsmap, startBlock + i);
    }
    //Update map on disk 
    LBAwrite(fsmap, VCB->bit_map_size, VCB->bit_map_index);
}

////allocate free space with the minimum and maximum(block size) limit  
//encapsulate the functionality for other freespace system  
struct extent* allocateSpace(uint64_t block_amount, uint64_t min_block_count) {
    // Start from the root directory after vcb and bitmap
    int rootDirLocation = VCB->bit_map_index + VCB->bit_map_size; 
    int startBlock = -1; // no valid starting block has been found yet.
    int extentsNeeded = 0;
    int blocksRemaining = block_amount;

    // Determine chunk size based on provided minimum block count
    int chunkSize = min_block_count < blocksRemaining ? min_block_count : blocksRemaining;
    struct extent tempExtents[DEFAULT_AMOUNT];

    // Loop until all blocks are allocated or there's not enough space
    while (blocksRemaining > 0) {
        // Flag to break out of the loop if no suitable space found
        int forceBreak = 1;

        // Iterate through blocks to find free space
        for (int i = rootDirLocation; i < VCB->block_size; i++) {
            if (get_bit(fsmap, i) == 0) {
                int tempStartBlock = i;
                int countedBlocks = 1;

                // Check consecutive blocks for availability
                while (countedBlocks < blocksRemaining && get_bit(fsmap, ++i) == 0) {
                    countedBlocks++;
                }

                // If enough consecutive blocks found, allocate space
                if (countedBlocks >= chunkSize) {
                    forceBreak = 0;
                    startBlock = tempStartBlock;// valid starting block has been found.
                    blocksRemaining -= countedBlocks;

                    // Mark allocated blocks as used in the free space map
                    for (int b = 0; b < countedBlocks; b++) {
                        set_bit(fsmap, startBlock + b);
                    }

                    // Store extent information
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
    if (blocksRemaining > 0 || extentsNeeded == 0 || extentsNeeded > DEFAULT_AMOUNT) {
        printf("Failed to allocate space from volume!\n");
        return NULL;
    }

    // Allocate memory for extents
    struct extent* extents = malloc(extentsNeeded * sizeof(struct extent));
    if (!extents) {
        return NULL;
    }

    // Copy extent information to allocated memory
    for (int i = 0; i < extentsNeeded; i++) {
        extents[i].start = tempExtents[i].start;
        extents[i].count = tempExtents[i].count;
    }

    // Update the free space map on disk
    LBAwrite(fsmap, VCB->bit_map_size, VCB->bit_map_index);
    
    // Update the free block that is used in VCB
    VCB->free_block_index += min_block_count;

    return extents;
}

int initFreeSpace(uint64_t block_amount) {
	int bytesNeeded = block_amount / 8; //1 bit per block (smallest addresable Byte)
	int bitmap_needed_block = (bytesNeeded + (MINBLOCKSIZE - 1)) / MINBLOCKSIZE; // floor operation 
	// printf("\n checking : %d \n",bitmap_needed_block); //5 

    fsmap = malloc(bitmap_needed_block * MINBLOCKSIZE); //bitmap space (5*512=2560 bytes)
	if (!fsmap) { 
    	printf("fail to malloc free space map");
		free(fsmap);
		fsmap=NULL;
		return -1; 
    }
	// Initialize free space to all zeros as free in bitmap
    memset(fsmap, 0, bitmap_needed_block * MINBLOCKSIZE);
    
    if(get_bit(fsmap,0)==0){ //check if it free
        set_bit(fsmap,0); //set block 0 as used for vcb
        VCB->bit_map_index=1; ///VCB take up block 0,thus start it at index 1
    }

	//iterate and set used from vcb(block 0) to needed block for bitmap  
    for (int i = 1; i <= bitmap_needed_block; i++) {
        if (get_bit(fsmap, i) == 0) { //check if it free
            set_bit(fsmap, i); // Set the bit to indicate it's used
            VCB->free_block_index = i; // Update the free block index in VCB
        }else{
            printf("Block %d is already in use.\n", i);
        }
    }

	//inital vcb 
	VCB->bit_map_size=bitmap_needed_block;
    VCB->free_block_index = VCB->bit_map_index;
	if (VCB->bit_map_size == -1) {
        printf("Failed to find a free block.\n");
        free(fsmap);
        fsmap=NULL;
        return -1;
    }
    // write 5 blocks starting from block 1 
    // printf("fsmap write %ld blocks in index %ld\n",VCB->bit_map_size,VCB->bit_map_index);
    LBAwrite(fsmap, VCB->bit_map_size, VCB->bit_map_index); 

	// Return number of the free space to the VCB init that called
	return VCB->bit_map_index;
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