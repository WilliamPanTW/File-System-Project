/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
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

extern struct dirEntry* rootDir; // Root pointer for mfs.c
extern struct dirEntry* cwDir; // current working directory for mfs.c

#endif
