/**
   @file uCDB.cpp
   uCDB implementation

   @author    Ioulianos Kakoulidis
   @date      2021   
   @copyright Public Domain
*/

#include "uCDB.h"

#define CDB_DESCRIPTOR_SIZE 2 * (sizeof (unsigned long))
#define CDB_HEADER_SIZE 256 * CDB_DESCRIPTOR_SIZE
#define CDB_BUFF_SIZE 64

static unsigned long unpack(const byte *buff);

uCDB::uCDB() {
  zero();  
  state = CDB_CLOSED;
}

cdbResult uCDB::open(const char *fileName, unsigned long (*userHashFunc)(const void *key, unsigned long keyLen)) {
  unsigned long htPos;
  unsigned long htSlotsNum;

  unsigned long dend;
  unsigned long snum;
  
  byte buff[CDB_DESCRIPTOR_SIZE];
  
  zero();
  if (cdb) {
    cdb.close(); // Close previously opened CDB file
  }

  if (!SD.exists(fileName)) {
    return (state = CDB_NOT_FOUND);
  }
  cdb = SD.open(fileName, FILE_READ);
  if (!cdb) {
    return (state = CDB_CLOSED);
  }
  
  // CDB hash tables position and slots number integrity check
  
  // CDB file size must be at least HEADER_SIZE bytes
  if (cdb.size() < CDB_HEADER_SIZE) {
    return (state = CDB_ERROR);
  }

  dend = cdb.size();
  snum = 0;

  for (unsigned long pos = 0; pos < CDB_HEADER_SIZE; pos += CDB_DESCRIPTOR_SIZE) {
    if (!readDescriptor(buff, pos)) {
      return (state = CDB_ERROR); // File read error is critical here.
    }

    htPos = unpack(buff);
    htSlotsNum = unpack(buff + 4);

    if (!htPos) {
      continue; // Empty hash table
    }
    if ((htPos < CDB_HEADER_SIZE) || (htPos > cdb.size())) {
      return (state = CDB_ERROR); // Critical CDB format or data integrity error          
    }
    if (((cdb.size() - htPos) >> 3) < htSlotsNum) {
      return (state = CDB_ERROR); // Critical CDB format or data integrity error          
    }
    
    // Adjust data end position and total slots number
    if (htPos < dend) {
      dend = htPos;
    }
    snum += htSlotsNum;

    if (((cdb.size() - dend) >> 3) < snum) {
      return (state = CDB_ERROR); // Critical CDB format or data integrity error          
    }
  }
  // Check total
  if ((cdb.size() - dend) != 8 * snum){
    return (state = CDB_ERROR); // Critical CDB format or data integrity error           
  }
  
  dataEndPos = dend;
  slotsNum = snum;
  hashFunc = userHashFunc;
  return (state = CDB_OK);
}

cdbResult uCDB::findKey(const void *key, unsigned long keyLen) {
  byte buff[CDB_DESCRIPTOR_SIZE];

  zero();
  // Check CDB state
  switch (state) {
    case CDB_CLOSED:
    case CDB_NOT_FOUND:
    case CDB_ERROR:
      return state;
    default:
      ;
  }

  keyHash = hashFunc(key, keyLen);
  key_ = static_cast<const byte *>(key);
  keyLen_ = keyLen;

  if (!readDescriptor(buff, (keyHash & 255) << 3)) {
    return (state = FILE_ERROR);
  }

  hashTabStartPos = unpack(buff);
  hashTabSlotsNum = unpack(buff + 4);
  hashTabEndPos = hashTabStartPos + hashTabSlotsNum * CDB_DESCRIPTOR_SIZE;
  slotsToScan = hashTabSlotsNum;
  nextSlotPos = hashTabStartPos + ((keyHash >> 8) % hashTabSlotsNum) * 8;

  return findNextValue();
}

