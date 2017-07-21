# time-support
a C++14 implementation of a stopwatch (a timer) for testing speed at hot spots.

## Requirements

The cmake file compiles with `-std=c++14`.
The code is only for linux 64 bit arch and processors having the Time Stamp Counter (TSC) register present. 

## Install and Run Unit Tests

```bash
$ git clone https://github.com/massimo-marino/time-support.git
$ cd time-support
$ mkdir build
$ cd build
$ cmake ..
$ make
$ cd src/unitTests
$ ./unitTests
```
The unit tests provide examples of usage of the class.

The unit tests are implemented in googletest: be sure you have installed googletest to compile.
