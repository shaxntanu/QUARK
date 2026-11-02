param(
    [switch]$IncludeFirmware
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot

Push-Location $repoRoot
try {
    Write-Host 'Checking pinned Python dependencies...'
    & py -3.12 -m pip check
    if ($LASTEXITCODE -ne 0) { throw 'Python dependency check failed.' }

    Write-Host 'Running Python tests...'
    & py -3.12 -m unittest discover -s tests -v
    if ($LASTEXITCODE -ne 0) { throw 'Python unit tests failed.' }

    Write-Host 'Compiling Python source...'
    & py -3.12 -m compileall -q quark.py tests scripts
    if ($LASTEXITCODE -ne 0) { throw 'Python bytecode compilation failed.' }

    Write-Host 'Checking repository documentation and paths...'
    & py -3.12 scripts/check_docs.py
    if ($LASTEXITCODE -ne 0) { throw 'Repository documentation checks failed.' }

    Write-Host 'Building the Windows executable...'
    & .\scripts\build_exe.ps1
    if ($LASTEXITCODE -ne 0) { throw 'Windows executable build failed.' }

    if ($IncludeFirmware) {
        if (-not (Get-Command arduino-cli -ErrorAction SilentlyContinue)) {
            throw 'Arduino CLI was requested but is not installed or not on PATH.'
        }
        Write-Host 'Compiling ESP32-C3 firmware...'
        & arduino-cli compile --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc .
        if ($LASTEXITCODE -ne 0) { throw 'ESP32-C3 firmware compilation failed.' }
    }

    Write-Host 'QUARK verification completed successfully.'
}
finally {
    Pop-Location
}
