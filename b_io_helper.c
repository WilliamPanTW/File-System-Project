/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: John Cuevas, Michael Abolencia , Pan William , Tina Chou
* Student IDs:: 920542932, 917581956, 922867228 , 922911207
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

/****************************************************
*  b_open
****************************************************/

int createFile(struct pp_return_struct* info) {
    // Get the parent directory entry where the new file will be created
    struct dirEntry* entry = &ppinfo.parent[ppinfo.lastElementIndex];

    // Copy the name of the new file to the directory entry
    strncpy(entry->fileName, ppinfo.lastElementName, 256);

    // Initialize size and type of the new file
    entry->dir_size = 0;
    entry->isDirectory = 0;

    // Get current time
    time_t current_time;
	time(&current_time);
    entry->createDate = current_time;
    entry->modifyDate = current_time;

    // Update parent directory's modification date
    ppinfo.parent->modifyDate = current_time;


    // printf("Create file using %d blocks from index %d \n"
    //         ,ppinfo.parent->dir_size, ppinfo.parent->dir_index);


    // Write changes back disk
    LBAwrite(ppinfo.parent, ppinfo.parent->dir_size, ppinfo.parent->dir_index);

    return 0;
}

/****************************************************
*  Move file commmand
****************************************************/

int moveFile(const char *src, const char *dest) {
    // Check if the destination path is valid
    if (parsePath((char*)dest, &ppinfo) != 0) {
        return -1;
    }

    struct dirEntry* newDir;
    //If the parse path index equal to specified root from parse path function
    if (ppinfo.lastElementIndex == -1) {
        // And does not have a name 
        if (ppinfo.lastElementName != NULL) {
            freeLastElementName();
            freePathParent();
            return -1;
        }
        //Then root Load directory entry itself 
        newDir = loadDir(ppinfo.parent);
    } else {
        // Load directory entry for specified parent index 
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
    // Get the parent directory entry where the file will be copied
    struct dirEntry* entry = &info->parent[info->lastElementIndex];

    // Copy the file name from source to destination directory entry
    strncpy(entry->fileName, src->fileName, 256);

    // Copy other attributes from source to destination directory entry
    entry->dir_size = src->dir_size;
    entry->createDate = src->createDate;
    entry->modifyDate = src->modifyDate;
    entry->isDirectory = src->isDirectory;
    
    // Get current time
    time_t current_time;
	time(&current_time);
    // Update parent directory's modification date
    ppinfo.parent->modifyDate = current_time;

    // Write the parent directory entry to disk
    LBAwrite(ppinfo.parent, ppinfo.parent->dir_size, ppinfo.parent->dir_index);

    return 0;
}
