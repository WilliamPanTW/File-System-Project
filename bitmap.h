/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: John Cuevas, Michael Abolencia , Pan William , Tina Chou
* Student IDs:: 920542932, 917581956, 922867228 , 922911207
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: bitmap.h
*
* Description:: This is a free space system that contains functions 
* for bitmap which tracks,set,clear,allocation,release of blocks.
*
**************************************************************/
#ifndef _BITMAP_H
#define _BITMAP_H

#include <stdio.h>
#include <stdint.h>
#define BITMAP_POSITION 1 ///VCB take up block 0,thus start it at index 1

struct extent {
    int start;
    int count;
};

//allocate free space with the minimum and maximum(block size) limit  
//encapsulate the functionality for other freespace system  
struct extent* allocateSpace(uint64_t block_amount, uint64_t min_block_count);

// Free range of blocks in free space map(bitmap)
void releaseBlock(uint64_t startBlock, uint64_t block_amount);

int initFreeSpace(uint64_t block_amount);

// Set the bit at a specific position in the bitmap
void set_bit(char* fsmap, int block_number);

// Clear the bit at a specific position in the bitmap
void clear_bit(char* fsmap, int block_number);

// Get the bit at a specific position in the bitmap
int get_bit(char* fsmap, int block_number);

#endif

