# 更新记录 / Changelog

## 1.0.0

应用的初始版本；GitHub 上的实际发布日期以对应 Release 为准。

### 功能 / Features

- Windows x64 原生中文图形界面与独立分子计算引擎。
- 文件拖放、SMILES 输入、批量格式转换和单分子拆分。
- 二维/三维坐标、氢处理、pH 处理、最大连通片段、力场优化和部分电荷。
- 柔性/刚性 PDBQT 导出、中文属性 CSV 和二维示意预览。
- 后台任务、取消、输入级结果提交和错误诊断。

Native Chinese Windows GUI; batch conversion; 2D/3D coordinates; hydrogen and pH processing; force-field minimization; partial charges; PDBQT export; property tables and schematic previews.

### 引擎和验证 / Engine and validation

- 基于随附 Open Babel 3.2.1 源码，包含坐标回写及静态构建兼容补丁。
- 已有同源 Linux 引擎 13/13 项行为测试记录和 Windows PE 检查记录。
- Windows 实机图形界面验收尚未完成。

The Linux engine passed 13 behavioral tests. Windows binaries were compiled and inspected; interactive Windows validation remains outstanding.

### GitHub 分享文件 / GitHub sharing files

补充中英文 README、使用/构建/发布指南、第三方说明、贡献指南、问题与 PR 模板、Git 配置和引擎测试工作流。这次文件整理未修改应用或引擎源码，也未重新生成 Windows 二进制文件。

Added repository documentation, contribution templates, Git configuration, and an engine-test workflow. This documentation preparation does not modify the application or engine sources and does not rebuild the Windows binaries.
