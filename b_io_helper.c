/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: b_io_helper.c
*
* Description:: This header file provide necessary function for b_io.c
*
**************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "fsLow.h" //for lbawrite
#include "global.h" //for global parse path info
#include "mfs.h"  // for ppinfo strucuture 
#include "fsInit.h"  // for directory strucuture s

int createFile(struct pp_return_struct* info) {
    struct dirEntry* entry = &ppinfo.parent[ppinfo.lastElementIndex];
    strncpy(entry->fileName, ppinfo.lastElementName, 256);

    entry->dir_size = 0;
    entry->isDirectory = 0;

    time_t current_time;
	time(&current_time);
    entry->createDate = current_time;
    entry->modifyDate = current_time;
    

    // Write changes back disk
    time_t t = time(NULL);
    ppinfo.parent->modifyDate = t;
    LBAwrite(ppinfo.parent, ppinfo.parent->dir_size, ppinfo.parent->dir_index);

    return 0;
}
