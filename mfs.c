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

char cwdPath[MAX_FILENAME_LENGTH] = "/"; //inital root string current working directory 
char *cwdString = cwdPath; // global variable for the current working directory string


/****************************************************************************************
*  cd command 
****************************************************************************************/

// Misc directory functions
//linux chdir
int fs_setcwd(char *pathname){

    char tempPathName[MAX_FILENAME_LENGTH];
    //Step.1 take the path specified 
    strncpy(tempPathName, pathname, MAX_FILENAME_LENGTH);

    //Step.2 call parsepath to check if it valid
    if (parsePath((char*)pathname, &ppinfo) != 0) {
        printf("Invalid path!\n");
        return -1; //Invalid path!
    }

    struct dirEntry* temp;
    //Step.3 Look at parent[index], is it a directory
    if (ppinfo.lastElementIndex == -1) {
        if (ppinfo.lastElementName != NULL) {
            freeppinfo();
            return -1; //exit failed
        }
        //At root
        temp = ppinfo.parent;
    } else {
         //Step.4 Load Directory
        temp = loadDir (&ppinfo.parent[ppinfo.lastElementIndex]);
    }
    //step.5 if loadedCWD != loadedRootDiractory - free (loadedCWD)
     if (loadedCWD != loadedRoot && loadedCWD != ppinfo.parent) {
        free(loadedCWD);
    }
    //step.6 loadedCWD = temp
    loadedCWD = temp;

   

    return 0;
}


/****************************************************************************************
*  PWD command and ls command  
****************************************************************************************/

// provides CWD 
char *fs_getcwd(char *pathbuffer, size_t size) {
    // Copy the cwd string back to the buffer with the limit of size 
    return strncpy(pathbuffer, cwdPath, size);
}


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



/****************************************************************************************
*  displayFiles for use by ls command
****************************************************************************************/


struct fs_diriteminfo *fs_readdir(fdDir *dirp) {
    struct dirEntry* entry = dirp->directory;
    while (dirp->dirEntryPosition <= dirp->d_reclen) {
        entry = &dirp->directory[dirp->dirEntryPosition];
        dirp->dirEntryPosition++;
        if (entry->fileName[0] == '\0') {
            continue;
        }
        struct fs_diriteminfo* info = dirp->di;
        strncpy(info->d_name, entry->fileName, 256);
        if (isDirectory(entry)) {
            info->fileType = 'D';
        } else {
            info->fileType = 'F';
        }
        return info;
    }
    return NULL;//End of file or error 
}

int fs_stat(const char *path, struct fs_stat *buf) {
    if (buf == NULL) {
        return -1;
    }
    if (parsePath((char*)path, &ppinfo) != 0) {
        return -1;
    }
    if (ppinfo.lastElementIndex == -1) {
        freeppinfo();
        return -1;
    }
    struct dirEntry* entry = &ppinfo.parent[ppinfo.lastElementIndex];
    
    //Provide necessary information
    if (isDirectory(entry)) {
        buf->st_size = entry->entry_amount * sizeof(struct dirEntry*);
    } else {
        buf->st_size = entry->dir_size;
    }
    buf->st_blksize = MINBLOCKSIZE;
    buf->st_blocks = entry->dir_size;
    buf->st_accesstime = entry->modifyDate;
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
*  Ls commmand
****************************************************************************************/
// Directory iteration functions

fdDir * fs_opendir(const char *pathname) {
    if (parsePath((char*)pathname, &ppinfo) != 0) {
        return NULL;
    }
    
    struct dirEntry* entry;
    //If the directory doesn't exists
    if (ppinfo.lastElementIndex == -1) {
        if (ppinfo.lastElementName != NULL) {
            freeLastElementName();
            freeppinfo();
            return NULL;
        }
        entry = loadDir(ppinfo.parent);
    } else {
        entry = loadDir(&ppinfo.parent[ppinfo.lastElementIndex]);
    }

    //This function doesn't need path info from this point forward
    freeppinfo();

    struct fs_diriteminfo* di = malloc(sizeof(struct fs_diriteminfo));
    if (!di) {
        return NULL;
    }

    fdDir* result = malloc(sizeof(fdDir));
    if (!result) {
        return NULL;
    }

    result->directory = entry;
    result->di = di;
    result->dirEntryPosition = 2; //dot and dot dot take up zero and one
    result->d_reclen = entry->entry_amount - 1;
    printf("result %d/n",entry->entry_amount - 1);
    printf("Can you keep up? %d/n",result->d_reclen);

    return result;
}

//return 1 if directory, 0 otherwise
int fs_isDir(char * pathname) {
    if (parsePath(pathname, &ppinfo) != 0) {
        return -1;
    }

    struct dirEntry* entry = loadedCWD;

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
        printf("You are directory\n");
        freeppinfo();
        return 1;//It's directory
    }
    printf("why are you not directory?\n");
    freeppinfo();
    return 0;
}
/****************************************************************************************
*  Rm commmand
****************************************************************************************/

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
    printf("deallocate from %d index to %d blocks:\n", entry->dir_index,entry->dir_size);
    deallocateSpace(entry->dir_index, entry->dir_size);

    //Set directory as unused in its parent
    entry->fileName[0] = '\0'; //TO DO only array 0 will be null ternimate 

    // Write DIR changes back disk
    LBAwrite(ppinfo.parent, entry->dir_size, entry->dir_index);

    freePathParent();

    printf("I don't think you free sucess\n");
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

    //Free extents associated with file
    freeExtents(entry);  

    //Set directory as unused in its parent
    entry->fileName[0] = '\0'; //TO DO only array 0 will be null ternimate 

    // Write DIR changes back disk
    LBAwrite(ppinfo.parent, entry->dir_size, entry->dir_index);

    
    freePathParent();
    printf("I don't think it was remove\n");
    return 0;
}	

/****************************************************************************************
*  md commmand
****************************************************************************************/

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