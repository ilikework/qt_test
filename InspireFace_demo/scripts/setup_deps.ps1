param(
    [string]$DemoRoot = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = "Stop"
$thirdParty = Join-Path $DemoRoot "third_party"
$modelsDir = Join-Path $DemoRoot "models\buffalo_l"
$ortDir = Join-Path $thirdParty "onnxruntime"

New-Item -ItemType Directory -Force -Path $thirdParty, $modelsDir, (Join-Path $DemoRoot "demo_output") | Out-Null

# ONNX Runtime Windows x64 (CPU)
$ortVersion = "1.17.3"
$ortZip = "onnxruntime-win-x64-$ortVersion.zip"
$ortUrl = "https://github.com/microsoft/onnxruntime/releases/download/v$ortVersion/$ortZip"
$ortZipPath = Join-Path $thirdParty $ortZip

if (-not (Test-Path (Join-Path $ortDir "include\onnxruntime_cxx_api.h"))) {
    Write-Host "Downloading ONNX Runtime $ortVersion ..."
    Invoke-WebRequest -Uri $ortUrl -OutFile $ortZipPath
    Expand-Archive -Path $ortZipPath -DestinationPath $thirdParty -Force
    $extracted = Join-Path $thirdParty "onnxruntime-win-x64-$ortVersion"
    if (Test-Path $ortDir) { Remove-Item -Recurse -Force $ortDir }
    Move-Item $extracted $ortDir
}

# buffalo_l ONNX models (same family as InspireFace dense landmark)
$hfBase = "https://huggingface.co/public-data/insightface/resolve/main/models/buffalo_l"
foreach ($name in @("det_10g.onnx", "2d106det.onnx")) {
    $out = Join-Path $modelsDir $name
    if (-not (Test-Path $out)) {
        Write-Host "Downloading $name ..."
        Invoke-WebRequest -Uri "$hfBase/$name" -OutFile $out
    }
}

Write-Host "OK: dependencies ready"
Write-Host "  ORT    : $ortDir"
Write-Host "  models : $modelsDir"
