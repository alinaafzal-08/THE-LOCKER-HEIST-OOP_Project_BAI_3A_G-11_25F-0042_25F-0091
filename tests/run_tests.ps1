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
# STEP 5: Run End-to-End (E2E) automated flow tests
# ================================================================
Step-Header "STEP 4: Run End-to-End (E2E) Tests"

if (Test-Path $heistExe) {
    function Run-E2E([string]$testName, [string]$inputFile, [string]$heistArgs, [string[]]$requiredStrings) {
        Write-Host "  Running $testName..."
        $scoreBoardFile = Join-Path $projDir "leader\scoreBoard.txt"
        $scoreBoardBak  = Join-Path $projDir "leader\scoreBoard.txt.bak"
        $hasBackup = $false
        if (Test-Path $scoreBoardFile) {
            Copy-Item $scoreBoardFile $scoreBoardBak -Force
            $hasBackup = $true
        }

        $outLog = Join-Path $resultDir "$($testName).txt"
        $testPassed = $true

        Push-Location $projDir
        try {
            $fullInputPath = Join-Path $PSScriptRoot "inputs\$inputFile"
            $output = $null
            if ($heistArgs -ne "") {
                $argList = $heistArgs -split ' '
                $output = Get-Content $fullInputPath | & $heistExe $argList 2>&1
            } else {
                $output = Get-Content $fullInputPath | & $heistExe 2>&1
            }

            $output | Out-File -FilePath $outLog -Encoding UTF8

            $outputText = ($output -join "`n")
            foreach ($req in $requiredStrings) {
                if ($outputText -notmatch [regex]::Escape($req)) {
                    Write-Host "    [FAIL] Required text missing: '$req'" -ForegroundColor Red
                    $testPassed = $false
                }
            }
        }
        catch {
            Write-Host "    [FAIL] Exception during execution: $_" -ForegroundColor Red
            $testPassed = $false
        }
        finally {
            Pop-Location
            if ($hasBackup -and (Test-Path $scoreBoardBak)) {
                Copy-Item $scoreBoardBak $scoreBoardFile -Force
                Remove-Item $scoreBoardBak -Force
            }
        }

        if ($testPassed) {
            Write-Host "  [PASS] $testName passed" -ForegroundColor Green
        } else {
            Write-Host "  [FAIL] $testName failed" -ForegroundColor Red
            $script:overallFailed++
        }
    }

    # E2E-01: Main menu navigation (About, Rules, Leaderboard, Exit)
    Run-E2E "e2e01_menu" "e2e01_menu.txt" "" @(
        "THE LOCKER HEIST",
        "ABOUT THE GAME",
        "GAME RULES",
        "Max turns allowed",
        "Agent_007",
        "Goodbye"
    )

    # E2E-02: Name validation & space replacement
    Run-E2E "e2e02_name_validation" "e2e02_name_validation.txt" "" @(
        "Name cannot be empty",
        "Name must not contain '|'",
        "Your name contains spaces",
        "A culprit has been secretly chosen. The game begins!",
        "Credibility: 100",
        "Quit to main menu? No score will be recorded",
        "Goodbye"
    )

    # E2E-03: Forced culprits 1 to 7 game setup and clean quit
    for ($c = 1; $c -le 7; $c++) {
        Run-E2E "e2e03_culprit$c" "e2e03_quit.txt" "--culprit $c" @(
            "A culprit has been secretly chosen. The game begins!",
            "Quit to main menu? No score will be recorded",
            "Goodbye"
        )
    }

    # EOF handling: immediate EOF exits cleanly without hanging
    Run-E2E "e2e_eof" "e2e_eof.txt" "" @(
        "THE LOCKER HEIST"
    )

    # Invalid CLI argument: --culprit 9 prints usage and exits with error code
    Write-Host "  Running CLI argument validation (--culprit 9)..."
    Push-Location $projDir
    try {
        $badOutput = & $heistExe --culprit 9 2>&1
        $badExit = $LASTEXITCODE
        $badText = ($badOutput -join "`n")
        if ($badExit -ne 0 -and $badText -match "Usage:") {
            Write-Host "  [PASS] CLI validation (--culprit 9) exited non-zero with usage message" -ForegroundColor Green
        } else {
            Write-Host "  [FAIL] CLI validation (--culprit 9) did not report usage or exited 0" -ForegroundColor Red
            $overallFailed++
        }
    }
    finally {
        Pop-Location
    }
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
