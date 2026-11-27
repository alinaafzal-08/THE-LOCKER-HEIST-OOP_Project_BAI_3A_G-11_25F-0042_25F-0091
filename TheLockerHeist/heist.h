#pragma once

// ============================================================
// heist.h  --  THE LOCKER HEIST
// All declarations: enums, structs, exceptions, utilities,
// character hierarchy, board models, and class templates live
// here and nowhere else. Only heist.cpp and main.cpp include
// this file. Having a single header keeps the entire architecture
// visible in one place and prevents circular dependency issues.
// ============================================================

// The header includes only what declarations themselves need.
// Implementation-only headers (<fstream>, <cstdlib>, <ctime>)
// reside in .cpp files that actually use them to minimize rebuild times.
#include <iostream>   // std::ostream used in operator<< declarations and display()
#include <string>     // std::string for textual data and messages


// Forward declaration for GameEngine (friend class across domain types)
class GameEngine;


// ============================================================
// SECTION 1 -- ENUMS (Commit 1)
// Scoped enums prevent accidental conversion to int and avoid
// global namespace pollution.
// ============================================================

// WordRow identifies which target word a tile or clue belongs to.
// Word1 represents the 7-letter culprit role (e.g., "ALFREDO").
// Word2 represents the 8-letter suspect trait (e.g., "SPAGHETI").
// Numeric values 0 and 1 correspond to rack row indices.
enum class WordRow
{
    Word1 = 0,  // first secret word row
    Word2 = 1   // second secret word row
};

// GameState represents the overall phase of execution in the game loop.
enum class GameState
{
    Menu,     // navigating the main menu
    Playing,  // active investigation in progress
    Won,      // detective successfully accused the culprit
    Lost,     // turns expired or credibility reached zero
    Exit      // detective chose to exit the program
};

// MenuChoice maps numeric user menu selections directly to enum values.
enum class MenuChoice
{
    NewGame         = 1,  // start a fresh case
    ViewLeaderBoard = 2,  // display the Hall of Fame
    About           = 3,  // view author & course information
    Rules           = 4,  // view rules, penalties and scoring
    Exit            = 5   // terminate the session
};


// ============================================================
// SECTION 2 -- FOUNDATION STRUCTS (Commit 1)
// Plain-old-data structures with in-class initializers ensure
// deterministic default state upon construction.
// ============================================================

// Tile represents a single square on a rack or board.
struct Tile
{
    char tileLetter = ' ';   // letter stored on this tile (' ' = empty)
    bool filled     = false; // whether a valid letter has been placed
};

// Stream insertion operator for Tile
std::ostream& operator<<(std::ostream& out, const Tile& tile);

// Clue represents a single letter hint obtained via search or interview.
struct Clue
{
    char    clueLetter   = ' ';            // character revealed
    int     cluePosition = 0;              // 1-based target word position
    WordRow clueRow      = WordRow::Word1; // target row for this clue
};

// Location represents a searchable area in Mr. Leonardo's cottage.
struct Location
{
    std::string locName       = "";              // display name of the room
    char        letterClueLoc = ' ';            // clue letter present here
    int         boardPosLoc   = 0;              // 1-based position in Word 1
    WordRow     locRow        = WordRow::Word1; // always Word1 for locations
    bool        locSearchFlag = false;          // true if already searched
};

// Stream insertion operator for Location
std::ostream& operator<<(std::ostream& out, const Location& loc);


// ============================================================
// SECTION 3 -- RECORD STRUCTS (Commit 2)
// Raw records loaded from flat data files before being converted
// into runtime domain models.
// ============================================================

// RulesConfig stores parameters parsed from rules.txt.
// Penalties are stored as signed negative numbers (e.g., -5, -10)
// and added directly to credibility to maintain uniform arithmetic.
struct RulesConfig
{
    int         maxTurns            = 25;
    int         startingCredibility = 100;
    int         wrongTurnPenalty    = -5;
    int         wrongWordPenalty    = -10;
    std::string winCondition        = "Accuse_Correct_Culprit";
    std::string loseCondition       = "Credibility_Zero_Or_Turns_Exhausted";

