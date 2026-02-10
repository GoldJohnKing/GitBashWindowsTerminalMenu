# If not running as Administrator, re-launch the script with elevated privileges
if (-not ([Security.Principal.WindowsPrincipal]::new([Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator))) {
    Start-Process powershell.exe -ArgumentList "-NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File `"$($script:MyInvocation.MyCommand.Path)`"" -Verb RunAs -Wait -WindowStyle Minimized
    exit
}

$ScriptRoot = if ( $PSScriptRoot ) { $PSScriptRoot } else { ($(try { $script:psEditor.GetEditorContext().CurrentFile.Path } catch {}), $script:MyInvocation.MyCommand.Path, $script:PSCommandPath, $(try { $script:psISE.CurrentFile.Fullpath.ToString() } catch {}) | % { if ($_ ) { $_.ToLower() } } | Split-Path -EA 0 | Get-Unique ) | Get-Unique }

$ProductName = 'WSLUbuntuWTContextMenu'
$RegKeyPath = 'HKCU\SOFTWARE\Classes\WSLUbuntuWTContextMenu'

# Remove registry keys
REG DELETE "$RegKeyPath" /reg:64 /f
REG DELETE "$RegKeyPath" /reg:32 /f

# Remove Appx package
Get-AppxPackage -Name $ProductName | Remove-AppxPackage

# Remove installation directory
if ($ScriptRoot -eq "$Env:LOCALAPPDATA\Programs\$ProductName") {
    Remove-Item -Path "$Env:LOCALAPPDATA\Programs\$ProductName" -Recurse -Force
}
