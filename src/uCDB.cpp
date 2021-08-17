/**
   @file uCDB.cpp
   uCDB implementation

   Created by Ioulianos Kakoulidis, 2021.
   Released into the public domain.   
*/

#include "uCDB.h"

#define CDB_DESCRIPTOR_SIZE 2 * (sizeof (unsigned long))
#define CDB_HEADER_SIZE 256 * CDB_DESCRIPTOR_SIZE
#define CDB_BUFF_SIZE 64

static unsigned long unpack(const byte buff[]);

uCDB::uCDB() {
  cmp = COMPARE_KEY_EXACTLY;
  state = CDB_CLOSED;
}

cdbResult uCDB::open(const char fileName[], unsigned long (*userHashFunc)(const void *key, unsigned long keyLen)) {
  unsigned long htPos, pos = 0;
  unsigned long htSlotsNum;
  byte buff[CDB_DESCRIPTOR_SIZE];

  cdb.close(); //< Close previously opened CDB file

  if (!SD.exists(fileName)) {
    return (state = CDB_NOT_FOUND);
  }
  cdb = SD.open(fileName, FILE_READ);
  if (!cdb) {
    return (state = FILE_ERROR);
  }
  // CDB file size must be at least HEADER_SIZE bytes
  if (cdb.size() < CDB_HEADER_SIZE) {
    return (state = CDB_ERROR);
  }

  dataEndPos = cdb.size();

  for (int i = 0; i < 256; i++) {
    if (!readDescriptor(buff, pos)) {
      return (state = FILE_ERROR);
    }
    pos += 8;
    htPos = unpack(buff);
    htSlotsNum = unpack(buff + 4);

    if (!htPos) {
      continue; //< Empty hash table
    }
    if ((htPos < CDB_HEADER_SIZE) || ((htPos + htSlotsNum * 8) > cdb.size())) {
      return (state = CDB_ERROR);
    }
    if (htPos < dataEndPos) {
      dataEndPos = htPos;
    }
  }
  hashFunc = userHashFunc;
  return (state = CDB_OK);
}

cdbResult uCDB::findKey(const void *key, unsigned long keyLen) {
  byte buff[CDB_DESCRIPTOR_SIZE];

  // Check CDB state
  switch (state) {
    case CDB_CLOSED:
    case CDB_NOT_FOUND:
    case FILE_ERROR:
    case CDB_ERROR:
      return state;
    default:
      ;
  }

  keyHash = hashFunc(key, keyLen);
  key_ = (const byte *)key;
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

  // Check CDB state
  switch (state) {
    case CDB_CLOSED:
    case CDB_NOT_FOUND:
    case FILE_ERROR:
    case CDB_ERROR:
      return state;
    default:
      ;
  }

  while (slotsToScan) {
    if (!readDescriptor(buff, nextSlotPos)) {
      return (state = FILE_ERROR);
    }

    slotsToScan--;
    nextSlotPos += CDB_DESCRIPTOR_SIZE;
    if (nextSlotPos == hashTabEndPos) {
      nextSlotPos = hashTabStartPos;
    }

    slotHash = unpack(buff);
    dataPos = unpack(buff + 4);

    if (!dataPos) {
      slotsToScan = 0;
      nextSlotPos = 0;
      return (state = KEY_NOT_FOUND);
    }

    // Check data position
    if ((dataPos < CDB_HEADER_SIZE) || (dataPos > (dataEndPos - 8))) {
      return (state = CDB_ERROR);
    }

    if (slotHash == keyHash) {
      if (!readDescriptor(buff, dataPos)) {
        return (state = FILE_ERROR);
      }

      dataKeyLen = unpack(buff);
      dataValueLen = unpack(buff + 4);
      valueBytesAvail = dataValueLen;

      switch (cmp) {
        case COMPARE_HASH_ONLY:
          if (!cdb.seek(dataPos + CDB_DESCRIPTOR_SIZE + dataKeyLen)) {
            return (state = FILE_ERROR);
          }
          else {
            return (state = KEY_FOUND);
          }

        default:
          if (keyLen_ != dataKeyLen) {
            break; // Scan next slot
          }
          if (compareKey() == KEY_FOUND) {
            return (state = KEY_FOUND);
          }
      }
    }
  }

  slotsToScan = 0;
  nextSlotPos = 0;

  return (state = KEY_NOT_FOUND);
}

int uCDB::readValue() {
  if (state != KEY_FOUND) {
    return -1;
  }

  if (valueBytesAvail--) {
    return cdb.read();
  }
  else {
    return -1;
  }
}

int uCDB::readValue(void *buff, unsigned int byteNum) {
  if (state != KEY_FOUND) {
    return -1;
  }

  if (byteNum > valueBytesAvail) {
    byteNum = valueBytesAvail;
  }
  valueBytesAvail -= byteNum;

  return cdb.read(buff, byteNum);
}

void uCDB::compareKeyExactly() {
  cmp = COMPARE_KEY_EXACTLY;
}

void uCDB::compareHashOnly() {
  cmp = COMPARE_HASH_ONLY;
}

cdbResult uCDB::close() {
  cdb.close();
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
    // Compiler warning: comparison between signed and unsigned integer expressions [-Wsign-compare]
    if (cdb.read(buff, keyLen) != keyLen) {
      return FILE_ERROR;
    }
    if (memcmp(key, buff, keyLen)) {
      return KEY_NOT_FOUND;
    }
  }

  return KEY_FOUND;
}

bool uCDB::readDescriptor(byte buff[], unsigned long pos) {
  if (cdb.position() != pos) {
    if (!cdb.seek(pos)) {
      return false;
    }
  }

  if (cdb.read(buff, CDB_DESCRIPTOR_SIZE) == CDB_DESCRIPTOR_SIZE) {
    return true;
  }
  else {
    return false;
  }
}

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

// Static functions

unsigned long unpack(const byte buff[]) {
  unsigned long v = buff[3];

  v = (v << 8) + buff[2];
  v = (v << 8) + buff[1];
  v = (v << 8) + buff[0];

  return v;
}

#undef CDB_HEADER_SIZE
#undef CDB_DESCRIPTOR_SIZE
#undef CDB_BUFF_SIZE
