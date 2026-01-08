# libProteus

A global weather, geographic information, and navigation simulator library, used by SailNavSim (as seen here: https://8bitbyte.ca/sailnavsim/).

## Dependencies

### Libraries

- POSIX threads (pthread) library, with headers
- zlib library, with headers

### Build tools

- Make
- C compiler (gcc or clang, etc.)

### Tested build/run environments

- GNU/Linux (various distros), x86-64

## How to build

`make libproteus`

## Build and run tests

`make tests`

`./run_tests.sh`
