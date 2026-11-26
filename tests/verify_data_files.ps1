# verify_data_files.ps1
# Validates that every instructor-provided data file exists, has the correct
# heading row, and that the content rows satisfy the game's structural rules.
# Exit code 0 = all checks passed.  Exit code 1 = at least one check failed.
# Run this script from anywhere; it locates the project by its own path.

param()

$root     = Split-Path -Parent $PSScriptRoot          # repo root
$dataDir  = Join-Path $root "TheLockerHeist"           # runtime working dir
$ldrDir   = Join-Path $dataDir "leader"
$failed   = 0

function Fail([string]$msg) {
    Write-Host "  [FAIL] $msg" -ForegroundColor Red
    $script:failed++
}

function Pass([string]$msg) {
    Write-Host "  [PASS] $msg" -ForegroundColor Green
}

Write-Host ""
Write-Host "============================================"
Write-Host "  verify_data_files.ps1"
Write-Host "============================================"

# ----------------------------------------------------------------
# Helper: read all lines of a file and strip trailing blank lines
# ----------------------------------------------------------------
function Get-DataLines([string]$path) {
    $lines = Get-Content $path -Encoding UTF8
    while ($lines.Count -gt 0 -and $lines[-1].Trim() -eq "") {
        $lines = $lines[0..($lines.Count - 2)]
    }
    return $lines
}

# ================================================================
# 1. FILE EXISTENCE
# ================================================================
Write-Host "`n--- File existence ---"

$requiredFiles = @(
    (Join-Path $dataDir "clue.txt"),
    (Join-Path $dataDir "locations.txt"),
    (Join-Path $dataDir "characters.txt"),
    (Join-Path $dataDir "rules.txt"),
    (Join-Path $dataDir "about.txt"),
    (Join-Path $ldrDir  "scoreBoard.txt")
)

foreach ($f in $requiredFiles) {
    if (Test-Path $f) { Pass "Exists: $(Split-Path -Leaf $f)" }
    else              { Fail "MISSING: $f" }
}

# ================================================================
# 2. clue.txt
# ================================================================
Write-Host "`n--- clue.txt ---"

$clueFile = Join-Path $dataDir "clue.txt"
if (Test-Path $clueFile) {
    $clueLines = Get-DataLines $clueFile

    # Heading row
    if ($clueLines[0] -eq "Culprit_Role|Word_1|Word_2") { Pass "clue.txt heading correct" }
    else { Fail "clue.txt heading wrong: '$($clueLines[0])'" }

    # 7 data rows
    $clueData = $clueLines[1..$($clueLines.Count - 1)]
    if ($clueData.Count -eq 7) { Pass "clue.txt has 7 data rows" }
    else { Fail "clue.txt has $($clueData.Count) data rows (expected 7)" }

    # Each row: 3 pipe-separated fields; Word_1 is 7 letters uppercase;
    # Word_2 is 7 or 8 letters uppercase (SPAGHETI is 8, most are 7)
    $roles = @()
    foreach ($line in $clueData) {
        $parts = $line -split '\|'
        if ($parts.Count -ne 3) { Fail "clue.txt bad field count in: '$line'"; continue }
        $roles += $parts[0]
        $w1 = $parts[1]
        $w2 = $parts[2]
        if ($w1 -cmatch '^[A-Z]{7}$')   { Pass "clue.txt Word_1 '$w1' is 7 uppercase letters" }
        else                             { Fail "clue.txt Word_1 '$w1' is not 7 uppercase letters" }
        if ($w2 -cmatch '^[A-Z]{7,8}$') { Pass "clue.txt Word_2 '$w2' is 7-8 uppercase letters" }
        else                             { Fail "clue.txt Word_2 '$w2' is not 7-8 uppercase letters" }
    }

    # Verify the 7 culprit roles are the expected ones
    $expectedRoles = @("Cook","Guard","Servant","Partner 1","Partner 2","PA","Child")
    foreach ($er in $expectedRoles) {
        if ($roles -contains $er) { Pass "clue.txt contains role '$er'" }
        else                      { Fail "clue.txt missing role '$er'" }
    }
}

# ================================================================
# 3. locations.txt  -- and cross-check Word_1 letters
# ================================================================
Write-Host "`n--- locations.txt ---"

