/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: John Cuevas, Michael Abolencia , Pan William , Tina Chou
* Student IDs:: 920542932, 917581956, 922867228 , 922911207
* GitHub-Name:: WilliamPanTW
* Group-Name:: JMWT
* Project:: Basic File System
*
* File:: b_io_helper.h
*
* Description:: This header file provide necessary function for b_io.c
*
**************************************************************/

#ifndef _B_IO_HELPER_H
#define _B_IO_HELPER_H

#include "mfs.h"  // for ppinfo strucuture 
#include "global.h" //for global parse path info

int createFile(struct pp_return_struct* info);

int moveFile(const char *src, const char *dest);

int copyFile(struct pp_return_struct* info, struct dirEntry* src);

#endif
