# Class Inventory — The Locker Heist
**Binding architecture contract.  Names, visibilities, and relationships here govern every commit.**
Last updated: Commit 3 (Alina Afzal, 25F-0042).

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

### Implemented in Commit 2 (`heist.h`, `heist.cpp`)

| Item | Kind | Fields | Notes |
|------|------|--------|-------|
| `RulesConfig` | `struct` | `int maxTurns=25; int startingCredibility=100; int wrongTurnPenalty=-5; int wrongWordPenalty=-10; std::string winCondition; std::string loseCondition; static RulesConfig defaultRules();` | Loaded from rules.txt; penalties stored signed negative |
| `ClueProfile` | `struct` | `std::string culpritRole; std::string word1; std::string word2;` | One row of clue.txt (7-letter word1, 8-letter word2) |
| `LocationRecord` | `struct` | `std::string placeName; int word1Pos; char letters[7];` | One row of locations.txt; letter column order: Cook[0]…Child[6] |
| `CharacterRecord` | `struct` | `std::string name; std::string role; std::string nationality; int word2Pos; char letters[7]; bool isCulprit;` | One row of characters.txt; same column order; isCulprit initialized false |

---

## Exception Hierarchy — Implemented in Commit 2 (Module 7)

All exceptions inherit from `GameException` (built from scratch, not derived from `std::exception`). Thrown by value, caught polymorphically by reference (`const GameException&`).

```
GameException (protected: message, errorCode; virtual ~GameException(); getMessage(); getErrorCode(); virtual getCategory())
├── InvalidInputException (code 101, "Invalid Input")
├── FileNotFoundException (code 102, "File Not Found")
├── FileCorruptException (code 103, "File Corrupt")
├── InvalidPositionException (code 104, "Invalid Position")
├── SlotFilledException (code 105, "Slot Filled")
├── InvalidLetterException (code 106, "Invalid Letter")
├── WordIncompleteException (code 107, "Word Incomplete")
├── InvalidWordException (code 108, "Invalid Word")
├── RackFullException (code 109, "Rack Full")
├── RackEmptyException (code 110, "Rack Empty")
└── InterviewRestrictedException (code 111, "Interview Restricted")
```

---

## Utilities — Implemented in Commit 2 (Module 6)

| Class | Kind | Responsibility & Methods |
|-------|------|--------------------------|
| `StringUtils` | `static` | `trim()`, `splitByDelimiter()`, `toUpperCase()`, `isAllLetters()`, `isSingleLetter()`, `parseInt()`, `intToString()`, `padWithZeros()`, `equalsIgnoreCase()`, `replaceChar()`, `underscoreToSpace()`, `spaceToUnderscore()`, `currentDateDDMMYY()`, `formatPersonName()`, `formatPlaceName()`. Implemented without `<cctype>`, `<algorithm>`, or `<sstream>`. |
| `DataLoader` | `static` | Constants: `FILE_RULES`, `FILE_CLUE`, `FILE_LOCATIONS`, `FILE_CHARACTERS`, `FILE_ABOUT`, `FILE_SCOREBOARD`. Dimensions: `ROLE_COUNT=7`, `LOCATION_COUNT=7`, `CHARACTER_COUNT=8`, `WORD1_LENGTH=7`, `WORD2_LENGTH=8`. Methods: `loadRules()`, `loadClueProfiles()`, `loadLocations()`, `loadCharacters()`, `loadAboutText()`, `validateConsistency()`, `roleToColumn()`. Strictly validates data format and throws `FileNotFoundException` or `FileCorruptException`. |

### Scheduled for Commit 7

| Class | Kind | Responsibility |
|-------|------|---------------|
| `ScoreBoard` | `static` | Reads and writes `leader/scoreBoard.txt`; formats two-digit turns and DD-MM-YY dates |

### Scheduled for Commit 8

| Class | Kind | Responsibility |
|-------|------|---------------|
| `ConsoleUtils` | `static` | ASCII borders, screen-clear (system()), menu rendering |

---

## Character Hierarchy — Implemented in Commit 3 (Module 3 & 9)

```
Character  [abstract; private: characterName, characterRole, nationality, isCulprit, liveCount; friend GameEngine]
│  pure virtual: display(std::ostream&) const
│  virtual:      getMinimumCredibility(); hasClue(); getClue()
│  static:       getLiveCount()
│
├── Victim             [minCred: 80; overrides hasClue (true), getClue (Word2), display]
├── Detective          [credibility, turns, dynamic string* solvedWords[2]; deep copy ctor & op=; Malay default; minCred: 0]
└── Suspect            [abstract; public Character; private: letterClueChar, boardPosChar, clueRowChar; hasClue (true), getClue]
    ├── HouseHelp      [public Suspect; minCred: 0]
    │   ├── Cook       [public HouseHelp; minCred: 0]
    │   ├── Guard      [public HouseHelp; minCred: 0]
    │   └── Servant    [public HouseHelp; minCred: 0]
    ├── BusinessRelations  [virtual public Suspect; minCred: 0]
    ├── CloseRelations     [virtual public Suspect; minCred: 0]
    ├── Partner        [public BusinessRelations; partnerNumber (1 or 2); minCred: 50]
    ├── PA             [public BusinessRelations, public CloseRelations; diamond resolved via virtual Suspect; minCred: 35]
    └── Child          [public CloseRelations; static const DEFAULT_AGE=17; age validated 15-19; minCred: 75]
```

**CharacterFactory** — `static Character* create(const CharacterRecord& record, int culpritColumn);`
Sole polymorphic construction hub based on role strings; throws `InvalidInputException` for unknown roles.

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
- `GameException` data members: **protected**
