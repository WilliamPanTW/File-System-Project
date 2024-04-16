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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size);
int fs_setcwd(char *pathname);   //linux chdir

int fs_isFile(char * filename);	//return 1 if file, 0 otherwise
int fs_isDir(char * pathname);		//return 1 if directory, 0 otherwise

int fs_delete(char* filename);	//removes a file

// Directory iteration functions
fdDir * fs_opendir(const char *pathname);

struct fs_diriteminfo *fs_readdir(fdDir *dirp);

int fs_closedir(fdDir *dirp);


// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);


int fs_stat(const char *path, struct fs_stat *buf);

// HELPER 

//check if it mark as directory 
int isDirectory(struct dirEntry* entry) {
    return entry->isDirectory & 1; //0 as false and 1 as is directory 
}

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

int parsePath(char* path, struct get_path_Info* ppinfo) {
    if (path == NULL) {
        return -1;
    }
    if (ppinfo == NULL) {
        return -1;
    }

    char copyPath[256];
    strncpy(copyPath, path, 256);
    struct dirEntry* startDir;
    if (copyPath[0] == '/') {  //absolute
        startDir = rootDir; //already loaded into memroy ROOT
    }else{
        startDir = cwDir;//alread loaded into memory CWD
    }
    
    struct dirEntry* parent = startDir;
    char* saveptr;
    char* tokenOne = strtok_r(copyPath, "/", &saveptr);
    // NULL token cd "/" OR invalid path
    if (tokenOne == NULL) {
        if (strcmp(copyPath, "/") == 0) { //Nothing specified
            ppinfo->index = -1; //invalid path
            ppinfo->parent = parent;
            ppinfo->prevElement = NULL;
            return 0;
        }
        return -1; //invalid path
    }
    while (tokenOne != NULL) { //looking for home 
        char* tokenTwo = strtok_r(NULL, "/", &saveptr);
        int index = findDirEntryByName(parent, tokenOne);
        
        if (tokenTwo == NULL) { // On last element 
            ppinfo->index = index;
            ppinfo->parent = parent;
            ppinfo->prevElement = strdup(tokenOne);
            return 0; //EOA
        }

        if (index == -1) {
            return -2; //invalid path
        }

        if (!isDirectory(&parent[index])) {
            return -2; //invalid path
        }

        struct dirEntry* temp = loadDir(&parent[index]);

        //Free the directory entry we won't be using before assigning the next
        if (parent != startDir) {
            free(parent);
        }
        parent = temp;
        tokenOne = tokenTwo;
    }
    return -2;
}