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
identity.  You must make a correct accusation before your credibility
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
- **Visual Studio 2022** with the *Desktop development with C++* workload
- Git

### Build in Visual Studio (recommended)
1. Open `TheLockerHeist.sln`
2. Set the solution configuration to **x64 Debug**
3. Press **F5** to build and run  
   The working directory is `TheLockerHeist\` — data files are found automatically.

### Build from the command line (PowerShell)
```powershell
# From the repository root
.\tests\run_tests.ps1
```
This locates MSBuild via vswhere, builds Debug|x64, runs the data-file
verification script, and runs the self-test harness.

### Run manually
```powershell
# After building, from inside the TheLockerHeist folder:
cd TheLockerHeist
..\x64\Debug\heist.exe             # title banner
..\x64\Debug\heist.exe --selftest  # foundation self-tests (exit 0 = all pass)
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
    └── results\
        └── .gitkeep                   # keeps the results folder in git
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
generation.  Every team member reviews, understands, and can explain all
generated code.  The team is fully responsible for the final submission.

---

## Known Limitations

- **Build not yet verified** — Visual Studio C++ (MSVC) is not installed
  on the development machine at the time of Commit 1.  The build must be
  verified after VS 2022 is installed.
- Self-test ST-01 covers Module 1 types only; later commits add more tests.
- The game banner in `main.cpp` is a placeholder; the full game loop is
  added in Commit 8.

---

## Open Items

- [ ] Install Visual Studio 2022 (Desktop C++ workload) and verify the
      Debug|x64 and Release|x64 builds pass with zero warnings.
- [ ] Run `tests\verify_data_files.ps1` and `heist.exe --selftest` and
      save real output to `tests\results\`.
- [ ] Add `samreenkousar578` as a repository collaborator
      (Settings → Collaborators) so Commit 2 can be pushed.
- [ ] Alina should rewrite the five comment blocks noted in the review
      pause in her own words before confirming the commit.
