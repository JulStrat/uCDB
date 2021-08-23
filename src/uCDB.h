/**
   @file uCDB.h
   uCDB API

   Created by Ioulianos Kakoulidis, 2021.
   Released into the public domain.   
*/

#ifndef uCDB_h
#define uCDB_h

#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

enum cdbResult {
  CDB_OK = 0,
  CDB_CLOSED, // Initial state
  CDB_NOT_FOUND,
  CDB_ERROR,  // CDB data integrity error
  FILE_ERROR, // File operation (open/seek/read) error
  KEY_FOUND,
  KEY_NOT_FOUND
};

unsigned long DJBHash(const void *key, unsigned long keyLen);

class uCDB
{
  public:
    uCDB();

    /**
        Open CDB file
        @param[in] fileName  CDB filename
        @param[in] userHashFunc  User provided hash function, default - DJBHash
    */
    cdbResult open(const char fileName[], unsigned long (*userHashFunc)(const void *key, unsigned long keyLen) = DJBHash);

    /**
        Find `key'
    */
    cdbResult findKey(const void *key, unsigned long keyLen);

    /**
        Find next `value' after successful finKey() call
    */
    cdbResult findNextValue();

    /**
        Read next available `value' byte 
        after successful finKey() or findNextValue() call
    */
    int readValue();

    /**
        Read next available `value' byteNum bytes 
        after successful finKey() or findNextValue() call
    */
    int readValue(void *buff, unsigned int byteNum);

    /**
        Close CDB
    */
    cdbResult close();

  private:
    File cdb;
    cdbResult state;

    const byte *key_;
    unsigned long keyLen_;
    unsigned long keyHash;

    unsigned long dataEndPos;
    unsigned long slotsNum;

    //> Hash table descriptor (HEADER section)
    unsigned long hashTabStartPos;
    unsigned long hashTabSlotsNum;
    //< Hash table descriptor (HEADER section)
    unsigned long hashTabEndPos; // hashTabStartPos + 8 * hashTabSlotsNum

    //> Slot descriptor (HASH TABLE section)
    unsigned long slotHash;
    unsigned long dataPos;
    //< Slot descriptor (HASH TABLE section)

    unsigned long slotsToScan;
    unsigned long nextSlotPos;

    //> Data descriptor (DATA section)
    unsigned long dataKeyLen;
    unsigned long dataValueLen;
    //< Data descriptor (DATA section)
    unsigned long valueBytesAvail;

    bool readDescriptor(byte buff[], unsigned long pos);
    cdbResult compareKey();
    unsigned long (*hashFunc)(const void *key, unsigned long keyLen);
};

#endif
