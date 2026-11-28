# Class Inventory — The Locker Heist
**Binding architecture contract.  Names, visibilities, and relationships here govern every commit.**
Last updated: Commit 7 (Alina Afzal, 25F-0042).

---

## Enums and Structs

### Defined in Commit 1 & Extended in Commit 7 (`heist.h`, `heist.cpp`)

| Item | Kind | Fields / Values / Operators | Notes |
|------|------|-----------------------------|-------|
| `WordRow` | `enum class` | `Word1 = 0`, `Word2 = 1` | Used as array index in TileRack |
| `GameState` | `enum class` | `Menu`, `Playing`, `Won`, `Lost`, `Exit` | Controls game-loop branching |
| `MenuChoice` | `enum class` | `NewGame=1`, `ViewLeaderBoard=2`, `About=3`, `Rules=4`, `Exit=5` | Cast from player int input |
| `Tile` | `struct` | `char tileLetter=' '`; `bool filled=false`; <br> `bool operator==(const Tile&) const; bool operator!=(const Tile&) const;` <br> `friend operator<<; friend operator>>;` | One cell on a TileRack or Scrabble board. `operator==` compares `tileLetter` only per manual. Stream operators print `[L]` or `[ ]`, and extract character marking filled. |
| `Clue` | `struct` | `char clueLetter=' '`; `int cluePosition=0`; `WordRow clueRow=Word1`; <br> `bool operator==(const Clue&) const; bool operator!=(const Clue&) const;` | Letter hint discovered during investigation. `operator==` compares letter, position, and row. |
| `Location` | `struct` | `std::string locName=""`; `char letterClueLoc=' '`; `int boardPosLoc=0`; `WordRow locRow=Word1`; `bool locSearchFlag=false`; <br> `friend operator<<; friend operator>>;` | One searchable room in the mansion. `operator<<` renders name and search status (never exposes `letterClueLoc`). `operator>>` extracts name, letter, and position. |

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

## Utilities — Implemented in Commit 2 & 7 (Module 6)

| Class | Kind | Responsibility & Methods |
|-------|------|--------------------------|
| `StringUtils` | `static` | `trim()`, `splitByDelimiter()`, `toUpperCase()`, `isAllLetters()`, `isSingleLetter()`, `parseInt()`, `intToString()`, `padWithZeros()`, `equalsIgnoreCase()`, `replaceChar()`, `underscoreToSpace()`, `spaceToUnderscore()`, `currentDateDDMMYY()`, `formatPersonName()`, `formatPlaceName()`. Implemented without `<cctype>`, `<algorithm>`, or `<sstream>`. |
| `DataLoader` | `static` | Constants: `FILE_RULES`, `FILE_CLUE`, `FILE_LOCATIONS`, `FILE_CHARACTERS`, `FILE_ABOUT`, `FILE_SCOREBOARD`. Dimensions: `ROLE_COUNT=7`, `LOCATION_COUNT=7`, `CHARACTER_COUNT=8`, `WORD1_LENGTH=7`, `WORD2_LENGTH=8`. Methods: `loadRules()`, `loadClueProfiles()`, `loadLocations()`, `loadCharacters()`, `loadAboutText()`, `validateConsistency()`, `roleToColumn()`. Strictly validates data format and throws `FileNotFoundException` or `FileCorruptException`. |
| `ScoreBoard` | `class` | `filePath; ScoreBoard(path); addScore(name, won, cred, turns, culprit, date); displayAll(out); getEntryCount(); getFilePath();`. Appends records in strict format `Detective|Result|Credibility|Turns_Left|Culprit|Date`, auto-pads turns to two digits, converts culprit spaces to underscores, renders ASCII table (underscores displayed as spaces). Associated with `GameEngine` (main owns instance, engine holds pointer). Implemented in Commit 7. |

### Scheduled for Commit 8

| Class | Kind | Responsibility |
|-------|------|---------------|
| `ConsoleUtils` | `static` | ASCII borders, screen-clear (system()), menu rendering |

---

## Character Hierarchy — Implemented in Commit 3 (Module 3 & 9)

