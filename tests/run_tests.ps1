# run_tests.ps1
# Master test runner for The Locker Heist project.
# Steps:
#   1. Locate MSBuild via vswhere
#   2. Build Debug|x64
#   3. Run verify_data_files.ps1
#   4. Run heist.exe --selftest (working directory = TheLockerHeist)
#   5. Save self-test output to tests/results/selftest.txt
#   6. Exit non-zero if any step failed
#
# Later commits add more steps (integration tests, Release build, etc.)

param()

$root      = Split-Path -Parent $PSScriptRoot
$slnFile   = Join-Path $root "TheLockerHeist.sln"
$projDir   = Join-Path $root "TheLockerHeist"
$resultDir = Join-Path $PSScriptRoot "results"
$resultTxt = Join-Path $resultDir "selftest.txt"

$overallFailed = 0

function Step-Header([string]$title) {
    Write-Host ""
    Write-Host "============================================"
    Write-Host "  $title"
    Write-Host "============================================"
}

# ================================================================
# STEP 1: Locate MSBuild via vswhere
# ================================================================
Step-Header "STEP 1: Locate MSBuild"

$vswherePaths = @(
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\Installer\vswhere.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe",
    "C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe"
)

$vswhere = $null
foreach ($p in $vswherePaths) { if (Test-Path $p) { $vswhere = $p; break } }

$msbuild = $null
if ($vswhere) {
    $vsPath  = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath 2>$null
    $msbuild = "$vsPath\MSBuild\Current\Bin\MSBuild.exe"
    if (-not (Test-Path $msbuild)) { $msbuild = $null }
}

if ($msbuild) {
    Write-Host "  [PASS] MSBuild found: $msbuild"
} else {
    Write-Host "  [FAIL] MSBuild not found.  Install Visual Studio 2022 (Desktop C++ workload)." -ForegroundColor Red
    $overallFailed++
    # Cannot build -- skip to data file check
}

# ================================================================
# STEP 2: Build Debug|x64
# ================================================================
Step-Header "STEP 2: Build Debug|x64"

if ($msbuild -and (Test-Path $slnFile)) {
    & $msbuild $slnFile /p:Configuration=Debug /p:Platform=x64 /m /nologo /v:minimal 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  [PASS] Build succeeded"
    } else {
        Write-Host "  [FAIL] Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
        $overallFailed++
    }
} else {
    Write-Host "  [SKIP] Build skipped (MSBuild not available or solution not found)"
}

# ================================================================
# STEP 3: Verify data files
# ================================================================
Step-Header "STEP 3: Verify data files"

$verifyScript = Join-Path $PSScriptRoot "verify_data_files.ps1"
if (Test-Path $verifyScript) {
    & $verifyScript
    if ($LASTEXITCODE -ne 0) { $overallFailed += $LASTEXITCODE }
} else {
    Write-Host "  [FAIL] verify_data_files.ps1 not found at $verifyScript" -ForegroundColor Red
    $overallFailed++
}

# ================================================================
# STEP 4: Run heist.exe --selftest
# ================================================================
Step-Header "STEP 4: Run heist.exe --selftest"

$heistExe = Join-Path $root "x64\Debug\heist.exe"
if (Test-Path $heistExe) {
    # Run with working directory = TheLockerHeist so data files are reachable
    $output = & $heistExe --selftest 2>&1 | Tee-Object -Variable rawOutput
    $exitCode = $LASTEXITCODE

    # Save output to results file
    if (-not (Test-Path $resultDir)) { New-Item -ItemType Directory -Force $resultDir | Out-Null }
    $rawOutput | Out-File -FilePath $resultTxt -Encoding UTF8
    Write-Host "  Output saved to: $resultTxt"

    if ($exitCode -eq 0) {
        Write-Host "  [PASS] heist.exe --selftest exited 0"
    } else {
        Write-Host "  [FAIL] heist.exe --selftest exited $exitCode" -ForegroundColor Red
        $overallFailed++
    }
} else {
    Write-Host "  [SKIP] heist.exe not found at $heistExe (build may have been skipped)"
}

# ================================================================
# SUMMARY
# ================================================================
Write-Host ""
Write-Host "============================================"
if ($overallFailed -eq 0) {
    Write-Host "  All test steps PASSED." -ForegroundColor Green
} else {
    Write-Host "  $overallFailed step(s) FAILED." -ForegroundColor Red
}
Write-Host "============================================"
Write-Host ""

exit $overallFailed
