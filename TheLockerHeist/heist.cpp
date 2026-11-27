// ============================================================
// heist.cpp  --  THE LOCKER HEIST
// Definitions for all types declared in heist.h:
//   - RulesConfig default generator
//   - GameException family
//   - StringUtils static methods
//   - DataLoader file loaders and consistency validator
//   - Character hierarchy and CharacterFactory
//   - Board classes (TileRack and Scrabble using raw DMA)
// ============================================================

#include "heist.h"

// Implementation-only headers live here in .cpp files to maintain
// fast compilation and keep heist.h clean.
#include <fstream>    // std::ifstream, std::ofstream for data file handling
#include <cstdlib>    // rand, srand, system()
#include <ctime>      // std::time, localtime_s for timestamp generation


// ============================================================
// SECTION 1 -- RECORD STRUCT IMPLEMENTATIONS
// ============================================================

// defaultRules() returns the standard baseline configuration
// if rules.txt is corrupted or unavailable.
RulesConfig RulesConfig::defaultRules()
{
    RulesConfig config;
    config.maxTurns            = 25;
    config.startingCredibility = 100;
    config.wrongTurnPenalty    = -5;
    config.wrongWordPenalty    = -10;
    config.winCondition        = "Accuse_Correct_Culprit";
    config.loseCondition       = "Credibility_Zero_Or_Turns_Exhausted";
    return config;
}


// ============================================================
// SECTION 2 -- EXCEPTION HIERARCHY IMPLEMENTATIONS
// ============================================================

// Base GameException
GameException::GameException(const std::string& msg, int code)
    : message(msg), errorCode(code)
{
}

GameException::~GameException()
{
}

std::string GameException::getMessage() const
{
    return message;
}

int GameException::getErrorCode() const
{
    return errorCode;
}

std::string GameException::getCategory() const
{
    return "General Exception";
}


// InvalidInputException
InvalidInputException::InvalidInputException(const std::string& msg)
    : GameException(msg, 101)
{
}

std::string InvalidInputException::getCategory() const
{
    return "Invalid Input";
}


// FileNotFoundException
FileNotFoundException::FileNotFoundException(const std::string& fileName)
    : GameException("File not found: " + fileName, 102)
{
}

std::string FileNotFoundException::getCategory() const
{
    return "File Not Found";
}


// FileCorruptException
FileCorruptException::FileCorruptException(const std::string& fileName, int lineNumber, const std::string& detail)
    : GameException(fileName + " (line " + StringUtils::intToString(lineNumber) + "): " + detail, 103)
{
}

std::string FileCorruptException::getCategory() const
{
    return "File Corrupt";
}


// InvalidPositionException
InvalidPositionException::InvalidPositionException(int position, int maxPosition)
    : GameException("Invalid position " + StringUtils::intToString(position) +
                    "; must be between 1 and " + StringUtils::intToString(maxPosition), 104)
{
}

std::string InvalidPositionException::getCategory() const
{
    return "Invalid Position";
}


// SlotFilledException
SlotFilledException::SlotFilledException(int position)
    : GameException("Slot at position " + StringUtils::intToString(position) + " is already filled", 105)
{
}

std::string SlotFilledException::getCategory() const
{
    return "Slot Filled";
}


// InvalidLetterException
InvalidLetterException::InvalidLetterException(char letter, int position)
    : GameException(std::string("Invalid letter '") + letter +
                    "' for position " + StringUtils::intToString(position), 106)
{
}

std::string InvalidLetterException::getCategory() const
{
    return "Invalid Letter";
}


// WordIncompleteException
WordIncompleteException::WordIncompleteException(const std::string& rowName)
    : GameException("Word incomplete on row: " + rowName, 107)
{
}

std::string WordIncompleteException::getCategory() const
{
    return "Word Incomplete";
}


// InvalidWordException
InvalidWordException::InvalidWordException(const std::string& word)
    : GameException("Invalid word submitted: " + word, 108)
{
}

std::string InvalidWordException::getCategory() const
{
    return "Invalid Word";
}


// RackFullException
RackFullException::RackFullException(int capacity)
    : GameException("Tile rack is full (capacity " + StringUtils::intToString(capacity) + ")", 109)
{
}

std::string RackFullException::getCategory() const
{
    return "Rack Full";
}


// RackEmptyException
RackEmptyException::RackEmptyException()
    : GameException("Tile rack is empty", 110)
{
}

std::string RackEmptyException::getCategory() const
{
    return "Rack Empty";
}


// InterviewRestrictedException
InterviewRestrictedException::InterviewRestrictedException(const std::string& personName,
                                                           int requiredCredibility,
                                                           int currentCredibility)
    : GameException("Interview with " + personName + " restricted: requires " +
                    StringUtils::intToString(requiredCredibility) + " credibility, current is " +
                    StringUtils::intToString(currentCredibility), 111)
{
}

std::string InterviewRestrictedException::getCategory() const
{
    return "Interview Restricted";
}


// ============================================================
// SECTION 3 -- STRING UTILITY IMPLEMENTATIONS
// Written completely without <cctype>, <algorithm>, or <sstream>.
// ============================================================

// Strips whitespace characters (spaces, tabs, returns, newlines) from edges.
std::string StringUtils::trim(const std::string& str)
{
    if (str.empty())
    {
        return "";
    }
    int start = 0;
    int end   = static_cast<int>(str.length()) - 1;

    while (start <= end && (str[static_cast<size_t>(start)] == ' '  ||
                            str[static_cast<size_t>(start)] == '\t' ||
                            str[static_cast<size_t>(start)] == '\r' ||
                            str[static_cast<size_t>(start)] == '\n'))
    {
        ++start;
    }

    while (end >= start && (str[static_cast<size_t>(end)] == ' '  ||
                            str[static_cast<size_t>(end)] == '\t' ||
                            str[static_cast<size_t>(end)] == '\r' ||
                            str[static_cast<size_t>(end)] == '\n'))
    {
        --end;
    }

    if (start > end)
    {
        return "";
    }
    return str.substr(static_cast<size_t>(start), static_cast<size_t>(end - start + 1));
}

// Tokenizes a line into a fixed array using a single character delimiter.
int StringUtils::splitByDelimiter(const std::string& line, char delimiter, std::string parts[], int maxParts)
{
    int count = 0;
    std::string current = "";

    for (size_t i = 0; i < line.length(); ++i)
    {
        char c = line[i];
        if (c == delimiter)
        {
            if (count < maxParts)
            {
                parts[count++] = current;
                current = "";
            }
        }
        else
        {
            current += c;
        }
    }

    if (count < maxParts)
    {
        parts[count++] = current;
    }

    return count;
}

// Converts lowercase ASCII letters to uppercase.
std::string StringUtils::toUpperCase(const std::string& str)
{
    std::string result = "";
    for (size_t i = 0; i < str.length(); ++i)
    {
        char c = str[i];
        if (c >= 'a' && c <= 'z')
        {
            result += static_cast<char>(c - ('a' - 'A'));
        }
        else
        {
            result += c;
        }
    }
    return result;
}

