# OpenBabel 中文工作台 v1.0.0（预发布 / Pre-release）

面向 Windows 10/11 x64 的中文化学工具，基于 Open Babel 3.2.1 源码构建。

## 下载

- **普通用户：`OpenBabelCN_Windows64.zip`**。完整解压后双击 `OpenBabelCN.exe`，保留相邻 `engine` 文件夹，无需另装 Python 或 Open Babel。
- **开发者：`OpenBabelCN_GitHub.zip`**。完整源码及 GitHub 项目文件，包含原始 Open Babel 源码、Eigen 头文件、构建脚本、补丁和许可证。
- **校验：`SHA256SUMS.txt`**。列出以上两个 ZIP 的 SHA-256。

GitHub 自动生成的 Source code 压缩包是源码快照，不是可运行程序。

## 功能

批量化学格式转换、SMILES 输入、二维/三维坐标生成、氢与 pH 处理、最大连通片段、力场能量最小化、部分电荷、柔性/刚性 PDBQT、中文属性 CSV、二维示意预览及任务取消。计算在本地完成。

## 验证与已知限制

同源 Linux 引擎已有 13/13 项行为测试通过记录；Windows x64 二进制已交叉编译并完成 PE 检查。Windows 实机界面、文件对话框、拖放、DPI 和取消操作尚未完成交互验收，因此本次按预发布提供。

程序不执行分子对接搜索或评分。二维预览不完整呈现所有立体化学细节。坐标生成和优化限制 500 个重原子，预览限制 200 个重原子，每个输入任务最长 10 分钟。详细范围见 README 和验证说明。

这是独立中文界面项目，不是 Open Babel 官方 GUI 发行版。新增代码使用 GPL-2.0-only；第三方许可及完整对应源码随包提供。

---

## English

OpenBabel Chinese Workbench is a Chinese graphical application for Windows 10/11 x64, built from Open Babel 3.2.1 sources.

**End users:** download `OpenBabelCN_Windows64.zip`, extract all files, and run `OpenBabelCN.exe`. Keep the adjacent `engine` folder. No separate Python or Open Babel installation is required.

**Developers:** download `OpenBabelCN_GitHub.zip` for the full source, bundled dependencies, documented patches, build scripts, licenses, and repository documentation. `SHA256SUMS.txt` contains checksums for both archives.

Features include batch conversion, SMILES input, 2D/3D coordinates, hydrogen and pH processing, force-field minimization, partial charges, PDBQT export, Chinese property tables, previews, and task cancellation. Computations run locally.

The Linux engine passed 13 behavioral tests. Windows binaries were compiled and inspected, but interactive Windows validation remains outstanding. This release is therefore provided as a **pre-release**. It is an independent application, not an official Open Babel GUI. New code is GPL-2.0-only; third-party notices and corresponding source are included.