cdbResult uCDB::findNextValue() {
  byte buff[CDB_BUFF_SIZE];
  bool rd;

  // Check CDB state
  switch (state) {
    case CDB_CLOSED:
    case CDB_NOT_FOUND:
    case CDB_ERROR:
      return state;
    default:
      ;
  }

  while (slotsToScan) {
    rd = readDescriptor(buff, nextSlotPos);  
    // Adjust slotsToScan and next slot position
    --slotsToScan;
    nextSlotPos += CDB_DESCRIPTOR_SIZE;
    if (nextSlotPos == hashTabEndPos) {
      nextSlotPos = hashTabStartPos;
    }
    
    if (!rd) {
      return (state = FILE_ERROR);
    }

    slotHash = unpack(buff);
    dataPos = unpack(buff + 4);

    if (!dataPos) {
      zero();
      return (state = KEY_NOT_FOUND);
    }

    // Check data position
    if ((dataPos < CDB_HEADER_SIZE) || (dataPos > (dataEndPos - CDB_DESCRIPTOR_SIZE))) {
      return (state = CDB_ERROR); // Critical CDB format or data integrity error          
    }

    if (slotHash == keyHash) {
      if (!readDescriptor(buff, dataPos)) {
        return (state = FILE_ERROR);
      }

      dataKeyLen = unpack(buff);
      dataValueLen = unpack(buff + 4);
      
      //> key, value length check
      unsigned long t = dataPos + CDB_DESCRIPTOR_SIZE;
      if ((dataEndPos - t) < dataKeyLen) {
        return (state = CDB_ERROR); // Critical CDB format or data integrity error          
      }
      t += dataKeyLen;
      if ((dataEndPos - t) < dataValueLen) {
        return (state = CDB_ERROR); // Critical CDB format or data integrity error          
      }
      //< key, value length check

      if (keyLen_ == dataKeyLen) {
        switch (compareKey()) {
          case KEY_FOUND:
            valueBytesAvail = dataValueLen;          
            return (state = KEY_FOUND);             
          case FILE_ERROR:
            return (state = FILE_ERROR);             
          default:
            ;          
        }
      }
    }
  }

  zero(); // ?

  return (state = KEY_NOT_FOUND);
}

int uCDB::readValue() {
  if ((state == KEY_FOUND) && valueBytesAvail) {
    int rt = cdb.read();
    if (rt != -1) {
      --valueBytesAvail;
    }
    return rt;
  }

  return -1;
}

int uCDB::readValue(void *buff, unsigned int byteNum) {
  if (state == KEY_FOUND) {
    if (byteNum > valueBytesAvail) {
      byteNum = valueBytesAvail;
    }
    int br = cdb.read(buff, byteNum);
    if (br > 0) {
      valueBytesAvail -= br;  
    }
    return br;
  }

  return -1;
}

unsigned long uCDB::recordsNumber() {
  return (slotsNum >> 1);  
}

unsigned long uCDB::valueAvailable() {
  return ((state == KEY_FOUND) ? valueBytesAvail : 0);
}

cdbResult uCDB::close() {
  zero();  
  if (cdb) {
    cdb.close();
  }
  return (state = CDB_CLOSED);
}

// Private functions

cdbResult uCDB::compareKey() {
  const byte *key = key_;
  unsigned long keyLen = keyLen_;
  byte buff[CDB_BUFF_SIZE];

  while (keyLen >= CDB_BUFF_SIZE) {
    if (cdb.read(buff, CDB_BUFF_SIZE) != CDB_BUFF_SIZE) {
      return FILE_ERROR;
    }
    if (memcmp(key, buff, CDB_BUFF_SIZE)) {
      return KEY_NOT_FOUND;
    }
    keyLen -= CDB_BUFF_SIZE;
    key += CDB_BUFF_SIZE;
  }

  // keyLen < CDB_BUFF_SIZE
  if (keyLen) {
    if (cdb.read(buff, keyLen) != (int)keyLen) {
      return FILE_ERROR;
    }
    if (memcmp(key, buff, keyLen)) {
      return KEY_NOT_FOUND;
    }
  }

  return KEY_FOUND;
}

bool uCDB::readDescriptor(byte *buff, unsigned long pos) {
  if (cdb.position() != pos) {
    if (!cdb.seek(pos)) {
      return false;
    }
  }

  return (cdb.read(buff, CDB_DESCRIPTOR_SIZE) == CDB_DESCRIPTOR_SIZE);
}

void uCDB::zero() {
  dataEndPos = 0;
  slotsNum = 0;
    
  slotsToScan = 0;
  nextSlotPos = 0;
}

#define DJB_START_HASH 5381UL  
unsigned long DJBHash(const void *key, unsigned long keyLen) {
  unsigned long h = DJB_START_HASH;
  const byte *curr = static_cast<const byte *>(key);
  const byte *end = curr + keyLen;

  while (curr < end) {
    h = ((h << 5) + h) ^ *curr;
    ++curr;
  }

  return h;
}
#undef DJB_START_HASH

// Static functions

unsigned long unpack(const byte *buff) {
  unsigned long v = buff[3];

  v = (v << 8) + buff[2];
  v = (v << 8) + buff[1];
  v = (v << 8) + buff[0];

  return v;
}

#undef CDB_HEADER_SIZE
#undef CDB_DESCRIPTOR_SIZE
#undef CDB_BUFF_SIZE