// Returns true if the character is an ASCII alphabet letter.
bool StringUtils::isSingleLetter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Returns true if non-empty and every character is an alphabet letter.
bool StringUtils::isAllLetters(const std::string& str)
{
    if (str.empty())
    {
        return false;
    }
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (!isSingleLetter(str[i]))
        {
            return false;
        }
    }
    return true;
}

// Manually parses an integer with support for leading '-' and '+'.
int StringUtils::parseInt(const std::string& str)
{
    std::string trimmed = trim(str);
    if (trimmed.empty())
    {
        throw InvalidInputException("Cannot parse integer from empty string");
    }

    bool negative = false;
    size_t startIndex = 0;

    if (trimmed[0] == '-')
    {
        negative = true;
        startIndex = 1;
    }
    else if (trimmed[0] == '+')
    {
        startIndex = 1;
    }

    if (startIndex >= trimmed.length())
    {
        throw InvalidInputException("Invalid integer string: " + str);
    }

    int result = 0;
    for (size_t i = startIndex; i < trimmed.length(); ++i)
    {
        char c = trimmed[i];
        if (c < '0' || c > '9')
        {
            throw InvalidInputException("Invalid integer character '" + std::string(1, c) + "' in: " + str);
        }
        result = result * 10 + (c - '0');
    }

    return negative ? -result : result;
}

// Converts integer to string without using stringstream or std::to_string.
std::string StringUtils::intToString(int value)
{
    if (value == 0)
    {
        return "0";
    }

    unsigned int u = (value < 0) ? (0u - static_cast<unsigned int>(value)) : static_cast<unsigned int>(value);
    std::string rev = "";

    while (u > 0)
    {
        rev += static_cast<char>('0' + (u % 10));
        u /= 10;
    }

    if (value < 0)
    {
        rev += '-';
    }

    std::string res = "";
    for (int i = static_cast<int>(rev.length()) - 1; i >= 0; --i)
    {
        res += rev[static_cast<size_t>(i)];
    }

    return res;
}

// Pads an integer with leading zeros to meet target width.
std::string StringUtils::padWithZeros(int value, int width)
{
    std::string s = intToString(value);
    while (static_cast<int>(s.length()) < width)
    {
        s = "0" + s;
    }
    return s;
}

// Case-insensitive ASCII comparison.
bool StringUtils::equalsIgnoreCase(const std::string& a, const std::string& b)
{
    if (a.length() != b.length())
    {
        return false;
    }
    return toUpperCase(a) == toUpperCase(b);
}

// Replaces all occurrences of oldChar with newChar.
std::string StringUtils::replaceChar(const std::string& str, char oldChar, char newChar)
{
    std::string res = "";
    for (size_t i = 0; i < str.length(); ++i)
    {
        res += (str[i] == oldChar) ? newChar : str[i];
    }
    return res;
}

// Replaces underscores with spaces.
std::string StringUtils::underscoreToSpace(const std::string& str)
{
    return replaceChar(str, '_', ' ');
}

// Replaces spaces with underscores.
std::string StringUtils::spaceToUnderscore(const std::string& str)
{
    return replaceChar(str, ' ', '_');
}

// Generates date string formatted as DD-MM-YY.
std::string StringUtils::currentDateDDMMYY()
{
    std::time_t now = std::time(nullptr);
    struct tm tmNow;
    localtime_s(&tmNow, &now);

    int day   = tmNow.tm_mday;
    int month = tmNow.tm_mon + 1;
    int year  = (tmNow.tm_year + 1900) % 100;

    return padWithZeros(day, 2) + "-" + padWithZeros(month, 2) + "-" + padWithZeros(year, 2);
}

// Inserts a space after "Mr." or "Ms." if missing.
std::string StringUtils::formatPersonName(const std::string& name)
{
    if (name.length() > 3 && name[0] == 'M' && name[1] == 'r' && name[2] == '.' && name[3] != ' ')
    {
        return "Mr. " + name.substr(3);
    }
    if (name.length() > 3 && name[0] == 'M' && name[1] == 's' && name[2] == '.' && name[3] != ' ')
    {
        return "Ms. " + name.substr(3);
    }
    return name;
}

// Converts internal location identifiers (e.g., Bedroom_1) to friendly display titles.
std::string StringUtils::formatPlaceName(const std::string& name)
{
    return underscoreToSpace(name);
}


// ============================================================
// SECTION 4 -- DATA LOADER IMPLEMENTATIONS
// ============================================================

// Static file path constants
const char* const DataLoader::FILE_RULES      = "rules.txt";
const char* const DataLoader::FILE_CLUE       = "clue.txt";
const char* const DataLoader::FILE_LOCATIONS  = "locations.txt";
const char* const DataLoader::FILE_CHARACTERS = "characters.txt";
const char* const DataLoader::FILE_ABOUT      = "about.txt";
const char* const DataLoader::FILE_SCOREBOARD = "leader/scoreBoard.txt";

// Maps suspect role names to their corresponding 0-based column index.
int DataLoader::roleToColumn(const std::string& role)
{
    std::string r = StringUtils::trim(role);
    if (StringUtils::equalsIgnoreCase(r, "Cook")) return 0;
    if (StringUtils::equalsIgnoreCase(r, "Guard")) return 1;
    if (StringUtils::equalsIgnoreCase(r, "Servant")) return 2;
    if (StringUtils::equalsIgnoreCase(r, "Partner 1") ||
        StringUtils::equalsIgnoreCase(r, "Partner_1") ||
        StringUtils::equalsIgnoreCase(r, "P1")) return 3;
    if (StringUtils::equalsIgnoreCase(r, "Partner 2") ||
        StringUtils::equalsIgnoreCase(r, "Partner_2") ||
        StringUtils::equalsIgnoreCase(r, "P2")) return 4;
    if (StringUtils::equalsIgnoreCase(r, "PA")) return 5;
    if (StringUtils::equalsIgnoreCase(r, "Child")) return 6;

    throw InvalidInputException("Unknown role: " + role);
}

