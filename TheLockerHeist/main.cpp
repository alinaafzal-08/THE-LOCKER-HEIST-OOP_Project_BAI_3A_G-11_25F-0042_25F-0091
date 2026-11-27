// ============================================================
// main.cpp  --  THE LOCKER HEIST
// Driver and self-test harness for Commit 4.
//
// Operating modes:
//   heist.exe             -- Verifies data files via DataLoader and displays
//                            the banner with "Data files verified: OK".
//   heist.exe --selftest  -- Executes ST-01 (foundation tests),
//                            ST-02 (exceptions, StringUtils, DataLoader),
//                            ST-03 (character hierarchy, diamond, factory),
//                            and ST-04 (TileRack & Scrabble board DMA).
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
// ------------------------------------------------------------
static int runDataLoaderSelfTest()
{
    int passed = 0;
    int failed = 0;

    std::cout << "\n========================================\n";
    std::cout << "  ST-02  Module 6 & 7 Self-Test\n";
    std::cout << "========================================\n";

    // Part 1: GameException family throw and polymorphic catch
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

    // Part 2: StringUtils unit tests
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

    // Part 3: DataLoader loading authentic game files
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

    // Part 4: DataLoader corruption detection test fixtures
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
// runCharacterHierarchySelfTest()  --  ST-03 (Commit 3)
// ------------------------------------------------------------
static int runCharacterHierarchySelfTest()
{
    int passed = 0;
    int failed = 0;

    std::cout << "\n========================================\n";
    std::cout << "  ST-03  Character Hierarchy Self-Test\n";
    std::cout << "========================================\n";

    int initialLive = Character::getLiveCount();

    // 1. Build all 8 characters from authentic characters.txt with culpritColumn = 5 (PA)
    CharacterRecord charRecords[DataLoader::CHARACTER_COUNT];
    int loaded = DataLoader::loadCharacters(DataLoader::FILE_CHARACTERS, charRecords, DataLoader::CHARACTER_COUNT);
    check(loaded == 8, "ST-03: Loaded 8 character records for factory construction", passed, failed);

    Character* roster[8] = { nullptr };
    for (int i = 0; i < 8; ++i)
    {
        roster[i] = CharacterFactory::create(charRecords[i], 5);
    }
    check(Character::getLiveCount() == initialLive + 8,
          "ST-03: CharacterFactory constructed 8 instances; liveCount incremented by 8", passed, failed);

    // 2. Base pointer display() runtime polymorphism
    std::cout << "\n--- Polymorphic display() outputs ---\n";
    for (int i = 0; i < 8; ++i)
    {
        std::cout << "  ";
        roster[i]->display(std::cout);
        std::cout << "\n";
    }
    check(true, "ST-03: Polymorphic display() executed without exception", passed, failed);

    // 3. getMinimumCredibility() values in file order: 80, 0, 0, 0, 75, 50, 50, 35
    int expectedCreds[8] = { 80, 0, 0, 0, 75, 50, 50, 35 };
    bool allCredsMatch = true;
    for (int i = 0; i < 8; ++i)
    {
        if (roster[i]->getMinimumCredibility() != expectedCreds[i])
        {
            allCredsMatch = false;
        }
    }
    check(allCredsMatch,
          "ST-03: getMinimumCredibility() values match expected sequence (80, 0, 0, 0, 75, 50, 50, 35)",
          passed, failed);

    // 4. getClue on Victim returns letter 'P', pos 1, WordRow::Word2 for column 5 (PA)
    {
        Clue vClue = roster[0]->getClue();
        check(vClue.clueLetter == 'P' && vClue.cluePosition == 1 && vClue.clueRow == WordRow::Word2,
              "ST-03: Victim getClue() returns letter 'P' at position 1 (Word2)", passed, failed);
    }

    // 5. getClue on Detective throws InvalidInputException
    {
        Detective testDetective("Sherlock", 100);
        bool threwDetClue = false;
        try
        {
            testDetective.getClue();
        }
        catch (const InvalidInputException&)
        {
            threwDetClue = true;
        }
        check(threwDetClue,
              "ST-03: Detective getClue() throws InvalidInputException", passed, failed);
    }

    // 6 & 7. Diamond inheritance address resolution and single-object live counting
    {
        int liveBeforePA = Character::getLiveCount();
        PA* paObj = new PA("Mr.Jad", "Egyptian", 'S', 8);

        check(Character::getLiveCount() == liveBeforePA + 1,
              "ST-03: Constructing one PA increases liveCount by exactly 1 (virtual inheritance)", passed, failed);

        BusinessRelations* br = paObj;
        CloseRelations* cr    = paObj;
        Suspect* s1           = br;
        Suspect* s2           = cr;
        check(s1 == s2,
              "ST-03: Diamond ambiguity resolved: BusinessRelations* and CloseRelations* resolve to identical Suspect*",
              passed, failed);

        // Virtual destructor check via base pointer
        Character* charBase = paObj;
        delete charBase;
        check(Character::getLiveCount() == liveBeforePA,
              "ST-03: Deleting PA via Character* virtual destructor restores liveCount", passed, failed);
    }

    // 8. Detective deep copy semantics, self-assignment, and reduceCredibility
    {
        Detective original("Dupin", 100);
        original.setSolvedWord(WordRow::Word1, "PHARAOH");
        original.addTurn();
        original.addTurn();

        Detective copyDet = original; // Deep copy constructor
        Detective assignDet("Other", 50);
        assignDet = original;         // Deep copy assignment

        copyDet.setSolvedWord(WordRow::Word1, "MUTATED");
        assignDet.setSolvedWord(WordRow::Word1, "CHANGED");

        check(original.getSolvedWord(WordRow::Word1) == "PHARAOH",
              "ST-03: Detective deep copy: mutating copy leaves original solvedWords intact", passed, failed);
        check(copyDet.getSolvedWord(WordRow::Word1) == "MUTATED" &&
              assignDet.getSolvedWord(WordRow::Word1) == "CHANGED",
              "ST-03: Detective deep copy: copies manage independent heap memory", passed, failed);
        check(original.getTurnsUsed() == 2 && copyDet.getTurnsUsed() == 2,
              "ST-03: Detective turn counters copied properly", passed, failed);

        // Self assignment
        original = original;
        check(original.getSolvedWord(WordRow::Word1) == "PHARAOH",
              "ST-03: Detective self-assignment safe", passed, failed);

        // reduceCredibility
        original.reduceCredibility(30);
        check(original.getCredibility() == 70,
              "ST-03: Detective reduceCredibility(30) reduces 100 to 70", passed, failed);
        original.reduceCredibility(100);
        check(original.getCredibility() == 0,
              "ST-03: Detective reduceCredibility(100) clamps at 0", passed, failed);
    }

    // 9. Child age validation (14 and 20 throw InvalidInputException)
    {
        bool threw14 = false;
        try
        {
            Child c14("Baby", "French", 'X', 1, 14);
        }
        catch (const InvalidInputException&)
        {
            threw14 = true;
        }
        check(threw14, "ST-03: Child age 14 throws InvalidInputException", passed, failed);

        bool threw20 = false;
        try
        {
            Child c20("Adult", "French", 'X', 1, 20);
        }
        catch (const InvalidInputException&)
        {
            threw20 = true;
        }
        check(threw20, "ST-03: Child age 20 throws InvalidInputException", passed, failed);

        bool validChild = false;
        try
        {
            Child cValid("ValidChild", "French", 'X', 1, 17);
            validChild = (cValid.getAge() == 17);
        }
        catch (...)
        {
            validChild = false;
        }
        check(validChild, "ST-03: Child age 17 constructs successfully", passed, failed);
    }

    // 10. CharacterFactory throws on unknown role
    {
        bool threwUnknown = false;
        CharacterRecord badRec;
        badRec.name        = "Unknown";
        badRec.role        = "Astronaut";
        badRec.nationality = "Moon";
        badRec.word2Pos    = 1;
        try
        {
            Character* ptr = CharacterFactory::create(badRec, 5);
            delete ptr;
        }
        catch (const InvalidInputException&)
        {
            threwUnknown = true;
        }
        check(threwUnknown, "ST-03: CharacterFactory throws InvalidInputException on unknown role", passed, failed);
    }

    // Clean up roster through Character* base pointers
    for (int i = 0; i < 8; ++i)
    {
        delete roster[i];
        roster[i] = nullptr;
    }
    check(Character::getLiveCount() == initialLive,
          "ST-03: Deleting all 8 characters via Character* base restores original liveCount", passed, failed);

    std::cout << "----------------------------------------\n";
    std::cout << "  Result: " << passed << " passed, " << failed << " failed.\n";
    std::cout << "========================================\n\n";

    return failed;
}

// ------------------------------------------------------------
// runBoardClassesSelfTest()  --  ST-04 (Commit 4)
// Tests:
//   1. TileRack capacity, full/empty detection, RackFull & RackEmpty
//   2. TileRack duplicate positions, lowercase check, indexing
//   3. TileRack markPlaced, isPlaced, and display output
//   4. TileRack deep copy constructor, deep copy assignment, self-assignment
//   5. Scrabble target word validation (6-letter word throws)
//   6. Scrabble bounds checks: position 0, 8 on Word 1, 9 on Word 2
//   7. Scrabble placement: wrong letter, duplicate slot
//   8. Scrabble verifyWord on incomplete row, wrong word, correct lowercase
//   9. Scrabble PA full word placement (PHARAOH & PYRAMIDS), isComplete
//  10. Scrabble deep copy independence and display rendering
// ------------------------------------------------------------
static int runBoardClassesSelfTest()
{
    int passed = 0;
    int failed = 0;

    std::cout << "\n========================================\n";
    std::cout << "  ST-04  TileRack & Scrabble Self-Test\n";
    std::cout << "========================================\n";

    // --------------------------------------------------------
    // Part 1: TileRack tests
    // --------------------------------------------------------
    std::cout << "\n--- Part 1: TileRack Tests ---\n";

    // Capacity validation (< 1 throws)
    {
        bool threwCap0 = false;
        try { TileRack badRack(0); }
        catch (const InvalidInputException&) { threwCap0 = true; }
        check(threwCap0, "ST-04: TileRack constructor with capacity 0 throws InvalidInputException", passed, failed);
    }

    // Remove from empty rack
    {
        TileRack rack(3);
        check(rack.isEmpty(), "ST-04: TileRack is initially empty", passed, failed);
        check(!rack.isFull(), "ST-04: TileRack is not full initially", passed, failed);
        check(rack.getCount() == 0, "ST-04: TileRack count is 0 initially", passed, failed);

        bool threwEmpty = false;
        try { rack.removeTile(0); }
        catch (const RackEmptyException&) { threwEmpty = true; }
        check(threwEmpty, "ST-04: TileRack::removeTile from empty rack throws RackEmptyException", passed, failed);

        bool threwLastEmpty = false;
        try { rack.removeLast(); }
        catch (const RackEmptyException&) { threwLastEmpty = true; }
        check(threwLastEmpty, "ST-04: TileRack::removeLast from empty rack throws RackEmptyException", passed, failed);

        // Add tiles up to capacity
        Clue c1{'A', 1, WordRow::Word1};
        Clue c2{'B', 2, WordRow::Word1};
        Clue c3{'C', 3, WordRow::Word1};
        rack.addTile(c1);
        rack.addTile(c2);
        rack.addTile(c3);

        check(rack.isFull(), "ST-04: TileRack isFull() true at capacity (3/3)", passed, failed);
        check(rack.getCount() == 3, "ST-04: TileRack count is 3", passed, failed);

        // Add beyond capacity throws RackFullException
        bool threwFull = false;
        try { Clue c4{'D', 4, WordRow::Word1}; rack.addTile(c4); }
        catch (const RackFullException&) { threwFull = true; }
        check(threwFull, "ST-04: TileRack::addTile beyond capacity throws RackFullException", passed, failed);
    }

    // Duplicate position, lowercase letter, markPlaced, isPlaced
    {
        TileRack rack(5);
        Clue c1{'A', 1, WordRow::Word1};
        rack.addTile(c1);

        // Duplicate position throws SlotFilledException
        bool threwDupPos = false;
        try { Clue dup{'Z', 1, WordRow::Word1}; rack.addTile(dup); }
        catch (const SlotFilledException&) { threwDupPos = true; }
        check(threwDupPos, "ST-04: TileRack::addTile with duplicate position throws SlotFilledException", passed, failed);

        // Lowercase letter throws InvalidLetterException
        bool threwLower = false;
        try { Clue lower{'b', 2, WordRow::Word1}; rack.addTile(lower); }
        catch (const InvalidLetterException&) { threwLower = true; }
        check(threwLower, "ST-04: TileRack::addTile with lowercase letter throws InvalidLetterException", passed, failed);

        // markPlaced and isPlaced
        check(!rack.isPlaced(1), "ST-04: Tile at position 1 initially not placed", passed, failed);
        rack.markPlaced(1);
        check(rack.isPlaced(1), "ST-04: Tile at position 1 isPlaced() true after markPlaced()", passed, failed);
        check(rack.hasPosition(1), "ST-04: TileRack hasPosition(1) returns true", passed, failed);
        check(!rack.hasPosition(2), "ST-04: TileRack hasPosition(2) returns false", passed, failed);

        // Display check
        std::cout << "  Sample TileRack display output: ";
        rack.display(std::cout);
        std::cout << "\n";
        check(true, "ST-04: TileRack display() executed successfully", passed, failed);
    }

    // Deep copy independence & self-assignment
    {
        TileRack orig(4);
        orig.addTile(Clue{'X', 1, WordRow::Word1});
        orig.addTile(Clue{'Y', 2, WordRow::Word1});

        TileRack copyRack = orig; // Copy constructor
        TileRack assignRack(2);
        assignRack = orig;        // Assignment operator

        copyRack.addTile(Clue{'Z', 3, WordRow::Word1});
        assignRack.removeLast();

        check(orig.getCount() == 2 && orig.getLetterAt(0) == 'X' && orig.getLetterAt(1) == 'Y',
              "ST-04: TileRack deep copy: original unchanged when copy is modified", passed, failed);
        check(copyRack.getCount() == 3 && copyRack.getLetterAt(2) == 'Z',
              "ST-04: TileRack copy constructor instance operates on independent memory", passed, failed);
        check(assignRack.getCount() == 1 && assignRack.getLetterAt(0) == 'X',
              "ST-04: TileRack copy assignment instance operates on independent memory", passed, failed);

        // Self assignment
        orig = orig;
        check(orig.getCount() == 2 && orig.getLetterAt(0) == 'X' && orig.getLetterAt(1) == 'Y',
              "ST-04: TileRack self-assignment is safe and preserves data", passed, failed);
    }

    // --------------------------------------------------------
    // Part 2: Scrabble board tests
    // --------------------------------------------------------
    std::cout << "\n--- Part 2: Scrabble Board Tests ---\n";

    // Target word validation (6-letter word throws InvalidWordException)
    {
        Scrabble sInvalid;
        bool threw6 = false;
        try { sInvalid.loadTargetWords("PHARAO", "PYRAMIDS"); } // 6 letters on Word 1
        catch (const InvalidWordException&) { threw6 = true; }
        check(threw6, "ST-04: Scrabble::loadTargetWords with 6-letter Word 1 throws InvalidWordException", passed, failed);
    }

    // PA target words: PHARAOH (7) and PYRAMIDS (8)
    {
        Scrabble sPA("PHARAOH", "PYRAMIDS");
        check(sPA.getRowLength(WordRow::Word1) == 7, "ST-04: Scrabble Word 1 length is 7", passed, failed);
        check(sPA.getRowLength(WordRow::Word2) == 8, "ST-04: Scrabble Word 2 length is 8", passed, failed);

        // Position 0 on row 1 throws InvalidPositionException
        bool threwPos0 = false;
        try { sPA.placeLetter(Clue{'P', 0, WordRow::Word1}); }
        catch (const InvalidPositionException&) { threwPos0 = true; }
        check(threwPos0, "ST-04: Scrabble position 0 throws InvalidPositionException", passed, failed);

        // Position 8 on row 1 throws InvalidPositionException
        bool threwPos8Row1 = false;
        try { sPA.placeLetter(Clue{'P', 8, WordRow::Word1}); }
        catch (const InvalidPositionException&) { threwPos8Row1 = true; }
        check(threwPos8Row1, "ST-04: Scrabble position 8 on row 1 throws InvalidPositionException", passed, failed);

        // Position 9 on row 2 throws InvalidPositionException
        bool threwPos9Row2 = false;
        try { sPA.placeLetter(Clue{'S', 9, WordRow::Word2}); }
        catch (const InvalidPositionException&) { threwPos9Row2 = true; }
        check(threwPos9Row2, "ST-04: Scrabble position 9 on row 2 throws InvalidPositionException", passed, failed);

        // Position 8 on row 2 is valid!
        bool threwPos8Row2 = false;
        try { sPA.placeLetter(Clue{'S', 8, WordRow::Word2}); }
        catch (const InvalidPositionException&) { threwPos8Row2 = true; }
        check(!threwPos8Row2, "ST-04: Scrabble position 8 on row 2 is valid and succeeds", passed, failed);

        // Placing on already filled position throws SlotFilledException
        bool threwTwice = false;
        try { sPA.placeLetter(Clue{'S', 8, WordRow::Word2}); }
        catch (const SlotFilledException&) { threwTwice = true; }
        check(threwTwice, "ST-04: Scrabble placing on already filled slot throws SlotFilledException", passed, failed);

        // Wrong letter throws InvalidLetterException
        bool threwWrongLetter = false;
        try { sPA.placeLetter(Clue{'Z', 1, WordRow::Word1}); } // target is 'P'
        catch (const InvalidLetterException&) { threwWrongLetter = true; }
        check(threwWrongLetter, "ST-04: Scrabble placing incorrect letter throws InvalidLetterException", passed, failed);

        // verifyWord on incomplete row throws WordIncompleteException
        bool threwIncomplete = false;
        try { sPA.verifyWord(WordRow::Word1, "PHARAOH"); }
        catch (const WordIncompleteException&) { threwIncomplete = true; }
        check(threwIncomplete, "ST-04: Scrabble verifyWord on incomplete row throws WordIncompleteException", passed, failed);

        // Place all letters of Word 1: P, H, A, R, A, O, H
        sPA.placeLetter(Clue{'P', 1, WordRow::Word1});
        sPA.placeLetter(Clue{'H', 2, WordRow::Word1});
        sPA.placeLetter(Clue{'A', 3, WordRow::Word1});
        sPA.placeLetter(Clue{'R', 4, WordRow::Word1});
        sPA.placeLetter(Clue{'A', 5, WordRow::Word1});
        sPA.placeLetter(Clue{'O', 6, WordRow::Word1});
        sPA.placeLetter(Clue{'H', 7, WordRow::Word1});

        check(sPA.isRowComplete(WordRow::Word1), "ST-04: Scrabble Word 1 is complete after placing 7 letters", passed, failed);
        check(sPA.getRowString(WordRow::Word1) == "PHARAOH", "ST-04: Scrabble getRowString(Word 1) == 'PHARAOH'", passed, failed);

        // verifyWord with incorrect word throws InvalidWordException
        bool threwWrongWord = false;
        try { sPA.verifyWord(WordRow::Word1, "PIRATES"); }
        catch (const InvalidWordException&) { threwWrongWord = true; }
        check(threwWrongWord, "ST-04: Scrabble verifyWord with incorrect word throws InvalidWordException", passed, failed);

        // verifyWord with correct word in lowercase succeeds
        bool verifiedLower = false;
        try
        {
            sPA.verifyWord(WordRow::Word1, "pharaoh");
            verifiedLower = true;
        }
        catch (...)
        {
            verifiedLower = false;
        }
        check(verifiedLower, "ST-04: Scrabble verifyWord accepts lowercase input ('pharaoh')", passed, failed);

        // Place remaining letters of Word 2: P, Y, R, A, M, I, D (S@8 already placed)
        sPA.placeLetter(Clue{'P', 1, WordRow::Word2});
        sPA.placeLetter(Clue{'Y', 2, WordRow::Word2});
        sPA.placeLetter(Clue{'R', 3, WordRow::Word2});
        sPA.placeLetter(Clue{'A', 4, WordRow::Word2});
        sPA.placeLetter(Clue{'M', 5, WordRow::Word2});
        sPA.placeLetter(Clue{'I', 6, WordRow::Word2});
        sPA.placeLetter(Clue{'D', 7, WordRow::Word2});

        check(sPA.isRowComplete(WordRow::Word2), "ST-04: Scrabble Word 2 is complete after placing 8 letters", passed, failed);
        check(sPA.isComplete(), "ST-04: Scrabble isComplete() returns true when both rows are complete", passed, failed);

        // Scrabble display check
        std::cout << "\n--- Sample Scrabble Board Display ---\n";
        sPA.display(std::cout);
        check(true, "ST-04: Scrabble display() executed successfully", passed, failed);

        // Scrabble deep copy independence
        Scrabble sCopy = sPA;
        Scrabble sAssign;
        sAssign = sPA;

        check(sCopy.isComplete() && sAssign.isComplete(),
              "ST-04: Scrabble deep copy: copies replicate complete board state", passed, failed);
        check(sCopy.revealLetter(WordRow::Word1, 1) == 'P' &&
              sAssign.revealLetter(WordRow::Word2, 8) == 'S',
              "ST-04: Scrabble deep copy: revealed letters match original across copies", passed, failed);
    }

    std::cout << "----------------------------------------\n";
    std::cout << "  Result: " << passed << " passed, " << failed << " failed.\n";
    std::cout << "========================================\n\n";

    return failed;
}

// ------------------------------------------------------------
// runCottageAndDetectiveSelfTest()  --  ST-05 (Commit 5)
// ------------------------------------------------------------
static int runCottageAndDetectiveSelfTest()
{
    int passed = 0;
    int failed = 0;

    std::cout << "\n========================================\n";
    std::cout << "  ST-05  Cottage & Detective Self-Test\n";
    std::cout << "========================================\n";

    // 1. Load real locations.txt
    LocationRecord locRecords[DataLoader::LOCATION_COUNT];
    int loaded = DataLoader::loadLocations(DataLoader::FILE_LOCATIONS, locRecords, DataLoader::LOCATION_COUNT);
    check(loaded == 7, "ST-05: Loaded 7 location records from locations.txt", passed, failed);

    // 2. Build Cottage with culprit column 5 (PA)
    // Appendix A Table 8 / locations.txt for PA:
    // Room 1 (Bedroom_1):   Pos 1, 'P'
    // Room 2 (Bedroom_2):   Pos 2, 'H'
    // Room 3 (Study_Room):  Pos 3, 'A'
    // Room 4 (Garden):      Pos 4, 'R'
    // Room 5 (Kitchen):     Pos 5, 'A'
    // Room 6 (Dining_Room): Pos 6, 'O'
    // Room 7 (Living_Room): Pos 7, 'H'
    Cottage cottage(locRecords, loaded, 5);
    check(cottage.getLocationCount() == 7, "ST-05: Cottage locationCount is 7", passed, failed);
    check(cottage.getSearchedCount() == 0, "ST-05: Initially 0 locations searched", passed, failed);
    check(!cottage.allSearched(), "ST-05: Initially allSearched() is false", passed, failed);

    // Check displayLocations initially contains NOT Searched
    {
        std::string name1 = cottage.getLocationName(1);
        check(name1 == "Bedroom 1", "ST-05: getLocationName(1) converts underscores to spaces ('Bedroom 1')", passed, failed);
        check(!cottage.isLocationSearched(1), "ST-05: Location 1 is initially NOT searched", passed, failed);
    }

    // 3. Out-of-bounds positions throw InvalidPositionException (0 and 8)
    {
        bool caughtZero = false;
        try { cottage.searchLocation(0); }
        catch (const InvalidPositionException&) { caughtZero = true; }
        check(caughtZero, "ST-05: Cottage searchLocation(0) throws InvalidPositionException", passed, failed);

        bool caughtEight = false;
        try { cottage.searchLocation(8); }
        catch (const InvalidPositionException&) { caughtEight = true; }
        check(caughtEight, "ST-05: Cottage searchLocation(8) throws InvalidPositionException", passed, failed);

        bool caughtIsZero = false;
        try { cottage.isLocationSearched(0); }
        catch (const InvalidPositionException&) { caughtIsZero = true; }
        check(caughtIsZero, "ST-05: Cottage isLocationSearched(0) throws InvalidPositionException", passed, failed);

        bool caughtClueEight = false;
        try { cottage.getLocationClue(8); }
        catch (const InvalidPositionException&) { caughtClueEight = true; }
        check(caughtClueEight, "ST-05: Cottage getLocationClue(8) throws InvalidPositionException", passed, failed);

        bool caughtNameZero = false;
        try { cottage.getLocationName(0); }
        catch (const InvalidPositionException&) { caughtNameZero = true; }
        check(caughtNameZero, "ST-05: Cottage getLocationName(0) throws InvalidPositionException", passed, failed);
    }

    // 4. Set up Scrabble and Detective to demonstrate integration
    Scrabble scrabble("PHARAOH", "PYRAMIDS");
    Detective detective("Sherlock", 100);

    // Search locations 1 to 7 in a scrambled order: 3, 1, 7, 2, 5, 4, 6
    int scrambledOrder[7] = { 3, 1, 7, 2, 5, 4, 6 };
    char expectedLetters[7]   = { 'A', 'P', 'H', 'H', 'A', 'R', 'O' }; // corresponding to rooms 3, 1, 7, 2, 5, 4, 6
    int  expectedPositions[7] = { 3,   1,   7,   2,   5,   4,   6   }; // corresponding positions in Word 1

    bool cluesCorrect = true;
    for (int k = 0; k < 7; ++k)
    {
        int roomNum = scrambledOrder[k];
        Clue searchedClue = cottage.searchLocation(roomNum);

        if (searchedClue.clueLetter != expectedLetters[k] ||
            searchedClue.cluePosition != expectedPositions[k] ||
            searchedClue.clueRow != WordRow::Word1)
        {
            cluesCorrect = false;
        }

        if (!cottage.isLocationSearched(roomNum))
        {
            cluesCorrect = false;
        }

        // Repeat getLocationClue returns same clue without changing state
        Clue repeatClue = cottage.getLocationClue(roomNum);
        if (repeatClue.clueLetter != searchedClue.clueLetter ||
            repeatClue.cluePosition != searchedClue.cluePosition ||
            repeatClue.clueRow != searchedClue.clueRow)
        {
            cluesCorrect = false;
        }

        // Place clue into scrabble and detective's rack
        scrabble.placeLetter(searchedClue);
        detective.collectClue(searchedClue);
    }
    check(cluesCorrect, "ST-05: Scrambled searches return expected PA clues, set flags, and repeat cleanly", passed, failed);
    check(cottage.getSearchedCount() == 7, "ST-05: Cottage getSearchedCount() is 7 after all searches", passed, failed);
    check(cottage.allSearched(), "ST-05: Cottage allSearched() is true after searching all 7 rooms", passed, failed);

    // 5. Scrabble Word 1 completion and verification
    check(scrabble.isRowComplete(WordRow::Word1), "ST-05: Scrabble Word 1 row is complete after placing all 7 location clues", passed, failed);
    check(scrabble.getRowString(WordRow::Word1) == "PHARAOH", "ST-05: Scrabble Word 1 spelled out 'PHARAOH'", passed, failed);
    bool word1Verified = false;
    try
    {
        scrabble.verifyWord(WordRow::Word1, "PHARAOH");
        word1Verified = true;
    }
    catch (...) { word1Verified = false; }
    check(word1Verified, "ST-05: Scrabble verifyWord(WordRow::Word1, 'PHARAOH') succeeded", passed, failed);

    // 6. Display locations sample output
    std::cout << "\n--- Sample Cottage displayLocations() Output ---\n";
    cottage.displayLocations(std::cout);
    check(true, "ST-05: Cottage displayLocations() rendered with 'Searched' tags", passed, failed);

    // 7. Cottage DEEP COPY: copy constructor and assignment operator
    {
        Cottage freshCottage(locRecords, loaded, 5);
        freshCottage.searchLocation(1); // Search room 1 in original

        Cottage copyCottage = freshCottage; // Copy constructor
        Cottage assignCottage;
        assignCottage = freshCottage;       // Copy assignment

        check(copyCottage.isLocationSearched(1), "ST-05: Cottage copy constructor inherits searched status of room 1", passed, failed);
        check(assignCottage.isLocationSearched(1), "ST-05: Cottage copy assignment inherits searched status of room 1", passed, failed);

        // Search room 2 only in copyCottage
        copyCottage.searchLocation(2);
        check(copyCottage.isLocationSearched(2), "ST-05: copyCottage room 2 is searched", passed, failed);
        check(!freshCottage.isLocationSearched(2), "ST-05: freshCottage room 2 remains NOT searched (independent heap memory)", passed, failed);
        check(!assignCottage.isLocationSearched(2), "ST-05: assignCottage room 2 remains NOT searched", passed, failed);

        // Search room 3 in freshCottage
        freshCottage.searchLocation(3);
        check(freshCottage.isLocationSearched(3), "ST-05: freshCottage room 3 is searched", passed, failed);
        check(!assignCottage.isLocationSearched(3), "ST-05: assignCottage room 3 remains NOT searched (independent heap memory)", passed, failed);

        // Self-assignment
        freshCottage = freshCottage;
        check(freshCottage.getLocationCount() == 7 && freshCottage.isLocationSearched(1) && freshCottage.isLocationSearched(3),
              "ST-05: Cottage self-assignment is safe and preserves data", passed, failed);
    }

    // 8. Detective with composed racks
    {
        check(detective.getRack(WordRow::Word1).getCount() == 7,
              "ST-05: Detective locationRack contains 7 collected clues", passed, failed);
        check(detective.getRack(WordRow::Word2).getCount() == 0,
              "ST-05: Detective characterRack is initially empty", passed, failed);

        // Collect 8 clues for Word 2
        for (int i = 1; i <= 8; ++i)
        {
            detective.collectClue(Clue{static_cast<char>('A' + i - 1), i, WordRow::Word2});
        }
        check(detective.getRack(WordRow::Word2).getCount() == 8,
              "ST-05: Detective characterRack filled to capacity (8 tiles)", passed, failed);

        // Attempting to add a 9th clue to characterRack (capacity 8) throws RackFullException
        bool caughtRackFull = false;
        try
        {
            detective.collectClue(Clue{'Z', 9, WordRow::Word2});
        }
        catch (const RackFullException&)
        {
            caughtRackFull = true;
        }
        check(caughtRackFull, "ST-05: 9th clue collected for Word 2 throws RackFullException", passed, failed);

        // Detective deep copy test
        Detective detCopy = detective;
        check(detCopy.getRack(WordRow::Word1).getCount() == 7,
              "ST-05: Detective copy has 7 location tiles", passed, failed);

        // Modifying detCopy rack does not affect detective original rack
        detCopy.getRack(WordRow::Word1).removeLast();
        check(detCopy.getRack(WordRow::Word1).getCount() == 6,
              "ST-05: Detective copy location rack reduced to 6", passed, failed);
        check(detective.getRack(WordRow::Word1).getCount() == 7,
              "ST-05: Original Detective location rack remains intact at 7 (independent memory)", passed, failed);

        // Display racks check
        std::cout << "\n--- Sample Detective displayRacks() Output ---\n";
        detective.displayRacks(std::cout);
        check(true, "ST-05: Detective displayRacks() executed successfully", passed, failed);
    }

    std::cout << "----------------------------------------\n";
    std::cout << "  Result: " << passed << " passed, " << failed << " failed.\n";
    std::cout << "========================================\n\n";

    return failed;
}

// ------------------------------------------------------------
// runDynamicArraySelfTest()  --  ST-06 (Commit 6, Module 8)
// ------------------------------------------------------------
static int runDynamicArraySelfTest()
{
    int passed = 0;
    int failed = 0;

    std::cout << "\n========================================\n";
    std::cout << "  ST-06  DynamicArray<T> Template Self-Test\n";
    std::cout << "========================================\n";

    // 1. Initial capacity validation and error handling
    {
        bool caughtZero = false;
        try { DynamicArray<int> badArr(0); }
        catch (const InvalidInputException&) { caughtZero = true; }
        check(caughtZero, "ST-06: DynamicArray initialCapacity 0 throws InvalidInputException", passed, failed);

        bool caughtNegative = false;
        try { DynamicArray<int> badArr(-5); }
        catch (const InvalidInputException&) { caughtNegative = true; }
        check(caughtNegative, "ST-06: DynamicArray initialCapacity -5 throws InvalidInputException", passed, failed);
    }

    // 2. DynamicArray<int>: push 100 items, check auto-growth, read back
    {
        DynamicArray<int> intArr(4);
        check(intArr.isEmpty(), "ST-06: DynamicArray<int> is initially empty", passed, failed);
        check(intArr.getSize() == 0, "ST-06: DynamicArray<int> initial size is 0", passed, failed);
        check(intArr.getCapacity() == 4, "ST-06: DynamicArray<int> initial capacity is 4", passed, failed);

        // Push 100 items (0..99)
        for (int i = 0; i < 100; ++i)
        {
            intArr.push(i * 10);
        }
        check(intArr.getSize() == 100, "ST-06: DynamicArray<int> size is 100 after 100 pushes", passed, failed);
        check(intArr.getCapacity() >= 100, "ST-06: DynamicArray<int> auto-growth expanded capacity >= 100", passed, failed);
        check(!intArr.isEmpty(), "ST-06: DynamicArray<int> isEmpty() is false", passed, failed);

        // Read all 100 elements back
        bool elementsMatch = true;
        for (int i = 0; i < 100; ++i)
        {
            if (intArr[i] != i * 10)
            {
                elementsMatch = false;
            }
        }
        check(elementsMatch, "ST-06: DynamicArray<int> all 100 pushed elements read back correctly", passed, failed);

        // Bounds checking on operator[]
        bool caughtNeg = false;
        try { int x = intArr[-1]; (void)x; }
        catch (const InvalidPositionException&) { caughtNeg = true; }
        check(caughtNeg, "ST-06: DynamicArray<int> operator[] with index -1 throws InvalidPositionException", passed, failed);

        bool caughtSize = false;
        try { int x = intArr[100]; (void)x; }
        catch (const InvalidPositionException&) { caughtSize = true; }
        check(caughtSize, "ST-06: DynamicArray<int> operator[] with index size (100) throws InvalidPositionException", passed, failed);

        // Const operator[] check
        const DynamicArray<int>& constRef = intArr;
        check(constRef[0] == 0 && constRef[99] == 990, "ST-06: DynamicArray<int> const operator[] reads valid elements", passed, failed);
        bool constCaught = false;
        try { int x = constRef[105]; (void)x; }
        catch (const InvalidPositionException&) { constCaught = true; }
        check(constCaught, "ST-06: DynamicArray<int> const operator[] out of bounds throws", passed, failed);

        // removeAt
        intArr.removeAt(0); // removes 0; new index 0 should be 10
        check(intArr.getSize() == 99 && intArr[0] == 10, "ST-06: DynamicArray<int> removeAt(0) shifts elements left", passed, failed);

        // clear
        intArr.clear();
        check(intArr.isEmpty() && intArr.getSize() == 0, "ST-06: DynamicArray<int> clear() resets size to 0 and isEmpty is true", passed, failed);

        // resize
        intArr.resize(5);
        check(intArr.getSize() == 5, "ST-06: DynamicArray<int> resize(5) creates 5 elements", passed, failed);
        check(intArr[0] == 0 && intArr[4] == 0, "ST-06: DynamicArray<int> resize default-initializes newly created elements to 0", passed, failed);

        bool caughtResizeNeg = false;
        try { intArr.resize(-2); }
        catch (const InvalidInputException&) { caughtResizeNeg = true; }
        check(caughtResizeNeg, "ST-06: DynamicArray<int> resize(-2) throws InvalidInputException", passed, failed);
    }

    // 3. DynamicArray<char>: push, removeAt, check
    {
        DynamicArray<char> charArr(2);
        for (char c = 'A'; c <= 'Z'; ++c)
        {
            charArr.push(c);
        }
        check(charArr.getSize() == 26, "ST-06: DynamicArray<char> contains 26 uppercase letters", passed, failed);
        check(charArr[0] == 'A' && charArr[25] == 'Z', "ST-06: DynamicArray<char> first is 'A' and last is 'Z'", passed, failed);

        charArr.removeAt(1); // removes 'B'; index 1 should now be 'C'
        check(charArr.getSize() == 25 && charArr[1] == 'C', "ST-06: DynamicArray<char> removeAt(1) removed 'B' and shifted 'C' left", passed, failed);
    }

    // 4. DynamicArray<std::string>: deep copy and self-assignment
    {
        DynamicArray<std::string> strArr(2);
        strArr.push("London");
        strArr.push("Paris");
        strArr.push("Cairo");

        DynamicArray<std::string> copyArr = strArr; // Copy constructor
        DynamicArray<std::string> assignArr(1);
        assignArr = strArr;                         // Copy assignment

        check(copyArr.getSize() == 3 && assignArr.getSize() == 3,
              "ST-06: DynamicArray<string> copy ctor and assignment replicate size", passed, failed);

        copyArr[0]   = "Berlin";
        assignArr[0] = "Tokyo";

        check(strArr[0] == "London", "ST-06: Mutating string copies leaves original unchanged (deep copy)", passed, failed);
        check(copyArr[0] == "Berlin" && assignArr[0] == "Tokyo",
              "ST-06: Copies maintain independent heap data buffers", passed, failed);

        // Self-assignment
        strArr = strArr;
        check(strArr.getSize() == 3 && strArr[0] == "London" && strArr[2] == "Cairo",
              "ST-06: DynamicArray<string> self-assignment is safe and preserves data", passed, failed);
    }

    // 5. DynamicArray<Tile>: struct elements deep copy and mutation independence
    {
        DynamicArray<Tile> tileArr(2);
        Tile t1{'P', true};
        Tile t2{'A', false};
        tileArr.push(t1);
        tileArr.push(t2);

        DynamicArray<Tile> tileCopy = tileArr;
        tileCopy[0].tileLetter = 'X';
        tileCopy[0].filled = false;

        check(tileArr[0].tileLetter == 'P' && tileArr[0].filled == true,
              "ST-06: DynamicArray<Tile> copy mutation does not affect original Tile array", passed, failed);
    }

    // 6. DynamicArray<Location>: domain entity container operations
    {
        DynamicArray<Location> locArr(2);
        Location loc1;
        loc1.locName = "Study_Room";
        loc1.boardPosLoc = 3;
        loc1.letterClueLoc = 'A';
        loc1.locSearchFlag = false;

        locArr.push(loc1);
        check(locArr.getSize() == 1 && locArr[0].locName == "Study_Room",
              "ST-06: DynamicArray<Location> holds and retrieves Location struct", passed, failed);

        locArr[0].locSearchFlag = true;
        check(locArr[0].locSearchFlag == true,
              "ST-06: DynamicArray<Location> non-const operator[] allows modifying elements in-place", passed, failed);

        locArr.clear();
        check(locArr.isEmpty() && locArr.getSize() == 0,
              "ST-06: DynamicArray<Location> clear() empties container", passed, failed);
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
        totalFailures += runCharacterHierarchySelfTest();
        totalFailures += runBoardClassesSelfTest();
        totalFailures += runCottageAndDetectiveSelfTest();
        totalFailures += runDynamicArraySelfTest();
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
