/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: bitmap.h
*
* Description:: This is a file that implement bitmap 
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
struct extent* allocateSpace(uint64_t numberOfBlocks, uint64_t min_block_count);

int initFreeSpace(uint64_t numberOfBlocks);

int trackAndSetBit(char* fsmap, int numberOfBlocks);

// Set the bit at a specific position in the bitmap
void set_bit(char* fsmap, int block_number);

// Clear the bit at a specific position in the bitmap
void clear_bit(char* fsmap, int block_number);

// Get the bit at a specific position in the bitmap
int get_bit(char* fsmap, int block_number);

#endif