// Reads and validates configuration values from rules.txt.
RulesConfig DataLoader::loadRules(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw FileNotFoundException(path);
    }

    RulesConfig config;
    bool foundMaxTurns    = false;
    bool foundCredibility = false;
    bool foundWrongTurn   = false;
    bool foundWrongWord   = false;
    bool foundWinCond     = false;
    bool foundLoseCond    = false;

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line))
    {
        ++lineNum;
        std::string trimmed = StringUtils::trim(line);
        if (trimmed.empty()) continue;

        std::string parts[2];
        int count = StringUtils::splitByDelimiter(trimmed, '=', parts, 2);
        if (count != 2)
        {
            throw FileCorruptException(path, lineNum, "Expected Key=Value format");
        }

        std::string key   = StringUtils::trim(parts[0]);
        std::string value = StringUtils::trim(parts[1]);

        if (key == "Max_Turns")
        {
            int val = 0;
            try { val = StringUtils::parseInt(value); }
            catch (const InvalidInputException&)
            {
                throw FileCorruptException(path, lineNum, "Non-numeric value for Max_Turns: " + value);
            }
            if (val < 1 || val > 99)
            {
                throw FileCorruptException(path, lineNum, "Max_Turns out of range (1-99): " + value);
            }
            config.maxTurns = val;
            foundMaxTurns = true;
        }
        else if (key == "Starting_Credibility")
        {
            int val = 0;
            try { val = StringUtils::parseInt(value); }
            catch (const InvalidInputException&)
            {
                throw FileCorruptException(path, lineNum, "Non-numeric value for Starting_Credibility: " + value);
            }
            if (val < 1 || val > 100)
            {
                throw FileCorruptException(path, lineNum, "Starting_Credibility out of range (1-100): " + value);
            }
            config.startingCredibility = val;
            foundCredibility = true;
        }
        else if (key == "Every_Wrong_Turn")
        {
            int val = 0;
            try { val = StringUtils::parseInt(value); }
            catch (const InvalidInputException&)
            {
                throw FileCorruptException(path, lineNum, "Non-numeric value for Every_Wrong_Turn: " + value);
            }
            if (val > 0)
            {
                throw FileCorruptException(path, lineNum, "Every_Wrong_Turn must be 0 or negative: " + value);
            }
            config.wrongTurnPenalty = val;
            foundWrongTurn = true;
        }
        else if (key == "Every_Wrong_Word")
        {
            int val = 0;
            try { val = StringUtils::parseInt(value); }
            catch (const InvalidInputException&)
            {
                throw FileCorruptException(path, lineNum, "Non-numeric value for Every_Wrong_Word: " + value);
            }
            if (val > 0)
            {
                throw FileCorruptException(path, lineNum, "Every_Wrong_Word must be 0 or negative: " + value);
            }
            config.wrongWordPenalty = val;
            foundWrongWord = true;
        }
        else if (key == "Win_Condition")
        {
            if (value.empty())
            {
                throw FileCorruptException(path, lineNum, "Empty Win_Condition");
            }
            config.winCondition = value;
            foundWinCond = true;
        }
        else if (key == "Lose_Condition")
        {
            if (value.empty())
            {
                throw FileCorruptException(path, lineNum, "Empty Lose_Condition");
            }
            config.loseCondition = value;
            foundLoseCond = true;
        }
        else
        {
            throw FileCorruptException(path, lineNum, "Unknown rules key: " + key);
        }
    }

    if (!foundMaxTurns)    throw FileCorruptException(path, lineNum, "Missing required key: Max_Turns");
    if (!foundCredibility) throw FileCorruptException(path, lineNum, "Missing required key: Starting_Credibility");
    if (!foundWrongTurn)   throw FileCorruptException(path, lineNum, "Missing required key: Every_Wrong_Turn");
    if (!foundWrongWord)   throw FileCorruptException(path, lineNum, "Missing required key: Every_Wrong_Word");
    if (!foundWinCond)     throw FileCorruptException(path, lineNum, "Missing required key: Win_Condition");
    if (!foundLoseCond)    throw FileCorruptException(path, lineNum, "Missing required key: Lose_Condition");

    return config;
}

// Reads and validates clue profiles from clue.txt.
int DataLoader::loadClueProfiles(const std::string& path, ClueProfile out[], int capacity)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw FileNotFoundException(path);
    }

    std::string heading;
    if (!std::getline(file, heading))
    {
        throw FileCorruptException(path, 1, "Empty clue file");
    }
    if (StringUtils::trim(heading) != "Culprit_Role|Word_1|Word_2")
    {
        throw FileCorruptException(path, 1, "Invalid heading in clue.txt; expected 'Culprit_Role|Word_1|Word_2'");
    }

    std::string line;
    int lineNum = 1;
    int count   = 0;

    while (std::getline(file, line))
    {
        ++lineNum;
        std::string trimmed = StringUtils::trim(line);
        if (trimmed.empty()) continue;

        std::string parts[4];
        int partCount = StringUtils::splitByDelimiter(trimmed, '|', parts, 4);
        if (partCount != 3)
        {
            throw FileCorruptException(path, lineNum, "Expected exactly 3 fields in clue.txt");
        }

        std::string role  = StringUtils::trim(parts[0]);
        std::string word1 = StringUtils::trim(parts[1]);
        std::string word2 = StringUtils::trim(parts[2]);

        // Role verification
        roleToColumn(role);

        // Word 1 length and character validation
        if (word1.length() != static_cast<size_t>(WORD1_LENGTH))
        {
            throw FileCorruptException(path, lineNum, "Word_1 must have length " + StringUtils::intToString(WORD1_LENGTH));
        }
        for (size_t i = 0; i < word1.length(); ++i)
        {
            if (word1[i] < 'A' || word1[i] > 'Z')
            {
                throw FileCorruptException(path, lineNum, "Word_1 must contain only uppercase letters");
            }
        }

        // Word 2 length and character validation
        if (word2.length() != static_cast<size_t>(WORD2_LENGTH))
        {
            throw FileCorruptException(path, lineNum, "Word_2 must have length " + StringUtils::intToString(WORD2_LENGTH));
        }
        for (size_t i = 0; i < word2.length(); ++i)
        {
            if (word2[i] < 'A' || word2[i] > 'Z')
            {
                throw FileCorruptException(path, lineNum, "Word_2 must contain only uppercase letters");
            }
        }

        if (count >= capacity)
        {
            throw FileCorruptException(path, lineNum, "Exceeded profile array capacity");
        }

        out[count].culpritRole = role;
        out[count].word1       = word1;
        out[count].word2       = word2;
        ++count;
    }

    if (count != ROLE_COUNT)
    {
        throw FileCorruptException(path, lineNum, "Expected exactly " + StringUtils::intToString(ROLE_COUNT) + " clue profiles");
    }

    return count;
}

// Reads and validates location clue mappings from locations.txt.
int DataLoader::loadLocations(const std::string& path, LocationRecord out[], int capacity)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw FileNotFoundException(path);
    }

    std::string heading;
    if (!std::getline(file, heading))
    {
        throw FileCorruptException(path, 1, "Empty locations file");
    }
    if (StringUtils::trim(heading) != "Place|W1Pos|Cook|Guard|Servant|P1|P2|PA|Child")
    {
        throw FileCorruptException(path, 1, "Invalid heading in locations.txt");
    }

    bool seenPos[LOCATION_COUNT + 1] = { false };
    std::string line;
    int lineNum = 1;
    int count   = 0;

    while (std::getline(file, line))
    {
        ++lineNum;
        std::string trimmed = StringUtils::trim(line);
        if (trimmed.empty()) continue;

        std::string parts[10];
        int partCount = StringUtils::splitByDelimiter(trimmed, '|', parts, 10);
        if (partCount != 9)
        {
            throw FileCorruptException(path, lineNum, "Expected exactly 9 fields in locations.txt");
        }

        std::string placeName = StringUtils::trim(parts[0]);
        if (placeName.empty())
        {
            throw FileCorruptException(path, lineNum, "Empty place name in locations.txt");
        }

        int pos = 0;
        try { pos = StringUtils::parseInt(parts[1]); }
        catch (const InvalidInputException&)
        {
            throw FileCorruptException(path, lineNum, "Non-numeric W1Pos in locations.txt: " + parts[1]);
        }

        if (pos < 1 || pos > LOCATION_COUNT)
        {
            throw FileCorruptException(path, lineNum, "W1Pos out of range (1-" +
                                       StringUtils::intToString(LOCATION_COUNT) + "): " + parts[1]);
        }
        if (seenPos[pos])
        {
            throw FileCorruptException(path, lineNum, "Duplicate W1Pos: " + StringUtils::intToString(pos));
        }
        seenPos[pos] = true;

        if (count >= capacity)
        {
            throw FileCorruptException(path, lineNum, "Exceeded location array capacity");
        }

        out[count].placeName = placeName;
        out[count].word1Pos  = pos;

        for (int i = 0; i < ROLE_COUNT; ++i)
        {
            std::string let = StringUtils::trim(parts[2 + i]);
            if (let.length() != 1 || let[0] < 'A' || let[0] > 'Z')
            {
                throw FileCorruptException(path, lineNum, "Invalid clue letter '" + let +
                                           "' at column " + StringUtils::intToString(i));
            }
            out[count].letters[i] = let[0];
        }

        ++count;
    }

    if (count != LOCATION_COUNT)
    {
        throw FileCorruptException(path, lineNum, "Expected exactly " +
                                   StringUtils::intToString(LOCATION_COUNT) + " locations");
    }

    return count;
}

