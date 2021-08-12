# uCDB

Arduino library for querying [Constant DataBase](https://en.wikipedia.org/wiki/Cdb_(software)) (key, value) store.
Simple, fast and portable CDB file format was developed by D. J. Bernstein.

## API

```C++
    cdbResult open(const char fileName[], unsigned long (*userHashFunc)(const void *key, unsigned long keyLen) = DJBHash);

    cdbResult findKey(const void *key, unsigned long keyLen);

    cdbResult findNextValue();

    int readValue();

    int readValue(void *buff, unsigned int byteNum);

    void compareKeyExactly();

    void compareHashOnly();

    cdbResult close();
```    

## Usage example

`examples` folder contains `airports.ino` Arduino IDE sketch and data files.

<img src="https://github.com/JulStrat/uCDB/blob/master/examples/airports/airports.png">

## License

`uCDB` source code released into the public domain.

## Links

- [CDB](https://cr.yp.to/cdb.html)
- [airports](https://ourairports.com/data/)
