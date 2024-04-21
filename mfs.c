/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: mfs.c
*
* Description:: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/
#include "mfs.h"
#include "mfs_helper.h"
#include "fsInit.h"
#include "fsLow.h"
#include "global.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// Misc directory functions
char * fs_getcwd(char *pathname, size_t size);
int fs_setcwd(char *pathname);   //linux chdir

//return 1 if file, 0 otherwise
int fs_isFile(char * filename) {
    if (parsePath(filename, &ppinfo) != 0) {
        return -1;
    }

    //This function doesn't need last element
    freeLastElementName();

    //If the file doesn't exists
    if (ppinfo.lastElementIndex == -1) {
        printf("File doesn't exist\n");
        freePathParent();
        return -1;
    }

    struct dirEntry* entry = &ppinfo.parent[ppinfo.lastElementIndex];
    if (!isDirectory(entry)) {
        freePathParent();
        return 1;//it's a file 
    }
    freePathParent();
    return 0;
}	

//return 1 if directory, 0 otherwise
int fs_isDir(char * pathname) {
    if (parsePath(pathname, &ppinfo) != 0) {
        return -1;
    }

    struct dirEntry* entry = cwDir;

    //If the directory doesn't exists
    if (ppinfo.lastElementIndex == -1) {
        if (ppinfo.lastElementName != NULL) {
            freeppinfo();
            return -1;
        }
        entry = ppinfo.parent;
    } else {
        entry = &ppinfo.parent[ppinfo.lastElementIndex];
    }

    if (isDirectory(entry)) {
        freeppinfo();
        return 1;//It's directory
    }
    freeppinfo();
    return 0;
}

//removes a file
int fs_delete(char* filename){
    if (parsePath((char*)filename, &ppinfo) != 0) {
        printf("Invalid path!\n");
        return -1;
    }

    //This function doesn't need last element
    freeLastElementName();

    if (ppinfo.lastElementIndex == -1) {
        printf("failed to remove \'%s\': No such directory!\n", filename);
        freePathParent();
        return -1;
    } //Restrict the user from removing . and ..
    else if (ppinfo.lastElementIndex < 2) {
        printf("failed to remove \'%s\': Not valid!\n", filename);
        freePathParent();
        return -1;
    }
    struct dirEntry* entry = &ppinfo.parent[ppinfo.lastElementIndex];
    if (isDirectory(entry)) {
        printf("Not a file.\n");
        freePathParent();
        return -1;
    }

    // //Free extents associated with file
    // freeExtents(entry); //TODO 

    //Set directory as unused in its parent
    setDirEntryUnused(entry);

    //Finally write the changes to disk
    writeDirToDisk(ppinfo.parent);
    
    freePathParent();
    return 0;
}	

// Directory iteration functions
fdDir * fs_opendir(const char *pathname);

struct fs_diriteminfo *fs_readdir(fdDir *dirp);

int fs_closedir(fdDir *dirp);


// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode){
    int result;
    
    result = parsePath((char*)pathname, &ppinfo);
    if(result == -1){ 
        return -1; //invalid path
    }

    //Check the directory is not exist to create new directory  
    if(ppinfo.lastElementIndex!=-1){
        freeppinfo();
        return -1; //already exist 
    }

    //find free directory entries
    int index = findUnusedDE(ppinfo.parent);
    if (index == -1) {
        freeppinfo();
        return -1; //no entries space left 
    }

    // DE * newdir = createDirectory(50,ppinfo.parent);
    ppinfo.lastElementIndex = index; //update info index where free space locate 
    createDirectory(MIN_DE, &ppinfo);

    freeppinfo();
    return 0;
}

int fs_rmdir(const char *pathname) {
    int result = parsePath((char*)pathname, &ppinfo);
     if(result == -1){ 
        return -1; //invalid path
    }

    freeLastElementName();

    if (ppinfo.lastElementIndex == -1) {
        printf("failed to remove \'%s\': No such directory!\n", pathname);
        freePathParent();
        return -1;
    } //Restrict the user from removing . and ..
    else if (ppinfo.lastElementIndex < 2) {
        printf("failed to remove \'%s\': Not valid!\n", pathname);
        freePathParent();
        return -1;
    }

    //Check if directory before proceeding
    if (!isDirectory(&ppinfo.parent[ppinfo.lastElementIndex])) {
        printf("failed to remove \'%s\': File is not directory.\n", pathname);
        freePathParent();
        return -1;
    }

    //Load directory to check its entries if empty
    struct dirEntry* entry = loadDir(&ppinfo.parent[ppinfo.lastElementIndex]);
    if(!isDirEntryEmpty(entry)) {
        printf("failed to remove \'%s\': Directory not empty!\n", pathname);
        free(entry);
        freePathParent();
        return -1;
    }
    free(entry);

    entry = &ppinfo.parent[ppinfo.lastElementIndex];

    //Free blocks associated with directory
    deallocateSpace(entry->dir_index, entry->dir_size);

    //Set directory as unused in its parent
    setDirEntryUnused(entry);

    //Finally write the changes to disk
    writeDirToDisk(ppinfo.parent);

    freePathParent();
    return 0;
}


int fs_stat(const char *path, struct fs_stat *buf);