// Reads and validates character trait mappings from characters.txt.
int DataLoader::loadCharacters(const std::string& path, CharacterRecord out[], int capacity)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw FileNotFoundException(path);
    }

    std::string heading;
    if (!std::getline(file, heading))
    {
        throw FileCorruptException(path, 1, "Empty characters file");
    }
    if (StringUtils::trim(heading) != "Name|Role|Nationality|W2Pos|Cook|Guard|Servant|P1|P2|PA|Child|isCulprit")
    {
        throw FileCorruptException(path, 1, "Invalid heading in characters.txt");
    }

    bool seenPos[CHARACTER_COUNT + 1] = { false };
    std::string line;
    int lineNum = 1;
    int count   = 0;

    while (std::getline(file, line))
    {
        ++lineNum;
        std::string trimmed = StringUtils::trim(line);
        if (trimmed.empty()) continue;

        std::string parts[13];
        int partCount = StringUtils::splitByDelimiter(trimmed, '|', parts, 13);
        if (partCount != 12)
        {
            throw FileCorruptException(path, lineNum, "Expected exactly 12 fields in characters.txt");
        }

        std::string name        = StringUtils::trim(parts[0]);
        std::string role        = StringUtils::trim(parts[1]);
        std::string nationality = StringUtils::trim(parts[2]);

        int pos = 0;
        try { pos = StringUtils::parseInt(parts[3]); }
        catch (const InvalidInputException&)
        {
            throw FileCorruptException(path, lineNum, "Non-numeric W2Pos in characters.txt: " + parts[3]);
        }

        if (pos < 1 || pos > CHARACTER_COUNT)
        {
            throw FileCorruptException(path, lineNum, "W2Pos out of range (1-" +
                                       StringUtils::intToString(CHARACTER_COUNT) + "): " + parts[3]);
        }
        if (seenPos[pos])
        {
            throw FileCorruptException(path, lineNum, "Duplicate W2Pos: " + StringUtils::intToString(pos));
        }
        seenPos[pos] = true;

        if (count >= capacity)
        {
            throw FileCorruptException(path, lineNum, "Exceeded character array capacity");
        }

        out[count].name        = name;
        out[count].role        = role;
        out[count].nationality = nationality;
        out[count].word2Pos    = pos;

        for (int i = 0; i < ROLE_COUNT; ++i)
        {
            std::string let = StringUtils::trim(parts[4 + i]);
            if (let.length() != 1 || let[0] < 'A' || let[0] > 'Z')
            {
                throw FileCorruptException(path, lineNum, "Invalid trait letter '" + let +
                                           "' at column " + StringUtils::intToString(i));
            }
            out[count].letters[i] = let[0];
        }

        std::string culpritFlag = StringUtils::trim(parts[11]);
        if (culpritFlag != "0")
        {
            throw FileCorruptException(path, lineNum, "isCulprit must be '0' in characters.txt; found: " + culpritFlag);
        }
        out[count].isCulprit = false;

        ++count;
    }

    if (count != CHARACTER_COUNT)
    {
        throw FileCorruptException(path, lineNum, "Expected exactly " +
                                   StringUtils::intToString(CHARACTER_COUNT) + " characters");
    }

    return count;
}

// Reads the complete formatted about screen content from about.txt.
std::string DataLoader::loadAboutText(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw FileNotFoundException(path);
    }

    std::string text = "";
    std::string line;
    while (std::getline(file, line))
    {
        text += line + "\n";
    }

    if (StringUtils::trim(text).empty())
    {
        throw FileCorruptException(path, 0, "about.txt is empty");
    }

    return text;
}

// Verifies that location clue letters and character trait letters
// reconstruct Word 1 and Word 2 for every suspect role with 100% fidelity.
void DataLoader::validateConsistency(const ClueProfile profiles[],
                                     const LocationRecord locations[],
                                     const CharacterRecord characters[])
{
    for (int pIdx = 0; pIdx < ROLE_COUNT; ++pIdx)
    {
        const ClueProfile& prof = profiles[pIdx];
        int col = roleToColumn(prof.culpritRole);

        // Word 1: reconstructed from locations in W1Pos order (1-7)
        char w1Letters[WORD1_LENGTH + 1];
        w1Letters[WORD1_LENGTH] = '\0';
        for (int i = 0; i < LOCATION_COUNT; ++i)
        {
            int pos = locations[i].word1Pos;
            if (pos >= 1 && pos <= WORD1_LENGTH)
            {
                w1Letters[pos - 1] = locations[i].letters[col];
            }
        }

        std::string builtW1(w1Letters);
        if (builtW1 != prof.word1)
        {
            throw FileCorruptException(FILE_LOCATIONS, 0,
                "Inconsistent Word 1 for role " + prof.culpritRole +
                ": locations spell '" + builtW1 + "', but clue profile specifies '" + prof.word1 + "'");
        }

        // Word 2: reconstructed from characters in W2Pos order (1-8)
        char w2Letters[WORD2_LENGTH + 1];
        w2Letters[WORD2_LENGTH] = '\0';
        for (int i = 0; i < CHARACTER_COUNT; ++i)
        {
            int pos = characters[i].word2Pos;
            if (pos >= 1 && pos <= WORD2_LENGTH)
            {
                w2Letters[pos - 1] = characters[i].letters[col];
            }
        }

        std::string builtW2(w2Letters);
        if (builtW2 != prof.word2)
        {
            throw FileCorruptException(FILE_CHARACTERS, 0,
                "Inconsistent Word 2 for role " + prof.culpritRole +
                ": characters spell '" + builtW2 + "', but clue profile specifies '" + prof.word2 + "'");
        }
    }
}


// ============================================================
// SECTION 5 -- CHARACTER HIERARCHY IMPLEMENTATIONS (Commit 3)
// ============================================================

// Static live-object counter initialization
int Character::liveCount = 0;

// Default nationality for Detective
const std::string Detective::DEFAULT_NATIONALITY = "Malay";

// Character Base Implementation
Character::Character(const std::string& name, const std::string& role, const std::string& nat)
    : characterName(name), characterRole(role), nationality(nat), isCulprit(false)
{
    ++liveCount;
}

