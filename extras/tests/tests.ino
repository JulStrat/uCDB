/*
  uCDB test sketch

  Board: Arduino Uno
  SD card: SDHC 7.31Gb FAT32, sectorsPerCluster - 64
  SD chip select pin: 10
  Arduino IDE Serial Monitors settings: 115200 baud, no line ending.

  Created by Ioulianos Kakoulidis, 2021.
  Released into the public domain.
*/

#include "SdFat.h"

#define TRACE_CDB
#include "uCDB.hpp"

#define RAND_NAME "___45___.$$$"

SdFat fat;

bool gen_random(unsigned long sz) {
  long v, p = 8 * sz - 2056;
  
  File rf = fat.open(RAND_NAME, O_CREAT | O_WRITE | O_TRUNC);

  if (!rf) {
    return false;
  }
  randomSeed(random(0x7FFFFFFF));

  for(; sz > 0; --sz) {
    v = random(2048, p);
    rf.write(&v, sizeof (long));
    v = random(16);
    rf.write(&v, sizeof (long));
  }
  rf.close();

  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  if (!fat.begin(10)) {
    Serial.println("SD card initialization failed!");
    while (true) {
      ;
    }
  }
}

bool closed_test1() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (ucdb.findNextValue() != CDB_CLOSED)
    return false;
  if (ucdb.findKey("AAA", 3) != CDB_CLOSED)
    return false;
  if (ucdb.findNextValue() != CDB_CLOSED)
    return false;

  if (ucdb.readValue() != -1)
    return false;
  if (ucdb.readValue(buff, 64) != -1)
    return false;
  if (ucdb.valueAvailable() != 0)
    return false;
  if (ucdb.recordsNumber() != 0)
    return false;
  return true;
}

bool closed_test2() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (ucdb.open("XXXXXXXXXXXXXXXXXXXXX.$$$" ) != CDB_NOT_FOUND) {
    ucdb.close();
    return false;
  }

  if (ucdb.findNextValue() != CDB_CLOSED)
    return false;
  if (ucdb.findKey("AAA", 3) != CDB_CLOSED)
    return false;
  if (ucdb.findNextValue() != CDB_CLOSED)
    return false;

  if (ucdb.readValue() != -1)
    return false;
  if (ucdb.readValue(buff, 64) != -1)
    return false;
  if (ucdb.valueAvailable() != 0)
    return false;
  if (ucdb.recordsNumber() != 0)
    return false;
  return true;
}

bool closed_test3() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (ucdb.open("airports.cdb" ) != CDB_OK) {
    ucdb.close();
    return false;
  }
  ucdb.close();

  if (ucdb.findNextValue() != CDB_CLOSED)
    return false;
  if (ucdb.findKey("ZYGH", 3) != CDB_CLOSED)
    return false;
  if (ucdb.findNextValue() != CDB_CLOSED)
    return false;
  if (ucdb.findKey("AAAA", 3) != CDB_CLOSED)
    return false;

  if (ucdb.readValue() != -1)
    return false;
  if (ucdb.readValue(buff, 64) != -1)
    return false;
  if (ucdb.valueAvailable() != 0)
    return false;
  if (ucdb.recordsNumber() != 0)
    return false;
  return true;
}

int read_value_without_find_test() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (ucdb.open("airports.cdb" ) != CDB_OK) {
    ucdb.close();
    return -1;
  }

  if (ucdb.state() != CDB_OK)
    return -100;

  if (ucdb.valueAvailable() != 0)
    return -2;
  if (ucdb.readValue() != -1)
    return -3;
  if (ucdb.readValue(buff, 1) != -1)
    return -4;

  return 1;
}

int read_value_find_fail_test() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (ucdb.open("airports.cdb" ) != CDB_OK) {
    ucdb.close();
    return -1;
  }

  if (ucdb.state() != CDB_OK)
    return -100;

  if (ucdb.findKey("AAAAAAAA", 8) != KEY_NOT_FOUND)
    return -2;

  if (ucdb.valueAvailable() != 0)
    return -3;
  if (ucdb.readValue() != -1)
    return -4;
  if (ucdb.readValue(buff, 1) != -1)
    return -5;

  return 1;
}

int read_value_find_ok_test() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (ucdb.open("airports.cdb" ) != CDB_OK) {
    ucdb.close();
    return -1;
  }

  if (ucdb.state() != CDB_OK)
    return -100;

  if (ucdb.findKey("ZYGH", 4) != KEY_FOUND)
    return -2;

  if (ucdb.valueAvailable() <= 0)
    return -3;
  
  while (ucdb.readValue() >= 0);

  if (ucdb.valueAvailable() != 0)
    return -4;
  
  if (ucdb.readValue() != -1)
    return -5;
  if (ucdb.readValue(buff, 1) != 0)
    return -6;
  if (ucdb.valueAvailable() != 0)
    return -7;

  return 1;
}

int read_value_reopen_after_find_test() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (ucdb.open("airports.cdb" ) != CDB_OK) {
    ucdb.close();
    return -1;
  }

  if (ucdb.state() != CDB_OK)
    return -100;

  if (ucdb.findKey("SSS", 3) != KEY_FOUND)
    return -2;

  if (ucdb.open("airports.cdb" ) != CDB_OK) {
    ucdb.close();
    return -101;
  }

  if (ucdb.state() != CDB_OK)
    return -102;

  if (ucdb.valueAvailable() != 0)
    return -3;
  
  if (ucdb.readValue() != -1)
    return -4;
  if (ucdb.readValue(buff, 1) != -1)
    return -5;

  return 1;
}