    // Returns standard default rule values if rules.txt fails to load.
    static RulesConfig defaultRules();
};

// ClueProfile stores one row from clue.txt.
struct ClueProfile
{
    std::string culpritRole = "";  // suspect role (e.g., "Cook", "Guard")
    std::string word1       = "";  // 7-letter secret word
    std::string word2       = "";  // 8-letter secret word
};

// LocationRecord stores one row from locations.txt.
// Letter columns correspond to suspects: Cook[0], Guard[1], Servant[2],
// Partner 1[3], Partner 2[4], PA[5], Child[6].
struct LocationRecord
{
    std::string placeName = "";
    int         word1Pos  = 0;  // 1-based index (1-7)
    char        letters[7] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
};

// CharacterRecord stores one row from characters.txt.
// Letter columns match LocationRecord: Cook[0] through Child[6].
struct CharacterRecord
{
    std::string name        = "";
    std::string role        = "";
    std::string nationality = "";
    int         word2Pos    = 0;  // 1-based index (1-8)
    char        letters[7]  = { ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
    bool        isCulprit   = false;
};


// ============================================================
// SECTION 4 -- EXCEPTION HIERARCHY (Commit 2, Module 7)
// Custom exception hierarchy built from scratch without std::exception.
// Caught polymorphically via const GameException& to provide unified
// error handling across all modules.
// ============================================================

// GameException is the root base class for all game-specific exceptions.
class GameException
{
protected:
    std::string message;
    int         errorCode;

public:
    GameException(const std::string& msg, int code);
    virtual ~GameException();

    virtual std::string getMessage() const;
    int                 getErrorCode() const;
    virtual std::string getCategory() const;
};

// Thrown when user input fails syntax or semantic validation.
class InvalidInputException : public GameException
{
public:
    explicit InvalidInputException(const std::string& msg);
    std::string getCategory() const override;
};

// Thrown when an expected data file cannot be opened.
class FileNotFoundException : public GameException
{
public:
    explicit FileNotFoundException(const std::string& fileName);
    std::string getCategory() const override;
};

// Thrown when data file headers, fields, or constraints are corrupt.
class FileCorruptException : public GameException
{
public:
    FileCorruptException(const std::string& fileName, int lineNumber, const std::string& detail);
    std::string getCategory() const override;
};

// Thrown when a slot or board position is outside valid bounds.
class InvalidPositionException : public GameException
{
public:
    InvalidPositionException(int position, int maxPosition);
    std::string getCategory() const override;
};

// Thrown when attempting to place a tile in an already-occupied slot.
class SlotFilledException : public GameException
{
public:
    explicit SlotFilledException(int position);
    std::string getCategory() const override;
};

// Thrown when an unexpected character or non-matching letter is encountered.
class InvalidLetterException : public GameException
{
public:
    InvalidLetterException(char letter, int position);
    std::string getCategory() const override;
};

// Thrown when submitting an incomplete word rack for validation.
class WordIncompleteException : public GameException
{
public:
    explicit WordIncompleteException(const std::string& rowName);
    std::string getCategory() const override;
};

// Thrown when an entered word does not match the target secret word.
class InvalidWordException : public GameException
{
public:
    explicit InvalidWordException(const std::string& word);
    std::string getCategory() const override;
};

// Thrown when inserting into a tile rack that has reached capacity.
class RackFullException : public GameException
{
public:
    explicit RackFullException(int capacity);
    std::string getCategory() const override;
};

// Thrown when drawing from or manipulating an empty tile rack.
class RackEmptyException : public GameException
{
public:
    RackEmptyException();
    std::string getCategory() const override;
};

// Thrown when detective credibility is too low to interview a suspect.
class InterviewRestrictedException : public GameException
{
public:
    InterviewRestrictedException(const std::string& personName, int requiredCredibility, int currentCredibility);
    std::string getCategory() const override;
};


// ============================================================
// SECTION 5 -- STRING UTILITIES (Commit 2, Module 6)
// Pure static utility functions for manual string processing.
// Written entirely without forbidden STL algorithms, <sstream>, or <cctype>.
// ============================================================

class StringUtils
{
public:
    // Strips leading and trailing spaces, tabs, carriage returns, and newlines.
    static std::string trim(const std::string& str);

