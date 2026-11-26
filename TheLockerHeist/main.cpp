// ============================================================
// main.cpp  --  THE LOCKER HEIST
// Driver and self-test harness for Commit 1.
//
// Two modes of operation:
//   heist.exe             -- prints the game title banner and
//                            a note that this is the foundation build.
//   heist.exe --selftest  -- runs ST-01 (foundation self-tests) and
//                            exits with code 0 if all pass, 1 if any fail.
//
// The self-test harness is intentionally simple: check() receives a
// condition, a label, and running counters.  Every test prints PASS or
// FAIL so the developer can see at a glance what works and what does not.
// A non-zero exit code on failure lets run_tests.ps1 detect problems
// automatically in CI-style builds.
// ============================================================

#include "heist.h"
// No extra headers needed: std::string (from heist.h via <string>) lets us
// compare argv[1] safely without touching the forbidden <cstring> header.

// ------------------------------------------------------------
// check()
// The core test primitive.  Increments passCount or failCount,
// prints a one-line result, and returns whether the test passed.
// Using a function (not a macro) gives proper type checking and
// keeps the call sites clean.
// ------------------------------------------------------------
static bool check(bool condition, const char* label, int& passCount, int& failCount)
{
    if (condition)
    {
        std::cout << "  [PASS] " << label << "\n";
        ++passCount;
        return true;
    }
    else
    {
        std::cout << "  [FAIL] " << label << "\n";
        ++failCount;
        return false;
    }
}

