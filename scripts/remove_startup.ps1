$ErrorActionPreference = 'Stop'
$shortcutPath = Join-Path ([Environment]::GetFolderPath('Startup')) 'QUARK Companion.lnk'
$taskName = 'QUARK Companion'

if (Test-Path -LiteralPath $shortcutPath) {
    Remove-Item -LiteralPath $shortcutPath -Force
    Write-Host 'Removed the QUARK Companion startup shortcut.'
}

Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue
Write-Host 'QUARK will no longer start automatically at user sign-in.'