    // Splits a string by delimiter into parts array, returning total tokens found.
    static int splitByDelimiter(const std::string& line, char delimiter, std::string parts[], int maxParts);

    // Converts lowercase ASCII characters to uppercase.
    static std::string toUpperCase(const std::string& str);

    // Verifies whether every character in string is an ASCII alphabet letter.
    static bool isAllLetters(const std::string& str);

    // Verifies whether a character is an ASCII alphabet letter ('A'-'Z' or 'a'-'z').
    static bool isSingleLetter(char c);

    // Parses signed integer text; throws InvalidInputException on invalid characters.
    static int parseInt(const std::string& str);

    // Converts an integer to string representation without <sstream>.
    static std::string intToString(int value);

    // Pads a non-negative integer with leading zeros to achieve specified width.
    static std::string padWithZeros(int value, int width);

    // Case-insensitive ASCII string comparison.
    static bool equalsIgnoreCase(const std::string& a, const std::string& b);

    // Replaces all occurrences of oldChar with newChar.
    static std::string replaceChar(const std::string& str, char oldChar, char newChar);

    // Converts underscore characters to space characters.
    static std::string underscoreToSpace(const std::string& str);

    // Converts space characters to underscore characters.
    static std::string spaceToUnderscore(const std::string& str);

    // Formats current system date as "DD-MM-YY" using <ctime> and localtime_s.
    static std::string currentDateDDMMYY();

    // Inserts a space after "Mr." or "Ms." prefixes for clean UI output.
    static std::string formatPersonName(const std::string& name);

    // Converts internal location identifiers to friendly room titles.
    static std::string formatPlaceName(const std::string& name);
};


// ============================================================
// SECTION 6 -- DATA LOADER (Commit 2, Module 6)
// Loads, validates, and cross-checks all six data files.
// Throws strict exceptions upon any format or consistency flaw.
// ============================================================

class DataLoader
{
public:
    // Standard relative paths for project data files
    static const char* const FILE_RULES;
    static const char* const FILE_CLUE;
    static const char* const FILE_LOCATIONS;
    static const char* const FILE_CHARACTERS;
    static const char* const FILE_ABOUT;
    static const char* const FILE_SCOREBOARD;

    // Structural dimensions specified by manual
    static const int ROLE_COUNT      = 7;
    static const int LOCATION_COUNT  = 7;
    static const int CHARACTER_COUNT = 8;
    static const int WORD1_LENGTH    = 7;
    static const int WORD2_LENGTH    = 8;

    // Loaders for individual files
    static RulesConfig loadRules(const std::string& path);
    static int         loadClueProfiles(const std::string& path, ClueProfile out[], int capacity);
    static int         loadLocations(const std::string& path, LocationRecord out[], int capacity);
    static int         loadCharacters(const std::string& path, CharacterRecord out[], int capacity);
    static std::string loadAboutText(const std::string& path);

    // Cross-file consistency validation
    static void validateConsistency(const ClueProfile profiles[],
                                    const LocationRecord locations[],
                                    const CharacterRecord characters[]);

    // Maps role name ("Cook", "Guard", "Partner 1", "Partner_1", etc.) to 0-based column index (0-6)
    static int roleToColumn(const std::string& role);
};


// ============================================================
// SECTION 7 -- GENERIC CONTAINER TEMPLATE (Commit 6, Module 8)
// DynamicArray<T> provides a type-safe, automatically resizing
// heap buffer implementing generic programming principles.
// It eliminates boilerplate manual dynamic memory management (new[] / delete[])
// across board, rack, and location containers, enforcing RAII and
// deep copy semantics uniformly across the engine.
// ============================================================

template <typename T>
class DynamicArray
{
private:
    T*  data;      // Heap-allocated array storing elements of type T
    int capacity;  // Current total allocated storage capacity
    int size;      // Current number of active elements in the array

public:
    // Constructs dynamic array with specified initial capacity (minimum 1)
    explicit DynamicArray(int initialCapacity = 4)
        : data(nullptr), capacity(initialCapacity), size(0)
    {
        if (initialCapacity < 1)
        {
            throw InvalidInputException("Initial capacity must be at least 1; requested: " +
                                        StringUtils::intToString(initialCapacity));
        }
        data = new T[capacity];
    }