Character::Character(const Character& other)
    : characterName(other.characterName),
      characterRole(other.characterRole),
      nationality(other.nationality),
      isCulprit(other.isCulprit)
{
    ++liveCount;
}

Character& Character::operator=(const Character& other)
{
    if (this != &other)
    {
        characterName = other.characterName;
        characterRole = other.characterRole;
        nationality   = other.nationality;
        isCulprit     = other.isCulprit;
    }
    return *this;
}

Character::~Character()
{
    --liveCount;
}

std::string Character::getName() const
{
    return characterName;
}

std::string Character::getRole() const
{
    return characterRole;
}

std::string Character::getNationality() const
{
    return nationality;
}

bool Character::getIsCulprit() const
{
    return isCulprit;
}

int Character::getMinimumCredibility() const
{
    return 0;
}

bool Character::hasClue() const
{
    return false;
}

Clue Character::getClue() const
{
    throw InvalidInputException("Character does not possess a clue: " + characterName);
}

int Character::getLiveCount()
{
    return liveCount;
}


// Victim Implementation
Victim::Victim(const std::string& name, const std::string& nat, char letter, int pos)
    : Character(name, "Victim", nat),
      letterClueChar(letter),
      boardPosChar(pos),
      clueRowChar(WordRow::Word2)
{
}

Victim::~Victim()
{
}

char Victim::getLetterClue() const
{
    return letterClueChar;
}

int Victim::getBoardPos() const
{
    return boardPosChar;
}

WordRow Victim::getClueRow() const
{
    return clueRowChar;
}

void Victim::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [Victim] - Nationality: " << getNationality();
}

bool Victim::hasClue() const
{
    return true;
}

Clue Victim::getClue() const
{
    Clue c;
    c.clueLetter   = letterClueChar;
    c.cluePosition = boardPosChar;
    c.clueRow      = clueRowChar;
    return c;
}

int Victim::getMinimumCredibility() const
{
    return 80;
}


// Detective Implementation
Detective::Detective(const std::string& name, int startingCredibility)
    : Character(name, "Detective", DEFAULT_NATIONALITY),
      credibility(startingCredibility),
      turns(0),
      locationRack(7),
      characterRack(8)
{
    solvedWords = new std::string[2];
    solvedWords[0] = "";
    solvedWords[1] = "";
}

Detective::Detective(const Detective& other)
    : Character(other),
      credibility(other.credibility),
      turns(other.turns),
      locationRack(other.locationRack),
      characterRack(other.characterRack)
{
    solvedWords = new std::string[2];
    solvedWords[0] = other.solvedWords[0];
    solvedWords[1] = other.solvedWords[1];
}

Detective& Detective::operator=(const Detective& other)
{
    if (this != &other)
    {
        Character::operator=(other);
        credibility    = other.credibility;
        turns          = other.turns;
        solvedWords[0] = other.solvedWords[0];
        solvedWords[1] = other.solvedWords[1];
        locationRack   = other.locationRack;
        characterRack  = other.characterRack;
    }
    return *this;
}

Detective::~Detective()
{
    delete[] solvedWords;
    solvedWords = nullptr;
}

int Detective::getCredibility() const
{
    return credibility;
}

void Detective::reduceCredibility(int amount)
{
    if (amount <= 0)
    {
        return;
    }
    if (credibility <= amount)
    {
        credibility = 0;
    }
    else
    {
        credibility -= amount;
    }
}

void Detective::addTurn()
{
    ++turns;
}

int Detective::getTurnsUsed() const
{
    return turns;
}

void Detective::setSolvedWord(WordRow row, const std::string& word)
{
    solvedWords[static_cast<int>(row)] = word;
}

std::string Detective::getSolvedWord(WordRow row) const
{
    return solvedWords[static_cast<int>(row)];
}

bool Detective::isRowSolved(WordRow row) const
{
    return !solvedWords[static_cast<int>(row)].empty();
}

bool Detective::areBothSolved() const
{
    return isRowSolved(WordRow::Word1) && isRowSolved(WordRow::Word2);
}

// Composition: routing clues to composed locationRack and characterRack
void Detective::collectClue(const Clue& clue)
{
    if (clue.clueRow == WordRow::Word1)
    {
        locationRack.addTile(clue);
    }
    else
    {
        characterRack.addTile(clue);
    }
}

const TileRack& Detective::getRack(WordRow row) const
{
    return (row == WordRow::Word1) ? locationRack : characterRack;
}

TileRack& Detective::getRack(WordRow row)
{
    return (row == WordRow::Word1) ? locationRack : characterRack;
}

void Detective::displayRacks(std::ostream& out) const
{
    out << "Rack 1 (Locations): ";
    locationRack.display(out);
    out << "\n";
    out << "Rack 2 (People): ";
    characterRack.display(out);
    out << "\n";
}

void Detective::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [Detective] - Credibility: " << credibility
        << ", Turns: " << turns
        << ", Solved: [" << solvedWords[0] << ", " << solvedWords[1] << "]\n";
    displayRacks(out);
}


// Suspect Abstract Implementation
Suspect::Suspect(const std::string& name, const std::string& role, const std::string& nat,
                 char letter, int pos)
    : Character(name, role, nat),
      letterClueChar(letter),
      boardPosChar(pos),
      clueRowChar(WordRow::Word2)
{
}

Suspect::~Suspect()
{
}

char Suspect::getLetterClue() const
{
    return letterClueChar;
}

void Suspect::setLetterClue(char c)
{
    letterClueChar = c;
}

int Suspect::getBoardPos() const
{
    return boardPosChar;
}

void Suspect::setBoardPos(int p)
{
    boardPosChar = p;
}

WordRow Suspect::getClueRow() const
{
    return clueRowChar;
}

void Suspect::setClueRow(WordRow r)
{
    clueRowChar = r;
}

bool Suspect::hasClue() const
{
    return true;
}

Clue Suspect::getClue() const
{
    Clue c;
    c.clueLetter   = letterClueChar;
    c.cluePosition = boardPosChar;
    c.clueRow      = clueRowChar;
    return c;
}


// BusinessRelations Implementation
BusinessRelations::BusinessRelations(const std::string& name, const std::string& role,
                                     const std::string& nat, char letter, int pos)
    : Suspect(name, role, nat, letter, pos)
{
}

BusinessRelations::~BusinessRelations()
{
}

void BusinessRelations::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [" << getRole() << "] - Nationality: " << getNationality();
}


// CloseRelations Implementation
CloseRelations::CloseRelations(const std::string& name, const std::string& role,
                               const std::string& nat, char letter, int pos)
    : Suspect(name, role, nat, letter, pos)
{
}

CloseRelations::~CloseRelations()
{
}

void CloseRelations::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [" << getRole() << "] - Nationality: " << getNationality();
}


// HouseHelp Implementation
HouseHelp::HouseHelp(const std::string& name, const std::string& role, const std::string& nat,
                     char letter, int pos)
    : Suspect(name, role, nat, letter, pos)
{
}

HouseHelp::~HouseHelp()
{
}

int HouseHelp::getMinimumCredibility() const
{
    return 0;
}

void HouseHelp::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [" << getRole() << "] - Nationality: " << getNationality();
}


