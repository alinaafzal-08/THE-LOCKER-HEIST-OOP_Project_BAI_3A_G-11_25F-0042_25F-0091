# Design Decisions — The Locker Heist
**Binding rulings that govern every commit in this project.**
Last updated: Commit 1 (Alina Afzal, 25F-0042).

---

> [!IMPORTANT]
> These rulings are a contract.  Later commits must not contradict them.
> If a ruling must change, record the amendment here with the commit number and reason.

---

## R2 — Allowed Headers

Only these headers are permitted in any source file:

| Header | Reason |
|--------|--------|
| `<iostream>` | Console I/O (`std::cin`, `std::cout`) |
| `<fstream>` | File I/O for data files and scoreboard |
| `<string>` | `std::string` for name and role fields (manual specifies string members) |
| `<cstdlib>` | `rand`, `srand`, `system()` for screen-clear |
| `<ctime>` | `time()` to seed the RNG |
| `<cstdio>` | `std::remove()` in self-tests only |
| `<cstring>` | `std::strcmp()` in `main.cpp` self-test argument check only |

The manual forbids: `<vector>`, `<map>`, `<algorithm>`, `<sstream>`, `<iomanip>`, `<cctype>`, `<memory>`, `<utility>`, and similar STL containers/algorithms.

---

## R3 — Enum Class Names

The following `enum class` identifiers are fixed and must not be renamed:

- `WordRow` with `Word1 = 0`, `Word2 = 1`
- `GameState` with `Menu`, `Playing`, `Won`, `Lost`, `Exit`
- `MenuChoice` with `NewGame=1`, `ViewLeaderBoard=2`, `About=3`, `Rules=4`, `Exit=5`

---

## R5 — Characters File Structure

`characters.txt` has **8 data rows** (1 victim + 7 suspects).  
The culprit is chosen only from the **7 suspects** (all except Mr. Leonardo / Victim).  
Characters are matched by their **Role text** (e.g. "Cook", "Partner 1") not by row index.

---

## R6 — File Spelling Conventions

- In `clue.txt` and `characters.txt`: use `Partner 1` and `Partner 2` with a **space**.
- In `leader/scoreBoard.txt` Culprit column: use **underscores** (`Partner_2`).
- These spellings are determined by the instructor's sample data and must not be changed.

---

## R8 — Penalty Storage

Penalties are stored as **signed negative integers** (`-5`, `-10`) and **added** to the credibility score.  
No subtraction operator is used; this keeps arithmetic consistent: `credibility += penalty`.

---

## R9 — Scoreboard Formatting

| Field | Format |
|-------|--------|
| `Turns_Left` | Always **two digits** (e.g. `05`, `07`) |
| `Date` | `DD-MM-YY` (e.g. `11-10-26`) |

---

## R10 — File Loading and Error Handling

| File | Behaviour on Error |
|------|--------------------|
| `rules.txt` | **Fall back to defaults** (maxTurns=25, startingCredibility=100, wrongTurnPenalty=-5, wrongWordPenalty=-10) rather than crashing |
| `clue.txt`, `locations.txt`, `characters.txt` | **Throw** (FileNotFoundException or FileCorruptException); exception propagates to `main()` which ends the program cleanly with an error message |

DataLoader methods always throw; `main()` catches fatal exceptions and prints a user-friendly message before exiting.

---

## R11 — Culprit Selection

The culprit is chosen **once per New Game**, at the start of `GameEngine::startNewGame()`.  
The choice uses `rand() % 7` to pick from the 7 suspects.  
The culprit is not changed during a game session.

---

## R12 — Exception Catching Scope

| Scope | Catches |
|-------|---------|
| Inside `GameEngine` | Recoverable exceptions (e.g. InvalidInputException, SlotFilledException) — handled silently or with a user message |
| `main()` | Fatal exceptions (FileNotFoundException, FileCorruptException) — prints error and exits |

---

## R13 — Character is Abstract

`Character` is an abstract base class from Commit 3 onwards.  
It has at least one pure virtual method: `virtual void display(std::ostream&) const = 0`.  
No `Character` object is instantiated directly; only concrete subclasses are.

---

## R14 — Word-2 Attributes are Private

Word-2 character attributes (the letters that fill Word 2 of the Scrabble board) are **private** members of `Character` subclasses.  
Access is provided through **public getter methods** only.  
Direct field access from outside the class hierarchy is not permitted.

---

## R15 — TileRack Ownership

| Owner | TileRack(s) |
|-------|-------------|
| `Detective` | Two TileRacks (one per word row) — composition |
| `GameEngine` | One combined-clue TileRack (holds the merged clue tile sequence) — composition |

---

## R16 — Cottage-Location Relationship

The `Cottage` class holds an **array of Location** objects.  
This is drawn as **aggregation** in the UML diagram (open diamond at Cottage, arrow to Location), because Location is a plain struct that could exist independently of Cottage.

---

## R21 — Positions are 1-Based

All `W1Pos` and `W2Pos` values in the data files are **1-based** (1 to 7 / 1 to 8).  
Array indices used internally are **0-based** (subtract 1 when indexing).  
This convention must be respected in all DataLoader and TileRack code.

---

## R26 — Detective Nationality Default

The `Detective`'s `nationality` field defaults to **"Malay"** when not set by the player.

---

## R28 — Child Age Validation

The `Child` character has an `age` field stored as a **compile-time constant**: `static const int AGE = 17`.  
Valid range is **15 to 19** inclusive; the constructor (Commit 3) must validate and throw `InvalidInputException` if out of range.

---

## Game Rules D1 to D8

| Rule | Detail |
|------|--------|
| D1 | Wrong step = confirmed repeat search or interview already done → costs 1 turn and `-5` credibility |
| D2 | Interview is refused if the detective's credibility is below `getMinimumCredibility()` for that suspect |
| D3 | Wrong accusation = costs 1 turn and `-5` credibility |
| D4 | Wrong word = costs 1 turn and `-10` credibility |
| D5 | First visit to a location or first interview of a character costs 1 turn |
| D6 | Each "Form a Word" attempt costs 1 turn |
| D7 | Declining Y/N prompts, pressing [0] Back, viewing the board, and entering invalid input are **free** (no turn consumed) |
| D8 | After every action: check WIN first (correct accusation), then LOSE (credibility ≤ 0 OR turns used ≥ maxTurns) |

---

## Developer Hooks

| Flag | Behaviour |
|------|-----------|
| `--selftest` | Run all self-tests; exit non-zero if any fail |
| `--culprit N` | Force the culprit to suspect N (1=Cook, 2=Guard, 3=Servant, 4=Partner 1, 5=Partner 2, 6=PA, 7=Child) for testing |

---

## UI Conventions

- ASCII borders (no Unicode box-drawing characters)
- Sub-menus always include `[0] Back` to return to the previous menu
- Quit from in-game asks for confirmation and records a LOSE result in the scoreboard

---

## Open Items (Commit 1)

- [ ] Verify build passes with zero /W4 warnings after Visual Studio is installed.
- [ ] Record actual self-test output in `tests/results/selftest.txt`.