// ------------------------------------------------------------
// runFoundationSelfTest()  --  ST-01
// Tests all types defined in Module 1 (Commit 1):
//   - Tile default values
//   - Clue default values
//   - Location default values
//   - WordRow integer values (0 and 1) used as array indices
//   - MenuChoice integer values (1 to 5) matching menu input
//   - GameState values are all distinct from each other
//   - Structs can be assigned (copy-assignment) and copied
// Returns the number of failures so main() can exit non-zero.
// ------------------------------------------------------------
static int runFoundationSelfTest()
{
    int passed = 0;
    int failed = 0;

    std::cout << "\n========================================\n";
    std::cout << "  ST-01  Foundation Self-Test\n";
    std::cout << "========================================\n";

    // ------ Tile default values ------
    // A freshly constructed Tile should hold a space and be unfilled.
    {
        Tile t;
        check(t.tileLetter == ' ', "Tile default: tileLetter == ' '",  passed, failed);
        check(t.filled     == false, "Tile default: filled == false",   passed, failed);
    }

    // ------ Tile copy and assignment ------
    // If copy construction or assignment were broken the struct
    // could not be stored in arrays -- catch that early.
    {
        Tile a;
        a.tileLetter = 'X';
        a.filled     = true;
        Tile b = a;          // copy construction
        Tile c;
        c = a;               // copy assignment
        check(b.tileLetter == 'X' && b.filled == true,
              "Tile copy construction preserves fields", passed, failed);
        check(c.tileLetter == 'X' && c.filled == true,
              "Tile copy assignment preserves fields",   passed, failed);
    }

    // ------ Clue default values ------
    {
        Clue cl;
        check(cl.clueLetter   == ' ',           "Clue default: clueLetter == ' '",        passed, failed);
        check(cl.cluePosition == 0,             "Clue default: cluePosition == 0",         passed, failed);
        check(cl.clueRow      == WordRow::Word1, "Clue default: clueRow == Word1",         passed, failed);
    }

    // ------ Clue copy and assignment ------
    {
        Clue a;
        a.clueLetter   = 'Z';
        a.cluePosition = 3;
        a.clueRow      = WordRow::Word2;
        Clue b = a;
        Clue c;
        c = a;
        check(b.clueLetter == 'Z' && b.cluePosition == 3 && b.clueRow == WordRow::Word2,
              "Clue copy construction preserves fields", passed, failed);
        check(c.clueLetter == 'Z' && c.cluePosition == 3 && c.clueRow == WordRow::Word2,
              "Clue copy assignment preserves fields",   passed, failed);
    }

    // ------ Location default values ------
    {
        Location loc;
        check(loc.locName       == "",             "Location default: locName == \"\"",       passed, failed);
        check(loc.letterClueLoc == ' ',            "Location default: letterClueLoc == ' '",  passed, failed);
        check(loc.boardPosLoc   == 0,              "Location default: boardPosLoc == 0",      passed, failed);
        check(loc.locRow        == WordRow::Word1, "Location default: locRow == Word1",       passed, failed);
        check(loc.locSearchFlag == false,          "Location default: locSearchFlag == false", passed, failed);
    }

    // ------ Location copy and assignment ------
    {
        Location a;
        a.locName       = "Garden";
        a.letterClueLoc = 'R';
        a.boardPosLoc   = 4;
        a.locRow        = WordRow::Word1;
        a.locSearchFlag = true;
        Location b = a;
        Location c;
        c = a;
        check(b.locName == "Garden" && b.letterClueLoc == 'R' && b.boardPosLoc == 4
              && b.locSearchFlag == true,
              "Location copy construction preserves fields", passed, failed);
        check(c.locName == "Garden" && c.letterClueLoc == 'R' && c.boardPosLoc == 4
              && c.locSearchFlag == true,
              "Location copy assignment preserves fields",   passed, failed);
    }

    // ------ WordRow integer values ------
    // These values are used as array indices inside TileRack (Commit 4).
    // They must equal 0 and 1 exactly.
    check(static_cast<int>(WordRow::Word1) == 0, "WordRow::Word1 == 0", passed, failed);
    check(static_cast<int>(WordRow::Word2) == 1, "WordRow::Word2 == 1", passed, failed);

    // ------ MenuChoice integer values ------
    // The player types 1 to 5 and the input is cast to MenuChoice,
    // so each enumerator must have the matching integer value.
    check(static_cast<int>(MenuChoice::NewGame)         == 1, "MenuChoice::NewGame == 1",         passed, failed);
    check(static_cast<int>(MenuChoice::ViewLeaderBoard) == 2, "MenuChoice::ViewLeaderBoard == 2", passed, failed);
    check(static_cast<int>(MenuChoice::About)           == 3, "MenuChoice::About == 3",           passed, failed);
    check(static_cast<int>(MenuChoice::Rules)           == 4, "MenuChoice::Rules == 4",           passed, failed);
    check(static_cast<int>(MenuChoice::Exit)            == 5, "MenuChoice::Exit == 5",            passed, failed);

    // ------ GameState values are distinct ------
    // We only need them to be mutually distinguishable in switch statements.
    // Compare every pair to confirm no two enumerators share a value.
    {
        int menu    = static_cast<int>(GameState::Menu);
        int playing = static_cast<int>(GameState::Playing);
        int won     = static_cast<int>(GameState::Won);
        int lost    = static_cast<int>(GameState::Lost);
        int exit_   = static_cast<int>(GameState::Exit);

        check(menu != playing && menu != won && menu != lost && menu != exit_,
              "GameState::Menu is distinct from all others", passed, failed);
        check(playing != won && playing != lost && playing != exit_,
              "GameState::Playing is distinct from Won, Lost, Exit", passed, failed);
        check(won != lost && won != exit_,
              "GameState::Won is distinct from Lost and Exit",        passed, failed);
        check(lost != exit_,
              "GameState::Lost is distinct from Exit",                passed, failed);
    }

    // ------ Summary ------
    std::cout << "----------------------------------------\n";
    std::cout << "  Result: " << passed << " passed, " << failed << " failed.\n";
    std::cout << "========================================\n\n";

    return failed; // 0 means all clear
}

// ------------------------------------------------------------
// main()
// Decides the operating mode from the command-line argument:
//   --selftest  : run ST-01 and exit with the failure count
//   (none)      : show the game title banner (placeholder for Commit 8)
// ------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Check for the developer self-test flag.
    // We use std::string comparison so we do not need the forbidden <cstring> header.
    if (argc >= 2 && std::string(argv[1]) == "--selftest")
    {
        int failures = runFoundationSelfTest();
        // Exit code 0 = all tests passed; non-zero = at least one failure.
        return (failures > 0) ? 1 : 0;
    }

    // Normal launch: show the banner.
    // This text will be replaced with a full game loop in Commit 8.
    std::cout << "\n";
    std::cout << "+============================================================+\n";
    std::cout << "|          THE LOCKER HEIST  --  DETECTIVE GAME              |\n";
    std::cout << "+============================================================+\n";
    std::cout << "\n";
    std::cout << "  Foundation build.  Run with --selftest.\n";
    std::cout << "\n";

    return 0;
}
