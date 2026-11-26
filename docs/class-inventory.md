# Class Inventory — The Locker Heist
**Binding architecture contract.  Names, visibilities, and relationships here govern every commit.**
Last updated: Commit 1 (Alina Afzal, 25F-0042).

---

## Enums and Structs

### Defined in Commit 1 (`heist.h`)

| Item | Kind | Fields / Values | Notes |
|------|------|-----------------|-------|
| `WordRow` | `enum class` | `Word1 = 0`, `Word2 = 1` | Used as array index in TileRack |
| `GameState` | `enum class` | `Menu`, `Playing`, `Won`, `Lost`, `Exit` | Controls game-loop branching |
| `MenuChoice` | `enum class` | `NewGame=1`, `ViewLeaderBoard=2`, `About=3`, `Rules=4`, `Exit=5` | Cast from player int input |
| `Tile` | `struct` | `char tileLetter=' '`; `bool filled=false` | One cell on a TileRack or Scrabble board |
| `Clue` | `struct` | `char clueLetter=' '`; `int cluePosition=0`; `WordRow clueRow=Word1` | Letter hint discovered during investigation |
| `Location` | `struct` | `std::string locName=""`; `char letterClueLoc=' '`; `int boardPosLoc=0`; `WordRow locRow=Word1`; `bool locSearchFlag=false` | One searchable room in the mansion |

### Defined in Commit 2 (`heist.h`)

| Item | Kind | Fields | Notes |
|------|------|--------|-------|
| `RulesConfig` | `struct` | `int maxTurns`; `int startingCredibility`; `int wrongTurnPenalty`; `int wrongWordPenalty`; `std::string winCondition`; `std::string loseCondition` | Loaded from rules.txt; penalties stored signed |
| `ClueProfile` | `struct` | `std::string culpritRole`; `std::string word1`; `std::string word2` | One row of clue.txt |
| `LocationRecord` | `struct` | `std::string placeName`; `int word1Pos`; `char letters[7]` | One row of locations.txt; letter column order: Cook[0]…Child[6] |
| `CharacterRecord` | `struct` | `std::string name`; `std::string role`; `std::string nationality`; `int word2Pos`; `char letters[7]`; `bool isCulprit` | One row of characters.txt; same column order |

---

## Exception Hierarchy — Commit 2

All exceptions inherit from `GameException`.

```
GameException (message, errorCode; virtual destructor; getMessage(); getErrorCode(); getCategory())
├── InvalidInputException
├── FileNotFoundException
├── FileCorruptException
├── InvalidPositionException
├── SlotFilledException
├── InvalidLetterException
├── WordIncompleteException
├── InvalidWordException
├── RackFullException
├── RackEmptyException
└── InterviewRestrictedException
```

---

## Utilities — Commit 2

| Class | Kind | Responsibility |
|-------|------|---------------|
| `StringUtils` | `static` | String helpers (split, trim, toUpper, etc.) written without STL algorithms |
| `DataLoader` | `static` | Reads clue.txt, locations.txt, characters.txt, rules.txt; throws on error |

### Commit 7

| Class | Kind | Responsibility |
|-------|------|---------------|
| `ScoreBoard` | `static` | Reads and writes `leader/scoreBoard.txt`; formats two-digit turns and DD-MM-YY dates |

### Commit 8

| Class | Kind | Responsibility |
|-------|------|---------------|
| `ConsoleUtils` | `static` | ASCII borders, screen-clear (system()), menu rendering |

---

## Character Hierarchy — Commit 3

```
Character  [abstract; private: name, role, nationality, isCulprit; friend GameEngine]
│  pure virtual: display(std::ostream&) const
│  virtual:      getMinimumCredibility(); getClue()
│
├── Victim
├── Detective  [credibility, turns; solvedWords heap array[2]; racks from C5]
└── Suspect  [abstract]
    ├── HouseHelp  [abstract]
    │   ├── Cook
    │   ├── Guard
    │   └── Servant
    ├── BusinessRelations  [virtual public Suspect]
    ├── CloseRelations     [virtual public Suspect]
    ├── Partner            [BusinessRelations]
    ├── PA                 [BusinessRelations AND CloseRelations — diamond]
    └── Child              [CloseRelations; age const = 17, validated 15–19]
```

**CharacterFactory** — `static` create method; Commit 3.

---

## Board and Container Classes

| Class | Commit | Responsibility |
|-------|--------|---------------|
| `TileRack` | C4 | Fixed-size array of `Tile`; owns tiles; operator<<; friend of GameEngine |
| `Scrabble` | C4 | Two `TileRack` rows; owns `Clue` dependency; operator<< |
| `Cottage` | C5 | Array of `Location`; drawn as aggregation |
| `DynamicArray<T>` | C6 | Custom heap array template (no `<vector>`) |
| `GameEngine` | C8 | Singleton; owns Detective, Cottage, Scrabble, combined-clue TileRack, 8 Character* |

---

## Relationships Summary

| Relationship | Between | Type |
|-------------|---------|------|
| Owns | GameEngine → Detective | Composition |
| Owns | GameEngine → Cottage | Composition |
| Owns | GameEngine → Scrabble | Composition |
| Owns | GameEngine → TileRack (combined clue) | Composition |
| Owns | GameEngine → Character* [8] | Composition |
| Owns | Detective → TileRack [2] | Composition |
| Uses | Cottage → Location[] | Aggregation |
| Uses | TileRack → Tile[] | Aggregation |
| Uses | Scrabble → Tile rows | Aggregation |
| Points to | GameEngine → ScoreBoard | Association (main() owns ScoreBoard) |
| Uses | GameEngine → GameException | Dependency |
| Uses | DataLoader → GameException | Dependency |
| Uses | Scrabble → Clue | Dependency |
| Access | GameEngine ↔ Character | Friend |
| Output | operator<< for Tile, Character, Location, Scrabble | Friend function |
| Pattern | GameEngine | Singleton |
| Inheritance | PA | Diamond (BusinessRelations + CloseRelations, both virtual public Suspect) |

---

## Visibility Quick-Reference

- `Character` data members (name, role, nationality, isCulprit): **private**; accessible via `friend class GameEngine`
- Word-2 attributes in `Character` subclasses: **private** with public getters (Decision R14)
- `GameEngine` constructor: **private** (Singleton)
- `CharacterFactory::create`: **public static**
- `DataLoader` and `StringUtils` members: **public static**
