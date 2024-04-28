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

    // Write changes back disk
    LBAwrite(ppinfo.parent, ppinfo.parent->dir_size, ppinfo.parent->dir_index);

    return 0;
}

int moveFile(const char *srcPath, const char *destPath) {
    if (parsePath((char*)destPath, &ppinfo) != 0) {
        return -1;
    }

    //Load directory if exists
    struct dirEntry* newDir;
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

    //Don't continue if destination isn't a directory
    if (!isDirectory(newDir)) {
        free(newDir);
        return -1;
    }

    //Now check if file exists
    if (parsePath((char*)srcPath, &ppinfo) != 0) {
        free(newDir);
        return -1;
    }

    if (ppinfo.lastElementIndex == -1) {
        free(newDir);
        freeppinfo();
        return -1;
    }

    //If file is a directory, exit
    struct dirEntry* file = &ppinfo.parent[ppinfo.lastElementIndex];
    if (isDirectory(file)) {
        free(newDir);
        freeppinfo();
        return -1;
    }

    //Make sure the file isn't already in destination
    if (findDirEntry(newDir, ppinfo.lastElementName) != -1) {
        free(newDir);
        freeppinfo();
        return -1;
    }

    //Check if there's space to move file
    int newIndex = findUnusedDE(newDir);
    if (newIndex == -1) {
        free(newDir);
        freeppinfo();
        return -1;
    }

    //Insert file in destination
    struct pp_return_struct newInfo;
    newInfo.parent = newDir;
    newInfo.lastElementIndex = newIndex;

    copyFile(&newInfo, file);

    //Remove file in its parent
    file->fileName[0] = '\0';//set directory unsed 
    // Write DIR changes back disk
    time_t current_time;
	time(&current_time);
    ppinfo.parent->modifyDate = current_time;
    LBAwrite(ppinfo.parent, ppinfo.parent->dir_size, ppinfo.parent->dir_index);


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
    
    // Write DIR changes back disk
    time_t current_time;
	time(&current_time);
    ppinfo.parent->modifyDate = current_time;
    LBAwrite(ppinfo.parent, ppinfo.parent->dir_size, ppinfo.parent->dir_index);

    return 0;
}
