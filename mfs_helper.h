/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: mfs_helper.h
*
* Description:: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/

#ifndef _MFS_HELPER_H
#define _MFS_HELPER_H
#include "fsInit.h"
#include "fsLow.h"
#include "global.h"

// int cmd_pwd (int argcnt, char *argvec[]); //dummy check

//****************************Display file for md ****************************
struct fs_diriteminfo *fs_readdir(fdDir *dirp);

int fs_stat(const char *path, struct fs_stat *buf);

int fs_closedir(fdDir *dirp);

//****************************Rm command ****************************

// check if directory entry is empty 
int isDirEntryEmpty(struct dirEntry* dir);

void deallocateSpace(uint64_t startBlock, uint64_t numberOfBlocks);

//Free extents associated with file
int freeExtents(struct dirEntry* entry);

//****************************Parse Path****************************

//check if this entries is good or not 
int parsePath(char* path, struct pp_return_struct* ppinfo);

//Iterate through array of directory entries to find unused entry
int findUnusedDE(struct dirEntry* entry);

//find directory by it name 
int findDirEntry(struct dirEntry* entry, char* name);

//check if it mark as directory 
int isDirectory(struct dirEntry* entry);

//Loads a directory entry from disk into memory
struct dirEntry* loadDir(struct dirEntry* entry);
//****************************Free space****************************
void freeLastElementName();

void freePathParent();

void freeppinfo();
#endif