// Cook Implementation
Cook::Cook(const std::string& name, const std::string& nat, char letter, int pos)
    : HouseHelp(name, "Cook", nat, letter, pos)
{
}

Cook::~Cook()
{
}

void Cook::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [Cook] - Nationality: " << getNationality();
}


// Guard Implementation
Guard::Guard(const std::string& name, const std::string& nat, char letter, int pos)
    : HouseHelp(name, "Guard", nat, letter, pos)
{
}

Guard::~Guard()
{
}

void Guard::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [Guard] - Nationality: " << getNationality();
}


// Servant Implementation
Servant::Servant(const std::string& name, const std::string& nat, char letter, int pos)
    : HouseHelp(name, "Servant", nat, letter, pos)
{
}

Servant::~Servant()
{
}

void Servant::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [Servant] - Nationality: " << getNationality();
}


// Partner Implementation
Partner::Partner(const std::string& name, const std::string& nat, int number, char letter, int pos)
    : Suspect(name, (number == 1 ? "Partner 1" : "Partner 2"), nat, letter, pos),
      BusinessRelations(name, (number == 1 ? "Partner 1" : "Partner 2"), nat, letter, pos),
      partnerNumber(number)
{
}

Partner::~Partner()
{
}

int Partner::getPartnerNumber() const
{
    return partnerNumber;
}

int Partner::getMinimumCredibility() const
{
    return 50;
}

void Partner::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [" << getRole() << "] - Nationality: " << getNationality();
}


// PA Implementation (resolves diamond inheritance from BusinessRelations and CloseRelations)
PA::PA(const std::string& name, const std::string& nat, char letter, int pos)
    : Suspect(name, "PA", nat, letter, pos),
      BusinessRelations(name, "PA", nat, letter, pos),
      CloseRelations(name, "PA", nat, letter, pos)
{
}

PA::~PA()
{
}

int PA::getMinimumCredibility() const
{
    return 35;
}

void PA::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [PA] - Nationality: " << getNationality();
}


// Child Implementation
Child::Child(const std::string& name, const std::string& nat, char letter, int pos, int a)
    : Suspect(name, "Child", nat, letter, pos),
      CloseRelations(name, "Child", nat, letter, pos),
      age(a)
{
    if (a < 15 || a > 19)
    {
        throw InvalidInputException("Child age must be between 15 and 19; provided: " +
                                    StringUtils::intToString(a));
    }
}

Child::~Child()
{
}

int Child::getAge() const
{
    return age;
}

int Child::getMinimumCredibility() const
{
    return 75;
}

void Child::display(std::ostream& out) const
{
    out << StringUtils::formatPersonName(getName())
        << " [Child, Age: " << age << "] - Nationality: " << getNationality();
}


// CharacterFactory Implementation
Character* CharacterFactory::create(const CharacterRecord& record, int culpritColumn)
{
    if (culpritColumn < 0 || culpritColumn >= DataLoader::ROLE_COUNT)
    {
        throw InvalidInputException("Invalid culprit column index: " +
                                    StringUtils::intToString(culpritColumn));
    }

    char letter = record.letters[culpritColumn];
    int pos     = record.word2Pos;
    std::string role = StringUtils::trim(record.role);

    if (role == "Victim")
    {
        return new Victim(record.name, record.nationality, letter, pos);
    }
    else if (role == "Cook")
    {
        return new Cook(record.name, record.nationality, letter, pos);
    }
    else if (role == "Guard")
    {
        return new Guard(record.name, record.nationality, letter, pos);
    }
    else if (role == "Servant")
    {
        return new Servant(record.name, record.nationality, letter, pos);
    }
    else if (role == "Partner 1" || role == "Partner_1")
    {
        return new Partner(record.name, record.nationality, 1, letter, pos);
    }
    else if (role == "Partner 2" || role == "Partner_2")
    {
        return new Partner(record.name, record.nationality, 2, letter, pos);
    }
    else if (role == "PA")
    {
        return new PA(record.name, record.nationality, letter, pos);
    }
    else if (role == "Child")
    {
        return new Child(record.name, record.nationality, letter, pos);
    }
    else
    {
        throw InvalidInputException("Unknown character role: " + record.role);
    }
}


// ============================================================
// SECTION 6 -- BOARD CLASSES IMPLEMENTATIONS (Commit 4 Raw DMA)
// All comments in this section use single-line style (//) to avoid
// nested block comment syntax issues when refactoring in Commit 6.
// ============================================================

// Stream insertion operators
std::ostream& operator<<(std::ostream& out, const Tile& tile)
{
    out << "[" << tile.tileLetter << "]";
    return out;
}

std::ostream& operator<<(std::ostream& out, const Location& loc)
{
    out << loc.locName << " (Pos: " << loc.boardPosLoc << ", Letter: " << loc.letterClueLoc << ")";
    return out;
}

