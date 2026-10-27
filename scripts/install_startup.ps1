param(
    [string]$ExePath = (Join-Path (Split-Path -Parent $PSScriptRoot) 'dist\QUARK.exe')
)

$ErrorActionPreference = 'Stop'
$taskName = 'QUARK Companion'
$source = (Resolve-Path -LiteralPath $ExePath).Path
$installDirectory = Join-Path $env:LOCALAPPDATA 'QUARK'
$installedExe = Join-Path $installDirectory 'QUARK.exe'
$startupDirectory = [Environment]::GetFolderPath('Startup')
$shortcutPath = Join-Path $startupDirectory 'QUARK Companion.lnk'

New-Item -ItemType Directory -Path $installDirectory -Force | Out-Null
New-Item -ItemType Directory -Path $startupDirectory -Force | Out-Null
Copy-Item -LiteralPath $source -Destination $installedExe -Force

# Prefer the current user's Startup folder so no administrator rights are needed.
Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut($shortcutPath)
$shortcut.TargetPath = $installedExe
$shortcut.WorkingDirectory = $installDirectory
$shortcut.Description = 'Start the QUARK USB companion when this user signs in.'
$shortcut.WindowStyle = 7
$shortcut.Save()

Start-Process -FilePath $installedExe -WorkingDirectory $installDirectory -WindowStyle Hidden
Start-Sleep -Seconds 2
$running = @(Get-Process QUARK -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $installedExe })
if (-not $running) {
    throw 'QUARK.exe did not remain running. Check %LOCALAPPDATA%\QUARK\quark.log.'
}

Write-Host "Installed $installedExe"
Write-Host "Startup shortcut: $shortcutPath"
Write-Host 'The companion is running and will start when this Windows user signs in.'
Write-Host "Log: $installDirectory\quark.log"
