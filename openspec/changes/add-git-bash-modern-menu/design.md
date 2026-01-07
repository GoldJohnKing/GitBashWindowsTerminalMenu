# Technical Design: Git Bash Modern Context Menu

## Architecture Overview

The solution consists of three main components:

1. **COM Server DLL** (`GitBashWindowsTerminalContextMenu.dll`): Implements `IExplorerCommand` interface to handle context menu operations
2. **Windows App Package** (`GitBashWindowsTerminalModernContextMenu.appx`): Package manifest that registers the menu extension
3. **MSI Installer**: Windows Installer package for easy deployment

## Component Details

### 1. COM Server DLL

**File**: `src/explorer_command.cc`

**Key Classes**:
- `ExplorerCommandHandler`: Implements `IExplorerCommand` interface

**Interface Methods**:
- `GetTitle()`: Retrieves menu title from registry or uses default "在 Git Bash 中打开 (Windows Terminal)"
- `GetIcon()`: Returns Git Bash icon path
- `GetState()`: Returns `ECS_ENABLED` to make menu item always enabled
- `Invoke()`: Executes the Git Bash command with selected path
  - Retrieves selected file/folder path from `IShellItemArray`
  - Constructs command: `wt.exe -p "Git Bash" -d "<path>"`
  - Executes using `ShellExecuteW()`

**Build Configuration**:
- Defines: `DIR_NAME="Git"`, `EXE_NAME="git-bash.exe"`
- Compiler flags: `/D DLL_UUID="{CLSID}"` per architecture
- Dependencies: Windows SDK, Windows Implementation Library (WIL)

### 2. Windows App Package Manifest

**File**: `template/AppxManifest.xml`

**Key Elements**:

```xml
<Identity Name="GitBashWindowsTerminal.Modern.ContextMenu" ... />

<Applications>
  <Application Id="GitBashWindowsTerminal" Executable="wt.exe" ...>
    <Extensions>
      <desktop4:Extension Category="windows.fileExplorerContextMenus">
        <desktop4:FileExplorerContextMenus>
          <desktop4:ItemType Type="*">
            <desktop4:Verb Id="OpenGitBashWindowsTerminal" Clsid="{CLSID}" />
          </desktop4:ItemType>
          <desktop5:ItemType Type="Directory">
            <desktop5:Verb Id="OpenGitBashWindowsTerminal" Clsid="{CLSID}" />
          </desktop5:ItemType>
          <desktop5:ItemType Type="Directory\Background">
            <desktop5:Verb Id="OpenGitBashWindowsTerminal" Clsid="{CLSID}" />
          </desktop5:ItemType>
          <desktop10:ItemType Type="Drive">
            <desktop10:Verb Id="OpenGitBashWindowsTerminal" Clsid="{CLSID}" />
          </desktop10:ItemType>
        </desktop4:FileExplorerContextMenus>
      </desktop4:Extension>
      <com:Extension Category="windows.comServer">
        <com:ComServer>
          <com:SurrogateServer DisplayName="Git Bash Windows Terminal Modern Context Menu">
            <com:Class Id="{CLSID}" Path="GitBashWindowsTerminalContextMenu.dll" ThreadingModel="STA"/>
          </com:SurrogateServer>
        </com:ComServer>
      </com:Extension>
    </Extensions>
  </Application>
</Applications>
```

**Supported Item Types**:
- Files (`Type="*"`): All file types
- Directories (`Type="Directory"`): Folders
- Directory Background (`Type="Directory\Background"`): Empty space in folder
- Drives (`Type="Drive"`): Disk drives

### 3. Package Generation Script

**File**: `scripts/generate_pkg.py`

**Function**:
- Replaces template placeholders in `AppxManifest.xml`:
  - `@@PackageName@@` → `GitBashWindowsTerminal.Modern.ContextMenu`
  - `@@PackageDisplayName@@` → `Git Bash Windows Terminal Modern Context Menu`
  - `@@CLSID@@` → Architecture-specific CLSID
  - `@@MenuID@@` → `OpenGitBashWindowsTerminal`
  - `@@PackageDLL@@` → `GitBashWindowsTerminalContextMenu.dll`

**CLSID Mapping**:
```python
gitbash_windows_terminal_clsid_map = {
  'x86': '...',  # To be generated
  'x64': '...',  # To be generated
  'arm64': '...'  # To be generated
}
```

### 4. Build System

**File**: `main.gyp`

**Build Targets**:
- `gitbash_windows_terminal_context_menu.dll`: COM server library
- Generates DLL for each architecture (x86, x64, arm64)

**Build Process**:
1. Compile C++ source with node-gyp/vcpkg
2. Generate AppxManifest for each architecture
3. Create .appx package files
4. Build MSI installer with WiX

## Command Execution Flow

1. User right-clicks on file/folder/drive
2. Windows Explorer invokes COM server (registered via AppxManifest)
3. `ExplorerCommandHandler.Invoke()` is called
4. Extract selected path from `IShellItemArray`
5. Quote path for command-line safety
6. Execute: `wt.exe -p "Git Bash" -d "<quoted_path>"`
7. Windows Terminal opens Git Bash at specified location

## Path Resolution Strategy

**Primary Path** (from DLL location):
```
<DLL Location>/../../Git/bin/git-bash.exe
```

**Fallback Path**:
```
C:\Program Files\Git\bin\git-bash.exe
C:\Program Files (x86)\Git\bin\git-bash.exe
```

**Registry Configuration**:
```
HKLM\Software\Classes\GitBashWindowsTerminalModernContextMenu\Title
  = "在 Git Bash 中打开 (Windows Terminal)" (optional, for customization)
```

## Installation & Registration

**MSI Installer Actions**:
1. Copy `GitBashWindowsTerminalContextMenu.dll` to program files
2. Register Appx package via PowerShell
3. Create registry key for title customization
4. Restart Windows Explorer (handled by OS)

**Uninstaller Actions**:
1. Remove Appx package
2. Delete DLL files
3. Clean registry entries
4. Restart Windows Explorer

## Security Considerations

1. **Code Signing**: Package should be signed with valid certificate
2. **UAC**: Requires admin privileges for installation
3. **Developer Mode**: May need to temporarily enable during installation
4. **Virus Detection**: Unsigned packages may trigger AV false positives

## Icon Assets

**Location**: `resources/app/resources/win32/`
- `code_150x150.png`: Menu icon (use Git Bash icon)
- `code_70x70.png`: Small icon

**Icon Source**: Git Bash executable icon from `C:\Program Files\Git\mingw64\share\git\git-for-windows.ico`

## Testing Strategy

1. **Manual Testing**:
   - Install MSI on Windows 11
   - Verify menu appears for files, folders, drives
   - Test Git Bash launches correctly
   - Verify uninstall removes all components

2. **Automated Testing** (future):
   - Unit tests for path quoting logic
   - Integration tests with test appx package

## Deployment

**Distribution Methods**:
- Direct MSI download
- Package manager (winget, chocolatey) - future
- GitHub Releases

**System Requirements**:
- Windows 11 (Build 26100+)
- Windows Terminal installed
- Git Bash profile in Windows Terminal
- Admin rights for installation
