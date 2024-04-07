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
	int startBlock = 1;
	int bytesNeeded = numberOfBlocks / 8; //8 bits(1 bytes) 8 blocks 
	int blocksNeeded = (bytesNeeded + (BLOCK_SIZE - 1)) / BLOCK_SIZE;

	//**************************VCB**************************//


    // Check if the signature matches
	if (vcb.signature == PART_SIGNATURE) {
        printf("Volume already initialized.\n");
		return 0; // Volume already initialized, return success
	}
	
   
	//initialize value in Volume control block 
	vcb.signature = PART_SIGNATURE;
	vcb.block_size = numberOfBlocks;

	//**************************FreeSpace**************************//
	//Malloc a Block of memory as your VCB pointer  

	
	//**************************Root**************************//




	//**************************Write to Disk**************************//
	LBAwrite(&vcb, 1, 0);
	if (LBAread(&vcb, 1, 0) != 1) {
        printf("Failed to read first block.\n");
        return -1;
    }
	
	return 0;
	}
	
	
void exitFileSystem ()
	{



	printf ("System exiting\n");
	}

