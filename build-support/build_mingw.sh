#!/usr/bin/env bash
set -euo pipefail
task_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$task_root"
python3 build-support/prepare_sources.py
cmake -S . -B build-win -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$task_root/build-support/mingw.cmake" \
  -DCMAKE_BUILD_TYPE=Release -DEIGEN3_INCLUDE_DIR="$task_root/vendor/eigen3" \
  -DPTHREAD_LIBRARY="$(x86_64-w64-mingw32-gcc-posix -print-file-name=libwinpthread.a)"
cmake --build build-win --target OpenBabelCN obengine obabel --parallel 2
# Cross-compilation does not run Windows executables. Produce the format list
# with the native build from the same sources and options.
cmake -S . -B build-native -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DEIGEN3_INCLUDE_DIR="$task_root/vendor/eigen3"
cmake --build build-native --target obengine --parallel 2
python3 build-support/package.py --build build-win --format-engine build-native/bin/obengine --destination dist/OpenBabelCN
