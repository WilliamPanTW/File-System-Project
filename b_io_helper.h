// /**************************************************************
// * Class::  CSC-415-01 Spring 2024
// * Name:: Pan William
// * Student IDs:: 922867228
// * GitHub-Name:: WilliamPanTW
// * Group-Name:: JMWT
// * Project:: Basic File System
// *
// * File:: b_io_helper.h
// *
// * Description:: This header file provide necessary function for b_io.c
// *
// **************************************************************/

#ifndef _B_IO_HELPER_H
#define _B_IO_HELPER_H

#include "mfs.h"  // for ppinfo strucuture 
#include "global.h" //for global parse path info

int createFile(struct pp_return_struct* info);

#endif
