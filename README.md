# The Locker Heist — OOP Project (Fall 2026)

**CL1004 Object Oriented Programming | Fall 2026**  
**FAST-NUCES Faisalabad-Chiniot Campus**

---

## Game Description

*The Locker Heist* is a console-based detective mystery game written in C++.
A priceless artefact has been stolen from the mansion of Mr. Leonardo.
Seven suspects live under the same roof, but only one is the culprit.
As a detective, you search locations, interview suspects, collect letter
clues, and use them to form two secret words that reveal the culprit's
identity. You must make a correct accusation before your credibility
runs out or your turns are exhausted.

---

## Team

| Name | Roll Number | GitHub |
|------|-------------|--------|
| Alina Afzal | 25F-0042 | [alinaafzal-08](https://github.com/alinaafzal-08) |
| Samreen Kousar | 25F-0091 | [samreenkousar578](https://github.com/samreenkousar578) |

---

## Repository

`OOP_Project_BAI_A_G11_25F-0042_25F-0091`

---

## How to Build and Run

### Prerequisites
- **Visual Studio 2022** with the *Desktop development with C++* workload (or MinGW-w64 `g++` with C++17 support)
- Git

### Build in Visual Studio (recommended)
1. Open `TheLockerHeist.sln`
2. Set the solution configuration to **x64 Debug**
3. Press **F5** to build and run  
   The working directory is `TheLockerHeist\` — data files are found automatically.

### Automated Build & Test Suite (PowerShell)
```powershell
# From the repository root
.\tests\run_tests.ps1
```
This script:
1. Locates MSBuild via `vswhere` (or falls back to `g++` if MSVC is not installed).
2. Builds the `Debug|x64` target executable (`heist.exe`).
3. Runs `tests\verify_data_files.ps1` to validate all instructor data files.
4. Runs `heist.exe --selftest` with working directory set to `TheLockerHeist\`.
5. Logs results to `tests\results\selftest.txt`.

### Run manually
```powershell
# From inside the TheLockerHeist folder:
cd TheLockerHeist
..\x64\Debug\heist.exe               # starts interactive GameEngine main menu (Commit 8)
..\x64\Debug\heist.exe --selftest    # executes ST-01 through ST-08 self-tests
..\x64\Debug\heist.exe --culprit 1   # runs game forcing Cook (column 1) as culprit
```

---

## Folder Map

```
The-Locker-Heist-OOP-Project\
├── TheLockerHeist.sln                  # Visual Studio solution
├── .gitignore
├── .gitattributes
├── README.md
│
├── TheLockerHeist\                     # VS project + runtime working directory
│   ├── TheLockerHeist.vcxproj
│   ├── TheLockerHeist.vcxproj.filters
│   ├── heist.h                         # all declarations
│   ├── heist.cpp                       # all definitions
│   ├── main.cpp                        # driver + self-test harness
│   ├── clue.txt
│   ├── locations.txt
│   ├── characters.txt
│   ├── rules.txt
│   ├── about.txt
│   └── leader\
│       └── scoreBoard.txt
│
├── docs\
│   ├── class-inventory.md              # binding architecture contract
│   └── decisions.md                   # binding design decisions
│
└── tests\
    ├── verify_data_files.ps1           # validates all data files
    ├── run_tests.ps1                   # builds + runs all tests
    ├── inputs\                         # automated E2E stdin test scripts
    │   ├── e2e01_menu.txt
    │   ├── e2e02_name_validation.txt
    │   ├── e2e03_quit.txt
    │   ├── e2e04_location.txt
    │   ├── e2e05_interview.txt
    │   ├── e2e06_credibility_restrictions.txt
    │   ├── e2e07_collection_display.txt
    │   ├── e2e11_lose_credibility.txt
    │   ├── e2e12_lose_turns.txt
    │   ├── e2e13_turn_credibility_board.txt
    │   ├── e2e18_quit_lose.txt
    │   └── e2e_eof.txt
    └── results\
        ├── .gitkeep                   # keeps results folder in version control
        ├── selftest.txt               # logged self-test run output
        └── e2e*.txt                   # logged E2E automated test run outputs
```

---

## Data Files

| File | Description |
|------|-------------|
| `clue.txt` | 7 culprit roles, each with two 7-8 letter secret words |
| `locations.txt` | 7 searchable rooms with per-role letter clues |
| `characters.txt` | 8 characters (1 victim + 7 suspects) with per-role attributes |
| `rules.txt` | Game parameters (turns, credibility, penalties) |
| `about.txt` | About screen shown from the main menu |
| `leader/scoreBoard.txt` | Persistent leaderboard (Detective, Result, Credibility, Turns_Left, Culprit, Date) |

> **Important:** These files are part of the instructor's dataset.
> Their content, field order, and heading rows must never be changed.

---

## AI-Use Statement

This project was developed with AI assistance (Google Antigravity) in
accordance with the CL1004 OOP Project Manual, Section 5.2, Instruction 9
(page 1), which explicitly permits AI tools for development including code
generation. Every team member reviews, understands, and can explain all
generated code. The team is fully responsible for the final submission.

---

## Self-Test Suites

| Suite | Scope | Status |
|-------|-------|--------|
| **ST-01** | Module 1 Foundation Types (Tile, Clue, Location, WordRow, GameState, MenuChoice) | 27 / 27 PASS |
| **ST-02** | Module 6 & 7 (GameException family, StringUtils, DataLoader, Corruption Detection) | 40 / 40 PASS |
| **ST-03** | Module 3 & 9 (Character hierarchy, Diamond inheritance, Factory, Deep Copy, Virtual Destructors) | 20 / 20 PASS |
| **ST-04** | Module 4 & 5 (TileRack parallel arrays, Scrabble jagged 2D array, Raw DMA, Deep Copy, Bounds, Exceptions) | 39 / 39 PASS |
| **ST-05** | Module 4 & 5 (Cottage raw DMA array of pointers, Detective composed racks, Word 1 search & verification) | 34 / 34 PASS |
| **ST-06** | Module 8 (DynamicArray<T> generic container, auto-doubling, deep copy, refactored TileRack/Scrabble/Cottage) | 29 / 29 PASS |
| **ST-07** | Module 6 & 11 (ScoreBoard persistence & ASCII table, equality operators, TileRack::operator+ with selection sort & clue text, friend stream operators) | 41 / 41 PASS |
| **ST-08** | Module 2 & 12 (GameEngine Singleton, forced culprit setup 1-7, cast construction, Cottage & Scrabble target synchronization, sanitiseName, loadRulesOrDefaults) | 77 / 77 PASS |
| **ST-09** | Module 13 & 14 (Exploration, Location Visits, Suspect Interviews, Polymorphic Minimum Credibility Thresholds, Turn & Credibility Penalties, LOSE Conditions) | 32 / 32 PASS |
| **E2E Tests** | Automated end-to-end flow tests (E2E-01 to E2E-07, E2E-11 to E2E-14, E2E-18, EOF handling, CLI validation) | 20 / 20 PASS |

---

## Known Limitations

- Visual Studio MSVC is not yet installed on this workstation environment; automated tests run through `g++` (C++17) during local testing, producing identical standard-compliant behavior.
- Word solving ([4] Form a Word) and formal accusations ([5] Accuse Culprit / WIN condition) arrive in Commit 10. Commit 9 delivers location visits, suspect interviews with credibility thresholds, turn counting, credibility penalties, and game-over LOSE handling.