std::ostream& operator<<(std::ostream& out, const TileRack& rack)
{
    rack.display(out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const Scrabble& scrabble)
{
    scrabble.display(out);
    return out;
}

// TileRack Implementation using raw dynamic memory
TileRack::TileRack(int cap)
    : capacity(cap), count(0)
{
    if (capacity < 1)
    {
        throw InvalidInputException("TileRack capacity must be at least 1; requested: " +
                                    StringUtils::intToString(cap));
    }
    tiles = new Tile[capacity];
    positions = new int[capacity];
    for (int i = 0; i < capacity; ++i)
    {
        tiles[i] = Tile{};
        positions[i] = 0;
    }
}

TileRack::TileRack(const TileRack& other)
    : capacity(other.capacity), count(other.count)
{
    tiles = new Tile[capacity];
    positions = new int[capacity];
    for (int i = 0; i < capacity; ++i)
    {
        tiles[i] = other.tiles[i];
        positions[i] = other.positions[i];
    }
}

TileRack& TileRack::operator=(const TileRack& other)
{
    if (this != &other)
    {
        delete[] tiles;
        delete[] positions;

        capacity  = other.capacity;
        count     = other.count;
        tiles     = new Tile[capacity];
        positions = new int[capacity];
        for (int i = 0; i < capacity; ++i)
        {
            tiles[i] = other.tiles[i];
            positions[i] = other.positions[i];
        }
    }
    return *this;
}

TileRack::~TileRack()
{
    delete[] tiles;
    delete[] positions;
    tiles = nullptr;
    positions = nullptr;
}

void TileRack::addTile(const Clue& clue)
{
    if (isFull())
    {
        throw RackFullException(capacity);
    }
    if (clue.clueLetter < 'A' || clue.clueLetter > 'Z')
    {
        throw InvalidLetterException(clue.clueLetter, clue.cluePosition);
    }
    if (clue.cluePosition < 1)
    {
        throw InvalidPositionException(clue.cluePosition, 8);
    }
    if (hasPosition(clue.cluePosition))
    {
        throw SlotFilledException(clue.cluePosition);
    }

    tiles[count].tileLetter = clue.clueLetter;
    tiles[count].filled     = false;
    positions[count]        = clue.cluePosition;
    ++count;
}

void TileRack::removeTile(int index)
{
    if (isEmpty())
    {
        throw RackEmptyException();
    }
    if (index < 0 || index >= count)
    {
        throw InvalidPositionException(index, count - 1);
    }

    for (int i = index; i < count - 1; ++i)
    {
        tiles[i]     = tiles[i + 1];
        positions[i] = positions[i + 1];
    }
    --count;
    tiles[count]     = Tile{};
    positions[count] = 0;
}

void TileRack::removeLast()
{
    if (isEmpty())
    {
        throw RackEmptyException();
    }
    removeTile(count - 1);
}

void TileRack::clear()
{
    count = 0;
    for (int i = 0; i < capacity; ++i)
    {
        tiles[i] = Tile{};
        positions[i] = 0;
    }
}

int TileRack::getCount() const
{
    return count;
}

int TileRack::getCapacity() const
{
    return capacity;
}

bool TileRack::isFull() const
{
    return count >= capacity;
}

bool TileRack::isEmpty() const
{
    return count == 0;
}

char TileRack::getLetterAt(int index) const
{
    if (index < 0 || index >= count)
    {
        throw InvalidPositionException(index, count - 1);
    }
    return tiles[index].tileLetter;
}

int TileRack::getPositionAt(int index) const
{
    if (index < 0 || index >= count)
    {
        throw InvalidPositionException(index, count - 1);
    }
    return positions[index];
}

bool TileRack::hasPosition(int position) const
{
    for (int i = 0; i < count; ++i)
    {
        if (positions[i] == position)
        {
            return true;
        }
    }
    return false;
}

void TileRack::markPlaced(int position)
{
    for (int i = 0; i < count; ++i)
    {
        if (positions[i] == position)
        {
            tiles[i].filled = true;
            return;
        }
    }
    throw InvalidPositionException(position, 8);
}

bool TileRack::isPlaced(int position) const
{
    for (int i = 0; i < count; ++i)
    {
        if (positions[i] == position)
        {
            return tiles[i].filled;
        }
    }
    return false;
}

void TileRack::display(std::ostream& out) const
{
    if (isEmpty())
    {
        out << "Rack: (empty)";
        return;
    }
    out << "Rack: ";
    for (int i = 0; i < count; ++i)
    {
        out << "[" << tiles[i].tileLetter << "@" << positions[i];
        if (tiles[i].filled)
        {
            out << "*";
        }
        out << "] ";
    }
}

// Scrabble Jagged Board Implementation using raw dynamic memory
Scrabble::Scrabble()
    : targetsLoaded(false)
{
    rowLength[0] = 7;
    rowLength[1] = 8;
    targetWord[0] = "";
    targetWord[1] = "";

    grid = new Tile*[2];
    grid[0] = new Tile[7];
    grid[1] = new Tile[8];
}

Scrabble::Scrabble(const std::string& word1, const std::string& word2)
    : targetsLoaded(false)
{
    rowLength[0] = 7;
    rowLength[1] = 8;
    targetWord[0] = "";
    targetWord[1] = "";

    grid = new Tile*[2];
    grid[0] = new Tile[7];
    grid[1] = new Tile[8];

    loadTargetWords(word1, word2);
}

Scrabble::Scrabble(const Scrabble& other)
    : targetsLoaded(other.targetsLoaded)
{
    rowLength[0]  = other.rowLength[0];
    rowLength[1]  = other.rowLength[1];
    targetWord[0] = other.targetWord[0];
    targetWord[1] = other.targetWord[1];

    grid = new Tile*[2];
    grid[0] = new Tile[rowLength[0]];
    grid[1] = new Tile[rowLength[1]];

    for (int i = 0; i < rowLength[0]; ++i)
    {
        grid[0][i] = other.grid[0][i];
    }
    for (int i = 0; i < rowLength[1]; ++i)
    {
        grid[1][i] = other.grid[1][i];
    }
}

Scrabble& Scrabble::operator=(const Scrabble& other)
{
    if (this != &other)
    {
        delete[] grid[0];
        delete[] grid[1];
        delete[] grid;

        rowLength[0]  = other.rowLength[0];
        rowLength[1]  = other.rowLength[1];
        targetWord[0] = other.targetWord[0];
        targetWord[1] = other.targetWord[1];
        targetsLoaded = other.targetsLoaded;

        grid = new Tile*[2];
        grid[0] = new Tile[rowLength[0]];
        grid[1] = new Tile[rowLength[1]];

        for (int i = 0; i < rowLength[0]; ++i)
        {
            grid[0][i] = other.grid[0][i];
        }
        for (int i = 0; i < rowLength[1]; ++i)
        {
            grid[1][i] = other.grid[1][i];
        }
    }
    return *this;
}

Scrabble::~Scrabble()
{
    delete[] grid[0];
    delete[] grid[1];
    delete[] grid;
    grid = nullptr;
}

void Scrabble::loadTargetWords(const std::string& word1, const std::string& word2)
{
    if (word1.length() != 7 || !StringUtils::isAllLetters(word1))
    {
        throw InvalidWordException("Word 1 must be 7 uppercase letters: " + word1);
    }
    for (size_t i = 0; i < word1.length(); ++i)
    {
        if (word1[i] < 'A' || word1[i] > 'Z')
        {
            throw InvalidWordException("Word 1 contains non-uppercase letter: " + word1);
        }
    }

    if (word2.length() != 8 || !StringUtils::isAllLetters(word2))
    {
        throw InvalidWordException("Word 2 must be 8 uppercase letters: " + word2);
    }
    for (size_t i = 0; i < word2.length(); ++i)
    {
        if (word2[i] < 'A' || word2[i] > 'Z')
        {
            throw InvalidWordException("Word 2 contains non-uppercase letter: " + word2);
        }
    }

    targetWord[0] = word1;
    targetWord[1] = word2;
    targetsLoaded = true;
}

void Scrabble::placeLetter(const Clue& clue)
{
    if (!targetsLoaded)
    {
        throw InvalidWordException("Target words are not loaded on Scrabble board");
    }

    int r = static_cast<int>(clue.clueRow);
    if (r < 0 || r > 1)
    {
        throw InvalidPositionException(r, 1);
    }

    if (clue.cluePosition < 1 || clue.cluePosition > rowLength[r])
    {
        throw InvalidPositionException(clue.cluePosition, rowLength[r]);
    }

    int idx = clue.cluePosition - 1;
    if (grid[r][idx].filled)
    {
        throw SlotFilledException(clue.cluePosition);
    }

    if (clue.clueLetter != targetWord[r][idx])
    {
        throw InvalidLetterException(clue.clueLetter, clue.cluePosition);
    }

    grid[r][idx].tileLetter = clue.clueLetter;
    grid[r][idx].filled     = true;
}

char Scrabble::revealLetter(WordRow row, int position) const
{
    int r = static_cast<int>(row);
    if (position < 1 || position > rowLength[r])
    {
        throw InvalidPositionException(position, rowLength[r]);
    }

    int idx = position - 1;
    return grid[r][idx].filled ? grid[r][idx].tileLetter : ' ';
}

bool Scrabble::isRowComplete(WordRow row) const
{
    int r = static_cast<int>(row);
    for (int i = 0; i < rowLength[r]; ++i)
    {
        if (!grid[r][i].filled)
        {
            return false;
        }
    }
    return true;
}

bool Scrabble::isComplete() const
{
    return isRowComplete(WordRow::Word1) && isRowComplete(WordRow::Word2);
}

int Scrabble::getRowLength(WordRow row) const
{
    return rowLength[static_cast<int>(row)];
}

int Scrabble::getFilledCount(WordRow row) const
{
    int r = static_cast<int>(row);
    int filledCount = 0;
    for (int i = 0; i < rowLength[r]; ++i)
    {
        if (grid[r][i].filled)
        {
            ++filledCount;
        }
    }
    return filledCount;
}

std::string Scrabble::getRowString(WordRow row) const
{
    int r = static_cast<int>(row);
    std::string s = "";
    for (int i = 0; i < rowLength[r]; ++i)
    {
        s += (grid[r][i].filled ? grid[r][i].tileLetter : '_');
    }
    return s;
}

std::string Scrabble::getTargetWord(WordRow row) const
{
    return targetWord[static_cast<int>(row)];
}

void Scrabble::verifyWord(WordRow row, const std::string& typedWord) const
{
    int r = static_cast<int>(row);
    if (!isRowComplete(row))
    {
        throw WordIncompleteException(r == 0 ? "Word 1" : "Word 2");
    }

    if (!StringUtils::equalsIgnoreCase(typedWord, targetWord[r]))
    {
        throw InvalidWordException(typedWord);
    }
}

void Scrabble::display(std::ostream& out) const
{
    out << "Word 1: ";
    for (int i = 0; i < rowLength[0]; ++i)
    {
        if (grid[0][i].filled)
        {
            out << "[" << grid[0][i].tileLetter << "] ";
        }
        else
        {
            out << "[ ] ";
        }
    }
    if (isRowComplete(WordRow::Word1))
    {
        out << "COMPLETE";
    }

    out << "\nWord 2: ";
    for (int i = 0; i < rowLength[1]; ++i)
    {
        if (grid[1][i].filled)
        {
            out << "[" << grid[1][i].tileLetter << "] ";
        }
        else
        {
            out << "[ ] ";
        }
    }
    if (isRowComplete(WordRow::Word2))
    {
        out << "COMPLETE";
    }
    out << "\n";
}


// ============================================================
// SECTION 7 -- COTTAGE IMPLEMENTATION (Commit 5 Raw DMA)
// UML Design Note (Decision R16): In the class diagram, Cottage and Location
// are related via aggregation (open diamond at Cottage, arrow to Location)
// because a Location struct is a general domain concept representing rooms
// that could logically exist independently of a Cottage container. At the C++
// implementation level, Cottage owns the lifetime of its dynamic Location
// instances allocated via new and released via delete.
// ============================================================

Cottage::Cottage()
    : locations(nullptr),
      locationCount(0)
{
}

Cottage::Cottage(const LocationRecord records[], int count, int culpritColumn)
    : locations(nullptr),
      locationCount(0)
{
    if (count < 0)
    {
        throw InvalidInputException("Location count cannot be negative");
    }
    if (culpritColumn < 0 || culpritColumn >= DataLoader::ROLE_COUNT)
    {
        throw InvalidInputException("Invalid culprit column index: " + StringUtils::intToString(culpritColumn));
    }

    if (count > 0)
    {
        locations = new Location*[count];
        locationCount = count;
        for (int i = 0; i < count; ++i)
        {
            locations[i] = new Location;
            locations[i]->locName       = records[i].placeName;
            locations[i]->letterClueLoc = records[i].letters[culpritColumn];
            locations[i]->boardPosLoc   = records[i].word1Pos;
            locations[i]->locRow        = WordRow::Word1;
            locations[i]->locSearchFlag = false;
        }
    }
}

Cottage::Cottage(const Cottage& other)
    : locations(nullptr),
      locationCount(other.locationCount)
{
    if (other.locationCount > 0 && other.locations != nullptr)
    {
        locations = new Location*[other.locationCount];
        for (int i = 0; i < other.locationCount; ++i)
        {
            locations[i] = new Location;
            *(locations[i]) = *(other.locations[i]);
        }
    }
    else
    {
        locations = nullptr;
        locationCount = 0;
    }
}

Cottage& Cottage::operator=(const Cottage& other)
{
    if (this != &other)
    {
        // Delete current dynamically allocated locations
        if (locations != nullptr)
        {
            for (int i = 0; i < locationCount; ++i)
            {
                delete locations[i];
            }
            delete[] locations;
            locations = nullptr;
        }
        locationCount = 0;

        // Perform deep copy from other
        if (other.locationCount > 0 && other.locations != nullptr)
        {
            locations = new Location*[other.locationCount];
            locationCount = other.locationCount;
            for (int i = 0; i < other.locationCount; ++i)
            {
                locations[i] = new Location;
                *(locations[i]) = *(other.locations[i]);
            }
        }
    }
    return *this;
}

Cottage::~Cottage()
{
    if (locations != nullptr)
    {
        for (int i = 0; i < locationCount; ++i)
        {
            delete locations[i];
        }
        delete[] locations;
        locations = nullptr;
    }
    locationCount = 0;
}

int Cottage::getLocationCount() const
{
    return locationCount;
}

bool Cottage::isLocationSearched(int number) const
{
    if (number < 1 || number > locationCount)
    {
        throw InvalidPositionException(number, locationCount);
    }
    return locations[number - 1]->locSearchFlag;
}

Clue Cottage::searchLocation(int number)
{
    if (number < 1 || number > locationCount)
    {
        throw InvalidPositionException(number, locationCount);
    }
    locations[number - 1]->locSearchFlag = true;

    Clue c;
    c.clueLetter   = locations[number - 1]->letterClueLoc;
    c.cluePosition = locations[number - 1]->boardPosLoc;
    c.clueRow      = locations[number - 1]->locRow;
    return c;
}

Clue Cottage::getLocationClue(int number) const
{
    if (number < 1 || number > locationCount)
    {
        throw InvalidPositionException(number, locationCount);
    }
    Clue c;
    c.clueLetter   = locations[number - 1]->letterClueLoc;
    c.cluePosition = locations[number - 1]->boardPosLoc;
    c.clueRow      = locations[number - 1]->locRow;
    return c;
}

std::string Cottage::getLocationName(int number) const
{
    if (number < 1 || number > locationCount)
    {
        throw InvalidPositionException(number, locationCount);
    }
    return StringUtils::underscoreToSpace(locations[number - 1]->locName);
}

int Cottage::getSearchedCount() const
{
    int searched = 0;
    for (int i = 0; i < locationCount; ++i)
    {
        if (locations[i] != nullptr && locations[i]->locSearchFlag)
        {
            ++searched;
        }
    }
    return searched;
}

bool Cottage::allSearched() const
{
    return (locationCount > 0 && getSearchedCount() == locationCount);
}

void Cottage::displayLocations(std::ostream& out) const
{
    for (int i = 1; i <= locationCount; ++i)
    {
        out << "[" << i << "] " << getLocationName(i) << " - "
            << (isLocationSearched(i) ? "Searched" : "NOT Searched") << "\n";
    }
}

std::ostream& operator<<(std::ostream& out, const Cottage& cottage)
{
    cottage.displayLocations(out);
    return out;
}
