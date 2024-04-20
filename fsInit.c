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

	free(rootDir);
	rootDir=NULL;
	free(cwDir);
	cwDir=NULL;

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

	//buffer of parent 
    struct dirEntry* parent = &dirEntries[0];
	//If parent is provided include root directory
    if (ppinfo != NULL && ppinfo->parent != NULL) {
		parent = ppinfo->parent;

		//Copy directory entry to its parent's free entry slot
		strcpy(parent[ppinfo->lastElementIndex].fileName, ppinfo->lastElementName);
		parent[ppinfo->lastElementIndex].isDirectory = dirEntries[0].isDirectory;

		parent[ppinfo->lastElementIndex].dir_index = location->start;  
		parent[ppinfo->lastElementIndex].dir_size = location->count;
		
		parent[ppinfo->lastElementIndex].entry_amount = dirEntries[0].entry_amount;

		parent[ppinfo->lastElementIndex].createDate = dirEntries[0].createDate;
		parent[ppinfo->lastElementIndex].modifyDate = dirEntries[0].modifyDate;
		printf("Your parent address: %p \n", (void *)ppinfo->parent);
		//Write to the volume starting where the parent starts
		LBAwrite(parent, block_num, parent[0].dir_index);
	}
	
	//Root Directory entry one, cd dot dot should point itself
	set_Dir("..",1,dirEntryAmount,dirEntries,location);

    // Write ROOT directory in number of block starting after bitmap block
	// printf("write %ld blocks of direntries from index %ld\n",VCB->root_dir_size, VCB->root_dir_index);
    LBAwrite(dirEntries,VCB->root_dir_size, VCB->root_dir_index);
	//Finally free directory if not root, keep track of the root and current directories
    if (ppinfo && ppinfo->parent) {
        free(dirEntries);
    } else {
       	rootDir = dirEntries;
    	cwDir = rootDir;   
    }
	//Assign length to Volume Control Block
	VCB->root_dir_index = location->start; //set root index only when inital 
	VCB->root_dir_size = location->count; // amount of blocks of Root Dir 

    printf("Created directory using %d blocks starting at block %d\n", block_num, location->start);

    return location->start;
	// strcpy(ppinfo.parent[index].name , ppinfo.lastElementName);
    // ppinfo.parent[index].size=newdir[0].size;
    // writeDir(ppinfo.parent);
    // ppinfo.lastElementIndex = index;
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



