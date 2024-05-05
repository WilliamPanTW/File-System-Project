/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: John Cuevas, Michael Abolencia , Pan William , Tina Chou
* Student IDs:: 920542932, 917581956, 922867228 , 922911207
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
#include "mfs.h"
#include "fsInit.h"
#include "bitmap.h"

struct vcb *VCB; //Volume control block pointer
char * fsmap; //pointer for free space(bitmap)

struct pp_return_struct ppinfo; //global parse path info

struct dirEntry* loadedRoot; // Root pointer for mfs.c
struct dirEntry* loadedCWD; // current working directory for mfs
