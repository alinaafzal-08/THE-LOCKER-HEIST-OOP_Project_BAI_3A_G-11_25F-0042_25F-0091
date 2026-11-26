// ============================================================
// main.cpp  --  THE LOCKER HEIST
// Driver and self-test harness for Commit 2.
//
// Operating modes:
//   heist.exe             -- Verifies data files via DataLoader and displays
//                            the banner with "Data files verified: OK".
//   heist.exe --selftest  -- Executes ST-01 (foundation tests) and
//                            ST-02 (exceptions, StringUtils, DataLoader).
//                            Exits with 0 if all tests pass, 1 if any fail.
// ============================================================

#include "heist.h"
#include <fstream>   // std::ofstream for creating temporary test fixtures
#include <cstdio>    // std::remove for deleting temporary test files

// ------------------------------------------------------------
// check()
// Evaluates condition, records pass/fail count, and logs status.
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
// runFoundationSelfTest()  --  ST-01 (Commit 1)
// ------------------------------------------------------------
static int runFoundationSelfTest()
{
    int passed = 0;
    int failed = 0;

    std::cout << "\n========================================\n";
    std::cout << "  ST-01  Foundation Self-Test\n";
    std::cout << "========================================\n";

    // Tile default values
    {
        Tile t;
        check(t.tileLetter == ' ', "Tile default: tileLetter == ' '",  passed, failed);
        check(t.filled     == false, "Tile default: filled == false",   passed, failed);
    }

    // Tile copy and assignment
    {
        Tile a;
        a.tileLetter = 'X';
        a.filled     = true;
        Tile b = a;
        Tile c;
        c = a;
        check(b.tileLetter == 'X' && b.filled == true,
              "Tile copy construction preserves fields", passed, failed);
        check(c.tileLetter == 'X' && c.filled == true,
              "Tile copy assignment preserves fields",   passed, failed);
    }

    // Clue default values
    {
        Clue cl;
        check(cl.clueLetter   == ' ',           "Clue default: clueLetter == ' '",        passed, failed);
        check(cl.cluePosition == 0,             "Clue default: cluePosition == 0",         passed, failed);
        check(cl.clueRow      == WordRow::Word1, "Clue default: clueRow == Word1",         passed, failed);
    }

    // Clue copy and assignment
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

    // Location default values
    {
        Location loc;
        check(loc.locName       == "",             "Location default: locName == \"\"",       passed, failed);
        check(loc.letterClueLoc == ' ',            "Location default: letterClueLoc == ' '",  passed, failed);
        check(loc.boardPosLoc   == 0,              "Location default: boardPosLoc == 0",      passed, failed);
        check(loc.locRow        == WordRow::Word1, "Location default: locRow == Word1",       passed, failed);
        check(loc.locSearchFlag == false,          "Location default: locSearchFlag == false", passed, failed);
    }

    // Location copy and assignment
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

    // WordRow enum values
    check(static_cast<int>(WordRow::Word1) == 0, "WordRow::Word1 == 0", passed, failed);
    check(static_cast<int>(WordRow::Word2) == 1, "WordRow::Word2 == 1", passed, failed);

    // MenuChoice enum values
    check(static_cast<int>(MenuChoice::NewGame)         == 1, "MenuChoice::NewGame == 1",         passed, failed);
    check(static_cast<int>(MenuChoice::ViewLeaderBoard) == 2, "MenuChoice::ViewLeaderBoard == 2", passed, failed);
    check(static_cast<int>(MenuChoice::About)           == 3, "MenuChoice::About == 3",           passed, failed);
    check(static_cast<int>(MenuChoice::Rules)           == 4, "MenuChoice::Rules == 4",           passed, failed);
    check(static_cast<int>(MenuChoice::Exit)            == 5, "MenuChoice::Exit == 5",            passed, failed);

    // GameState distinct values
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

    std::cout << "----------------------------------------\n";
    std::cout << "  Result: " << passed << " passed, " << failed << " failed.\n";
    std::cout << "========================================\n\n";

    return failed;
}

