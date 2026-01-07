# Modern Context Menu Integration Capability

## Overview
This capability defines requirements for integrating applications into the Windows 11 Modern Context Menu via COM server implementation and Windows App Package registration.

## ADDED Requirements

### Requirement: COM Server Explorer Command Handler
The system SHALL provide a COM server that implements the `IExplorerCommand` interface to handle context menu operations.

#### Scenario: Handle Menu Title Request
**Given** a COM server implementing `IExplorerCommand`
**When** Windows Explorer requests the menu title via `GetTitle()`
**Then** the server SHALL return "Git Bash Here" from registry or use default value
**And** the title SHALL be returned as a valid string pointer allocated with `SHStrDup()`

#### Scenario: Handle Menu Icon Request
**Given** a COM server implementing `IExplorerCommand`
**When** Windows Explorer requests the menu icon via `GetIcon()`
**Then** the server SHALL locate Git Bash executable
**And** the server SHALL return the Git Bash icon path
**And** the icon path SHALL be returned as a valid string pointer allocated with `SHStrDupW()`

#### Scenario: Handle Command Invocation
**Given** a COM server implementing `IExplorerCommand`
**When** a user clicks the menu item and `Invoke()` is called
**Then** the server SHALL extract the selected path from `IShellItemArray`
**And** the server SHALL construct command: `wt.exe -p "Git Bash" -d "<path>"`
**And** the server SHALL properly quote the path using `QuoteForCommandLineArg()`
**And** the server SHALL execute the command using `ShellExecuteW()`
**And** the server SHALL return `S_OK` on success or appropriate error code

#### Scenario: Handle Menu State
**Given** a COM server implementing `IExplorerCommand`
**When** Windows Explorer requests the menu state via `GetState()`
**Then** the server SHALL return `ECS_ENABLED` to keep the menu item always active

### Requirement: Windows App Package Manifest
The system SHALL provide an AppxManifest.xml that registers the context menu extension with Windows 11.

#### Scenario: Register File Type Menu
**Given** an AppxManifest.xml with file explorer context menu extension
**When** defining menu items for files
**Then** the manifest SHALL include `desktop4:ItemType Type="*"`
**And** the manifest SHALL specify Verb Id "OpenGitBash"
**And** the manifest SHALL reference the correct CLSID for the COM server

#### Scenario: Register Directory Menu
**Given** an AppxManifest.xml with file explorer context menu extension
**When** defining menu items for directories
**Then** the manifest SHALL include `desktop5:ItemType Type="Directory"`
**And** the manifest SHALL specify Verb Id "OpenGitBash"
**And** the manifest SHALL reference the correct CLSID for the COM server

#### Scenario: Register Directory Background Menu
**Given** an AppxManifest.xml with file explorer context menu extension
**When** defining menu items for directory background (empty space in folder)
**Then** the manifest SHALL include `desktop5:ItemType Type="Directory\Background"`
**And** the manifest SHALL specify Verb Id "OpenGitBash"
**And** the manifest SHALL reference the correct CLSID for the COM server

#### Scenario: Register Drive Menu
**Given** an AppxManifest.xml with file explorer context menu extension
**When** defining menu items for drives
**Then** the manifest SHALL include `desktop10:ItemType Type="Drive"`
**And** the manifest SHALL specify Verb Id "OpenGitBash"
**And** the manifest SHALL reference the correct CLSID for the COM server

#### Scenario: Register COM Server
**Given** an AppxManifest.xml
**When** defining the COM server registration
**Then** the manifest SHALL include `com:Extension Category="windows.comServer"`
**And** the manifest SHALL specify the DLL path "GitBashWindowsTerminalContextMenu.dll"
**And** the manifest SHALL use STA threading model

### Requirement: Multi-Architecture Support
The system SHALL provide COM server DLLs and packages for x86, x64, and ARM64 architectures.

