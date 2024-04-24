/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: b_io.c
*
* Description:: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "b_io.h"
#include "b_io_helper.h"
#include "fsInit.h"
#include "mfs_helper.h"
#include "mfs.h"
#include "global.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int currentPosition;
	int currentBlock;
	int mode;
	struct dirEntry * file;	     //hold the file directory 
	struct dirEntry * parent;	 //hold the parent directory 
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
		
	if (startup == 0) b_init();  //Initialize our system
	
	
	if (parsePath(filename, &ppinfo) != 0) {
		return -1; // invalid path 
	}

	struct dirEntry* parent = ppinfo.parent;
	struct dirEntry* entry;

	//remove everything in contents
	if (flags & O_TRUNC) {
		if (ppinfo.lastElementIndex != -1) {
			entry = &parent[ppinfo.lastElementIndex];
			freeExtents(entry);
			entry->dir_size = 0;
		}
	}

	//File must not exist, create new file
	if (flags & O_CREAT) {
		if (ppinfo.lastElementIndex == -2) {
			int index = findUnusedDE(ppinfo.parent);
			if (index == -1) {
				printf("No space left!\n");
				freeppinfo(NULL);
				return -1;
			}

			ppinfo.lastElementIndex = index;
				
			if (createFile(&ppinfo) == -1) {
				freeppinfo(NULL);
				return -1;
			}
				printf("Create file\n");
		}
	}

	if (ppinfo.lastElementIndex == -1) {
		printf("Doesn't exist\n");
		freeppinfo(NULL);
		return -1;
	}
	entry = &parent[ppinfo.lastElementIndex];

	fcbArray[returnFd].buf = malloc(B_CHUNK_SIZE);
	if (!fcbArray[returnFd].buf) {
		freeppinfo(NULL);
		return -1;
	}

	//In case get file info failed 
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	
	if (returnFd == -1){
		return -1; // No free FCB available
	} 

	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].buflen = 0;

	fcbArray[returnFd].file = entry;
	fcbArray[returnFd].parent = parent; 

	fcbArray[returnFd].currentBlock = 0;
	fcbArray[returnFd].currentPosition = 0;
	fcbArray[returnFd].mode = flags;

	//If file needs to append, start file at the end
	if (flags & O_APPEND) {
		for (int i = 0; i < MIN_DE; i++) {
			if (entry->dir_index == 0) {
				fcbArray[returnFd].currentBlock = i;
			}
		}
		fcbArray[returnFd].currentPosition = entry->dir_size;
	}

	return (returnFd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	return (0);	//Change this
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
		freeppinfo();
		if (fcbArray[fd].buf == NULL) {
			return 0;
		}
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;

		fcbArray[fd].mode = 0;
	}
