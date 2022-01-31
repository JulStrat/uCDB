/*
  uCDBMaker example sketch
  
  Connect to Ardino board. Compile and upload sketch. Open Serial Monitor.
  
  Board: Arduino Uno
  SD card: SDHC 7.31Gb FAT32, sectorsPerCluster - 64
  SD chip select pin: 10
  Arduino IDE Serial Monitors settings: 115200 baud, no line ending.

  Created by Ioulianos Kakoulidis, 2022.
  Released into the public domain.     
*/

#include "SdFat.h"

#define SD_CS_PIN 10

#define TRACE_CDB
#define USE_UCDB_MAKER
#include "uCDB.hpp"

SdFat fat;
uCDBMaker<SdFat, File> maker(fat);
uCDB<SdFat, File> ucdb(fat);

unsigned long startMillis;

void printKey(const void *key, unsigned long keyLen) {
  Serial.write((const byte *)key, keyLen);
}

void printValue() {
  int c;

  while ((c = ucdb.readValue()) != -1) {
    Serial.write((byte)c);
  }
}

void query(const void *key, unsigned long keyLen) {
  cdbResult rt;
  
  Serial.println();
  Serial.print("Query millisec: ");
  startMillis = millis();
  rt = ucdb.findKey(key, keyLen);
  Serial.println(millis() - startMillis);
  switch (rt) {
    case KEY_FOUND:
      Serial.print("Key found: ");
      printKey(key, keyLen);
      Serial.println();
      printValue();
      break;
    
    case KEY_NOT_FOUND:
      Serial.print("Key not found: ");
      printKey(key, keyLen);
      break;
      
    default:
      Serial.println("ERROR");
      break;
  }
  Serial.println();  
}

void setup() {
  char key[16];
  cdbResult rt;

  Serial.begin(115200);
  while (!Serial) {
    yield();
  }

  if (!fat.begin(SD_CS_PIN)) {
    fat.initErrorHalt(&Serial);
    return;
  }
  
  Serial.println("Creating CDB file - testCDB...\nRange [0, 10000), step - 5.\nPlease wait!");
  rt = maker.init("testCDB");
  
  if (rt == CDB_ERROR) {
    Serial.println("Init() error");
    return;    
  }
  
  startMillis = millis();
  for (unsigned long i = 0; i < 10000; i += 5) {
    sprintf(key, "%lu", i);
    rt = maker.appendKeyValue(key, strlen(key), key, strlen(key));
    if (rt == CDB_ERROR) {
      Serial.print("appendKeyValue error: "); Serial.println(key);
      return;
    }
  }
  Serial.print("Records number: "); Serial.println(maker.recordsNumber());
  Serial.println(millis() - startMillis);
  
  startMillis = millis();
  rt = maker.finalize();
  Serial.println(millis() - startMillis);  
  Serial.println(rt);    
  Serial.println("testCDB created.");    
  
  if (ucdb.open("testCDB.cdb") == CDB_OK) {
    Serial.print("Total records number: ");
    Serial.println(ucdb.recordsNumber());
  }
  else {
    Serial.println("Invalid CDB: testCDB.cdb");
    return;
  }
}

void loop() {
  String code;
  Serial.println("Enter key and press `Enter'");
  while (!Serial.available()) {}
  code = Serial.readString();
  query(code.c_str(), code.length());  
}