bool random_test1() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (!gen_random(10)) {
    return false;    
  }

  if (ucdb.open(RAND_NAME) != CDB_ERROR) {
    ucdb.close();
    return false;
  }

  if (ucdb.findNextValue() != CDB_ERROR)
    return false;
  if (ucdb.findKey("ZYGH", 3) != CDB_ERROR)
    return false;
  if (ucdb.findNextValue() != CDB_ERROR)
    return false;
  if (ucdb.findKey("AAAA", 3) != CDB_ERROR)
    return false;

  if (ucdb.readValue() != -1)
    return false;
  if (ucdb.readValue(buff, 64) != -1)
    return false;
  if (ucdb.valueAvailable() != 0)
    return false;
  if (ucdb.recordsNumber() != 0)
    return false;
  return true;
}


bool random_test2() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (!gen_random(512)) {
    return false;    
  }

  if (ucdb.open(RAND_NAME) != CDB_ERROR) {
    ucdb.close();
    return false;
  }

  if (ucdb.findNextValue() != CDB_ERROR)
    return false;
  if (ucdb.findKey("ZYGH", 3) != CDB_ERROR)
    return false;
  if (ucdb.findNextValue() != CDB_ERROR)
    return false;
  if (ucdb.findKey("AAAA", 3) != CDB_ERROR)
    return false;

  if (ucdb.readValue() != -1)
    return false;
  if (ucdb.readValue(buff, 64) != -1)
    return false;
  if (ucdb.valueAvailable() != 0)
    return false;
  if (ucdb.recordsNumber() != 0)
    return false;
  return true;
}

bool random_test3() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (!gen_random(1024*4)) {
    return false;    
  }

  if (ucdb.open(RAND_NAME) != CDB_ERROR) {
    ucdb.close();
    return false;
  }

  if (ucdb.findNextValue() != CDB_ERROR)
    return false;
  if (ucdb.findKey("ZYGH", 3) != CDB_ERROR)
    return false;
  if (ucdb.findNextValue() != CDB_ERROR)
    return false;
  if (ucdb.findKey("AAAA", 3) != CDB_ERROR)
    return false;

  if (ucdb.readValue() != -1)
    return false;
  if (ucdb.readValue(buff, 64) != -1)
    return false;
  if (ucdb.valueAvailable() != 0)
    return false;
  if (ucdb.recordsNumber() != 0)
    return false;
  return true;
}

bool random_test4() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (!gen_random(1024*8)) {
    return false;    
  }

  if (ucdb.open(RAND_NAME) != CDB_ERROR) {
    ucdb.close();
    return false;
  }

  if (ucdb.findNextValue() != CDB_ERROR)
    return false;
  if (ucdb.findKey("ZYGH", 3) != CDB_ERROR)
    return false;
  if (ucdb.findNextValue() != CDB_ERROR)
    return false;
  if (ucdb.findKey("AAAA", 3) != CDB_ERROR)
    return false;

  if (ucdb.readValue() != -1)
    return false;
  if (ucdb.readValue(buff, 64) != -1)
    return false;
  if (ucdb.valueAvailable() != 0)
    return false;
  if (ucdb.recordsNumber() != 0)
    return false;
  return true;
}

bool random_test5() {
  uCDB<SdFat, File> ucdb(fat);
  byte buff[64];

  if (!gen_random(7)) {
    return false;    
  }

  if (ucdb.open(RAND_NAME) != CDB_ERROR) {
    ucdb.close();
    return false;
  }

  if (ucdb.findKey("ZYGH", 3) != CDB_ERROR)
    return false;
  if (ucdb.findNextValue() != CDB_ERROR)
    return false;
  if (ucdb.findKey("AAAA", 3) != CDB_ERROR)
    return false;
  if (ucdb.findNextValue() != CDB_ERROR)
    return false;

  if (ucdb.recordsNumber() != 0)
    return false;
  if (ucdb.readValue(buff, 64) != -1)
    return false;
  if (ucdb.readValue() != -1)
    return false;
  if (ucdb.valueAvailable() != 0)
    return false;

  return true;
}

void loop() {
  char str[16];
  long key;
  unsigned long startMillis;
  cdbResult rt;
  int br;

  Serial.println("Press any key to start test");
  while (!Serial.available()) {
    ;
  }

  Serial.println(closed_test1());
  Serial.println(closed_test2());
  Serial.println(closed_test3());
  
  Serial.println("read_value_without_find_test");
  Serial.println(read_value_without_find_test());
  
  Serial.println("read_value_find_fail_test");
  Serial.println(read_value_find_fail_test());

  Serial.println("read_value_find_ok_test");
  Serial.println(read_value_find_ok_test());

  Serial.println("read_value_reopen_after_find_test");
  Serial.println(read_value_reopen_after_find_test());  
  
  Serial.println(random_test1());
  Serial.println(random_test2());
  Serial.println(random_test3());
  Serial.println(random_test4());
  Serial.println(random_test5());  

  while (Serial.available()) {
    Serial.read();
  }
}