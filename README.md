

radix_tree
=====

[![Build Status](https://travis-ci.org/ytakano/radix_tree.svg?branch=master)](https://travis-ci.org/ytakano/radix_tree)

STL like container of radix tree in C++

Usage
=====
It's a header-only library. Just include it. See [examples](examples/).

Develop
=====
Requirements: any C++20 compiler (`g++` or `clang++`), `cmake`
Tested with CLang++ 17.0.0 on macOS 15
Tested with VS2019 and VS2026 on Windows 11

Comparing with original version
=====
- optimized memory usage

```
~/radix_tree $ mkdir build && cd build
~/radix_tree/build $ cmake .. -DBUILD_TESTS=On
~/radix_tree/build $ make check
```

Copyright
=====
See [COPYING](COPYING).
