/**
   @file DJBHash.cpp
   uCDB standard hash function implementation

   Created by Ioulianos Kakoulidis, 2021.
   Released into the public domain.
*/

#include "DJBHash.h"

unsigned long DJBHash(const void *key, unsigned long keyLen) {
  unsigned long h = 5381;
  const byte *curr = (const byte *)key;
  const byte *end = curr + keyLen;

  while (curr < end) {
    h = ((h << 5) + h) ^ *curr;
    ++curr;
  }

  return h;
}