```
Character  [abstract; private: characterName, characterRole, nationality, isCulprit, liveCount; friend GameEngine; friend operator<<; friend operator>>]
│  pure virtual: display(std::ostream&) const
│  virtual:      getMinimumCredibility(); hasClue(); getClue()
│  static:       getLiveCount()
│  friend:       GameEngine (access to private fields, exercised in Commit 8)
│  friend:       operator>> (direct assignment to private characterName)
│  friend:       operator<< (polymorphic stream output via virtual display)
│
├── Victim             [minCred: 80; overrides hasClue (true), getClue (Word2), display]
├── Detective          [credibility, turns, dynamic string* solvedWords[2], composed TileRack locationRack (cap 7) & characterRack (cap 8); deep copy ctor & op=; Malay default; minCred: 0]
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

## Generic Container Template — Implemented in Commit 6 (Module 8)

| Class | Kind | Fields & Methods | Notes |
|-------|------|------------------|-------|
| `DynamicArray<T>` | Class Template | `T* data; int capacity; int size;` <br> `DynamicArray(int cap = 4); DynamicArray(const DynamicArray&); DynamicArray& operator=(const DynamicArray&); ~DynamicArray();` <br> `push(item); operator[](idx); removeAt(idx); clear(); resize(newSize); getSize(); getCapacity(); isEmpty();` | Type-safe generic heap buffer template. Eliminates repeated manual `new[]`/`delete[]` across domain containers. Reallocates with power-of-two growth; bounds-checked indexing throws `InvalidPositionException`. |

---

## Board and Container Classes — Refactored in Commit 6 & Extended in Commit 7 (Module 4, 5, 8, & 11)

| Class | Kind | Fields & Methods | Notes |
|-------|------|------------------|-------|
| `TileRack` | Dynamic Container | `DynamicArray<Tile> tiles; DynamicArray<int> positions; int capacity;` <br> `TileRack(int cap); ~TileRack(); TileRack(const TileRack&); TileRack& operator=(const TileRack&);` <br> `addTile(); removeTile(); removeLast(); markPlaced(); isPlaced(); hasPosition(); getCapacity(); getCount(); isEmpty(); isFull(); clear(); display();` <br> `operator+(other); getClueText(); addRawTile(); friend operator<<;` | Refactored in C6 to use `DynamicArray<Tile>` and `DynamicArray<int>`. In C7, added `operator+` (custom selection sort by board position, separator space tile at pos 0, deep copy) and `getClueText()` for combined clue string. Friend of `GameEngine`. |
| `Scrabble` | Dynamic Jagged Board | `DynamicArray<Tile> rows[2]; int rowLength[2]; std::string targetWord[2]; bool targetsLoaded;` <br> `Scrabble(); Scrabble(string w1, string w2); ~Scrabble(); Scrabble(const Scrabble&); Scrabble& operator=(const Scrabble&);` <br> `loadTargetWords(); placeLetter(); revealLetter(); verifyWord(); isRowComplete(); isComplete(); getRowLength(); getRowString(); display(); friend operator<<; friend operator>>;` | Refactored in C6 to two `DynamicArray<Tile>` rows (`rows[0]` resized to 7, `rows[1]` resized to 8). In C7, `placeLetter` uses `Tile::operator==`; added friend stream extraction `operator>>` and insertion `operator<<`. |
| `Cottage` | Dynamic Container | `DynamicArray<Location> locations;` <br> `Cottage(); Cottage(const LocationRecord records[], int count, int culpritColumn); ~Cottage(); Cottage(const Cottage&); Cottage& operator=(const Cottage&);` <br> `getLocationCount(); isLocationSearched(num); searchLocation(num); getLocationClue(num); getLocationName(num); getSearchedCount(); allSearched(); displayLocations(); friend operator<<;` | Refactored in C6 to `DynamicArray<Location>` (stores `Location` structs by value). Original C5 raw dynamic pointer array `Location**` preserved in single-line comments. Aggregation relationship with `Location[]` (Decision R16). |

### Scheduled for Commit 8
| Class | Kind | Responsibility |
|-------|------|---------------|
| `GameEngine` | Singleton | Singleton; owns Detective, Cottage, Scrabble, combined-clue TileRack, 8 Character* |

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
| Access | GameEngine ↔ Character | Friend (exercised in Commit 8) |
| Output | operator<< for Tile, Character, Location, TileRack, Scrabble, Cottage | Friend function |
| Input | operator>> for Tile, Character, Location, Scrabble | Friend function |
| Merge | TileRack + TileRack → TileRack | Operator overloading (Module 11) |
| Equality | Tile == Tile, Clue == Clue | Operator overloading (Module 11) |
| Pattern | GameEngine | Singleton |
| Inheritance | PA | Diamond (BusinessRelations + CloseRelations, both virtual public Suspect) |
| Output | operator<< for Tile, Character, Location, TileRack, Scrabble, Cottage | Friend function |
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
