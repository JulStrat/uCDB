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
#include <SPI.h>
#include <SD.h>

#define TRACE_CDB
#define USE_UCDB_MAKER
#include "uCDB.hpp"

uCDBMaker<SDClass, File> maker(SD);

unsigned long startMillis;

void printKey(const void *key, unsigned long keyLen) {
  Serial.write((const byte *)key, keyLen);
}

void printValue() {
  int c;

/*
  while ((c = ucdb.readValue()) != -1) {
    Serial.write((byte)c);
  }
*/  
}

void query(const void *key, unsigned long keyLen) {
  cdbResult rt;
  
  /*
  Serial.println();
  Serial.print("Query millisec: ");
  startMillis = millis();
  rt = ucdb.findKey(key, keyLen);
  Serial.println(millis() - startMillis);
  switch (rt) {
    case KEY_FOUND:
      Serial.print("Airport found: ");
      printKey(key, keyLen);
      Serial.println();
      printValue();
      break;
    
    case KEY_NOT_FOUND:
      Serial.print("Airport not found: ");
      printKey(key, keyLen);
      break;
      
    default:
      Serial.println("ERROR");
      break;
  }
  Serial.println();  
  */
}

void setup() {
  // const char fileName[] = "airports.cdb";
  // const char *air[] = {"SBGL", "00AR", "PG-TFI", "US-0480", "ZYGH"};
  // char *key = "key";
  // char *value = "value";
  char key[16];
  cdbResult rt;

  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  
  if (SD.begin(10)) {
    Serial.println("SPI OK.");
  }
  else {
    Serial.println("SPI error.");
    while (true) {
      ;
    }
  }

  rt = maker.init("test");
  Serial.println(rt);
  Serial.print("Records number: "); Serial.println(maker.recordsNumber());  
  
  for (unsigned long i = 0; i < 1000; i += 5) {
    sprintf(key, "%lu", i);
    rt = maker.appendKeyValue(key, strlen(key), key, strlen(key));
    if (rt == CDB_ERROR) {
      Serial.println(i);    
      Serial.println(rt);    
      break;
    }
  }
  Serial.print("Records number: "); Serial.println(maker.recordsNumber());
  
  rt = maker.finalize();
  Serial.println(rt);    

/*
  if (ucdb.open(fileName) == CDB_OK) {
    Serial.print("Total records number: ");
    Serial.println(ucdb.recordsNumber());

    for (unsigned int i = 0; i < sizeof (air) / sizeof (const char *); i++) {
      query(air[i], strlen(air[i]));
    }

    query("AAAA", 4);
    query("BBBB", 4);
    query("CCCC", 4);
    query("YYYY", 4);
  }
  else {
    Serial.print("Invalid CDB: ");
    Serial.println(fileName);
  }
*/
  
}

void loop() {
  String code;
  Serial.println("Enter airport code and press `Enter'");
  while (!Serial.available()) {}
  code = Serial.readString();
}
