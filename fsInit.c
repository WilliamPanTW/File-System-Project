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

char * freeSpace; //global bitmap


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */




	//Malloc a Block of memory as your VCB pointer and LBAread block 0
	freeSpace = malloc(vcb.block_size * BLOCK_SIZE);

	//initialize value in vcb 
	vcb.signature = PART_SIGNATURE;
	vcb.block_size = numberOfBlocks;
	// LBAwrite the VCB to block 0
	LBAwrite(&vcb, 1, 0);



	void set_bit(char* bitmap, int position);
	int get_bit(char* bitmap, int position);
	void clear_bit(char* bitmap, int position);


	int bytesNeeded = numberOfBlocks / 8; //8 bits(1 bytes) 8 blocks 

	return 0;
	}
	
	
void exitFileSystem ()
	{

	//Free bitmap
    free(freeSpace);
    freeSpace = NULL;


	printf ("System exiting\n");
	}

