改了一点自用，效果如图，可以在win11右键一级菜单里使用VScode
<br>
<img src="https://guiguisocute-1318017707.cos.ap-guangzhou.myqcloud.com/20251003230218172.png"/>


#### 吐槽
不知道微软哪次更新把win11的win10右键中"在终端打开"给删了，现在只有win11菜单能用，调了半天都没法回来，不想回到地址栏输powershell的苦日子了，只能选择这种曲线救国方式，所幸还蛮好看的，但我估计还得适应一阵子


#### Git Bash 版本
本项目现在还支持在 Windows 11 右键菜单中添加"在 Git Bash 中打开 (Windows Terminal)"选项。

构建 Git Bash 版本：
```powershell
# 构建 x64 版本的 Git Bash MSI 安装包
.\build-msi.ps1 -Variant 'gitbash' -Platform 'x64'

# 构建 arm64 版本的 Git Bash MSI 安装包
.\build-msi.ps1 -Variant 'gitbash' -Platform 'arm64'
```

安装后会添加"在 Git Bash 中打开 (Windows Terminal)"到右键菜单。

**前置条件：**
- Windows 11 (Build 26100+)
- Windows Terminal 已安装
- Git for Windows 已安装
- Windows Terminal 中配置了 "Git Bash" 配置文件

***
> 以下为源仓库README原文
# Code Modern Explorer Menu
An MSI package that adds the Windows 11 Modern Explorer menu for Microsoft Visual Studio Code.
  
> [!NOTE]
> Please restart Windows Explorer after installation.
> 
> Installation requires admin rights and accepting UAC prompt to temporarily enable Developer Mode if required and restore its initial status after installation.

> [!CAUTION]
> AV may flag this as a virus due to the lack of a signature and self-elevation.

## Requirements:
- Windows 11+
- VSCode installed
- Admin rights

## Features:
- does not interfere with the classic menu
- does not interfere with the original VSCode Insiders menu
- should not interfere when VSCode stable introduces the menu
- works with both system and user installation locations
- support the case when VSCode runs as Administrator, thanks to  [ArcticLampyrid](https://github.com/microsoft/vscode-explorer-command/pull/17)
- Also works for Devices and drives, thanks to [AndromedaMelody](https://github.com/microsoft/vscode-explorer-command/pull/16)
- Future VSCode updates won’t break the menu, thanks to [huutaiii](https://github.com/huutaiii/vscode-explorer-command)

## Project changes:
- replace Azure DevOps with GitHub Actions
- removed C++ dependencies from the repository
- added vcpkg package manager
