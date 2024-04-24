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
// char *cwdString = cwdPath; // global variable for the current working directory string


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
*  Ls commmand
****************************************************************************************/

//opens a directory stream corresponding to the directory name
fdDir * fs_opendir(const char *pathname) {
    //check path name is valid or not  
    if (parsePath((char*)pathname, &ppinfo) != 0) {
        return NULL; //invalid path 
    }
    
    struct dirEntry* entry;
    //If the pare path index equal to specified root from parepath func
    if (ppinfo.lastElementIndex == -2) {
        // And does not have a name 
        if (ppinfo.lastElementName != NULL) {
            freeLastElementName();
            freeppinfo();
            return NULL;//otherwise free and return null
        }
        //Then root Load directory entry itself 
        entry = loadDir(ppinfo.parent);
    } else { //Else pare path return 0 mean it is directory 
        // Load directory entry for specified parent index 
        entry = loadDir(&ppinfo.parent[ppinfo.lastElementIndex]);
    }

    freeppinfo();
    // Allocate memory for directory item info to provide caller with information
    struct fs_diriteminfo* dirItemInfo = malloc(sizeof(struct fs_diriteminfo));
    if (!dirItemInfo) {
        return NULL;//fail to allocated
    }

    // Allocate memory for fdDir structure to keep track current process
    fdDir* dirStream = malloc(sizeof(fdDir));
    if (!dirStream) {
        return NULL;//fail to allocated
    }

    //initial struct fdDir 
    dirStream->dirEntryPosition = 2; //dot and dot dot take up zero and one
    dirStream->directory = entry;//point to the loaded directory 
    dirStream->di = dirItemInfo; //pointer have return from read
    dirStream->dirEntriesCount = entry->entry_amount - 1;
    
    //returns a pointer to the directory stream. 
    return dirStream;
}

//return 1 if directory, 0 otherwise
int fs_isDir(char * pathname) {
    if (parsePath(pathname, &ppinfo) != 0) {
        return -1;
    }

    struct dirEntry* entry = loadedCWD;

    //Check if the parent directory doesn't exists
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
        // printf("You are directory\n");
        freeppinfo();
        return 1;//It's directory
    }
    freeppinfo();
    return 0;
}

/****************************************************************************************
*  cd command 
****************************************************************************************/

// Misc directory functions
// Part1 file system needs 
int fs_setcwd(char *pathname){

    //Step.1 take the path specified to temp buffer
    char tempPathName[MAX_FILENAME_LENGTH];
    strncpy(tempPathName, pathname, MAX_FILENAME_LENGTH);

    //Step.2 call parsepath to check if it valid
    if (parsePath((char*)pathname, &ppinfo) != 0) {
        return -1; //Invalid path
    }

    //Step.3 Check if the parent directory doesn't exists
    if (ppinfo.lastElementIndex == -1) {
        freeppinfo();
        return -1; //exit it does not exist
    } 

    //step.4 Look at parent[index], is it a directory
     if (!isDirectory(&ppinfo.parent[ppinfo.lastElementIndex])) {
        freeppinfo();
        return -1; //exit it is not directory
    }

    //Step.5 Load Directory
    struct dirEntry* temp;
    temp = ppinfo.parent;
    temp = loadDir (&ppinfo.parent[ppinfo.lastElementIndex]);

    //***************Part.2 updating the string for the user****************

    //If start with absolute path("/"), copy the path
    if (tempPathName[0] == '/') {
        strncpy(cwdPath, tempPathName, MAX_FILENAME_LENGTH);// just copy path
    } else { 
        // If the path is relative
        strcat(cwdPath, tempPathName);//Concate with current working directory  
    }
    

    char* saveptr;//track position of token
    char* list[(MAX_FILENAME_LENGTH / 2) + 1];//vector table

    char* currentToken  = strtok_r(cwdPath, "/", &saveptr);
    char* token;//current tokenized substring

    int listIndex = 0;
    while (currentToken  != NULL) {

    token = currentToken ;
    currentToken  = strtok_r(NULL, "/", &saveptr);

    // If token is ".", ignore it
    if (strcmp(token, ".") == 0) {
        continue;
    } else if (strcmp(token, "..") == 0) {
        if (listIndex > 0) {
            listIndex -= 1;//If it's "dot dot"go back one directory
        }
        continue; //unless already at the root
    }
    list[listIndex] = token;//add in to the array of integers 
    listIndex++;//increment index
    }
    
    // build the new path based on processed
    char newPath[MAX_FILENAME_LENGTH / 2] = "/";
    for (int i = 0; i < listIndex; i++) {
        char* element = list[i];
        strcat(newPath, element);
        if (i < listIndex - 1) {
            strcat(newPath, "/");
        }
    }
    strncpy(cwdPath, newPath, MAX_FILENAME_LENGTH);

    // Ensure the current working directory ends with "/"
    if (cwdPath[strlen(cwdPath)-1] != '/') {
    // If last character is not "/", append "/"
        strcat(cwdPath, "/");    
    }


    //step.6 Check CWD is neither root directory nor parent directory
    if (loadedCWD != loadedRoot&& loadedCWD != ppinfo.parent) {
        free(loadedCWD);
    }

    //step.7 loadedCWD = temp
    loadedCWD = temp;

    char *dummy[] = {"d", NULL}; 
    cmd_pwd (1, dummy);//dummy to Prints the working directory
    freeppinfo();
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

    //Check if the parent directory doesn't exists
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
*  Rm commmand
****************************************************************************************/

int fs_rmdir(const char *pathname) {
    int result = parsePath((char*)pathname, &ppinfo);
     if(result == -1){ 
        return -1; //invalid path
    }

    freeLastElementName();
    //Check if the parent directory doesn't exists
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
    //Check if the parent directory doesn't exists
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

    //compare the new directory is exist or not
    if(ppinfo.lastElementIndex!=-1){
        printf("mkdir: cannot create directory '%s' : File exists\n",pathname);
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