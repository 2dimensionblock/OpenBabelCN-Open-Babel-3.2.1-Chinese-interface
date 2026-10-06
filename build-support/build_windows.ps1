# Run from a terminal with Python, CMake, Ninja, Perl, MinGW-w64 gcc/g++/windres on PATH.
$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path -Parent $PSScriptRoot
Set-Location $taskRoot
python build-support/prepare_sources.py
if ($LASTEXITCODE -ne 0) { throw '源码准备失败' }
cmake -S . -B build-win -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_RC_COMPILER=windres "-DEIGEN3_INCLUDE_DIR=$taskRoot/vendor/eigen3" -DPTHREAD_LIBRARY=winpthread
if ($LASTEXITCODE -ne 0) { throw '配置失败' }
cmake --build build-win --target OpenBabelCN obengine obabel --parallel 2
if ($LASTEXITCODE -ne 0) { throw '编译失败' }
python build-support/package.py --build build-win --destination dist/OpenBabelCN
if ($LASTEXITCODE -ne 0) { throw '打包失败' }
Write-Host '构建完成：dist/OpenBabelCN/OpenBabelCN.exe'
