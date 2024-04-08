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
* Description:: 
*
**************************************************************/
#ifndef FSINIT
#define FSINIT

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#define MAX_FILENAME_LENGTH 255
#define BLOCK_SIZE 512


struct dirEntry 
	{
    char filename[MAX_FILENAME_LENGTH];// char name size with null-terminator

    uint64_t location;               // long type of address(location)

    uint32_t isDirectory;            // indicating it's a directory(1) or a file(0)
    uint32_t dirSize;               // unsigned integer of directory size

    time_t createDate;           // integer or long depend on implementation
    time_t modifyDate;           // integer or long depend on implementation
    time_t accessDate;          // integer or long depend on implementation
	} dirEntry ;


struct vcb
	{
	uint64_t signature;			//Long type unique identify (8bytes) generate by magic number 

    uint64_t block_size;		//Total Number of blocks
	uint64_t block_index;		//number of free blocks in volume
    
	uint64_t free_block_size;	//Total number of free blocks(bitmap length)
	uint64_t free_block_index;	//location of the free space in bitmap
	
	uint64_t root_dir_size;		//Total number  of the root directory 
	uint64_t root_dir_index;	//Location of the root directory 
	} vcb;

	void set_bit(char* bitmap, int position);
	int get_bit(char* bitmap, int position);
	void clear_bit(char* bitmap, int position);

	//**************************Helper function**************************//
	int initFreeSpace(uint64_t numberOfBlocks);


	void set_bit(char* bitmap, int position);
	int get_bit(char* bitmap, int position);
	void clear_bit(char* bitmap, int position);



#endif