    // Deep copy constructor: creates an independent duplicate of source buffer
    DynamicArray(const DynamicArray& other)
        : data(nullptr), capacity(other.capacity), size(other.size)
    {
        data = new T[capacity];
        for (int i = 0; i < size; ++i)
        {
            data[i] = other.data[i];
        }
    }

    // Deep copy assignment operator: self-assignment safe
    DynamicArray& operator=(const DynamicArray& other)
    {
        if (this != &other)
        {
            T* newData = new T[other.capacity];
            for (int i = 0; i < other.size; ++i)
            {
                newData[i] = other.data[i];
            }
            delete[] data;
            data     = newData;
            capacity = other.capacity;
            size     = other.size;
        }
        return *this;
    }

    // Destructor releasing allocated heap storage
    ~DynamicArray()
    {
        delete[] data;
        data = nullptr;
        capacity = 0;
        size = 0;
    }

    // Appends an element to the end; doubles buffer capacity when full
    void push(const T& item)
    {
        if (size >= capacity)
        {
            int newCapacity = capacity * 2;
            T* newData = new T[newCapacity];
            for (int i = 0; i < size; ++i)
            {
                newData[i] = data[i];
            }
            delete[] data;
            data     = newData;
            capacity = newCapacity;
        }
        data[size++] = item;
    }

    // Subscript operator (non-const): validates 0-based index and throws InvalidPositionException
    T& operator[](int index)
    {
        if (index < 0 || index >= size)
        {
            // Converts 0-based index to 1-based position for user-facing exception
            throw InvalidPositionException(index + 1, size);
        }
        return data[index];
    }

    // Subscript operator (const): validates 0-based index and throws InvalidPositionException
    const T& operator[](int index) const
    {
        if (index < 0 || index >= size)
        {
            throw InvalidPositionException(index + 1, size);
        }
        return data[index];
    }

    // Removes element at specified 0-based index, shifting subsequent elements left
    void removeAt(int index)
    {
        if (index < 0 || index >= size)
        {
            throw InvalidPositionException(index + 1, size);
        }
        for (int i = index; i < size - 1; ++i)
        {
            data[i] = data[i + 1];
        }
        --size;
    }

    // Resets size to 0 while keeping capacity allocated
    void clear()
    {
        size = 0;
    }

    // Resizes buffer to newSize; initializes newly expanded elements to T()
    void resize(int newSize)
    {
        if (newSize < 0)
        {
            throw InvalidInputException("New size cannot be negative: " +
                                        StringUtils::intToString(newSize));
        }
        if (newSize > capacity)
        {
            int newCap = capacity * 2;
            if (newCap < newSize)
            {
                newCap = newSize;
            }
            T* newData = new T[newCap];
            for (int i = 0; i < size; ++i)
            {
                newData[i] = data[i];
            }
            delete[] data;
            data     = newData;
            capacity = newCap;
        }
        // If expanding, default-initialize new slots
        for (int i = size; i < newSize; ++i)
        {
            data[i] = T();
        }
        size = newSize;
    }

    // State accessors
    int  getSize() const     { return size; }
    int  getCapacity() const { return capacity; }
    bool isEmpty() const     { return size == 0; }
};


// ============================================================
// SECTION 7B -- BOARD AND CONTAINER CLASSES (Refactored Module 8)
// TileRack, Scrabble, and Cottage refactored to use DynamicArray<T>.
// Raw DMA versions from Modules 4 and 5 are preserved in comments below.
// ============================================================

// TileRack manages an inventory of collected clue letter tiles.
// Uses DynamicArray<Tile> and DynamicArray<int> instead of manual raw arrays.
class TileRack
{
private:
    // RAW DMA VERSION (Modules 4 and 5) - kept for reference, replaced by DynamicArray<T> in Module 8:
    // Tile* tiles;      // Dynamic array of Tile instances
    // int*  positions;  // Parallel dynamic array of 1-based board positions
    // int   count;      // Current number of tiles in the rack

