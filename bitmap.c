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

#include "fsLow.h"
#include "bitmap.h"

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