$ErrorActionPreference='Stop'
$workspace=Split-Path $PSScriptRoot -Parent
$destination=Join-Path $workspace 'dist/Pluguin Slice RONI RESO'
New-Item -ItemType Directory -Path $destination -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $workspace 'bin/PluguinSliceRONIRESO.exe') -Destination $destination
Get-ChildItem -LiteralPath (Join-Path $workspace 'bin') -Filter '*.dll' | Copy-Item -Destination $destination
$data=Join-Path $destination 'data'
New-Item -ItemType Directory -Path $data -Force | Out-Null
foreach($folder in @('shaders','presets')){
    Copy-Item -LiteralPath (Join-Path $workspace "bin/data/$folder") -Destination $data -Recurse -Force
}
Copy-Item -LiteralPath (Join-Path $workspace 'ejemplo_advanced.xml') -Destination (Join-Path $data 'ejemplo_advanced.xml') -Force
Copy-Item -LiteralPath (Join-Path $workspace 'README.md') -Destination $destination -Force
Copy-Item -LiteralPath (Join-Path $workspace 'docs') -Destination $destination -Recurse -Force
$licenses=Join-Path $destination 'licenses'
New-Item -ItemType Directory -Path $licenses -Force | Out-Null
$msysLicenses=Join-Path $licenses 'msys2'
New-Item -ItemType Directory -Path $msysLicenses -Force | Out-Null
Get-ChildItem -LiteralPath 'C:/msys64/mingw64/share/licenses' | Copy-Item -Destination $msysLicenses -Recurse -Force
foreach($library in @('kiss','tess2','videoInput')){
    $source=Join-Path $workspace "vendor/of/libs/$library/license"
    if(Test-Path -LiteralPath $source){
        $target=Join-Path $licenses $library
        New-Item -ItemType Directory -Path $target -Force | Out-Null
        Get-ChildItem -LiteralPath $source | Copy-Item -Destination $target -Recurse -Force
    }
}
foreach($entry in @(
    @('vendor/of/LICENSE.md','openFrameworks.txt'),
    @('vendor/of/addons/ofxImGui/License.md','ofxImGui.txt'),
    @('vendor/of/addons/ofxImGui/libs/imgui/src/LICENSE.txt','DearImGui.txt'),
    @('vendor/Spout2-master/LICENSE','Spout.txt'),
    @('vendor/ofxNDI-master/LICENSE','ofxNDI.txt'),
    @('vendor/ofxNDI-master/libs/NDI/include/Processing.NDI.Lib.h','NDI-header-and-license-reference.txt')
)){
    $source=Join-Path $workspace $entry[0]
    if(Test-Path -LiteralPath $source){Copy-Item -LiteralPath $source -Destination (Join-Path $licenses $entry[1]) -Force}
}
Copy-Item -LiteralPath (Join-Path $workspace 'docs/THIRD_PARTY.md') -Destination $licenses -Force
Compress-Archive -LiteralPath $destination -DestinationPath (Join-Path $workspace 'dist/PluguinSliceRONIRESO-Windows.zip') -Force
Write-Output $destination
