/**
   @file DJBHash.cpp
   uCDB standard hash function implementation
   
   Created by Ioulianos Kakoulidis, 2021.
   Released into the public domain.   
*/

#include "DJBHash.h"

unsigned long DJBHash(const void *key, unsigned long keyLen) {
  unsigned long h = 5381;
  const byte *k = (const byte *)key;

  for (unsigned long i = 0; i < keyLen; i++) {
    h = ((h << 5) + h) ^ k[i];
  }

  return h;
}
