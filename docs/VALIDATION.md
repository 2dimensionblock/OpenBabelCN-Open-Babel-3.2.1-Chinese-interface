# 验证范围 / Validation scope

## 已有记录

原始验证说明日期为 **2026-09-30**，不是本次 GitHub Actions 的执行记录。

| 对象 | 已完成 | 未涵盖 |
| --- | --- | --- |
| Windows x64 文件 | 已交叉编译；检查 PE 节完整性及系统 DLL 依赖 | Windows 实际启动与 GUI 交互 |
| 同源 Linux 引擎 | 13/13 项行为测试通过 | Windows 环境、所有格式与所有分子 |
| 本次 GitHub 分享整理 | 补齐文档、模板与工作流；校验引用路径和 YAML | GitHub 托管运行器上的实际执行 |

记录文件：[test_results.json](../tests/test_results.json)、[windows_binary_check.json](../tests/windows_binary_check.json)、[验证说明.txt](../验证说明.txt)。

## 引擎行为测试

现有测试覆盖：核心格式注册、多分子二维转换及往返、已知分子性质、显式氢增删、pH 处理、三维生成及 MMFF94 优化、柔性/刚性 PDBQT、MOL2 部分电荷、分子拆分及单分子格式保护、最大连通片段、失败处理、InChI，以及中文路径与名称。

这些测试只证明相应输入和检查条件通过。不能据此声称所有格式、力场、化学空间或 Windows 交互均已验证。

## Windows 手动验收记录表

以下各项目前均为**待验证**，在实际 Windows 设备上完成后填写系统版本、日期和结果。

| 操作 | 预期结果 | 状态 |
| --- | --- | --- |
| 完整解压并启动 | 正常显示中文窗口，可找到计算引擎 | 待验证 |
| 中文安装路径、输入路径和输出路径 | 正常读取、计算、保存 | 待验证 |
| 文件选择、多文件拖放、SMILES 输入 | 输入内容正确进入任务 | 待验证 |
| 示例分子导出属性 | 表头中文，乙醇分子式 C2H6O、相对分子质量约 46.068 | 待验证 |
| 格式转换及单分子拆分 | 生成相应格式的可读输出 | 待验证 |
| 三维生成、优化及 PDBQT | 输出有效结构；柔性与刚性模式符合设置 | 待验证 |
| 无效输入 | 显示诊断，当前输入不提交部分成功输出 | 待验证 |
| 取消正在计算的任务 | 后台计算停止，界面可继续使用 | 待验证 |
| 输入成功后再发生失败 | 已成功输入的结果仍保留 | 待验证 |
| 100%、150%、200% 显示缩放 | 控件可见、文字可读、关键按钮可操作 | 待验证 |
| 关闭后再次启动 | 能恢复已记录的输出位置 | 待验证 |

若公开分享时仍未完成上表，可将 Release 标记为预发布，并保留这些已知限制。

## English

The historical record contains 13 passing Linux engine tests and Windows binary compilation/PE checks. It does not include interactive Windows testing. Manual Windows startup, Unicode paths, file dialogs, drag-and-drop, task cancellation, output handling, and DPI behavior remain to be checked. The added GitHub Actions workflow has not yet executed in the destination repository.
