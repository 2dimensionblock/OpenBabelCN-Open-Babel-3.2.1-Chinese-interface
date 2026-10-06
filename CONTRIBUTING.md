# 贡献指南 / Contributing

欢迎报告问题、改进中文说明、补充格式兼容性测试及提交代码修复。请先在 Issues 中查找已有讨论；较大的功能改动可以先说明使用场景。

## 报告问题

请使用问题模板，提供程序版本、Windows 版本、操作步骤、预期与实际结果。对于化学结果问题，还需提供输入/输出格式、力场、电荷、pH 等参数，以及可以公开的最小分子示例。

诊断日志可能包含本地路径和分子名称，提交前请检查内容。不要提交不能公开的研究数据。

## 提交代码

1. Fork 仓库并创建主题分支。
2. 按[构建指南](docs/BUILDING.md)准备环境。
3. 使改动集中于一个问题；化学行为变更应附能体现正确性的最小回归用例。
4. 引擎变更运行 `tests/test_engine.py`，并记录测试平台；界面变更在 Windows 上验证对应操作。
5. 提交 Pull Request，描述问题、解决方式、验证结果及已知限制。

新增界面文案使用中文。第三方格式标识、化学表达式、力场名称和原始诊断可以保留通用写法。避免把三维、立体化学或能量计算的变化仅当作显示改动处理。

## 依赖与许可

新增项目代码沿用 GPL-2.0-only。提交内容应有权按项目许可贡献；引用第三方代码时保留原始声明，并更新第三方说明。

`vendor/openbabel-openbabel-3-2-1.zip` 保持原始字节不变。上游修补通过 `build-support/prepare_sources.py` 和 `upstream-patches.diff` 记录，不提交生成的 `vendor/openbabel/` 目录。依赖升级应同时更新哈希、补丁、构建说明和测试记录。

## English

Please search existing issues before opening a new report. Include the application and Windows versions, reproduction steps, relevant chemical parameters, and a minimal input that can be shared publicly.

Keep pull requests focused. Add meaningful regression cases for chemical-behavior changes, run the engine tests for engine changes, and test GUI changes on Windows. Report the actual platform and validation scope. Follow the existing GPL-2.0-only license for new project code and preserve third-party notices. Keep the original vendor archive unchanged and record upstream patches explicitly.
