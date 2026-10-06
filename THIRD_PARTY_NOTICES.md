# 第三方组件与许可 / Third-party notices

本文件说明随包组件和声明位置，不替代各组件的完整许可证。第三方源码中的版权与许可声明均予保留。

| 组件 / Component | 用途 / Role | 许可与位置 / License and location |
| --- | --- | --- |
| OpenBabelCN 新增代码 | Win32 中文 GUI、独立工作进程、构建支持 | GPL-2.0-only；根目录 `LICENSE` |
| Open Babel 3.2.1 | 分子格式、结构处理、力场与描述符 | GPL v2 及源码内各文件声明；`licenses/OpenBabel_GPL-2.0.txt`，原始 ZIP 内 `COPYING` |
| InChI | InChI 读写 | MIT；`licenses/InChI_MIT.txt`，源码随 Open Babel 提供 |
| Eigen 3.4.0 | 数值计算头文件 | MPL-2.0 等逐文件许可；`licenses/Eigen_版权与许可.txt`、`licenses/MPL-2.0.txt`、`vendor/eigen3/` |
| MinGW-w64 | Windows 编译和运行时支持 | 组件各自许可；`licenses/MinGW_版权与许可.txt` |
| GCC 运行库 | 静态 C/C++ 运行时支持 | 相关 GPL 条款与 GCC Runtime Library Exception；`licenses/GCC_版权与运行库许可.txt`、`licenses/GPL-3.txt` |

## 源码来源与改动

Open Babel 使用开发此程序时提供的完整 `openbabel-openbabel-3-2-1.zip`。版本号来自该源码的 `CMakeLists.txt`，原始压缩包 SHA-256 为：

```text
b97271d69c50556eb3698edd84d760c706b425985ae527d6b71c31ed9fbc86a8
```

源码准备脚本校验上述哈希，再解压并应用已记录修补。修补包括 gen3D 快速返回分支的坐标回写、MinGW 静态 InChI 兼容以及数据头生成输出缓冲；顶层 CMake 另设置静态链接和可选功能。详见[源码说明](源码说明.md)及[补丁文件](build-support/upstream-patches.diff)。

原始源码包和 Eigen 头文件随本仓库提供。发布 Windows 便携包时，一并提供这一版本的完整源码包及许可证文件，保留取得源码的清晰路径。本项目不宣称获得上游官方认可。

## English

New application code is GPL-2.0-only. Third-party components retain their original copyrights and licenses; the table above locates the relevant notices. The original Open Babel archive and Eigen headers are bundled. The source-preparation script verifies the archive hash and applies the documented patches. Distribute the matching source package and notices alongside the Windows binary package. This application is not an official or endorsed Open Babel GUI release.

## 上游参考 / Upstream references

- [Open Babel](https://openbabel.org/)
- [Open Babel source repository](https://github.com/openbabel/openbabel)
- [InChI Trust](https://www.inchi-trust.org/)
- [Eigen](https://eigen.tuxfamily.org/)
- [MinGW-w64](https://www.mingw-w64.org/)
- [GNU GPL version 2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
