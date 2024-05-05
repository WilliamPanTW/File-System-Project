/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: John Cuevas, Michael Abolencia , Pan William , Tina Chou
* Student IDs:: 920542932, 917581956, 922867228 , 922911207
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: global.h
*
* Description:: This header file share the global variable that provide 
* other file to access prevent multiple declaration throughout the project.
*
**************************************************************/

#ifndef _GLOBAL_H
#define _GLOBAL_H

extern struct pp_return_struct ppinfo; //global parse path info

extern struct vcb *VCB; // Volume control block pointer
extern char * fsmap; //pointer for free space(bitmap)

extern struct dirEntry* loadedRoot; // store memory address of root directory 
extern struct dirEntry* loadedCWD; // store memory address of current working directory 

#endif
