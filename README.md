# n e s

[![Build Status][travis-badge]][travis]

## Description

Modular cycle-accurate NES emulator implemented from scratch in modern C++ and
built with the help of modern CMake, or that's the goal at least. If all goes
according to plan, you should be able to rip the 6502 CPU library out of this
and make a C64 or Apple ][ or whatever emulator based on it.

## Building

Clone!

`git clone https://github.com/evilcorpltd/n_e_s.git`

Build!

```sh
mkdir build_ && cd build_
cmake ..
make
make test
```