    DynamicArray<Tile> tiles;      // Dynamic array of Tile instances
    DynamicArray<int>  positions;  // Parallel dynamic array of 1-based board positions
    int                capacity;   // Logical maximum capacity enforced by game rules

    friend class GameEngine;

public:
    // Constructs a rack with the specified capacity (minimum 1)
    explicit TileRack(int capacity = 8);

    // Deep copy constructor (delegates to DynamicArray member deep copies)
    TileRack(const TileRack& other);

    // Deep copy assignment operator (delegates to DynamicArray member assignment)
    TileRack& operator=(const TileRack& other);

    // Destructor (DynamicArray members automatically release their own memory)
    ~TileRack();

    // Adds a clue tile; throws if full, letter invalid, position < 1, or duplicate position
    void addTile(const Clue& clue);

    // Removes tile at index, shifting remaining tiles left
    void removeTile(int index);

    // Removes the most recently added tile
    void removeLast();

    // Clears all tiles from the rack
    void clear();

    // Accessors
    int  getCount() const;
    int  getCapacity() const;
    bool isFull() const;
    bool isEmpty() const;

    char getLetterAt(int index) const;
    int  getPositionAt(int index) const;
    bool hasPosition(int position) const;

    // Status tracking for placed tiles
    void markPlaced(int position);
    bool isPlaced(int position) const;

    // Renders rack contents to the output stream
    void display(std::ostream& out) const;
};

// Stream insertion operator for TileRack
std::ostream& operator<<(std::ostream& out, const TileRack& rack);

// Scrabble board manages the 2-row jagged crossword grid (7 letters for Word 1, 8 for Word 2).
// Refactored from raw dynamic Tile** jagged array to two DynamicArray<Tile> rows.
class Scrabble
{
private:
    // RAW DMA VERSION (Modules 4 and 5) - kept for reference, replaced by DynamicArray<T> in Module 8:
    // Tile** grid;           // Jagged array: grid[0] has 7 tiles, grid[1] has 8 tiles

    DynamicArray<Tile> rows[2];        // Two DynamicArray rows: rows[0] length 7, rows[1] length 8
    int                rowLength[2];   // Length of each row (7, 8)
    std::string        targetWord[2];  // Solution words for Word 1 and Word 2
    bool               targetsLoaded;  // Whether target words have been configured

    friend class GameEngine;

public:
    // Default constructor: allocates empty grid rows via DynamicArray
    Scrabble();

    // Parameterized constructor: loads target solution words
    Scrabble(const std::string& word1, const std::string& word2);

    // Deep copy constructor (delegates to DynamicArray deep copies)
    Scrabble(const Scrabble& other);

    // Deep copy assignment operator (delegates to DynamicArray assignment)
    Scrabble& operator=(const Scrabble& other);

    // Destructor (DynamicArray members automatically release their own memory)
    ~Scrabble();

    // Configures target solution words (must be 7 and 8 uppercase letters)
    void loadTargetWords(const std::string& word1, const std::string& word2);

    // Places a clue letter into the grid matching target letter and position
    void placeLetter(const Clue& clue);

    // Reveals letter at specified 1-based position or space if unfilled
    char revealLetter(WordRow row, int position) const;

    // Verification and state inspection
    bool isRowComplete(WordRow row) const;
    bool isComplete() const;
    int  getRowLength(WordRow row) const;
    int  getFilledCount(WordRow row) const;
    std::string getRowString(WordRow row) const;
    std::string getTargetWord(WordRow row) const;

    // Verifies full row matches typed guess case-insensitively
    void verifyWord(WordRow row, const std::string& typedWord) const;

    // Renders the current state of both word rows
    void display(std::ostream& out) const;
};

// Stream insertion operator for Scrabble
std::ostream& operator<<(std::ostream& out, const Scrabble& scrabble);

// Cottage manages the searchable rooms of Mr. Leonardo's mansion.
// Refactored from raw Location** pointer array to DynamicArray<Location> (stored by value).
class Cottage
{
private:
    // RAW DMA VERSION (Modules 4 and 5) - kept for reference, replaced by DynamicArray<T> in Module 8:
    // Location** locations;     // Dynamic array of pointers to Location instances
    // int        locationCount; // Total number of rooms in the cottage