// ------------------------------------------------------------
// runDataLoaderSelfTest()  --  ST-02 (Commit 2)
// Tests:
//   1. Polymorphic throw/catch on all 11 GameException types.
//   2. StringUtils helper methods (trim, split, parseInt, pad, format).
//   3. DataLoader loading authentic game files.
//   4. DataLoader corruption detection on fixture files.
// ------------------------------------------------------------
static int runDataLoaderSelfTest()
{
    int passed = 0;
    int failed = 0;

    std::cout << "\n========================================\n";
    std::cout << "  ST-02  Module 6 & 7 Self-Test\n";
    std::cout << "========================================\n";

    // --------------------------------------------------------
    // Part 1: GameException family throw and polymorphic catch
    // --------------------------------------------------------
    std::cout << "\n--- Part 1: Exception Hierarchy Polymorphic Tests ---\n";
    try { throw InvalidInputException("bad input"); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "Invalid Input" && e.getMessage() == "bad input" && e.getErrorCode() == 101,
              "InvalidInputException polymorphic catch, category, code", passed, failed);
    }

    try { throw FileNotFoundException("missing.txt"); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "File Not Found" && e.getErrorCode() == 102,
              "FileNotFoundException polymorphic catch, category, code", passed, failed);
    }

    try { throw FileCorruptException("bad.txt", 4, "bad field"); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "File Corrupt" && e.getErrorCode() == 103,
              "FileCorruptException polymorphic catch, category, code", passed, failed);
    }

    try { throw InvalidPositionException(8, 7); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "Invalid Position" && e.getErrorCode() == 104,
              "InvalidPositionException polymorphic catch, category, code", passed, failed);
    }

    try { throw SlotFilledException(3); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "Slot Filled" && e.getErrorCode() == 105,
              "SlotFilledException polymorphic catch, category, code", passed, failed);
    }

    try { throw InvalidLetterException('5', 2); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "Invalid Letter" && e.getErrorCode() == 106,
              "InvalidLetterException polymorphic catch, category, code", passed, failed);
    }

    try { throw WordIncompleteException("Word1"); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "Word Incomplete" && e.getErrorCode() == 107,
              "WordIncompleteException polymorphic catch, category, code", passed, failed);
    }

    try { throw InvalidWordException("WRONG"); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "Invalid Word" && e.getErrorCode() == 108,
              "InvalidWordException polymorphic catch, category, code", passed, failed);
    }

    try { throw RackFullException(7); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "Rack Full" && e.getErrorCode() == 109,
              "RackFullException polymorphic catch, category, code", passed, failed);
    }

    try { throw RackEmptyException(); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "Rack Empty" && e.getErrorCode() == 110,
              "RackEmptyException polymorphic catch, category, code", passed, failed);
    }

    try { throw InterviewRestrictedException("Mr.Lukas", 60, 45); }
    catch (const GameException& e)
    {
        check(e.getCategory() == "Interview Restricted" && e.getErrorCode() == 111,
              "InterviewRestrictedException polymorphic catch, category, code", passed, failed);
    }

    // --------------------------------------------------------
    // Part 2: StringUtils unit tests
    // --------------------------------------------------------
    std::cout << "\n--- Part 2: StringUtils Tests ---\n";
    {
        std::string s = " \r\t hello \r\n";
        check(StringUtils::trim(s) == "hello",
              "StringUtils::trim strips spaces, tabs, and carriage returns", passed, failed);
    }

    {
        std::string line = "A|B|C|D|E|F|G|H";
        std::string parts[8];
        int count = StringUtils::splitByDelimiter(line, '|', parts, 8);
        check(count == 8 && parts[0] == "A" && parts[7] == "H",
              "StringUtils::splitByDelimiter parses 8-field line", passed, failed);
    }

    {
        check(StringUtils::parseInt("-10") == -10,
              "StringUtils::parseInt parses negative integer '-10'", passed, failed);
        check(StringUtils::parseInt("25") == 25,
              "StringUtils::parseInt parses positive integer '25'", passed, failed);

        bool threwOnAbc = false;
        try { StringUtils::parseInt("abc"); }
        catch (const InvalidInputException&) { threwOnAbc = true; }
        check(threwOnAbc, "StringUtils::parseInt throws InvalidInputException on 'abc'", passed, failed);
    }

    {
        check(StringUtils::padWithZeros(5, 2) == "05",
              "StringUtils::padWithZeros(5, 2) == '05'", passed, failed);
        check(StringUtils::spaceToUnderscore("Partner 1") == "Partner_1",
              "StringUtils::spaceToUnderscore('Partner 1') == 'Partner_1'", passed, failed);
        check(StringUtils::underscoreToSpace("Bedroom_1") == "Bedroom 1",
              "StringUtils::underscoreToSpace('Bedroom_1') == 'Bedroom 1'", passed, failed);
    }

    {
        std::string d = StringUtils::currentDateDDMMYY();
        bool validDate = (d.length() == 8 && d[2] == '-' && d[5] == '-');
        check(validDate, "StringUtils::currentDateDDMMYY matches DD-MM-YY pattern", passed, failed);
    }

    {
        check(StringUtils::formatPersonName("Mr.Leonardo") == "Mr. Leonardo",
              "StringUtils::formatPersonName inserts space after Mr.", passed, failed);
        check(StringUtils::formatPlaceName("Study_Room") == "Study Room",
              "StringUtils::formatPlaceName converts underscores to spaces", passed, failed);
    }

    // --------------------------------------------------------
    // Part 3: DataLoader loading authentic game files
    // --------------------------------------------------------
    std::cout << "\n--- Part 3: DataLoader Authentic File Verification ---\n";
    ClueProfile profiles[DataLoader::ROLE_COUNT];
    LocationRecord locations[DataLoader::LOCATION_COUNT];
    CharacterRecord characters[DataLoader::CHARACTER_COUNT];

    int profLoaded = DataLoader::loadClueProfiles(DataLoader::FILE_CLUE, profiles, DataLoader::ROLE_COUNT);
    check(profLoaded == 7, "DataLoader::loadClueProfiles loaded 7 profiles", passed, failed);

    int locLoaded = DataLoader::loadLocations(DataLoader::FILE_LOCATIONS, locations, DataLoader::LOCATION_COUNT);
    check(locLoaded == 7, "DataLoader::loadLocations loaded 7 locations", passed, failed);

    int charLoaded = DataLoader::loadCharacters(DataLoader::FILE_CHARACTERS, characters, DataLoader::CHARACTER_COUNT);
    check(charLoaded == 8, "DataLoader::loadCharacters loaded 8 characters", passed, failed);

    RulesConfig rules = DataLoader::loadRules(DataLoader::FILE_RULES);
    check(rules.maxTurns == 25 && rules.startingCredibility == 100 &&
          rules.wrongTurnPenalty == -5 && rules.wrongWordPenalty == -10,
          "DataLoader::loadRules loaded 25 turns, 100 cred, -5, -10 penalties", passed, failed);

    std::string about = DataLoader::loadAboutText(DataLoader::FILE_ABOUT);
    check(!about.empty(), "DataLoader::loadAboutText loaded non-empty content", passed, failed);

    bool consistencyPassed = false;
    try
    {
        DataLoader::validateConsistency(profiles, locations, characters);
        consistencyPassed = true;
    }
    catch (const GameException&)
    {
        consistencyPassed = false;
    }
    check(consistencyPassed, "DataLoader::validateConsistency passed on authentic dataset", passed, failed);

    check(DataLoader::roleToColumn("Partner 2") == 4,
          "DataLoader::roleToColumn('Partner 2') == 4", passed, failed);

    // --------------------------------------------------------
    // Part 4: DataLoader corruption detection test fixtures
    // --------------------------------------------------------
    std::cout << "\n--- Part 4: DataLoader Corruption Detection Tests ---\n";

    // 1. Missing file -> FileNotFoundException
    {
        bool caught = false;
        try { DataLoader::loadRules("non_existent_file_xyz.txt"); }
        catch (const FileNotFoundException&) { caught = true; }
        check(caught, "Corruption Case 1: Missing file throws FileNotFoundException", passed, failed);
    }

    // 2. Wrong heading row -> FileCorruptException
    {
        const char* fName = "temp_bad_heading.txt";
        {
            std::ofstream out(fName);
            out << "Bad|Heading|Row\nCook|ALFREDO|SPAGHETI\n";
        }
        bool caught = false;
        try
        {
            ClueProfile dummy[7];
            DataLoader::loadClueProfiles(fName, dummy, 7);
        }
        catch (const FileCorruptException&) { caught = true; }
        std::remove(fName);
        check(caught, "Corruption Case 2: Wrong heading throws FileCorruptException", passed, failed);
    }

    // 3. 5-field row in clue.txt -> FileCorruptException
    {
        const char* fName = "temp_extra_field.txt";
        {
            std::ofstream out(fName);
            out << "Culprit_Role|Word_1|Word_2\nCook|ALFREDO|SPAGHETI|EXTRA|FIELD\n";
        }
        bool caught = false;
        try
        {
            ClueProfile dummy[7];
            DataLoader::loadClueProfiles(fName, dummy, 7);
        }
        catch (const FileCorruptException&) { caught = true; }
        std::remove(fName);
        check(caught, "Corruption Case 3: 5-field clue row throws FileCorruptException", passed, failed);
    }

    // 4. Word of length 6 in clue.txt -> FileCorruptException
    {
        const char* fName = "temp_short_word.txt";
        {
            std::ofstream out(fName);
            out << "Culprit_Role|Word_1|Word_2\nCook|ALFRED|SPAGHETI\n";
        }
        bool caught = false;
        try
        {
            ClueProfile dummy[7];
            DataLoader::loadClueProfiles(fName, dummy, 7);
        }
        catch (const FileCorruptException&) { caught = true; }
        std::remove(fName);
        check(caught, "Corruption Case 4: 6-letter Word 1 throws FileCorruptException", passed, failed);
    }

    // 5. Digit as clue letter in locations.txt -> FileCorruptException
    {
        const char* fName = "temp_digit_letter.txt";
        {
            std::ofstream out(fName);
            out << "Place|W1Pos|Cook|Guard|Servant|P1|P2|PA|Child\nBedroom_1|1|9|G|L|T|M|P|R\n";
        }
        bool caught = false;
        try
        {
            LocationRecord dummy[7];
            DataLoader::loadLocations(fName, dummy, 7);
        }
        catch (const FileCorruptException&) { caught = true; }
        std::remove(fName);
        check(caught, "Corruption Case 5: Digit as letter throws FileCorruptException", passed, failed);
    }

    // 6. Duplicate W1Pos in locations.txt -> FileCorruptException
    {
        const char* fName = "temp_dup_pos.txt";
        {
            std::ofstream out(fName);
            out << "Place|W1Pos|Cook|Guard|Servant|P1|P2|PA|Child\n";
            out << "Bedroom_1|1|A|G|L|T|M|P|R\n";
            out << "Bedroom_2|1|L|E|O|R|I|H|E\n";
        }
        bool caught = false;
        try
        {
            LocationRecord dummy[7];
            DataLoader::loadLocations(fName, dummy, 7);
        }
        catch (const FileCorruptException&) { caught = true; }
        std::remove(fName);
        check(caught, "Corruption Case 6: Duplicate W1Pos throws FileCorruptException", passed, failed);
    }

    // 7. W2Pos == 9 in characters.txt -> FileCorruptException
    {
        const char* fName = "temp_pos_nine.txt";
        {
            std::ofstream out(fName);
            out << "Name|Role|Nationality|W2Pos|Cook|Guard|Servant|P1|P2|PA|Child|isCulprit\n";
            out << "Mr.Leonardo|Victim|French|9|S|S|H|S|R|P|B|0\n";
        }
        bool caught = false;
        try
        {
            CharacterRecord dummy[8];
            DataLoader::loadCharacters(fName, dummy, 8);
        }
        catch (const FileCorruptException&) { caught = true; }
        std::remove(fName);
        check(caught, "Corruption Case 7: W2Pos = 9 throws FileCorruptException", passed, failed);
    }

    // 8. isCulprit == 1 in characters.txt -> FileCorruptException
    {
        const char* fName = "temp_culprit_one.txt";
        {
            std::ofstream out(fName);
            out << "Name|Role|Nationality|W2Pos|Cook|Guard|Servant|P1|P2|PA|Child|isCulprit\n";
            out << "Mr.Leonardo|Victim|French|1|S|S|H|S|R|P|B|1\n";
        }
        bool caught = false;
        try
        {
            CharacterRecord dummy[8];
            DataLoader::loadCharacters(fName, dummy, 8);
        }
        catch (const FileCorruptException&) { caught = true; }
        std::remove(fName);
        check(caught, "Corruption Case 8: isCulprit = 1 throws FileCorruptException", passed, failed);
    }

    // 9. Rules key missing -> FileCorruptException
    {
        const char* fName = "temp_missing_key.txt";
        {
            std::ofstream out(fName);
            out << "Starting_Credibility=100\nEvery_Wrong_Turn=-5\nEvery_Wrong_Word=-10\n";
            out << "Win_Condition=Accuse_Correct_Culprit\nLose_Condition=Credibility_Zero_Or_Turns_Exhausted\n";
        }
        bool caught = false;
        try { DataLoader::loadRules(fName); }
        catch (const FileCorruptException&) { caught = true; }
        std::remove(fName);
        check(caught, "Corruption Case 9: Missing Max_Turns key throws FileCorruptException", passed, failed);
    }

    // 10. Rules value 'abc' -> FileCorruptException
    {
        const char* fName = "temp_bad_val.txt";
        {
            std::ofstream out(fName);
            out << "Max_Turns=abc\nStarting_Credibility=100\nEvery_Wrong_Turn=-5\nEvery_Wrong_Word=-10\n";
            out << "Win_Condition=Accuse_Correct_Culprit\nLose_Condition=Credibility_Zero_Or_Turns_Exhausted\n";
        }
        bool caught = false;
        try { DataLoader::loadRules(fName); }
        catch (const FileCorruptException&) { caught = true; }
        std::remove(fName);
        check(caught, "Corruption Case 10: Non-numeric rule value throws FileCorruptException", passed, failed);
    }

    // 11. Inconsistent letter in locations copy -> FileCorruptException
    {
        LocationRecord corruptedLocations[7];
        for (int i = 0; i < 7; ++i)
        {
            corruptedLocations[i] = locations[i];
        }
        // Mutate Cook's clue letter at Bedroom_1 from 'A' to 'Z'
        corruptedLocations[0].letters[0] = 'Z';

        bool caught = false;
        try
        {
            DataLoader::validateConsistency(profiles, corruptedLocations, characters);
        }
        catch (const FileCorruptException&) { caught = true; }
        check(caught, "Corruption Case 11: Inconsistent letter throws FileCorruptException", passed, failed);
    }

    std::cout << "----------------------------------------\n";
    std::cout << "  Result: " << passed << " passed, " << failed << " failed.\n";
    std::cout << "========================================\n\n";

    return failed;
}

