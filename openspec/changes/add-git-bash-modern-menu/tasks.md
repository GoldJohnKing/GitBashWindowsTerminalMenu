# Implementation Tasks: Git Bash Modern Context Menu

## Phase 1: Project Setup

- [ ] Initialize Git repository
- [ ] Create project directory structure
  - `src/` - C++ source files
  - `template/` - AppxManifest template
  - `scripts/` - Build scripts
  - `msi/` - WiX installer files
  - `resources/` - Icon assets
  - `out/` - Build output (gitignored)
- [ ] Copy and adapt `.gitignore` from reference project
- [ ] Set up `vcpkg.json` for dependencies
- [ ] Create `package.json` for project metadata (if needed)

## Phase 2: COM Server Implementation

- [x] Copy `src/explorer_command.cc` from reference project
- [x] Modify DLL exports and class UUID references for Git Bash Windows Terminal
- [x] Update registry key name from `CodeModernExplorerMenu` to `GitBashWindowsTerminalModernContextMenu`
- [x] Change DIR_NAME to `"Git"`
- [x] Change EXE_NAME to `"wt.exe"` (for Windows Terminal)
- [x] Update command construction in `Invoke()` method:
  ```cpp
  std::wstring cmd = L"wt.exe -p \"Git Bash\" -d " + QuoteForCommandLineArg(path);
  ```
- [x] Create `src/explorer_command_gitbash.def` with proper exports
- [ ] Create `src/explorer_command.idl` if needed for COM registration

## Phase 3: Build System Configuration

- [x] Copy and modify `main.gyp` for Git Bash Windows Terminal context menu
- [x] Update output DLL name to `GitBashWindowsTerminalContextMenu.dll`
- [x] Configure build targets for x86, x64, arm64
- [ ] Create `config.gypi` with build configuration
- [ ] Add gyp_library.py if needed for build automation

## Phase 4: AppxManifest Template

- [x] Create `template/AppxManifest.xml` from reference
- [x] Update package identity:
  - Name: `GitBashWindowsTerminal.Modern.ContextMenu`
  - Publisher: Appropriate publisher name
  - DisplayName: `Git Bash Windows Terminal Modern Context Menu`
- [x] Update Application element:
  - Executable: `wt.exe`
  - ApplicationIdShort: `GitBashWindowsTerminal`
- [x] Update MenuID to `OpenGitBashWindowsTerminal`
- [x] Update PackageDLL to `GitBashWindowsTerminalContextMenu.dll`
- [x] Update CLSID placeholder (will be replaced by script)
- [x] Verify all ItemTypes: *, Directory, Directory\Background, Drive

## Phase 5: Package Generation Script

- [x] Create `scripts/generate_pkg.py`
- [x] Add CLSID mapping for Git Bash Windows Terminal (generate new UUIDs):
  ```python
  gitbash_windows_terminal_clsid_map = {
    'x86': '<new-uuid>',
    'x64': '<new-uuid>',
    'arm64': '<new-uuid>'
  }
  ```
- [x] Update placeholder replacements for Git Bash Windows Terminal branding
- [ ] Test script generates correct AppxManifest.xml for each architecture

## Phase 6: Build Infrastructure

- [x] Set up `node-gyp` submodule or clone
- [ ] Configure `vcpkg` dependencies (wil, fmt, etc.)
- [x] Create PowerShell build script `build-msi.ps1` from reference
- [x] Update build script for Git Bash Windows Terminal project names
- [ ] Add build steps for:
  1. Compile DLL for each architecture
  2. Generate AppxManifest files
  3. Create .appx packages
  4. Build MSI installer

## Phase 7: MSI Installer Configuration

- [x] Create `msi/Product.wxs` (WiX source)
- [x] Configure product information for Git Bash Windows Terminal Modern Context Menu
- [x] Add component for DLL installation
- [x] Add custom action for Appx package registration
- [x] Configure registry key creation for title customization
- [x] Add uninstall actions

## Phase 8: Icon Assets

- [ ] Obtain Git Bash icon from `C:\Program Files\Git\mingw64\share\git\git-for-windows.ico`
- [ ] Create `code_150x150.png` (150x150 pixels) - *Requires manual conversion*
- [ ] Create `code_70x70.png` (70x70 pixels) - *Requires manual conversion*
- [ ] Place in `resources/app/resources/win32/` directory (ICON_README.md provided)

## Phase 9: Documentation

- [x] Create `README.md` with:
  - Project description
  - Installation instructions
  - Usage guide
  - System requirements
  - Troubleshooting
- [x] Document prerequisite: Windows Terminal with Git Bash profile
- [ ] Document code signing process
- [ ] Create `LICENSE` file (choose appropriate license)

## Phase 10: Testing

- [ ] Build DLL for all architectures (x86, x64, arm64)
- [ ] Build .appx packages
- [ ] Build MSI installer
- [ ] Install MSI on Windows 11 test machine
- [ ] Verify menu appears for:
  - [ ] Files
  - [ ] Directories
  - [ ] Directory background
  - [ ] Drives
- [ ] Test Git Bash opens correctly at selected location
- [ ] Test icon displays correctly
- [ ] Test uninstall removes all components
- [ ] Test on clean Windows 11 installation

## Phase 11: CI/CD (Optional)

- [ ] Set up GitHub Actions workflow
- [ ] Configure automatic build on push to main
- [ ] Add artifact generation for MSI and appx packages
- [ ] Configure release workflow

## Phase 12: Distribution

- [ ] Create first GitHub release
- [ ] Upload MSI installer
- [ ] Upload .appx packages for manual installation
- [ ] Document installation options
- [ ] Prepare for winget submission (future)

## Validation Criteria

Each task should be considered complete when:
- Code compiles without errors
- Tests pass (if applicable)
- Documentation is updated
- Manual testing confirms functionality works as designed
- No regressions introduced

## Dependencies

- **Phase 1-5**: Can be done in parallel after project setup
- **Phase 6**: Depends on Phases 2-4
- **Phase 7**: Depends on Phase 6
- **Phase 8**: Independent, can be done anytime
- **Phase 9**: Should start early, updated throughout
- **Phase 10**: Depends on Phases 1-8 being complete
- **Phase 11**: Depends on Phase 10
- **Phase 12**: Depends on Phase 10
