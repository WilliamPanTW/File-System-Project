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
#include "mfs_helper.h"  // access function 
#include "fsInit.h"  // for directory strucuture s
#include "b_io_helper.h"  // access function 

int createFile(struct pp_return_struct* info) {
    struct dirEntry* entry = &ppinfo.parent[ppinfo.lastElementIndex];
    strncpy(entry->fileName, ppinfo.lastElementName, 256);

    entry->dir_size = 0;
    entry->isDirectory = 0;

    time_t current_time;
	time(&current_time);
    entry->createDate = current_time;
    entry->modifyDate = current_time;
    ppinfo.parent->modifyDate = current_time;
    printf("Create file using %d blocks from index %d \n"
            ,ppinfo.parent->dir_size, ppinfo.parent->dir_index);
    // Write changes back disk
    LBAwrite(ppinfo.parent, ppinfo.parent->dir_size, ppinfo.parent->dir_index);

    return 0;
}

int moveFile(const char *src, const char *dest) {
    // Check if the destination path is valid
    if (parsePath((char*)dest, &ppinfo) != 0) {
        return -1;
    }

    struct dirEntry* newDir;
    // Check if the destination directory exists
    if (ppinfo.lastElementIndex == -1) {
        if (ppinfo.lastElementName != NULL) {
            freeLastElementName();
            freePathParent();
            return -1;
        }
        newDir = loadDir(ppinfo.parent);
    } else {
        newDir = loadDir(&ppinfo.parent[ppinfo.lastElementIndex]);
    }

    //check if destination is directory 
    if (!isDirectory(newDir)) {
        free(newDir);
        return -1; // Error destination is not directory 
    }

    //check if source path is valid path 
    if (parsePath((char*)src, &ppinfo) != 0) {
        free(newDir);
        return -1;
    }

    //check If file is a directory
    struct dirEntry* file = &ppinfo.parent[ppinfo.lastElementIndex];
    if (isDirectory(file)) {
        free(newDir);
        freeppinfo();
        return -1;
    }

    //check if file exist in new directory 
    if (findDirEntry(newDir, ppinfo.lastElementName) != -1) {
        free(newDir);
        freeppinfo();
        return -1;
    }

    //Check free space for new file
    int newIndex = findUnusedDE(newDir);
    if (newIndex == -1) {
        free(newDir);
        freeppinfo();
        return -1;
    }

    // Copy the file to the destination directory
    struct pp_return_struct newInfo;
    newInfo.parent = newDir;
    newInfo.lastElementIndex = newIndex;
    copyFile(&newInfo, file);

    // Update modification time of the parent directory
    time_t current_time;
	time(&current_time);
    ppinfo.parent->modifyDate = current_time;

   
    //write it back to disk 
    LBAwrite(newInfo.parent, newInfo.parent->dir_size, newInfo.parent->dir_index);
    // printf("move file using %d blocks to index %d \n"
            // ,newInfo.parent->dir_size, newInfo.parent->dir_index);

    freeppinfo();
    return 0;
}

int copyFile(struct pp_return_struct* info, struct dirEntry* src) {
    struct dirEntry* entry = &info->parent[info->lastElementIndex];

    strncpy(entry->fileName, src->fileName, 256);

    entry->dir_size = src->dir_size;
    
    entry->createDate = src->createDate;
    entry->modifyDate = src->modifyDate;
    entry->isDirectory = src->isDirectory;
    
    time_t current_time;
	time(&current_time);
    ppinfo.parent->modifyDate = current_time;
    LBAwrite(ppinfo.parent, ppinfo.parent->dir_size, ppinfo.parent->dir_index);

    return 0;
}
