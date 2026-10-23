# Compile only the standalone lifecycle fixtures with preinstalled MSVC.
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if ($LASTEXITCODE -ne 0 -or -not $installation) {
    throw 'Preinstalled Visual Studio C++ tools were not found.'
}
& (Join-Path $installation 'Common7\Tools\Launch-VsDevShell.ps1') -Arch amd64 -HostArch amd64 -SkipAutomaticLocation

Write-Output "Host: $([System.Runtime.InteropServices.RuntimeInformation]::OSDescription)"
Write-Output "Architecture: $([System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture)"
Write-Output "PowerShell: $($PSVersionTable.PSVersion) .NET: $([System.Environment]::Version)"
Write-Output "ImageOS=$env:ImageOS ImageVersion=$env:ImageVersion"
Write-Output "MSVC: $((Get-Command cl.exe -ErrorAction Stop).Source)"
Write-Output "VCToolsVersion=$env:VCToolsVersion WindowsSDKVersion=$env:WindowsSDKVersion"

# A fresh directory prevents stale executables from satisfying a failed build.
$build = Join-Path ([System.IO.Path]::GetTempPath()) ([System.IO.Path]::GetRandomFileName())
New-Item -ItemType Directory -Path $build -ErrorAction Stop | Out-Null
Write-Output "Standalone outputs: $build"
Push-Location $build
try {
    $sources = @(
        (Join-Path $repo 'arcified\native\model\lifecycle.cc'),
        (Join-Path $repo 'arcified\native\model\lifecycle_test.cc')
    )
    # /W4 and /WX preserve strict warnings; /permissive- requests conformance.
    # /Bv records the actual compiler/toolchain version during compilation.
    & cl.exe /Bv /TP /std:c++17 /EHsc /W4 /WX /permissive- @sources /Fe:lifecycle-test.exe
    if ($LASTEXITCODE -ne 0) { throw "MSVC compilation failed: $LASTEXITCODE" }
    & (Join-Path $build 'lifecycle-test.exe')
    if ($LASTEXITCODE -ne 0) { throw "Lifecycle fixtures failed: $LASTEXITCODE" }
} finally {
    Pop-Location
}
