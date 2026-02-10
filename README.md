# WSL Ubuntu Windows Terminal Context Menu

An MSI package that adds "在 Ubuntu 中打开" (Open in Ubuntu) to the Windows 11 Modern Explorer menu, using Windows Terminal with WSL.

> [!NOTE]
> Please restart Windows Explorer after installation.
>
> Installation requires admin rights and accepting UAC prompt to temporarily enable Developer Mode if required and restore its initial status after installation.

> [!CAUTION]
> AV may flag this as a virus due to the lack of a signature and self-elevation.

## Requirements:
- Windows 11 (Build 26100+)
- Windows Terminal installed (standard installation includes Ubuntu profile)
- Ubuntu WSL installed (or WSL with Ubuntu distribution)
- Admin rights

## Features:
- Adds "在 Ubuntu 中打开" context menu option in Windows 11 modern menu
- Works with folders, folder background, and drives (not files)
- Does not interfere with classic menu
- Opens Ubuntu WSL in Windows Terminal at the selected location
- Support for x64 and ARM64 architectures

## Building

### Prerequisites
- Python 3.x
- node-gyp (submodule included)
- Visual Studio 2022 with C++ build tools
- vcpkg package manager
- PowerShell with PSMSI module

### Build Steps

```powershell
# Build x64 version
.\build-msi.ps1 -Variant 'wslubuntu' -Platform 'x64'

# Build ARM64 version
.\build-msi.ps1 -Variant 'wslubuntu' -Platform 'arm64'
```

The output MSI will be created in the `output` directory.

## Installation

1. Run the MSI installer as administrator
2. Accept the UAC prompt
3. Restart Windows Explorer (or sign out/sign in)
4. Right-click on a folder, folder background, or drive to see "在 Ubuntu 中打开" in the modern menu

## Uninstallation

Use "Add or Remove Programs" in Windows Settings to uninstall.

## Project Details

This project provides a focused implementation for WSL Ubuntu integration with Windows 11's modern context menu. It uses Windows Terminal as the terminal emulator.

### Build System
- Uses GYP (Generate Your Projects) via node-gyp submodule for build configuration
- Compiles with Visual Studio MSBuild
- No npm/bun dependencies (build system uses Python scripts)

### Architecture
- Shell extension DLL (`explorer_command_wslubuntu.cc`) that implements the context menu handler
- CLSID-based registration for Windows 11 modern menu integration
- Support for x64 and ARM64 architectures

## Credits

This is a fork/adaptation of [microsoft/vscode-explorer-command](https://github.com/microsoft/vscode-explorer-command), modified specifically for WSL Ubuntu integration with Windows Terminal.
