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

//Hexdump/hexdump.linux SampleVolume --start 1 --count 2
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsInit.h"
#include "bitmap.h"
#include "global.h"

#define vcbSIG 0x7760602795671593 //magic number signature for VCB

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
	
	// int bitmap_status ;
	// for(int i=0;i<=VCB->bit_map_size+1;i++){
	// 	bitmap_status = get_bit(fsmap, i);
	// 	printf("bitmap index %d is %d\n",i,bitmap_status); //free as 0 and used as 1 
	// } 

	//**************************Root**************************//


	if (createDirectory(MIN_DE,NULL) == -1) {
        return -1; //fail to inital Root directory  
    }

	//**************************Check**************************//
	// int bitmap_status ;
	// for(int i=0;i<=VCB->root_dir_index+VCB->root_dir_size;i++){
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

	free(loadedRoot);
	loadedRoot=NULL;
	free(loadedCWD);
	loadedCWD=NULL;

    free(fsmap); //bitmap
    fsmap = NULL;

	free(VCB); //volume control block 
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

int createDirectory(uint64_t entries_number, struct pp_return_struct* ppinfo) {
	int dirEntrySize = sizeof(struct dirEntry); // directory entry size (ex.60 bytes)
    int dirEntry_bytes =  dirEntrySize * entries_number ; // byte needed for Root directory (ex.3000 bytes)
    int block_num = (dirEntry_bytes + (MINBLOCKSIZE - 1)) / MINBLOCKSIZE; //floor operator (ex.6 blocks)
	int block_byte  = block_num * MINBLOCKSIZE; // The actual size we can allocated by block (ex.3072 bytes)
	int dirEntryAmount = block_byte / dirEntrySize; // result in less waste  (ex.3072/60 = 51 entries)
	dirEntry_bytes = dirEntrySize * dirEntryAmount; // update the actual byte dirtory could allocate  (ex.60*51= 3060)
	
	//encapsulate the functionality for other freespace system  
	int min_block_count=block_num;
	//allocate free space with the minimum and maximum(block size) limit  
    struct extent *location = allocateSpace(block_num, min_block_count);
	if (location == NULL) {
		printf("There is not enough space to allocate, please check butmap status");
	    return -1;
	}

	// if location of start return negative or exceed limit
    // if (location->start == -1 || location->count != dirEntryAmount) {
    //     free(location);
    //     return -1; // return error 
    // }

	// Create a pointer to an array of directory entries
	struct dirEntry *dirEntries = malloc(dirEntry_bytes);
    if (!dirEntries) {
        printf("Directory failed to allocate dicttory memory.\n");
        return -1;
    }

    // Initialize Root directory entries
	for (int i = 0; i < dirEntryAmount; i++) {
		dirEntries[i].fileName[0] = '\0';// Null terminated  
		// dirEntries[i].isDirectory = 0;
	}

	
	//Directory entry zero, cd dot should point current
	set_Dir(".",0,dirEntryAmount,dirEntries,location);

	//buffer of parent(ex.cwd root) 
    struct dirEntry* parent = &dirEntries[0];
	//If parent is provided include root directory
    if (ppinfo != NULL && ppinfo->parent != NULL) {
		parent = ppinfo->parent;
		// parepath return avaiable free space index
		int index =ppinfo->lastElementIndex;
		//copy the name of new directory to parent(ex.root entries) of new directory 
		strcpy(parent[index].fileName, ppinfo->lastElementName);

		parent[index].isDirectory = dirEntries[0].isDirectory;//set as direcotry

		parent[index].dir_index = location->start;//get the new free spce allocate   
		parent[index].dir_size = location->count; //amount of free space allocate
		
		parent[index].entry_amount = dirEntries[0].entry_amount;//set amount of entries

		parent[index].createDate = dirEntries[0].createDate;//set time for new dir
		parent[index].modifyDate = dirEntries[0].modifyDate;//set time for new dir
		//Update the root dictory back to disk 
		LBAwrite(parent, block_num, VCB->root_dir_index);
	}
	
	strcpy(dirEntries[1].fileName, "..");
	dirEntries[1].entry_amount = parent->entry_amount;
    dirEntries[1].dir_index = parent->dir_index;
    dirEntries[1].dir_size = parent->dir_size;
    dirEntries[1].createDate = parent->createDate;
    dirEntries[1].modifyDate = parent->modifyDate;
    dirEntries[1].isDirectory = parent->isDirectory;
	
	printf("Created directory using %d blocks starting at block %d\n", block_num, location->start);
    // Write amount of block from index get by allocateSpace to directory  entries
    LBAwrite(dirEntries,location->count, location->start);

	//free directory if not root, keep track of the root and current directories
    if (ppinfo && ppinfo->parent) {
        free(dirEntries);
    } else {
       	loadedRoot = dirEntries;
    	loadedCWD = loadedRoot;   
    }
	//Assign length to Volume Control Block
	VCB->root_dir_index = location->start; //set root index only when inital 
	VCB->root_dir_size = location->count; // amount of blocks of Root Dir 

	free(location);
	// int bitmap_status ;
	// for(int i=0;i<=64;i++){
	// 	bitmap_status = get_bit(fsmap, i);
	// 	printf("bitmap index %d is %d\n",i,bitmap_status); //free as 0 and used as 1 
	// } 

    return location->start;
}

void set_Dir(
	char *name,
	int index,
	int dirEntryAmount,
	struct dirEntry *dirEntries, 
	struct extent *location
	){	
	// Get current time
	time_t current_time;
	time(&current_time);

	// Copy name to directory entry
	strcpy(dirEntries[index].fileName, name);
	// Set directory flag
	dirEntries[index].isDirectory = 1; 

    // Set directory index and size
    dirEntries[index].dir_index = location->start;
    dirEntries[index].dir_size = location->count;

    // Set entry amount
    dirEntries[index].entry_amount = dirEntryAmount;

    // Set creation and modification dates
    dirEntries[index].createDate = current_time;
    dirEntries[index].modifyDate = current_time;
}



