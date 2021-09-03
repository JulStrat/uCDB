[![arduino-lint](https://github.com/JulStrat/uCDB/actions/workflows/arduino-lint.yml/badge.svg)](https://github.com/JulStrat/uCDB/actions/workflows/arduino-lint.yml)
[![GitHub license](https://img.shields.io/github/license/JulStrat/uCDB)](https://github.com/JulStrat/uCDB/blob/master/LICENSE.md)

# uCDB

Arduino library for querying [Constant DataBase](https://en.wikipedia.org/wiki/Cdb_(software)) (key, value) store.
Simple, :cyclone: fast and portable CDB file format was developed by D. J. Bernstein.

Compatible storage libraries:
- official [Arduino SD](https://github.com/arduino-libraries/SD)
- [Greiman SdFat](https://github.com/greiman/SdFat)
- [SdFat - Adafruit fork](https://github.com/adafruit/SdFat)
- [Adafruit SPIFlash](https://github.com/adafruit/Adafruit_SPIFlash)

## API

```C++
cdbResult open(const char *fileName, unsigned long (*userHashFunc)(const void *key, unsigned long keyLen) = DJBHash);

cdbResult findKey(const void *key, unsigned long keyLen);

cdbResult findNextValue();

int readValue();

int readValue(void *buff, unsigned int byteNum);

unsigned long recordsNumber();

unsigned long valueAvailable();

cdbResult close();
```    

States transitions

<img src="https://github.com/JulStrat/uCDB/blob/master/docs/uCDB_state.png>

## Usage examples

`examples` folder contains `airports.ino` Arduino IDE sketch, Python converter `airports.py` script and data files.

<img src="https://github.com/JulStrat/uCDB/blob/master/examples/airports/airports.png">

`benchmark` sketch.

<img src="https://github.com/JulStrat/uCDB/blob/master/examples/benchmark/benchmark.png">

## License

`uCDB` source code released into the public domain.

## Links

- [CDB](https://cr.yp.to/cdb.html)
- [airports](https://ourairports.com/data/)
