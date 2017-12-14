# time-support
a C++17 implementation of a stopwatch (a timer) for testing speed at hot spots.

## Requirements

`cmake` is used to compile the sources.

The default compiler used is `clang++-5.0`.

The cmake files compile with `-std=c++17`.

The unit tests are implemented in `googletest`: be sure you have installed `googletest` to compile.


The code is only for linux 64 bit arch and processors having the Time Stamp Counter (TSC) register present. 

## Install and Run Unit Tests

Some tests require su rights.
Therefore, use `sudo` to run the tests.


```bash
$ git clone https://github.com/massimo-marino/time-support.git
$ cd time-support
$ mkdir build
$ cd build
$ cmake ..
$ make
$ cd src/unitTests
$ sudo ./unitTests
```
The unit tests provide examples of usage of the class.

The unit tests are implemented in googletest: be sure you have installed googletest to compile.