$locFile = Join-Path $dataDir "locations.txt"
if (Test-Path $locFile) {
    $locLines = Get-DataLines $locFile

    if ($locLines[0] -eq "Place|W1Pos|Cook|Guard|Servant|P1|P2|PA|Child") { Pass "locations.txt heading correct" }
    else { Fail "locations.txt heading wrong: '$($locLines[0])'" }

    $locData = $locLines[1..$($locLines.Count - 1)]
    if ($locData.Count -eq 7) { Pass "locations.txt has 7 data rows" }
    else { Fail "locations.txt has $($locData.Count) data rows (expected 7)" }

    # W1Pos 1 to 7 unique
    $w1Positions = @()
    foreach ($line in $locData) {
        $parts = $line -split '\|'
        if ($parts.Count -ne 9) { Fail "locations.txt bad field count in: '$line'"; continue }
        $pos = [int]$parts[1]
        if ($w1Positions -contains $pos) { Fail "locations.txt duplicate W1Pos $pos" }
        else { $w1Positions += $pos; Pass "locations.txt W1Pos $pos is unique" }
    }
    $sorted = $w1Positions | Sort-Object
    if (($sorted -join ',') -eq '1,2,3,4,5,6,7') { Pass "locations.txt W1Pos covers 1-7" }
    else { Fail "locations.txt W1Pos does not cover 1-7: $($w1Positions -join ',')" }

    # Build Word_1 for each role from the location letters (sorted by W1Pos)
    # Column indices in locations.txt: Cook=2, Guard=3, Servant=4, P1=5, P2=6, PA=7, Child=8
    $roleColIdx = @{ "Cook"=2; "Guard"=3; "Servant"=4; "Partner 1"=5; "Partner 2"=6; "PA"=7; "Child"=8 }

    # Load clue.txt Word_1 for comparison
    $clueWord1 = @{}
    $clueRoleMap = @{"Cook"=0;"Guard"=0;"Servant"=0;"Partner 1"=0;"Partner 2"=0;"PA"=0;"Child"=0}
    if (Test-Path $clueFile) {
        foreach ($cl in (Get-DataLines $clueFile)[1..7]) {
            $cp = $cl -split '\|'
            if ($cp.Count -eq 3) { $clueWord1[$cp[0]] = $cp[1] }
        }
    }

    # Sort loc rows by W1Pos
    $sortedLoc = $locData | Sort-Object { [int]($_ -split '\|')[1] }

    foreach ($role in $roleColIdx.Keys) {
        $col    = $roleColIdx[$role]
        $built  = ""
        foreach ($line in $sortedLoc) {
            $parts = $line -split '\|'
            if ($parts.Count -gt $col) { $built += $parts[$col] }
        }
        $expected = $clueWord1[$role]
        if ($built -eq $expected) { Pass "locations.txt Word_1 for '$role' = '$built'" }
        else                      { Fail "locations.txt Word_1 for '$role': built '$built', expected '$expected'" }
    }
}

# ================================================================
# 4. characters.txt  -- and cross-check Word_2 letters
# ================================================================
Write-Host "`n--- characters.txt ---"

