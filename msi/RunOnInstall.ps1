# If not running as Administrator, re-launch the script with elevated privileges
if (-not ([Security.Principal.WindowsPrincipal]::new([Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator))) {
    Start-Process powershell.exe -ArgumentList "-NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File `"$($script:MyInvocation.MyCommand.Path)`"" -Verb RunAs -Wait -WindowStyle Minimized
    exit
}

$ScriptRoot = if ( $PSScriptRoot ) { $PSScriptRoot } else { ($(try { $script:psEditor.GetEditorContext().CurrentFile.Path } catch {}), $script:MyInvocation.MyCommand.Path, $script:PSCommandPath, $(try { $script:psISE.CurrentFile.Fullpath.ToString() } catch {}) | % { if ($_ ) { $_.ToLower() } } | Split-Path -EA 0 | Get-Unique ) | Get-Unique }

$ProductName = 'WSLUbuntuWTContextMenu'
$MenuName = "在 Ubuntu 中打开 (Windows Terminal)"

$ProductPath = "$Env:LOCALAPPDATA\Programs\$ProductName"

if (-not (Test-Path $ProductPath)) {
    New-Item -Path $ProductPath -Force
}

# Create registry key for context menu
$RegKeyPath = 'HKCU:\SOFTWARE\Classes\WSLUbuntuWTContextMenu'
if (-not (Test-Path $RegKeyPath)) {
    New-Item -Path $RegKeyPath -Force | Out-Null
}
Set-ItemProperty -Path $RegKeyPath -Name "Title" -Value $MenuName -Force

# Temporary enable Developer Mode if initially disabled
$RegPath = "SOFTWARE\Microsoft\Windows\CurrentVersion\AppModelUnlock"
$RegKeyName = "AllowDevelopmentWithoutDevLicense"

$value = Get-ItemProperty -Path ('HKLM:\' + $RegPath) -Name $RegKeyName -ErrorAction SilentlyContinue
if ($value -eq $null -or $value.$RegKeyName -ne 1) {
    $DeveloperModeInitialStatus = 'Disabled'
    REG ADD "HKLM\$RegPath" /t REG_DWORD /v "$RegKeyName" /d "1" /reg:64 /f
    REG ADD "HKLM\$RegPath" /t REG_DWORD /v "$RegKeyName" /d "1" /reg:32 /f
}

Add-AppxPackage -Path "$ScriptRoot\AppxManifest.xml" -Register -ExternalLocation $ProductPath

# Restore Developer Mode to previous settings
if ($DeveloperModeInitialStatus -eq 'Disabled') {
    REG DELETE "HKLM\$RegPath" /v "$RegKeyName" /reg:64 /f
    REG DELETE "HKLM\$RegPath" /v "$RegKeyName" /reg:32 /f
}