#### Scenario: Generate x86 Package
**Given** a build script with architecture parameter
**When** generating package for x86 architecture
**Then** the script SHALL use x86-specific CLSID
**And** the script SHALL compile DLL for x86
**And** the script SHALL generate AppxManifest with x86 CLSID

#### Scenario: Generate x64 Package
**Given** a build script with architecture parameter
**When** generating package for x64 architecture
**Then** the script SHALL use x64-specific CLSID
**And** the script SHALL compile DLL for x64
**And** the script SHALL generate AppxManifest with x64 CLSID

#### Scenario: Generate ARM64 Package
**Given** a build script with architecture parameter
**When** generating package for ARM64 architecture
**Then** the script SHALL use ARM64-specific CLSID
**And** the script SHALL compile DLL for ARM64
**And** the script SHALL generate AppxManifest with ARM64 CLSID

### Requirement: Path Resolution
The system SHALL locate Windows Terminal executable using multiple fallback strategies.

#### Scenario: Find Git Bash via Relative Path
**Given** the COM server DLL is installed
**When** locating Git Bash executable
**Then** the server SHALL first check relative path: `<DLL_Location>/../../Git/bin/git-bash.exe`
**And** the server SHALL use this path if the file exists

#### Scenario: Find Git Bash via Program Files
**Given** the COM server DLL is installed
**When** Git Bash is not found at relative path
**Then** the server SHALL check `C:\Program Files\Git\bin\git-bash.exe`
**And** the server SHALL use this path if the file exists

#### Scenario: Handle Git Bash Not Found
**Given** the COM server DLL is installed
**When** Git Bash is not found at any expected location
**Then** the server SHALL return error `E_FAIL`
**And** the menu operation SHALL be cancelled

### Requirement: MSI Installer
The system SHALL provide an MSI installer package for easy deployment.

#### Scenario: Install Components
**Given** an MSI installer package
**When** the installer runs on Windows 11
**Then** the installer SHALL copy `GitBashWindowsTerminalContextMenu.dll` to program files
**And** the installer SHALL register the Appx package
**And** the installer SHALL create registry key for title customization
**And** the installer SHALL prompt for administrator privileges if needed

#### Scenario: Uninstall Components
**Given** the package is installed
**When** the user runs uninstaller
**Then** the uninstaller SHALL remove the Appx package
**And** the uninstaller SHALL delete DLL files
**And** the uninstaller SHALL remove registry entries

#### Scenario: Restart Explorer
**Given** the package is installed or uninstalled
**When** the installation completes
**Then** the system SHALL restart Windows Explorer to reload context menu extensions

### Requirement: Security
The system SHALL implement appropriate security measures for package distribution.

#### Scenario: Code Signing
**Given** an MSI or Appx package
**When** preparing for distribution
**Then** the package SHALL be signed with a valid code signing certificate
**And** the signature SHALL be verifiable by Windows

#### Scenario: Admin Privileges
**Given** an MSI installer
**When** the installation begins
**Then** the installer SHALL verify administrator privileges
**And** the installer SHALL prompt for elevation if not running as admin

## MODIFIED Requirements
None - This is a new capability.

## REMOVED Requirements
None - This is a new capability.

## Cross-References

Related capabilities:
- None yet (first capability in the project)

## Notes

- Windows 11 minimum version: 10.0.26100.0
- Requires Windows Terminal to be installed on target system
- Requires Git Bash to be installed at default path: `C:\Program Files\Git\bin\git-bash.exe`
- Requires "Git Bash" profile to exist in Windows Terminal settings
- COM server must implement all required `IExplorerCommand` methods
- CLSIDs must be unique per architecture and different from the reference project
- Default menu title: "在 Git Bash 中打开 (Windows Terminal)"
- Icon source: Git Bash icon from `git-for-windows.ico`
- Package naming: Uses "GitBashWindowsTerminal" prefix to clarify Git Bash opens via Windows Terminal
