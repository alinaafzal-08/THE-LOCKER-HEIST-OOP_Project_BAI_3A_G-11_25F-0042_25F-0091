# run_tests.ps1
# Master test runner for The Locker Heist project.
# Steps:
#   1. Locate MSBuild via vswhere (or fallback to g++ if MSVC is not installed)
#   2. Build Debug|x64 executable (heist.exe)
#   3. Run verify_data_files.ps1
#   4. Run heist.exe --selftest (working directory = TheLockerHeist)
#   5. Save self-test output to tests/results/selftest.txt
#   6. Exit non-zero if any step failed

param()

$root      = Split-Path -Parent $PSScriptRoot
$slnFile   = Join-Path $root "TheLockerHeist.sln"
$projDir   = Join-Path $root "TheLockerHeist"
$outDir    = Join-Path $root "x64\Debug"
$heistExe  = Join-Path $outDir "heist.exe"
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
# STEP 1 & 2: Locate Compiler & Build Debug|x64
# ================================================================
Step-Header "STEP 1: Locate Compiler & Build Debug|x64"

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
    if ($vsPath) {
        $candidate = "$vsPath\MSBuild\Current\Bin\MSBuild.exe"
        if (Test-Path $candidate) { $msbuild = $candidate }
    }
}

if (-not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Force $outDir | Out-Null
}

$buildSucceeded = $false

if ($msbuild -and (Test-Path $slnFile)) {
    Write-Host "  MSBuild located: $msbuild"
    & $msbuild $slnFile /p:Configuration=Debug /p:Platform=x64 /m /nologo /v:minimal 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  [PASS] MSBuild Debug|x64 succeeded" -ForegroundColor Green
        $buildSucceeded = $true
    } else {
        Write-Host "  [FAIL] MSBuild failed with exit code $LASTEXITCODE" -ForegroundColor Red
        $overallFailed++
    }
} else {
    Write-Host "  [NOTE] MSBuild not found via vswhere. Checking for g++ compiler..."
    $gpp = where.exe g++.exe 2>$null
    if ($gpp) {
        Write-Host "  Compiler found: $gpp"
        & $gpp -Wall -Wextra -std=c++17 "$projDir\heist.cpp" "$projDir\main.cpp" -o "$heistExe" 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  [PASS] Build succeeded with g++ -> $heistExe" -ForegroundColor Green
            $buildSucceeded = $true
        } else {
            Write-Host "  [FAIL] g++ build failed with exit code $LASTEXITCODE" -ForegroundColor Red
            $overallFailed++
        }
    } else {
        Write-Host "  [FAIL] Neither MSBuild nor g++ is available. Cannot compile." -ForegroundColor Red
        $overallFailed++
    }
}

# ================================================================
# STEP 3: Verify data files
# ================================================================
Step-Header "STEP 2: Verify data files"

$verifyScript = Join-Path $PSScriptRoot "verify_data_files.ps1"
if (Test-Path $verifyScript) {
    & $verifyScript
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  [FAIL] verify_data_files.ps1 reported failures" -ForegroundColor Red
        $overallFailed += $LASTEXITCODE
    } else {
        Write-Host "  [PASS] Data files verification passed" -ForegroundColor Green
    }
} else {
    Write-Host "  [FAIL] verify_data_files.ps1 not found at $verifyScript" -ForegroundColor Red
    $overallFailed++
}

# ================================================================
# STEP 4: Run heist.exe --selftest
# ================================================================
Step-Header "STEP 3: Run heist.exe --selftest"

if (Test-Path $heistExe) {
    # Working directory MUST be TheLockerHeist so relative paths find data files
    Push-Location $projDir
    try {
        $rawOutput = & $heistExe --selftest 2>&1
        $exitCode = $LASTEXITCODE

        # Print output to console
        $rawOutput | ForEach-Object { Write-Host $_ }

        # Save output to tests/results/selftest.txt
        if (-not (Test-Path $resultDir)) { New-Item -ItemType Directory -Force $resultDir | Out-Null }
        $rawOutput | Out-File -FilePath $resultTxt -Encoding UTF8
        Write-Host "`n  Output saved to: $resultTxt"

        if ($exitCode -eq 0) {
            Write-Host "  [PASS] heist.exe --selftest exited 0 (all test suites passed)" -ForegroundColor Green
        } else {
            Write-Host "  [FAIL] heist.exe --selftest exited with error code $exitCode" -ForegroundColor Red
            $overallFailed++
        }
    }
    finally {
        Pop-Location
    }
} else {
    Write-Host "  [SKIP] heist.exe not found at $heistExe (compilation failed or skipped)" -ForegroundColor Yellow
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
