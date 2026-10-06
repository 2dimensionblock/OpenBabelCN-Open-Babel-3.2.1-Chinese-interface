# GitHub 分享步骤

这份文件包已经具备可直接提交的源码、双语 README、许可说明、协作模板和 CI 配置。发布操作由你在自己的 GitHub 账号中完成；以下过程不要求提供账号密码或令牌给他人。

## 一、准备文件

| 文件 | 放在哪里 | 用途 |
| --- | --- | --- |
| `OpenBabelCN_GitHub.zip` 解压后的 `OpenBabelCN` 文件夹内容 | GitHub 仓库根目录 | 可浏览和克隆的完整项目 |
| `OpenBabelCN_Windows64.zip` | 对应版本 Release 的附件 | 普通用户运行程序 |
| `OpenBabelCN_GitHub.zip` 原文件 | 同一 Release 的附件 | 对应的完整源码和项目文件 |
| `SHA256SUMS.txt` | 同一 Release 的附件 | 下载校验 |

不要将整个项目 ZIP 作为仓库根目录唯一文件，否则 GitHub 首页不能直接显示 README 和源码。无需把 EXE 提交到 Git 历史。

原始 Open Babel ZIP 约 **53.5 MiB**。GitHub 网页的单文件上传限制是 **25 MiB**，因此完整仓库建议使用 **GitHub Desktop** 或 Git 推送。普通 Git 对超过 50 MiB 的文件会提示警告，超过 100 MiB 才会阻止；本项目保留的 ZIP 低于该硬限制，无需为它启用 Git LFS。见 [GitHub 官方说明](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github)。

## 二、使用 GitHub Desktop 创建并上传

1. 安装并登录 [GitHub Desktop](https://desktop.github.com/)。
2. 选择 **File → New repository**，名称可用 `OpenBabelCN`，选择本地保存位置。
3. README、Git ignore、License 的自动生成选项保持未选或 None，因为文件包已经提供这些文件。
4. 创建本地仓库后，将分享包中 `OpenBabelCN` 文件夹的**全部内容**复制进去，包括 `.github`、`.gitignore`、`.gitattributes` 和 `.editorconfig`。
5. 检查根目录能够直接看到 `README.md`、`src`、`vendor`、`licenses`，而不是再嵌套一层 `OpenBabelCN`。
6. 在 Desktop 左侧查看改动，在 Summary 填写 `Initial release of OpenBabel Chinese Workbench`，提交到当前分支。
7. 点击 **Publish repository**，填写简介，取消勾选 **Keep this code private** 以公开分享，然后发布。

如已创建 GitHub 仓库，先在 Desktop 中克隆该仓库，再将文件复制到克隆目录、提交并 Push。保留已有 Git 历史，不要重新初始化或强制推送。

官方操作参考：[创建第一个仓库](https://docs.github.com/en/desktop/overview/creating-your-first-repository-using-github-desktop)。

## 三、创建程序下载页

1. 打开仓库网页的 **Releases → Draft a new release**。
2. 创建标签 **`v1.0.0`**，指向包含本次完整源码的提交或分支。
3. 标题填写 **`OpenBabel 中文工作台 v1.0.0（预发布）`**。
4. 将 [RELEASE_NOTES_v1.0.0.md](RELEASE_NOTES_v1.0.0.md) 的内容复制到说明框。
5. 在附件区上传 `OpenBabelCN_Windows64.zip`、`OpenBabelCN_GitHub.zip`、`SHA256SUMS.txt`。
6. 由于 Windows 实机界面验收尚未完成，勾选 **This is a pre-release**。
7. 检查附件和说明，点击 **Publish release**。

软件版本仍为 1.0.0。这里的预发布标记用于说明验证状态，不表示重新编译或改变了现有程序版本。以后修改程序并重新构建时，应使用新版本号、重新生成源码包和校验文件。

发布参考：[管理 Releases](https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository)。随二进制一起提供对应源码和许可证的依据见 [GNU GPL v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)。

## 四、填写仓库简介

中文简介：

> 基于 Open Babel 3.2.1 的 Windows 中文图形工作台，支持化学格式转换、三维结构处理、力场优化、PDBQT 导出及分子属性计算。

英文简介：

> A Chinese Windows GUI built on Open Babel 3.2.1 for chemical format conversion, 3D structure processing, force-field minimization, PDBQT export, and molecular properties.

可选 Topics：`openbabel`、`cheminformatics`、`chemistry`、`molecular-modeling`、`windows`、`gui`、`cpp`、`chinese`、`pdbqt`。

## 五、检查公开页面

- 首页显示中文 README，English 链接能打开英文版本。
- Releases 中能下载运行包、完整源码和校验文件。
- 下载并完整解压运行包，按[验证表](VALIDATION.md)记录实际 Windows 结果。
- Actions 中查看 Engine CI 的真实运行结果；不要把历史 JSON 结果当作本次 CI 结果。
- Issues 中能选择问题反馈和功能建议模板。

当前文件没有编造作者姓名、邮箱、仓库地址或 DOI，也没有依赖尚不存在的徽章地址。你可以在确定署名和仓库 URL 后补充作者信息或截图。

## 校验下载

Windows PowerShell：

```powershell
Get-FileHash .\OpenBabelCN_Windows64.zip -Algorithm SHA256
Get-FileHash .\OpenBabelCN_GitHub.zip -Algorithm SHA256
```

将输出与 `SHA256SUMS.txt` 比较。校验文件对应本次交付的 ZIP 字节；重新压缩或更改文件后应重新计算。

## 可选：Git 命令方式

在解压后的项目根目录执行；以下仅适用于尚无 Git 历史的新项目。先在 GitHub 创建空仓库，不勾选自动生成 README、License 或 .gitignore。

```bash
git init
git add .
git commit -m "Initial release of OpenBabel Chinese Workbench"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/OpenBabelCN.git
git push -u origin main
```

将 `YOUR_USERNAME` 替换为你的 GitHub 用户名，仓库名称以实际创建的名称为准。使用 GitHub 支持的认证方式，不要将令牌写入源码或说明文件。
