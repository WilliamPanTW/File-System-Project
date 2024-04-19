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
#include "fsInit.h"
#include "fsLow.h"
#include "global.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

  
// Misc directory functions
char * fs_getcwd(char *pathname, size_t size);
int fs_setcwd(char *pathname);   //linux chdir

int fs_isFile(char * filename){

    return 1; 
}	//return 1 if file, 0 otherwise

int fs_isDir(char * pathname);		//return 1 if directory, 0 otherwise

int fs_delete(char* filename);	//removes a file

// Directory iteration functions
fdDir * fs_opendir(const char *pathname);

struct fs_diriteminfo *fs_readdir(fdDir *dirp);

int fs_closedir(fdDir *dirp);


// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode){
    int result;
    struct pp_return_struct ppinfo;
    result = parsePath((char*)pathname, &ppinfo);
    if(result == -1){ 
        return -1; //invalid path
    }
    if(ppinfo.lastElementIndex!=-1){
        return -1; //already exist 
    }
    printf("--------------Hello world%d--------------\n",result);
    return 0;
}

int fs_rmdir(const char *pathname);


int fs_stat(const char *path, struct fs_stat *buf);



//**************helper function for parePath**************

//check if it mark as directory 
int isDirectory(struct dirEntry* entry) {
    return entry->isDirectory & 1; //0 as false and 1 as is directory 
}

// find directory by it name 
int findDirEntryByName(struct dirEntry* dirEntries, char* name) {
    int numEntries = dirEntries->dirSize;
    printf("Entries suppose to be: %d\n",numEntries);
    for (int i = 0; i < numEntries; i++) {
        if (strcmp(dirEntries[i].fileName, name) == 0) {
            return i;
        }
    }
    return -1;
}

struct dirEntry* loadDir(struct dirEntry* entry) {
    if (entry == NULL) {
        return NULL;
    }
    struct dirEntry* loadDir;
    int startBlock = entry->location;

    int blocksNeeded = entry->dirSize;
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
    // cd "/" to root OR invalid path
    if (tokenOne == NULL) {
        if (copyPath[0] !=  '/'){//Nothing specified invalid path
            return -1; //invalid path
        }
        else  { 
            ppinfo->parent = parent; // root is their own parent
            ppinfo->lastElementName = NULL; // no name 
            ppinfo->lastElementIndex = -2; // NOT exist 
            return 0;
        }
    }
    while (tokenOne != NULL) {  // find if it in directory 
        char* tokenTwo = strtok_r(NULL, "/", &saveptr); 
        int index = findDirEntryByName(parent, tokenOne);//return if it valid diretory
        
        if (tokenTwo == NULL) { // On last element 
            ppinfo->parent = parent;
            ppinfo->lastElementName = strdup(tokenOne);
            ppinfo->lastElementIndex = index;
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