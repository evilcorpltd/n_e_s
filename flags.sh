#!/bin/sh

add_flag() { CXXFLAGS="$CXXFLAGS $@"; }

add_flag -Wall
add_flag -Werror
add_flag -Wextra
add_flag -Wshadow
add_flag -Wnon-virtual-dtor
add_flag -pedantic-errors

export CXXFLAGS=$CXXFLAGS
