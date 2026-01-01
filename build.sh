#!/bin/sh
set -e

DEFAULT_BUILD_TYPE="release"
DEFAULT_TARGET_TYPE="app"

BUILD_TYPE=${1:-$DEFAULT_BUILD_TYPE}
TARGET_TYPE=${2:-$DEFAULT_TARGET_TYPE}

case "$TARGET_TYPE" in
    asset)
        CMAKE_TARGETS="GNVEModels"
        ;;
    app)
        CMAKE_TARGETS="GNVEApp"
        ;;
    engine)
        CMAKE_TARGETS="GNVEngine"
        ;;
    all)
        CMAKE_TARGETS="GNVModels GNVEApp GNVEngine"
        ;;
    *)
        echo "Invalid target type: $TARGET_TYPE"
        echo "Valid options: asset, app, engine, all"
        exit 1
        ;;
esac

echo "Configuring CMake with Ninja ($BUILD_TYPE)..."
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo "Building target(s) '$CMAKE_TARGETS' with Ninja..."
for TARGET in $CMAKE_TARGETS; do
    cmake --build build --parallel --target "$TARGET"
done

echo "Installing..."
rm -rf dist
cmake --install build --prefix dist

echo "Running..."
cd dist || exit 1
./GNVEApp.exe
