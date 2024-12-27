#!/bin/bash

rm -rf ./build

case $1 in
  cmake)
    cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=DEBUG
    cmake -S . -B build/release -DCMAKE_BUILD_TYPE=RELEASE
    cmake -S . -B build/test -DCMAKE_BUILD_TYPE=TEST
  ;;
  meson)
    meson setup --buildtype debug                    ./build/debug   .
    meson setup --buildtype release --optimization 3 ./build/release .
  ;;
esac
