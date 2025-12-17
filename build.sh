#!/bin/sh
set -e

BUILD_TYPE=${1:-Release}

echo "Configuring CMake with Ninja ($BUILD_TYPE)..."
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=$BUILD_TYPE

echo "Building with Ninja..."
cmake --build build --parallel

echo "Installing..."
rm -rf dist
mkdir -p dist
cp -a build/bin/. dist/bin

echo "Running..."
cd dist/bin || exit 1
./GNVEApp.exe
