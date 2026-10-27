$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$python = 'py'
$distPath = Join-Path $repoRoot 'dist'
$buildPath = Join-Path $repoRoot 'build\pyinstaller'
$specPath = Join-Path $repoRoot 'build'

Push-Location $repoRoot
try {
    & $python -3.12 -m pip install -r requirements.txt -r requirements-build.txt
    if ($LASTEXITCODE -ne 0) { throw 'Could not install QUARK build dependencies.' }

    & $python -3.12 -m PyInstaller `
        --noconfirm `
        --clean `
        --onefile `
        --windowed `
        --name QUARK `
        --distpath $distPath `
        --workpath $buildPath `
        --specpath $specPath `
        --collect-submodules serial.tools `
        --collect-all psutil `
        quark.py
    if ($LASTEXITCODE -ne 0) { throw 'PyInstaller failed to build QUARK.exe.' }

    $exe = Join-Path $distPath 'QUARK.exe'
    if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw 'The expected QUARK.exe was not created.' }
    Write-Host "Built $exe"
}
finally {
    Pop-Location
}
