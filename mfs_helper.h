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

//****************************Rm command ****************************
int isDirEntryEmpty(struct dirEntry* dir);

int writeDirToDisk(struct dirEntry* entry);

int setDirEntryUnused(struct dirEntry* dir);

void deallocateSpace(uint64_t startBlock, uint64_t numberOfBlocks);

//****************************Parse Path****************************
int findUnusedDE(struct dirEntry* entry);

int parsePath(char* path, struct pp_return_struct* ppinfo);

int findDirEntry(struct dirEntry* entry, char* name);

int isDirectory(struct dirEntry* entry);

struct dirEntry* loadDir(struct dirEntry* entry);
//****************************Free space****************************
void freeLastElementName();

void freePathParent();

void freeppinfo();
#endif