    DynamicArray<Location> locations; // Dynamic array of Location values

    friend class GameEngine;

public:
    // Default constructor: creates an empty cottage
    Cottage();

    // Parameterized constructor: creates Location instances from data records
    Cottage(const LocationRecord records[], int count, int culpritColumn);

    // Deep copy constructor (delegates to DynamicArray member deep copy)
    Cottage(const Cottage& other);

    // Deep copy assignment operator (self-assignment safe)
    Cottage& operator=(const Cottage& other);

    // Destructor (DynamicArray members automatically release their own memory)
    ~Cottage();

    // Accessors using 1-based room numbers (1 to location count); throws InvalidPositionException
    int         getLocationCount() const;
    bool        isLocationSearched(int number) const;
    Clue        searchLocation(int number);
    Clue        getLocationClue(int number) const;
    std::string getLocationName(int number) const;
    int         getSearchedCount() const;
    bool        allSearched() const;

    // Renders list of locations with search status to output stream
    void displayLocations(std::ostream& out = std::cout) const;
};

// Stream insertion operator for Cottage
std::ostream& operator<<(std::ostream& out, const Cottage& cottage);


// ============================================================
// SECTION 8 -- CHARACTER HIERARCHY (Commit 3, Module 3 & 9)
// Abstract Character base class, polymorphic hierarchy, multiple and
// virtual inheritance for the PA diamond problem, and CharacterFactory.
// ============================================================

// Abstract base class representing all entities in the game.
class Character
{
private:
    std::string characterName;
    std::string characterRole;
    std::string nationality;
    bool        isCulprit;

    static int liveCount; // Tracks currently active objects for memory leak tests

    friend class GameEngine;

public:
    Character(const std::string& name, const std::string& role, const std::string& nat);
    Character(const Character& other);
    Character& operator=(const Character& other);
    virtual ~Character();

    std::string getName() const;
    std::string getRole() const;
    std::string getNationality() const;
    bool        getIsCulprit() const;

    // Pure virtual method rendering Character abstract
    virtual void display(std::ostream& out) const = 0;

    // Virtual hooks with default behaviors
    virtual int  getMinimumCredibility() const;
    virtual bool hasClue() const;
    virtual Clue getClue() const;

    static int getLiveCount();
};

// Victim represents Mr. Leonardo, who provides the first Word 2 clue letter.
class Victim : public Character
{
private:
    char    letterClueChar;
    int     boardPosChar;
    WordRow clueRowChar;

public:
    Victim(const std::string& name, const std::string& nat, char letter, int pos);
    virtual ~Victim() override;

    char    getLetterClue() const;
    int     getBoardPos() const;
    WordRow getClueRow() const;

    void display(std::ostream& out) const override;
    bool hasClue() const override;
    Clue getClue() const override;
    int  getMinimumCredibility() const override;
};

// Detective represents the playable investigator, managing credibility, turns,
// dynamic arrays of solved words, and composed TileRacks with deep copy semantics.
class Detective : public Character
{
private:
    int          credibility;
    int          turns;
    std::string* solvedWords; // Dynamic array of 2 strings [Word1, Word2]

    // Composition: Detective directly owns two TileRack instances for clue storage (R15)
    TileRack locationRack;  // Holds Word 1 letter clues (capacity 7)
    TileRack characterRack; // Holds Word 2 letter clues (capacity 8)

    friend class GameEngine;

public:
    static const std::string DEFAULT_NATIONALITY;

    explicit Detective(const std::string& name = "Sherlock", int startingCredibility = 100);
    Detective(const Detective& other);
    Detective& operator=(const Detective& other);
    virtual ~Detective() override;

    int         getCredibility() const;
    void        reduceCredibility(int amount);
    void        addTurn();
    int         getTurnsUsed() const;
    void        setSolvedWord(WordRow row, const std::string& word);
    std::string getSolvedWord(WordRow row) const;
    bool        isRowSolved(WordRow row) const;
    bool        areBothSolved() const;

