# VSL
[![Build Status](https://travis-ci.org/vsl-lang/vsl-in-cpp.svg?branch=master)](https://travis-ci.org/vsl-lang/vsl-in-cpp)

This is a rewrite of the [VSL Compiler](https://github.com/vsl-lang/VSL) in C++.

## Compiling
TODO Windows/Mac.

Linux:
```sh
# clone the repository
git clone https://github.com/vsl-lang/vsl-in-cpp
cd vsl-in-cpp
# create the build directory
# this can really be done anywhere
mkdir build
cd build
# run cmake to generate a build system
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DVSL_INCLUDE_TESTS=On ..
# build the compiler
# optionally add "-j<some number>" to the make command for parallel compilation
make
# run the tests
make check
# generate documentation in the docs folder
make docs
```
