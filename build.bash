#!/bin/bash

mkdir -p bin

# copy readme
sed 's/\r$//' README.md | sed 's/$/\r/' > bin/vstouch.txt

# update version string
VERSION='v0.2'
GITHASH=`git rev-parse --short HEAD`
echo -n "$VERSION ( $GITHASH )" > "VERSION"
cat << EOS | sed 's/\r$//' | sed 's/$/\r/' > 'src/ver.h'
#pragma once
#define VERSION "$VERSION ( $GITHASH )"
EOS

# build
# using packages:
#   pacman -S mingw-w64-i686-ninja
#   pacman -S mingw-w64-i686-clang
#   pacman -S mingw-w64-i686-cmake

mkdir -p build
rm -rf build/*
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang ..
ninja
cd ..

rm -rf bin/vstouch.exe
cp build/src/vstouch.exe bin/vstouch.exe
cp build/src/vslib.dll bin/vslib.dll

cd bin
zip vstouch_wip.zip vstouch.txt vstouch.exe
cd ..
