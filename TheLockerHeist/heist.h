#pragma once

// ============================================================
// heist.h  --  THE LOCKER HEIST
// All declarations: enums, structs, exceptions, utilities, and
// class templates live here and nowhere else. Only heist.cpp and
// main.cpp include this file. Having a single header keeps the
// entire architecture visible in one place and prevents circular
// dependency issues across the project.
// ============================================================

// The header includes only what declarations themselves need.
// Implementation-only headers (<fstream>, <cstdlib>, <ctime>)
// reside in .cpp files that actually use them to minimize rebuild times.
#include <iostream>   // std::ostream used in operator<< declarations
#include <string>     // std::string for textual data and messages


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