    // Clue and rack operations (composition)
    void            collectClue(const Clue& clue);
    const TileRack& getRack(WordRow row) const;
    TileRack&       getRack(WordRow row);
    void            displayRacks(std::ostream& out = std::cout) const;

    void display(std::ostream& out) const override;
};

// Abstract intermediate class representing anyone under suspicion.
class Suspect : public Character
{
private:
    char    letterClueChar;
    int     boardPosChar;
    WordRow clueRowChar;

public:
    Suspect(const std::string& name, const std::string& role, const std::string& nat,
            char letter = ' ', int pos = 0);
    virtual ~Suspect() override;

    char    getLetterClue() const;
    void    setLetterClue(char c);
    int     getBoardPos() const;
    void    setBoardPos(int p);
    WordRow getClueRow() const;
    void    setClueRow(WordRow r);

    bool hasClue() const override;
    Clue getClue() const override;

    void display(std::ostream& out) const override = 0;
};

// Virtual public inheritance branch of Suspect for business associates
class BusinessRelations : virtual public Suspect
{
public:
    BusinessRelations(const std::string& name, const std::string& role, const std::string& nat,
                      char letter = ' ', int pos = 0);
    virtual ~BusinessRelations() override;

    void display(std::ostream& out) const override;
};

// Virtual public inheritance branch of Suspect for family/personal associates
class CloseRelations : virtual public Suspect
{
public:
    CloseRelations(const std::string& name, const std::string& role, const std::string& nat,
                   char letter = ' ', int pos = 0);
    virtual ~CloseRelations() override;

    void display(std::ostream& out) const override;
};

// Base class for household domestic staff
class HouseHelp : public Suspect
{
public:
    HouseHelp(const std::string& name, const std::string& role, const std::string& nat,
              char letter = ' ', int pos = 0);
    virtual ~HouseHelp() override;

    int  getMinimumCredibility() const override;
    void display(std::ostream& out) const override;
};

// Concrete Cook character (Mr. Enzo)
class Cook : public HouseHelp
{
public:
    Cook(const std::string& name, const std::string& nat, char letter, int pos);
    virtual ~Cook() override;

    void display(std::ostream& out) const override;
};

// Concrete Guard character (Mr. Lukas)
class Guard : public HouseHelp
{
public:
    Guard(const std::string& name, const std::string& nat, char letter, int pos);
    virtual ~Guard() override;

    void display(std::ostream& out) const override;
};

// Concrete Servant character (Mr. Toby)
class Servant : public HouseHelp
{
public:
    Servant(const std::string& name, const std::string& nat, char letter, int pos);
    virtual ~Servant() override;

    void display(std::ostream& out) const override;
};

// Concrete Partner character (Ms. Ruslana, Mr. Dmitry)
class Partner : public BusinessRelations
{
private:
    int partnerNumber; // 1 or 2

public:
    Partner(const std::string& name, const std::string& nat, int number, char letter, int pos);
    virtual ~Partner() override;

    int  getPartnerNumber() const;
    int  getMinimumCredibility() const override;
    void display(std::ostream& out) const override;
};

// Concrete PA character (Mr. Jad) resolving diamond inheritance
class PA : public BusinessRelations, public CloseRelations
{
public:
    PA(const std::string& name, const std::string& nat, char letter, int pos);
    virtual ~PA() override;

    int  getMinimumCredibility() const override;
    void display(std::ostream& out) const override;
};

// Concrete Child character (Ms. Celine) with age validation
class Child : public CloseRelations
{
private:
    int age;

public:
    static const int DEFAULT_AGE = 17;

    Child(const std::string& name, const std::string& nat, char letter, int pos, int age = DEFAULT_AGE);
    virtual ~Child() override;

    int  getAge() const;
    int  getMinimumCredibility() const override;
    void display(std::ostream& out) const override;
};

// Factory producing concrete Character derived instances from data records
class CharacterFactory
{
public:
    static Character* create(const CharacterRecord& record, int culpritColumn);
};


