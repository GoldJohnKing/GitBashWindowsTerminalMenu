# Add Git Bash to Windows 11 Modern Context Menu

## Summary
Implement a Windows App Package that adds "Git Bash Here" command directly to the Windows 11 Modern Context Menu (first-level menu) for files, directories, and drives, without requiring a secondary menu expansion.

## Problem Statement
Windows 11 removed the traditional "Open in terminal" from the classic context menu, making it harder to quickly open Git Bash from file explorer. Users need a convenient way to open Git Bash directly from the right-click menu at the current location.

## Proposed Solution
Create a Windows App Package (MSI installer) similar to the Code Modern Explorer Menu project, but configured to launch Windows Terminal with Git Bash profile using the command: `wt.exe -p "Git Bash" -d "%V"`

### Key Requirements
- Add menu item to first-level Modern Context Menu (no secondary menu)
- Support multiple file types: files (*), directories, directory backgrounds, and drives
- Execute command: `wt.exe -p "Git Bash" -d "%V"` where %V is the selected path
- Provide appropriate icon for the menu item
- Does not interfere with classic menu or other context menu extensions

### Technical Approach
- Implement a C++ COM server DLL that implements `IExplorerCommand` interface
- Create an AppxManifest.xml that registers the menu extension using `desktop4:Extension Category="windows.fileExplorerContextMenus"`
- Generate Windows App Package (.appx) and MSI installer
- Use unique CLSID for Git Bash menu handler

## Alternatives Considered
1. **Registry-based classic menu**: Limited functionality in Windows 11 modern menu
2. **PowerShell script**: Cannot register as first-level modern menu item
3. **Third-party tools**: Dependency on external software

## Success Criteria
- User can right-click on file/folder/drive in Windows 11 File Explorer
- "在 Git Bash 中打开 (Windows Terminal)" appears in first-level modern context menu
- Clicking the menu opens Windows Terminal with Git Bash profile at the selected location
- Menu item displays Git Bash icon
- Installation/uninstallation works correctly

## Risks & Mitigations
- **Windows Security**: May flag unsigned package - document signing requirements
- **Terminal Profile**: Assumes "Git Bash" profile exists in Windows Terminal - document prerequisite
- **Windows Terminal Path**: Assumes wt.exe in PATH or standard location - add fallback paths

## Dependencies
- Windows 11 (MinVersion: 10.0.26100.0)
- Windows Terminal installed
- Git Bash installed at default path (C:\Program Files\Git\bin\git-bash.exe)
- Git Bash profile configured in Windows Terminal
- Visual Studio C++ build tools
- WiX Toolset for MSI creation
