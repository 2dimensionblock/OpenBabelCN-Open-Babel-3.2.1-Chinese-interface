# OpenBabel 中文工作台

**基于 Open Babel 3.2.1 的 Windows 64 位中文图形工具，用于化学文件转换、分子结构处理和属性计算。**

[English](README.en.md) · [使用指南](docs/USAGE.zh-CN.md) · [构建指南](docs/BUILDING.md) · [更新记录](CHANGELOG.md)

本项目包含原生 C++ 图形程序 `OpenBabelCN.exe` 和独立计算引擎 `obengine.exe`。常用操作可以通过中文界面完成；SMILES、格式标识、力场名称和上游原始诊断保留其通用写法。这是基于 Open Babel 的独立中文工作台，不是 Open Babel 官方 GUI 发行版。

> 当前应用版本：**1.0.0**。Windows EXE 已编译，同源 Linux 引擎的 13 项行为测试已通过；Windows 实机图形界面验收尚未完成。首次公开分享建议标记为预发布版，详见[验证范围](docs/VALIDATION.md)。

## 下载与启动

1. 打开本仓库右侧的 **Releases**，选择所需版本。
2. 下载 **`OpenBabelCN_Windows64.zip`**，完整解压。
3. 双击解压目录中的 **`OpenBabelCN.exe`**。
4. 添加化学文件或粘贴 SMILES，选择功能、参数及输出文件夹，开始处理。

请保留 EXE 旁的 `engine` 文件夹。面向 **Windows 10/11 x64**；便携版无需另行安装 Python、Open Babel 或编译工具，分子计算在本地完成。

GitHub 自动提供的 **Source code (zip)** 是源码快照。普通用户请下载上面的 Windows 便携包；开发者可下载完整的 `OpenBabelCN_GitHub.zip` 或克隆仓库。

## 主要功能

| 功能 | 内容 |
| --- | --- |
| 文件与批量处理 | 多文件选择、拖放、逐行 SMILES 输入、批量输出、单分子拆分 |
| 格式转换 | SDF、MOL、SMILES、PDB、PDBQT、MOL2、XYZ、InChI、SVG 等；以界面读写格式列表为准 |
| 坐标生成 | 二维及三维坐标生成，三维生成质量可选 |
| 氢与片段处理 | 加氢、去氢、仅保留极性氢、按指定 pH 处理、保留最大连通片段 |
| 能量最小化 | MMFF94、MMFF94s、UFF、GAFF、Ghemical 力场，优化步数可设 |
| 部分电荷 | Gasteiger、MMFF94、QEq 电荷计算 |
| 对接格式准备 | 柔性配体或刚性受体模式的 PDBQT 导出 |
| 分子属性 | 分子式、相对分子质量、精确质量、氢键供体/受体数、logP、TPSA、规范 SMILES 等，导出中文 CSV |
| 结果查看 | 首个成功分子的二维示意预览、任务摘要和错误诊断 |

程序支持任务取消。每个输入完整处理成功后再提交其结果；失败或取消的输入不会以成功结果形式提交，之前已成功的输入结果保留。

## 快速示例

在 SMILES 输入框中每行输入一个分子，也可在结构后添加名称：

```text
CCO 乙醇
CC(=O)Oc1ccccc1C(=O)O 阿司匹林
Cn1c(=O)c2c(ncn2C)n(C)c1=O 咖啡因
```

选择“分子属性”可导出中文表格；选择格式转换并生成二维坐标，可输出 SDF。准备 PDBQT 时，需要有效三维结构，或先选择三维坐标生成。

更多示例见 [examples](examples)，详细操作见随包提供的[使用说明](使用说明.html)。

## 功能边界

- PDBQT 导出用于格式准备，程序不执行分子对接搜索、结合能评分或蛋白缺失残基修复。
- 二维预览是结构示意，未完整绘制所有立体化学细节；需要可导出的二维图时可选择 SVG。
- 坐标生成和能量优化限制为每个分子最多 500 个重原子；界面预览最多 200 个重原子。普通格式转换和属性计算不采用该坐标处理上限。
- 每个输入任务最长运行 10 分钟；大分子或复杂输入可能需要拆分或改用命令行工具。
- 当前构建未启用 CML/libxml2、JSON、PNG/Cairo、压缩读写、Maestro 和外部 Coordgen。压缩输入应先解压。
- 力场、电荷和 pH 处理有各自适用范围，计算后的结构及参数需结合研究对象检查。

## 开发与测试

新增代码采用 C++17 和 Win32 Unicode API。计算引擎直接调用随附 Open Babel 源码的 C++ API，随包还提供 `obabel.exe` 原始命令行工具。

Windows 开发环境需具备 Python 3、CMake、Ninja、Perl 和 MinGW-w64 工具链，随后运行：

```powershell
.\build-support\build_windows.ps1
```

构建依赖、Linux 交叉编译、测试命令和 CI 范围见[构建指南](docs/BUILDING.md)。历史测试记录见 [tests/test_results.json](tests/test_results.json)；新增 GitHub Actions 工作流需在仓库首次运行后查看实际结果。

## 项目文件

| 路径 | 用途 |
| --- | --- |
| `src/` | 中文图形界面、计算引擎、图标和 Windows 资源 |
| `vendor/` | 完整原始 Open Babel 源码 ZIP 和 Eigen 头文件 |
| `build-support/` | 源码准备、上游修补、编译和打包 |
| `tests/` | 引擎行为测试和历史验证记录 |
| `docs/` | 使用、构建、验证和 GitHub 发布指南 |
| `licenses/` | 第三方版权与许可证 |
| `.github/` | 问题模板、Pull Request 模板和 CI 配置 |

## 反馈与贡献

问题和功能建议请提交至本仓库 **Issues**，附程序版本、Windows 版本、复现步骤及可公开的最小输入。参与开发请先阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。

## 许可证与致谢

本项目新增代码采用 **GPL-2.0-only**，见 [LICENSE](LICENSE)。第三方源码保留各自原始版权和许可证，详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。分发时请同时提供对应源码和许可文件。

核心化学功能来自 [Open Babel](https://openbabel.org/)；感谢 Open Babel、InChI、Eigen 和 MinGW-w64 等项目的贡献者。上游补丁和构建差异详见[源码说明](源码说明.md)。
