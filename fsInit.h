/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: fsInit.h
*
* Description:: Main driver header file 
* for file system assignment.
*
**************************************************************/
#ifndef _FSINIT_H
#define _FSINIT_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#define MAX_FILENAME_LENGTH 255
#define MIN_DE 50


struct dirEntry* rootDir;
struct dirEntry* cwDir;

struct get_path_Info 
{
    struct dirEntry* parent;
    char* prevElement;
    int index;
};


struct dirEntry 
	{
    char fileName[MAX_FILENAME_LENGTH];// char name size with null-terminator

    uint64_t location;               // long type of address(location)

    uint32_t isDirectory;            // indicating it's a directory(1) or a file(0)
    uint32_t dirSize;               // unsigned integer of directory size

    time_t createDate;           // integer or long depend on implementation
    time_t modifyDate;           // integer or long depend on implementation
};


struct vcb
	{
	uint64_t signature;			//Long type unique identify (8bytes) generate by magic number 

	uint64_t block_index;		//location of the VCB (block 0)
    uint64_t block_size;		//capacity of the volume (19531)
    
	uint64_t bit_map_index;	     //location of the bitmap
	uint64_t free_block_index;	//track location of the free space in bitmap
	uint64_t free_block_size;	//Total number of free blocks(bitmap length)
	
	uint64_t root_dir_index;	//Location of the root directory 
	uint64_t root_dir_size;		//Total number of the root directory 
} ;

	struct vcb *VCB;
	char * fsmap; //global unsign char fsmap pointer 

	void set_bit(char* bitmap, int position);
	int get_bit(char* bitmap, int position);
	void clear_bit(char* bitmap, int position);
	void set_Dir(struct dirEntry *dirEntries , int index,char *name,int dirEntryAmount);

	//**************************Helper function**************************//
	int initVolumeControlBlock(uint64_t numberOfBlocks);
	int initFreeSpace(uint64_t numberOfBlocks);
	int initRootDir(uint64_t numberOfBlocks);
	int trackAndSetBit(char* fsmap, int numberOfBlocks);
	void updateFreeSpace(int block_num);

	void set_bit(char* bitmap, int position);
	int get_bit(char* bitmap, int position);
	void clear_bit(char* bitmap, int position);



#endif

