param([string]$MsysRoot='C:\msys64')
$ErrorActionPreference='Stop'
$taskRoot=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$taskPreviousPath=$env:PATH
try {
    $env:PATH=(Join-Path $MsysRoot 'mingw64/bin')+';'+$taskPreviousPath
    & (Join-Path $MsysRoot 'mingw64/bin/cmake.exe') -S (Join-Path $taskRoot 'vendor/Spout2-master') -B (Join-Path $taskRoot 'vendor/spout-build') -G Ninja -DCMAKE_BUILD_TYPE=Release -DSPOUT_BUILD_LIBRARY=ON -DSPOUT_BUILD_SPOUTDX=OFF
    if($LASTEXITCODE -ne 0){throw 'No se pudo configurar Spout'}
    & (Join-Path $MsysRoot 'mingw64/bin/cmake.exe') --build (Join-Path $taskRoot 'vendor/spout-build') --target SpoutLibrary -j 4
    if($LASTEXITCODE -ne 0){throw 'No se pudo compilar Spout'}
    Copy-Item -LiteralPath (Join-Path $taskRoot 'vendor/spout-build/bin/libSpoutLibrary.dll') -Destination (Join-Path $taskRoot 'bin/SpoutLibrary.dll') -Force
} finally {$env:PATH=$taskPreviousPath}
