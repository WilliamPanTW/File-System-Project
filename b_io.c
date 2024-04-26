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
	char * buf;				 //holds the open file buffer
	int index;				 //holds the current position in the buffer
	int buflen;				 //holds how many valid bytes are in the buffer
	int currentPosition;	 //hold how many block file occupies (numBlocks)
	int currentBlock;		 //hold the current block number (currentBLK)
	int mode;				 //hold the mode from flag
	struct dirEntry * file;	 //hold the file directory 
	struct dirEntry * parent;//hold the parent directory 
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
	
	//check if the filename is valid 
	if (parsePath(filename, &ppinfo) != 0) {
		return -1; // invalid  
	}
	//retrieve parent directroy information 
	struct dirEntry* parent = ppinfo.parent;
	struct dirEntry* entry;

	//remove everything in contents if TRUNC flag is set
	if (flags & O_TRUNC) {
		if (ppinfo.lastElementIndex != -1) {
			entry = &parent[ppinfo.lastElementIndex];
			freeExtents(entry);
			entry->dir_size = 0;
		}
	}

	//Create a new file 
	if (flags & O_CREAT) {
		//check if file does exist
		if (ppinfo.lastElementIndex == -1) {
			int index = findUnusedDE(ppinfo.parent);
			// check what index by find unuesd directory  
			if (index == -1) {
				freeppinfo(NULL);
				return -1; //no Space left 
			}
			//assign new index 
			ppinfo.lastElementIndex = index;
			//check if create file fail 
			if (createFile(&ppinfo) == -1) {
				freeppinfo(NULL);
				return -1;
			}
		}
	}
	// if the newly assign index is still zero 
	if (ppinfo.lastElementIndex == -1) {
		freeppinfo(NULL);
		return -1; // ERROR
	}
	// Retrieve file parent information
	entry = &parent[ppinfo.lastElementIndex];

	// Get a free File Control Block (FCB), in case get file info failed 
	returnFd = b_getFCB();				// get our own file descriptor
										
	// check for error - all used FCB's
	if (returnFd == -1){
		return -1; // No free FCB available
	} 
	
	// Allocate memory for the file buffer
	fcbArray[returnFd].buf = malloc(B_CHUNK_SIZE);
	if (!fcbArray[returnFd].buf) {
		freeppinfo(NULL);
		return -1;
	}

	//Initialize FCB with file information
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].buflen = 0;

	fcbArray[returnFd].file = entry;
	fcbArray[returnFd].parent = parent; 

	fcbArray[returnFd].currentBlock = 0;
	fcbArray[returnFd].currentPosition = 0;
	fcbArray[returnFd].mode = flags%16; //the reminder of flag is mod
	
	// Set file position to end if O_APPEND flag is set
	if (flags & O_APPEND) {
		//iterate to find an available block
		for (int i = 0; i < MIN_DE; i++) {
			// Check if the current block is empty
			if (entry->dir_index == 0) {
				//// Set the current block to the empty block index
				fcbArray[returnFd].currentBlock = i;
			}
		}
		//// Set the current position of the FCB to the end of the file
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
	
	int bytesRead; 				 // for our read
	int bytesReturned; 			 // what we will return
	int part1, part2, part3; 	 // hold the three potential copy length 
	int numberOfBlocksTocopy;	 // hold the number of while blocks that need copy 
	int remainingBytesInMyBuffer;// hold how many bytes are left in my buffer

	if (startup == 0) b_init();  // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					// invalid file descriptor
		}

	// number of bytes available to copy from buffer 
	remainingBytesInMyBuffer = fcbArray[fd].buflen - fcbArray[fd].index;

	// amount have given to user
	int amountAlreadyDelivered = (fcbArray[fd].currentBlock * B_CHUNK_SIZE) - remainingBytesInMyBuffer;

	// Limit count to file length to handle End of file
	// by checking if amount they ask(count) plus already given exceed the file size 
    if ((count + amountAlreadyDelivered) > fcbArray[fd].file->dir_size) {
		// reduce count size and we could not go beyond file size
        count = fcbArray[fd].file->dir_size - amountAlreadyDelivered;
        if (count < 0) {
            return -1; 
        }
	}

	// Part 1: is the first copy of data which will be from the current buffer 
	// It will be the lesser of the requested amount or the number of byte
	if (remainingBytesInMyBuffer >= count) { // we have enought in buffer 
		part1 = count;	// Completetely buffer (the requested amount)
		part2 = 0;		
		part3 = 0;		// Do not need anything from the "next" buffer
	} else {
		part1 = remainingBytesInMyBuffer;  // spanning buffer 

		// Part 1 is not enought - set part 3 to how much more is needed 
		part3 = count - remainingBytesInMyBuffer;

		// The following calutation how many 512 bytes chuncks need to be 
		// the caller buffer from the count of what is left to copy
		numberOfBlocksTocopy=part3/ B_CHUNK_SIZE; 
		part2=numberOfBlocksTocopy *B_CHUNK_SIZE;

		// Reduce part 3 by the number of bytes that can be copied in chunk
		// Part 3 at this point it must be less than block size
		part3 = part3-part2; // equal to part3 % B_CHUNK_SIZE
	}

	if(part1 > 0) // memcpy part1
		{
		// that move my buffer plus my index to there buffer 
		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, part1);
		fcbArray[fd].index = fcbArray[fd].index + part1;//adjust where is starting
		}

	if (part2 > 0) // block to copy direct to callers buffer
		{
		// limit block to blocks left

		//LBAread to their buffer
		bytesRead = LBAread(buffer + part1, numberOfBlocksTocopy, 
								fcbArray[fd].file->dir_index);

		fcbArray[fd].currentBlock += numberOfBlocksTocopy;

		part2 = bytesRead * B_CHUNK_SIZE; 
		}

	if (part3 > 0) // we need to refill out buffer to copy more bytes
		{

		//try to read B_CHUNK_SIZE bytes into our buffer
		bytesRead = LBAread(fcbArray[fd].buf, 1, 
								fcbArray[fd].file->dir_index);
		
		bytesRead = bytesRead * B_CHUNK_SIZE;
		
		fcbArray[fd].currentBlock += 1; // aleardy readed next block

		//reset the offset and buffer length
		fcbArray[fd].index = 0;
		fcbArray[fd].buflen = bytesRead; //how many bytes are actually read
		
		if (bytesRead < part3)// Not even enough left to statisfy read requested from caller
			{
			part3 = bytesRead;
			}

		if (part3 > 0) //memcpy bytesRead
			{
			memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].index, part3);
			fcbArray[fd].index = fcbArray[fd].index + part3; // adjust index
			}
		}

	bytesReturned = part1 + part2 + part3;
	return (bytesReturned);
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
