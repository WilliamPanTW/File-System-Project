/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: mfs_helper.c
*
* Description:: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mfs_helper.h"
#include "fsInit.h"
#include "fsLow.h"
#include "global.h"
//*********************************Rm command *********************************
int isDirEntryEmpty(struct dirEntry* dir) {
    if (dir == NULL) {
        return -1;
    }
    int numEntries = dir->entry_amount;
    for (int i = 2; i < numEntries; i++) {
        if (dir[i].fileName[0] != '\0') {
            return 0;
        }
    }
    return 1;
}

int writeDirToDisk(struct dirEntry* entry) {
    time_t t = time(NULL);
    entry->modifyDate = t;
    return LBAwrite(entry, entry->dir_size, entry->dir_index);
}

int setDirEntryUnused(struct dirEntry* dir) {
    dir->fileName[0] = '\0';
    return 0;
}

void deallocateSpace(uint64_t startBlock, uint64_t numberOfBlocks) {
    if (startBlock < 0 || startBlock >= VCB->block_size) {
        return;
    }
    for (int i = 0; i < numberOfBlocks; i++) {
        clear_bit(fsmap, startBlock + i);
    }
    LBAwrite(fsmap, VCB->bit_map_size, VCB->bit_map_size);
}

//**********************************Parse Path*********************************

int findUnusedDE(struct dirEntry* entry) {
    if (entry == NULL) {
        return -1;
    }
    int numEntries = entry->entry_amount;
    // iterate through all directory entires
    for (int i = 0; i < numEntries; i++) {
        //if found null ternimate then return index
        if (entry[i].fileName[0] == '\0') {
            return i;
        }
    }
    return -1;//nothing found return error 
}


//check if it mark as directory 
int isDirectory(struct dirEntry* entry) {
    return entry->isDirectory & 1; //1 indicate it's directory zero as not
}

// find directory by it name 
int findDirEntry(struct dirEntry* entry, char* name) {
    int numEntries = entry->entry_amount;
    // iterate all directory entires 
    for (int i = 0; i < numEntries; i++) {
        // if any name match return index 
        if (strcmp(entry[i].fileName, name) == 0) {
            return i;  
        }
    }
    return -1;//no directory found 
}

struct dirEntry* loadDir(struct dirEntry* entry) {
    if (entry == NULL) {
        return NULL;
    }
    struct dirEntry* loadDir;
    int startBlock = entry->dir_index;

    int blocksNeeded = entry->entry_amount;
    int bytesNeeded = blocksNeeded * MINBLOCKSIZE;

    loadDir = malloc(bytesNeeded);
    if (!loadDir) {
        return NULL;
    }

    int result = LBAread(loadDir, blocksNeeded, startBlock);
    if (result != blocksNeeded) {
        return NULL;
    }
    return loadDir;
}

//check if this entries is good or not 
int parsePath(char* path, struct pp_return_struct* ppinfo) {
    if (path == NULL) {
        return -1;
    }
    if (ppinfo == NULL) {
        return -1;
    }

    char copyPath[MAX_FILENAME_LENGTH];
    strncpy(copyPath, path, MAX_FILENAME_LENGTH);

    struct dirEntry* startparent;
    if (copyPath[0] == '/') {  //absolute
        startparent = rootDir; //already loaded into memroy ROOT
    }else{
        startparent = cwDir;//alread loaded into memory CWD
    }
    
    struct dirEntry* parent = startparent;
    char* saveptr;
    char* tokenOne = strtok_r(copyPath, "/", &saveptr);
    // printf("--------------Path name token1: %s--------------\n",tokenOne); //pathname 

    // cd "/" to root OR invalid path
    if (tokenOne == NULL) {
        if (copyPath[0] !=  '/'){//Nothing specified invalid path
            return -1; //invalid path
        }
        else  {  // specified root 
            ppinfo->parent = parent; // root is their own parent
            ppinfo->lastElementName = NULL; // no name 
            ppinfo->lastElementIndex = -2; // NOT exist 
            return 0;
        }
    }

    while (tokenOne != NULL) {  // find if it in directory 
        char* tokenTwo = strtok_r(NULL, "/", &saveptr); //token the last element
        int index = findDirEntry(parent, tokenOne);//Check if it is DE 
        if (tokenTwo == NULL) { // On last element 
            ppinfo->parent = parent; //EQUAL ROOT 
            ppinfo->lastElementName = strdup(tokenOne);
            ppinfo->lastElementIndex = index;
            // printf("parse path parent name check: %s\n",ppinfo->parent->fileName);
            return 0; //reach end of file
        }

        if (index == -1) { //must exist 
            return -1; //invalid path
        }

        if (!isDirectory(&parent[index])) { // must be directory 
            return -1; //invalid path
        }

        // load directory of address of parent index 
        struct dirEntry* tempparent = loadDir(&parent[index]);
        //Free the directory entry we won't be using before assigning the next
        if (parent != startparent) {
            free(parent);
        }
        parent = tempparent; // root directory 
        tokenOne = tokenTwo;
    }
    return -2;
}

//**********************************Free space*********************************

// free ppinfo
void freeLastElementName() {
    if (ppinfo.lastElementName) {
        free(ppinfo.lastElementName);
        ppinfo.lastElementName = NULL;
    }
}
void freePathParent() {
    if (ppinfo.parent) {
        if (ppinfo.parent != cwDir && ppinfo.parent != rootDir) {
            free(ppinfo.parent);
        }
        ppinfo.parent = NULL;  
    }
}
void freeppinfo() {
    ppinfo.lastElementIndex = -1;
    freeLastElementName();
    freePathParent();
}