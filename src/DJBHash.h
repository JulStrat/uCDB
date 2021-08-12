/**
   @file DJBHash.h
   uCDB standard hash function
   
   Created by Ioulianos Kakoulidis, 2021.
   Released into the public domain.   
*/

#ifndef DJBHash_h
#define DJBHash_h
#include "Arduino.h"

unsigned long DJBHash(const void *key, unsigned long keyLen);

#endif