// ------------------------------------------------------------
// main()
// ------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Execute automated self-tests if --selftest argument is passed
    if (argc >= 2 && std::string(argv[1]) == "--selftest")
    {
        int totalFailures = 0;
        totalFailures += runFoundationSelfTest();
        totalFailures += runDataLoaderSelfTest();
        return (totalFailures > 0) ? 1 : 0;
    }

    // Normal startup execution: display banner and verify authentic data files
    std::cout << "\n";
    std::cout << "+============================================================+\n";
    std::cout << "|          THE LOCKER HEIST  --  DETECTIVE GAME              |\n";
    std::cout << "+============================================================+\n";
    std::cout << "\n";

    try
    {
        ClueProfile profiles[DataLoader::ROLE_COUNT];
        LocationRecord locations[DataLoader::LOCATION_COUNT];
        CharacterRecord characters[DataLoader::CHARACTER_COUNT];

        DataLoader::loadRules(DataLoader::FILE_RULES);
        DataLoader::loadClueProfiles(DataLoader::FILE_CLUE, profiles, DataLoader::ROLE_COUNT);
        DataLoader::loadLocations(DataLoader::FILE_LOCATIONS, locations, DataLoader::LOCATION_COUNT);
        DataLoader::loadCharacters(DataLoader::FILE_CHARACTERS, characters, DataLoader::CHARACTER_COUNT);
        DataLoader::loadAboutText(DataLoader::FILE_ABOUT);
        DataLoader::validateConsistency(profiles, locations, characters);

        std::cout << "  Data files verified: OK\n";
        std::cout << "  (Temporary startup output -- replaced in Commit 8)\n\n";
        return 0;
    }
    catch (const GameException& ex)
    {
        std::cout << "  [ERROR] " << ex.getCategory() << ": " << ex.getMessage() << "\n\n";
        return 1;
    }
}
