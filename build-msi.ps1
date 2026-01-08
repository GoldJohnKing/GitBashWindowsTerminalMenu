param($Variant = 'gitbash', $Platform = 'x64', $Version = '1.0.0')

Import-Module PSMSI

$ScriptRoot = if ( $PSScriptRoot ) { $PSScriptRoot} else { ($(try { $script:psEditor.GetEditorContext().CurrentFile.Path } catch {}), $script:MyInvocation.MyCommand.Path, $script:PSCommandPath, $(try { $script:psISE.CurrentFile.Fullpath.ToString() } catch {}) | % { if ($_ ) { $_.ToLower() } } | Split-Path -EA 0 | Get-Unique ) | Get-Unique }

$OutputDirectory = "$ScriptRoot\output"

if (Test-Path $OutputDirectory) {
    Get-ChildItem -Path $OutputDirectory | ForEach-Object { Remove-Item -Path $_ -Force -Recurse  }
}

# Git Bash Windows Terminal Context Menu
$ProductName = 'GitBashWTContextMenu'
$ProductId = 'c745f6d0-0932-55f3-b5cc-59a0f837fb4c'
$UpgradeCode = '52e9cbda-cfb3-568c-bd11-9d396c2e9f2c'

$CustomAction = @(
    New-InstallerCustomAction -FileId 'RunOnInstall' -RunOnInstall
    New-InstallerCustomAction -FileId 'RunOnUninstall' -RunOnUninstall
)

$InstallerFile = {
    New-InstallerFile -Source "$ScriptRoot\[Content_Types].xml"
    New-InstallerFile -Source "$ScriptRoot\AppxBlockMap.xml"
    New-InstallerFile -Source "$ScriptRoot\out\$($Variant)_explorer_pkg_$($Platform)\AppxManifest.xml"
    New-InstallerFile -Source "$ScriptRoot\out\$ProductName $Platform.appx"
    New-InstallerFile -Source "$ScriptRoot\out\$ProductName.dll"
    New-InstallerFile -Source "$ScriptRoot\msi\RunOnInstall.ps1" -Id 'RunOnInstall'
    New-InstallerFile -Source "$ScriptRoot\msi\RunOnUninstall.ps1" -Id 'RunOnUninstall'
}

New-Installer -ProductName $ProductName -ProductId $ProductId -UpgradeCode $UpgradeCode -Platform $Platform -Version $Version -Content {
    New-InstallerDirectory -PredefinedDirectory "LocalAppDataFolder" -Content {
        New-InstallerDirectory -DirectoryName "Programs" -Content {
            New-InstallerDirectory -DirectoryName $ProductName -Content $InstallerFile
        }
    }
} -CustomAction $CustomAction -OutputDirectory $OutputDirectory #-RequiresElevation
