param([int]$Jobs = 4, [string]$MsysRoot = 'C:\msys64', [string]$Drive = 'R:')
$ErrorActionPreference = 'Stop'
$taskRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if (-not (Test-Path (Join-Path $taskRoot 'vendor/of/libs/openFrameworks/ofMain.h'))) {
    throw 'Falta el SDK openFrameworks en vendor/of. Ver README.md.'
}
if (Test-Path ($Drive + '\')) { throw "La unidad $Drive ya existe. Elegi otra con -Drive." }
# A temporary drive alias avoids spaces in paths inside the upstream oF makefiles.
& subst.exe $Drive $taskRoot
if ($LASTEXITCODE -ne 0) { throw 'No se pudo crear el alias de compilacion' }
try {
    $taskDrivePath = '/' + $Drive.Substring(0,1).ToLowerInvariant()
    & (Join-Path $MsysRoot 'usr/bin/bash.exe') '--noprofile' '--norc' "$taskDrivePath/scripts/build.sh" $taskDrivePath $Jobs
    if ($LASTEXITCODE -ne 0) { throw 'La compilacion fallo' }
} finally { & subst.exe $Drive /D }
