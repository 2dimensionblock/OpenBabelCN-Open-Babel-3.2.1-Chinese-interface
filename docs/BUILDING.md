# 构建与测试 / Building and testing

程序运行不依赖 Python；以下工具只用于开发、构建及测试。所有命令均在仓库根目录执行。

## 依赖

| 依赖 | 用途 |
| --- | --- |
| C++17 编译器 | 编译图形程序与工作进程；Windows 脚本使用 MinGW-w64 |
| CMake 3.16+，建议 3.x | 构建配置；本项目 CI 使用 Ubuntu 24.04 的 CMake 3.x |
| Ninja | 构建执行 |
| Perl | 上游数据头生成 |
| Python 3 | 源码准备、打包、行为测试；脚本只使用标准库 |
| Open Babel 3.2.1 源码 | 已随 `vendor/openbabel-openbabel-3-2-1.zip` 提供 |
| Eigen 3.4.0 头文件 | 已随 `vendor/eigen3/` 提供 |

CMake 4.x 未纳入验证，遇到上游旧 policy 相关错误时使用 CMake 3.x。本次分享整理未重新编译程序；原 Windows 交叉构建使用 MinGW-w64 GCC 13.2 POSIX 工具链。不同工具链构建不保证二进制哈希一致。

## Windows 原生构建

准备 64 位 MinGW-w64 工具链，并确保 `gcc`、`g++`、`windres`、`cmake`、`ninja`、`perl` 和 `python` 位于同一 PowerShell 会话的 PATH 中。避免混入其他架构或 MSVC 编译器。

```powershell
.\build-support\build_windows.ps1
```

脚本依次校验并准备源码、配置、编译和打包。结果为 `dist/OpenBabelCN/OpenBabelCN.exe` 及其配套文件。脚本不会安装开发工具。

构建后测试：

```powershell
$env:PYTHONUTF8 = '1'
python tests/test_engine.py --engine build-win/bin/obengine.exe --data vendor/openbabel/data --report test-report-windows.json
```

Windows 原生构建脚本随源码提供，但当前交付记录不包含 Windows 原生构建或实机执行验收。若 PowerShell 阻止脚本运行，请遵循本机管理策略；也可在工具已配置的终端中逐条执行脚本列出的命令。

## Linux 原生引擎

以下示例面向 Ubuntu 24.04。Linux 只构建计算引擎，不产生 Win32 界面。

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build perl python3
python3 build-support/prepare_sources.py
cmake -S . -B build-native -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DEIGEN3_INCLUDE_DIR="$PWD/vendor/eigen3"
cmake --build build-native --target obengine --parallel 2
python3 tests/test_engine.py \
  --engine build-native/bin/obengine \
  --data vendor/openbabel/data \
  --report test-report-native.json
```

## Linux 交叉编译 Windows x64

在上述 Linux 工具基础上，准备 `x86_64-w64-mingw32-gcc-posix`、`x86_64-w64-mingw32-g++-posix` 和 `x86_64-w64-mingw32-windres`。

```bash
bash build-support/build_mingw.sh
```

脚本先构建 Windows GUI、`obengine.exe` 和 `obabel.exe`，再构建同源 Linux 引擎以生成格式列表，最终写入 `dist/OpenBabelCN/`。交叉编译不会执行 Windows 程序；若 windres 找不到默认 GCC，请按系统工具链配置方式设置相应编译器名称。

## GitHub Actions

[工作流](../.github/workflows/ci.yml)在推送、Pull Request 或手动触发时运行，使用 `ubuntu-24.04`：

1. 校验原始 Open Babel 源码哈希并应用补丁。
2. 编译 Linux 原生 `obengine`。
3. 运行现有 13 项引擎行为测试。
4. 上传本次测试报告，保留 14 天。

工作流只请求仓库读取权限，不发布 Release。此次已校验配置语法和所引用路径，但没有在你的 GitHub 仓库执行该工作流。首次上传后请查看 Actions 中的真实结果。CI 不验证 Windows GUI。

## 源码与补丁

`prepare_sources.py` 将原始 ZIP 解压到 `vendor/openbabel/`，并应用文档记录的修补。该目录是生成目录，不提交 Git；原始 ZIP、Eigen 头文件、补丁及准备脚本应保留。源码准备需要完整 ZIP，不能用 Git LFS 指针文本代替。

## English quick reference

Install a C++17 compiler, CMake 3.x, Ninja, Perl, and Python 3. Open Babel sources and Eigen headers are bundled. On Windows, use a 64-bit MinGW-w64 toolchain and run `build-support/build_windows.ps1`. On Linux, use the native-engine commands above or run `bash build-support/build_mingw.sh` with the MinGW cross-toolchain installed.

The GitHub workflow builds and tests the Linux engine only. It has not yet run in the destination repository, and its result does not establish Windows GUI compatibility. See [VALIDATION.md](VALIDATION.md) for the historical test scope and manual Windows checks.
