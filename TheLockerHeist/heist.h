#pragma once

// ============================================================
// heist.h  --  THE LOCKER HEIST
// All declarations: enums, structs, and (later) class templates
// live here and nowhere else. Only heist.cpp and main.cpp include
// this file. Having a single header keeps the whole data model
// visible in one place and makes it easy to check consistency.
// ============================================================

// The header includes only what the declarations themselves need.
// Implementation headers (<fstream>, <cstdlib>, <ctime>) go in the
// .cpp files that actually use them -- putting them here would force
// every translation unit to compile them even when not needed.
#include <iostream>   // std::ostream used in operator<< declarations (added later)
#include <string>     // std::string is used directly in the Location struct


// ============================================================
// SECTION 1 -- ENUMS
// Using enum class (scoped enumerations) prevents name collisions
// and makes switch statements exhaustive-checkable by the compiler.
// ============================================================

// WordRow marks which of the two secret words a tile or clue
// belongs to. Word1 is the culprit-role word, Word2 is the
// character-attribute word.  The integer values (0 and 1) are
// used as array indices inside TileRack (Commit 4).
enum class WordRow
{
    Word1 = 0,  // first secret word  (culprit role, e.g. "ALFREDO")
    Word2 = 1   // second secret word (character trait, e.g. "SPAGHETI")
};


// GameState tracks where the player is in the overall flow so
// that the game loop in GameEngine (Commit 8) knows what to show.
enum class GameState
{
    Menu,     // player is at the main menu
    Playing,  // an investigation is in progress
    Won,      // player made a correct accusation
    Lost,     // credibility hit zero or turns ran out
    Exit      // player chose to quit
};


// MenuChoice maps the five numbered options in the main menu to
// meaningful names.  The integer values must match what the player
// types (1 to 5) so we can cast directly from an int.
enum class MenuChoice
{
    NewGame       = 1,  // start a fresh game
    ViewLeaderBoard = 2, // show the high-score table
    About         = 3,  // display about.txt
    Rules         = 4,  // display rules.txt
    Exit          = 5   // end the program
};


// ============================================================
// SECTION 2 -- STRUCTS
// Structs group tightly related fields so they can be passed and
// returned as a single unit.  Every struct uses in-class member
// initialisers to guarantee a safe default state so no constructor
// is strictly necessary for plain data types.
// ============================================================

// Tile represents one cell on a TileRack or Scrabble board.
// tileLetter holds the letter that occupies the slot; filled
// tracks whether a letter has been placed.  Default is an empty
// space that has not yet been filled.
struct Tile
{
    char tileLetter = ' ';   // the character placed in this slot (' ' = empty)
    bool filled     = false; // true once a letter has been assigned
};


// Clue carries a single letter hint discovered during the game.
// clueLetter is the actual letter, cluePosition is the 1-based
// column index on the word's tile rack, and clueRow says which
// of the two words this clue belongs to.
struct Clue
{
    char     clueLetter   = ' ';          // the letter revealed by this clue
    int      cluePosition = 0;            // 1-based position in the target word
    WordRow  clueRow      = WordRow::Word1; // which secret word this clue fills
};


// Location stores everything the game needs to know about one
// searchable room in the mansion.  locName is the display name,
// letterClueLoc is the letter found there for the current culprit,
// boardPosLoc is the 1-based position it fills in Word 1,
// locRow marks which word row it belongs to, and locSearchFlag
// records whether the detective has already searched this room
// (so a repeat search counts as a wrong action and costs a turn).
struct Location
{
    std::string locName       = "";              // e.g. "Bedroom_1"
    char        letterClueLoc = ' ';            // letter clue found at this location
    int         boardPosLoc   = 0;              // 1-based word position for the letter
    WordRow     locRow        = WordRow::Word1; // always Word1 for location clues
    bool        locSearchFlag = false;          // true once the detective has searched here
};
