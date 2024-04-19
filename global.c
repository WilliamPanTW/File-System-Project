/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: Pan William
* Student IDs:: 922867228
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: global.c
*
* Description:: This file will define the global variable
*
*
**************************************************************/


struct vcb *VCB; //Volume control block pointer
char * fsmap; //pointer for free space(bitmap)

struct dirEntry* rootDir; // Root pointer for mfs.c
struct dirEntry* cwDir; // current working directory for mfs
