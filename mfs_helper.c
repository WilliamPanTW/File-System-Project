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

/****************************************************************************************
*  displayFiles for use by ls command
****************************************************************************************/

struct fs_diriteminfo *fs_readdir(fdDir *dirp) {
    // Check if directory stream handle is valid
    if (!dirp) {
        return NULL; //invalid stream
    }
 
    // Iterate until reaching the end or finding a valid entry  
    while (dirp->dirEntryPosition <= dirp->dirEntriesCount) {
        //Get the directory entry at the next position
        struct dirEntry* nextEntry = &dirp->directory[dirp->dirEntryPosition];
        
        //Move to the next directory entry position
        dirp->dirEntryPosition++;

        //Skip empty directory entries
        if (nextEntry->fileName[0] == '\0') {
            continue;
        }

        // Copy directory entry information to fs_diriteminfo structure
        struct fs_diriteminfo* dirItemInfo = dirp->di;
        if (!dirItemInfo) {
            return NULL; // allocation fails
        }
        
        strncpy(dirItemInfo->d_name, nextEntry->fileName, MAX_FILENAME_LENGTH);
        
        //set the file type
        if (isDirectory(nextEntry)) {
            dirItemInfo->fileType = 'd';
        } else {
            dirItemInfo->fileType = 'f';
        }

        return dirItemInfo;// Return next directory entry information
    }
    return NULL;//End of file or error 
}

//retrieve file status information
int fs_stat(const char *path, struct fs_stat *buf) {
  
    if (buf == NULL) {
        return -1;
    }
    // check if directory is valid 
    if (parsePath((char*)path, &ppinfo) != 0) {
        return -1; //not valid
    }
    //Check if the parent directory doesn't exists
    if (ppinfo.lastElementIndex == -1) {
        freeppinfo();
        return -1;
    }

    // Get the directory entry corresponding to the last element in the path
    struct dirEntry* entry = &ppinfo.parent[ppinfo.lastElementIndex];
    
    // Set the size of the file or directory
    if (isDirectory(entry)) {
        // For directories, set the size based on the number of entries
        buf->st_size = entry->entry_amount * sizeof(struct dirEntry*);
    } else {
        // For files, set the size based on the directory size
        buf->st_size = entry->dir_size;
    }

    // Initial the data 
    buf->st_size = entry->dir_size*sizeof(struct dirEntry);
    buf->st_blksize = MINBLOCKSIZE;
    buf->st_blocks = entry->dir_size;
    buf->st_modtime = entry->modifyDate;
    buf->st_createtime = entry->createDate;
    return 0;
}

int fs_closedir(fdDir *dirp) {
    if (!dirp) {
        return 0;
    }
    
    free(dirp->directory);
    free(dirp->di);
    free(dirp);
    return 0;
}
/****************************************************************************************
*  Rm commmand
****************************************************************************************/

// check if directory entry is empty 
int isDirEntryEmpty(struct dirEntry* dir) {
    // IF is not empty then this function will not succeed. 
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


void deallocateSpace(uint64_t startBlock, uint64_t numberOfBlocks) {
    if (startBlock < 0 || startBlock >= VCB->block_size) {
        return;
    }
    for (int i = 0; i < numberOfBlocks; i++) {
        clear_bit(fsmap, startBlock + i);
    }
    LBAwrite(fsmap, VCB->bit_map_size, VCB->bit_map_size);
}

//Free extents associated with file
int freeExtents(struct dirEntry* entry) {
    for (int i = 0; entry->dir_index != 0; i++) {
        printf("Deallocate space start block : %d to num of block:%d \n",
        entry->dir_index,entry->dir_size);
        deallocateSpace(entry->dir_index, entry->dir_size);
        entry->dir_index = 0;
        entry->dir_size = 0;
    }
}
/****************************************************************************************
*  parse path for md commmand (mkdir)
****************************************************************************************/

//Iterate through array of directory entries to find unused entry
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

//Loads a directory entry from disk into memory
struct dirEntry* loadDir(struct dirEntry* entry) {
    if (entry == NULL) {
        return NULL;
    }
    struct dirEntry* loadDir;
    int startBlock = entry->dir_index;

    int blocksNeeded = entry->dir_size; 
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
        startparent = loadedRoot; //already loaded into memroy ROOT
    }else{
        startparent = loadedCWD;//alread loaded into memory CWD
    }
    
    struct dirEntry* parent = startparent;
    char* saveptr;
    char* tokenOne = strtok_r(copyPath, "/", &saveptr);
    // printf("--------------Path name token1: %s--------------\n",tokenOne); //pathname 

    // token null mean either specified just the slash or not specified anything
    if (tokenOne == NULL) {
        if (copyPath[0] !=  '/'){//Nothing specified invalid path
            return -1; //invalid path
        }else  {  // specified root 
            ppinfo->parent = parent; // root is their own parent
            ppinfo->lastElementName = NULL; // no name 
            ppinfo->lastElementIndex = -2; // NOT even exist 
            return 0;//success return  
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
        if (ppinfo.parent != loadedCWD && ppinfo.parent != loadedRoot) {
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