$charFile = Join-Path $dataDir "characters.txt"
if (Test-Path $charFile) {
    $charLines = Get-DataLines $charFile

    if ($charLines[0] -eq "Name|Role|Nationality|W2Pos|Cook|Guard|Servant|P1|P2|PA|Child|isCulprit") { Pass "characters.txt heading correct" }
    else { Fail "characters.txt heading wrong: '$($charLines[0])'" }

    $charData = $charLines[1..$($charLines.Count - 1)]
    if ($charData.Count -eq 8) { Pass "characters.txt has 8 data rows" }
    else { Fail "characters.txt has $($charData.Count) data rows (expected 8)" }

    # W2Pos 1 to 8 unique; isCulprit all 0
    $w2Positions = @()
    foreach ($line in $charData) {
        $parts = $line -split '\|'
        if ($parts.Count -ne 12) { Fail "characters.txt bad field count in: '$line'"; continue }
        $pos = [int]$parts[3]
        if ($w2Positions -contains $pos) { Fail "characters.txt duplicate W2Pos $pos" }
        else { $w2Positions += $pos; Pass "characters.txt W2Pos $pos is unique" }
        if ($parts[11] -eq "0") { Pass "characters.txt isCulprit=0 for $($parts[0])" }
        else { Fail "characters.txt isCulprit != 0 for $($parts[0]): '$($parts[11])'" }
    }
    $sorted2 = $w2Positions | Sort-Object
    if (($sorted2 -join ',') -eq '1,2,3,4,5,6,7,8') { Pass "characters.txt W2Pos covers 1-8" }
    else { Fail "characters.txt W2Pos does not cover 1-8: $($w2Positions -join ',')" }

    # Cross-check Word_2 against clue.txt
    # Column indices in characters.txt: Cook=4, Guard=5, Servant=6, P1=7, P2=8, PA=9, Child=10
    $roleColIdx2 = @{ "Cook"=4; "Guard"=5; "Servant"=6; "Partner 1"=7; "Partner 2"=8; "PA"=9; "Child"=10 }

    $clueWord2 = @{}
    if (Test-Path $clueFile) {
        foreach ($cl in (Get-DataLines $clueFile)[1..7]) {
            $cp = $cl -split '\|'
            if ($cp.Count -eq 3) { $clueWord2[$cp[0]] = $cp[2] }
        }
    }

    $sortedChar = $charData | Sort-Object { [int]($_ -split '\|')[3] }

    foreach ($role in $roleColIdx2.Keys) {
        $col   = $roleColIdx2[$role]
        $built = ""
        foreach ($line in $sortedChar) {
            $parts = $line -split '\|'
            if ($parts.Count -gt $col) { $built += $parts[$col] }
        }
        $expected = $clueWord2[$role]
        if ($built -eq $expected) { Pass "characters.txt Word_2 for '$role' = '$built'" }
        else                      { Fail "characters.txt Word_2 for '$role': built '$built', expected '$expected'" }
    }
}

# ================================================================
# 5. rules.txt  -- 6 required keys present
# ================================================================
Write-Host "`n--- rules.txt ---"

$rulesFile = Join-Path $dataDir "rules.txt"
if (Test-Path $rulesFile) {
    $rulesContent = Get-Content $rulesFile -Encoding UTF8
    $requiredKeys = @("Max_Turns","Starting_Credibility","Every_Wrong_Turn","Every_Wrong_Word","Win_Condition","Lose_Condition")
    foreach ($key in $requiredKeys) {
        $found = $rulesContent | Where-Object { $_ -match "^${key}=" }
        if ($found) { Pass "rules.txt has key '$key'" }
        else        { Fail "rules.txt missing key '$key'" }
    }
}

# ================================================================
# 6. scoreBoard.txt  -- heading and valid rows
# ================================================================
Write-Host "`n--- leader/scoreBoard.txt ---"

$sbFile = Join-Path $ldrDir "scoreBoard.txt"
if (Test-Path $sbFile) {
    $sbLines = Get-DataLines $sbFile

    if ($sbLines[0] -eq "Detective|Result|Credibility|Turns_Left|Culprit|Date") { Pass "scoreBoard.txt heading correct" }
    else { Fail "scoreBoard.txt heading wrong: '$($sbLines[0])'" }

    if ($sbLines.Count -ge 2) {
        foreach ($line in $sbLines[1..$($sbLines.Count - 1)]) {
            $parts = $line -split '\|'
            if ($parts.Count -ne 6) { Fail "scoreBoard.txt bad field count: '$line'"; continue }
            # Result must be WIN or LOSE
            if ($parts[1] -match '^(WIN|LOSE)$') { Pass "scoreBoard.txt Result valid: '$($parts[1])'" }
            else { Fail "scoreBoard.txt Result invalid: '$($parts[1])' in '$line'" }
            # Turns_Left is two digits
            if ($parts[3] -match '^\d{2}$') { Pass "scoreBoard.txt Turns_Left two-digit: '$($parts[3])'" }
            else { Fail "scoreBoard.txt Turns_Left not two-digit: '$($parts[3])'" }
            # Date is DD-MM-YY
            if ($parts[5] -match '^\d{2}-\d{2}-\d{2}$') { Pass "scoreBoard.txt Date format valid: '$($parts[5])'" }
            else { Fail "scoreBoard.txt Date format invalid: '$($parts[5])'" }
        }
    } else {
        Fail "scoreBoard.txt has no data rows"
    }
}

# ================================================================
# SUMMARY
# ================================================================
Write-Host ""
Write-Host "============================================"
if ($failed -eq 0) {
    Write-Host "  Result: ALL CHECKS PASSED" -ForegroundColor Green
} else {
    Write-Host "  Result: $failed FAILURE(S) DETECTED" -ForegroundColor Red
}
Write-Host "============================================"
Write-Host ""

exit $failed
