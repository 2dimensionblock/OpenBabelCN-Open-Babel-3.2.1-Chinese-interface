Open Babel 中文工作台 (OpenBabelCN) 1.0.0 — Windows 64 位便携版

1. 完整解压压缩包。
2. 双击 OpenBabelCN.exe。
3. 添加化学文件或粘贴 SMILES，选择功能和输出位置，开始处理。
4. 详细操作见“使用说明.html”。

请保留 engine 文件夹，不要只复制单个 EXE。
无需安装 Python、Open Babel 或其他运行库，不需要联网。
面向 Windows 10/11 64 位。

包含：批量格式转换、二维/三维生成、加氢/去氢、酸碱度处理、
能量最小化、部分电荷、去盐、柔性/刚性 PDBQT、中文属性 CSV、二维预览。

Windows 文件已编译；核心计算测试范围见“验证说明.txt”。
尚未完成 Windows 实机图形界面验证。

!注意：这是自定义中文界面，不是 Open Babel 官方 GUI 发行版！
！Note: This is a custom Chinese interface, not an official Open Babel GUI distribution！

程序说明
OpenBabel 中文工作台由二进制可执行程序和分子计算引擎组成。

#二进制可执行程序：适配 Windows 64 位系统，采用原生 C++ 开发，提供全中文图形界面。支持文件选择与拖放、SMILES 输入、批量任务、参数设置、结果导出及二维结构预览。程序以便携包形式提供，完整解压后即可启动，无需单独安装 Python 或 Open Babel。

#计算引擎：基于 Open Babel 3.2.1 源码构建，通过独立后台进程执行化学文件格式转换、二维与三维坐标生成、加氢与去氢、质子化状态调整、力场能量优化、部分电荷计算、PDBQT 导出及分子属性分析。全部计算在本地完成，支持任务取消、错误诊断和中文属性表输出。

Program statement
Open Babel Chinese Workbench consists of a compiled binary application and a molecular computation engine.

#Binary application: A native C++ application designed for 64-bit Windows, featuring a fully Chinese graphical interface. It supports file selection and drag-and-drop, SMILES input, batch processing, parameter configuration, result export, and 2D molecular previews. Distributed as a portable package, it can be launched after full extraction without separately installing Python or Open Babel.

#Computation engine: Built from the Open Babel 3.2.1 source code, the engine runs in a separate background process. It performs chemical file format conversion, 2D and 3D coordinate generation, hydrogen addition and removal, protonation-state adjustment, force-field energy minimization, partial-charge calculation, PDBQT export, and molecular property analysis. All computations run locally, with support for task cancellation, diagnostic reporting, and property tables with Chinese column headings.
