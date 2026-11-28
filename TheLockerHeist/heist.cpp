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
// SECTION 6B -- SCOREBOARD IMPLEMENTATION (Commit 7, Module 6)
// Manages persistent match results in leader/scoreBoard.txt.
// GameEngine is ASSOCIATED with ScoreBoard (the engine holds a pointer to it,
// while main() owns the lifetime of the ScoreBoard instance).
// ============================================================

ScoreBoard::ScoreBoard(const std::string& path)
    : filePath(path)
{
}

std::string ScoreBoard::getFilePath() const
{
    return filePath;
}

void ScoreBoard::addScore(const std::string& detectiveName,
                          bool won,
                          int credibility,
                          int turnsLeft,
                          const std::string& culpritRole,
                          const std::string& date)
{
    // Validate detective name: must be non-empty, no space, no '|'
    if (detectiveName.empty())
    {
        throw InvalidInputException("Detective name cannot be empty");
    }
    for (size_t i = 0; i < detectiveName.length(); ++i)
    {
        if (detectiveName[i] == ' ' || detectiveName[i] == '|')
        {
            throw InvalidInputException("Detective name cannot contain spaces or '|': " + detectiveName);
        }
    }

    bool fileExisted = false;
    bool endsWithNewline = true;

    // Check if the file already exists and inspect heading and trailing newline
    {
        std::ifstream checkIn(filePath, std::ios::binary);
        if (checkIn.is_open())
        {
            fileExisted = true;
            std::string heading;
            if (std::getline(checkIn, heading))
            {
                if (StringUtils::trim(heading) != "Detective|Result|Credibility|Turns_Left|Culprit|Date")
                {
                    throw FileCorruptException(filePath, 1,
                        "Invalid heading in scoreboard file; expected 'Detective|Result|Credibility|Turns_Left|Culprit|Date'");
                }
            }

            // Check if last byte is a newline to prevent line merging
            checkIn.seekg(0, std::ios::end);
            std::streampos sz = checkIn.tellg();
            if (sz > 0)
            {
                checkIn.seekg(-1, std::ios::end);
                char lastChar = 0;
                checkIn.get(lastChar);
                if (lastChar != '\n')
                {
                    endsWithNewline = false;
                }
            }
            checkIn.close();
        }
    }

    // If file does not exist, create it with heading row in existing folder
    if (!fileExisted)
    {
        std::ofstream newOut(filePath);
        if (!newOut.is_open())
        {
            // Folder does not exist or invalid path
            throw FileNotFoundException(filePath);
        }
        newOut << "Detective|Result|Credibility|Turns_Left|Culprit|Date\n";
        newOut.close();
        endsWithNewline = true;
    }

    // Format output fields according to specification
    std::string resultStr  = won ? "WIN" : "LOSE";
    std::string credStr    = StringUtils::intToString(credibility);
    std::string turnsStr   = StringUtils::padWithZeros(turnsLeft, 2);
    std::string culpritStr = StringUtils::spaceToUnderscore(culpritRole);
    std::string dateStr    = date.empty() ? StringUtils::currentDateDDMMYY() : date;

    // Append record to file
    std::ofstream appendOut(filePath, std::ios::app);
    if (!appendOut.is_open())
    {
        throw FileNotFoundException(filePath);
    }

    if (!endsWithNewline)
    {
        appendOut << "\n";
    }

    appendOut << detectiveName << "|"
              << resultStr << "|"
              << credStr << "|"
              << turnsStr << "|"
              << culpritStr << "|"
              << dateStr << "\n";
    appendOut.close();
}

int ScoreBoard::getEntryCount() const
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return 0;
    }

    std::string heading;
    if (!std::getline(file, heading))
    {
        return 0;
    }

    if (StringUtils::trim(heading) != "Detective|Result|Credibility|Turns_Left|Culprit|Date")
    {
        throw FileCorruptException(filePath, 1, "Invalid heading in scoreboard file");
    }

    int count = 0;
    std::string line;
    int lineNum = 1;
    while (std::getline(file, line))
    {
        ++lineNum;
        std::string trimmed = StringUtils::trim(line);
        if (trimmed.empty()) continue;

        std::string parts[6];
        int n = StringUtils::splitByDelimiter(trimmed, '|', parts, 6);
        if (n != 6)
        {
            throw FileCorruptException(filePath, lineNum,
                "Expected 6 fields in scoreboard row; found " + StringUtils::intToString(n));
        }

        if (parts[1] != "WIN" && parts[1] != "LOSE")
        {
            throw FileCorruptException(filePath, lineNum, "Invalid result; expected WIN or LOSE: " + parts[1]);
        }

        try { StringUtils::parseInt(parts[2]); }
        catch (const InvalidInputException&)
        {
            throw FileCorruptException(filePath, lineNum, "Non-numeric credibility: " + parts[2]);
        }

        try { StringUtils::parseInt(parts[3]); }
        catch (const InvalidInputException&)
        {
            throw FileCorruptException(filePath, lineNum, "Non-numeric turns: " + parts[3]);
        }

        if (parts[5].length() != 8 || parts[5][2] != '-' || parts[5][5] != '-')
        {
            throw FileCorruptException(filePath, lineNum, "Invalid date format; expected DD-MM-YY: " + parts[5]);
        }

        ++count;
    }

    return count;
}

void ScoreBoard::displayAll(std::ostream& out) const
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw FileNotFoundException(filePath);
    }

    std::string heading;
    if (!std::getline(file, heading))
    {
        out << "No games recorded yet.\n";
        return;
    }

    if (StringUtils::trim(heading) != "Detective|Result|Credibility|Turns_Left|Culprit|Date")
    {
        throw FileCorruptException(filePath, 1, "Invalid heading in scoreboard file");
    }

    struct ScoreRow
    {
        std::string detective;
        std::string result;
        std::string credibility;
        std::string turnsLeft;
        std::string culprit;
        std::string date;
    };

    DynamicArray<ScoreRow> rows(8);
    std::string line;
    int lineNum = 1;

    while (std::getline(file, line))
    {
        ++lineNum;
        std::string trimmed = StringUtils::trim(line);
        if (trimmed.empty()) continue;

        std::string parts[6];
        int n = StringUtils::splitByDelimiter(trimmed, '|', parts, 6);
        if (n != 6)
        {
            throw FileCorruptException(filePath, lineNum,
                "Expected 6 fields in scoreboard row; found " + StringUtils::intToString(n));
        }

        if (parts[1] != "WIN" && parts[1] != "LOSE")
        {
            throw FileCorruptException(filePath, lineNum, "Invalid result; expected WIN or LOSE: " + parts[1]);
        }

        try { StringUtils::parseInt(parts[2]); }
        catch (const InvalidInputException&)
        {
            throw FileCorruptException(filePath, lineNum, "Non-numeric credibility: " + parts[2]);
        }

        try { StringUtils::parseInt(parts[3]); }
        catch (const InvalidInputException&)
        {
            throw FileCorruptException(filePath, lineNum, "Non-numeric turns: " + parts[3]);
        }

        if (parts[5].length() != 8 || parts[5][2] != '-' || parts[5][5] != '-')
        {
            throw FileCorruptException(filePath, lineNum, "Invalid date format; expected DD-MM-YY: " + parts[5]);
        }

        ScoreRow row;
        row.detective   = parts[0];
        row.result      = parts[1];
        row.credibility = parts[2];
        row.turnsLeft   = parts[3];
        row.culprit     = StringUtils::underscoreToSpace(parts[4]); // underscores shown as spaces
        row.date        = parts[5];
        rows.push(row);
    }

    if (rows.isEmpty())
    {
        out << "No games recorded yet.\n";
        return;
    }

    int widthDet = 16;
    int widthRes = 8;
    int widthCrd = 13;
    int widthTrn = 12;
    int widthCul = 14;
    int widthDat = 10;

    for (int i = 0; i < rows.getSize(); ++i)
    {
        if (static_cast<int>(rows[i].detective.length()) + 2 > widthDet)
        {
            widthDet = static_cast<int>(rows[i].detective.length()) + 2;
        }
        if (static_cast<int>(rows[i].culprit.length()) + 2 > widthCul)
        {
            widthCul = static_cast<int>(rows[i].culprit.length()) + 2;
        }
    }

    auto printBorder = [&]()
    {
        out << "+-";
        for (int i = 0; i < widthDet; ++i) out << "-";
        out << "-+-";
        for (int i = 0; i < widthRes; ++i) out << "-";
        out << "-+-";
        for (int i = 0; i < widthCrd; ++i) out << "-";
        out << "-+-";
        for (int i = 0; i < widthTrn; ++i) out << "-";
        out << "-+-";
        for (int i = 0; i < widthCul; ++i) out << "-";
        out << "-+-";
        for (int i = 0; i < widthDat; ++i) out << "-";
        out << "-+\n";
    };

    auto padRight = [](const std::string& str, int width) -> std::string
    {
        std::string res = str;
        while (static_cast<int>(res.length()) < width)
        {
            res += ' ';
        }
        return res;
    };

    printBorder();
    out << "| " << padRight("Detective", widthDet)
        << " | " << padRight("Result", widthRes)
        << " | " << padRight("Credibility", widthCrd)
        << " | " << padRight("Turns Left", widthTrn)
        << " | " << padRight("Culprit", widthCul)
        << " | " << padRight("Date", widthDat)
        << " |\n";
    printBorder();

    for (int i = 0; i < rows.getSize(); ++i)
    {
        out << "| " << padRight(rows[i].detective, widthDet)
            << " | " << padRight(rows[i].result, widthRes)
            << " | " << padRight(rows[i].credibility, widthCrd)
            << " | " << padRight(rows[i].turnsLeft, widthTrn)
            << " | " << padRight(rows[i].culprit, widthCul)
            << " | " << padRight(rows[i].date, widthDat)
            << " |\n";
    }

    printBorder();
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
    return MIN_CREDIBILITY_INTERVIEW;
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
    return MIN_CREDIBILITY_INTERVIEW;
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
    return MIN_CREDIBILITY_INTERVIEW;
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
    return MIN_CREDIBILITY_INTERVIEW;
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

// ============================================================
// EQUALITY OPERATORS (Commit 7, Module 11)
// ============================================================

// Tile equality operator: compares tileLetter only per project manual specifications
bool Tile::operator==(const Tile& other) const
{
    return tileLetter == other.tileLetter;
}

// Tile inequality operator
bool Tile::operator!=(const Tile& other) const
{
    return !(*this == other);
}

// Clue equality operator: compares letter, board position, and row
bool Clue::operator==(const Clue& other) const
{
    return clueLetter == other.clueLetter &&
           cluePosition == other.cluePosition &&
           clueRow == other.clueRow;
}

// Clue inequality operator
bool Clue::operator!=(const Clue& other) const
{
    return !(*this == other);
}


// ============================================================
// FRIEND STREAM OPERATORS (Commit 7, Module 11)
// ============================================================

// Stream insertion operator for Tile: prints [L] if filled or [ ] if unfilled
// Friend access allows inspecting filled and tileLetter directly in stream context
std::ostream& operator<<(std::ostream& out, const Tile& tile)
{
    if (tile.filled)
    {
        out << "[" << tile.tileLetter << "]";
    }
    else
    {
        out << "[ ]";
    }
    return out;
}

// Stream extraction operator for Tile: reads one letter and marks tile filled
// Friend access permits direct mutation of tile fields without public setter boilerplate
std::istream& operator>>(std::istream& in, Tile& tile)
{
    char c = ' ';
    if (in >> c)
    {
        tile.tileLetter = c;
        tile.filled     = true;
    }
    return in;
}

// Stream insertion operator for Location: prints room name and Searched / NOT Searched
// Never prints the hidden letterClueLoc to maintain mystery integrity
// Friend access allows inspecting locSearchFlag and locName directly
std::ostream& operator<<(std::ostream& out, const Location& loc)
{
    out << loc.locName << " (" << (loc.locSearchFlag ? "Searched" : "NOT Searched") << ")";
    return out;
}

// Stream extraction operator for Location: reads name, letter, position
// Friend access allows deserializing into all internal Location fields in one operation
std::istream& operator>>(std::istream& in, Location& loc)
{
    std::string name;
    char letter = ' ';
    int pos = 0;
    if (in >> name >> letter >> pos)
    {
        loc.locName       = name;
        loc.letterClueLoc = letter;
        loc.boardPosLoc   = pos;
        loc.locRow        = WordRow::Word1;
        loc.locSearchFlag = false;
    }
    return in;
}

// Stream insertion operator for Character: polymorphic display delegation
// Non-member operator provides standard stream syntax while delegating to virtual display()
std::ostream& operator<<(std::ostream& out, const Character& character)
{
    character.display(out);
    return out;
}

// Stream extraction operator for Character: reads one token into private characterName
// Friend access is strictly required to modify private member characterName directly
std::istream& operator>>(std::istream& in, Character& character)
{
    std::string token;
    if (in >> token)
    {
        character.characterName = token;
    }
    return in;
}

// Stream insertion operator for TileRack
std::ostream& operator<<(std::ostream& out, const TileRack& rack)
{
    rack.display(out);
    return out;
}

// Stream insertion operator for Scrabble
std::ostream& operator<<(std::ostream& out, const Scrabble& scrabble)
{
    scrabble.display(out);
    return out;
}

// Stream extraction operator for Scrabble: reads row (1 or 2), position, letter,
// builds a Clue, and calls placeLetter (which validates and may throw)
// Friend access integrates extraction directly with board member functions
std::istream& operator>>(std::istream& in, Scrabble& scrabble)
{
    int rowNum = 0;
    int pos = 0;
    char letter = ' ';
    if (in >> rowNum >> pos >> letter)
    {
        WordRow row = (rowNum == 1) ? WordRow::Word1 : (rowNum == 2 ? WordRow::Word2 : static_cast<WordRow>(rowNum));
        Clue clue{letter, pos, row};
        scrabble.placeLetter(clue);
    }
    return in;
}

// ------------------------------------------------------------
// RAW DMA VERSION (Modules 4 and 5) - kept for reference, replaced by DynamicArray<T> in Module 8
// ------------------------------------------------------------
// TileRack::TileRack(int cap)
//     : capacity(cap), count(0)
// {
//     if (capacity < 1)
//     {
//         throw InvalidInputException("TileRack capacity must be at least 1; requested: " +
//                                     StringUtils::intToString(cap));
//     }
//     tiles = new Tile[capacity];
//     positions = new int[capacity];
//     for (int i = 0; i < capacity; ++i)
//     {
//         tiles[i] = Tile{};
//         positions[i] = 0;
//     }
// }
// 
// TileRack::TileRack(const TileRack& other)
//     : capacity(other.capacity), count(other.count)
// {
//     tiles = new Tile[capacity];
//     positions = new int[capacity];
//     for (int i = 0; i < capacity; ++i)
//     {
//         tiles[i] = other.tiles[i];
//         positions[i] = other.positions[i];
//     }
// }
// 
// TileRack& TileRack::operator=(const TileRack& other)
// {
//     if (this != &other)
//     {
//         delete[] tiles;
//         delete[] positions;
// 
//         capacity  = other.capacity;
//         count     = other.count;
//         tiles     = new Tile[capacity];
//         positions = new int[capacity];
//         for (int i = 0; i < capacity; ++i)
//         {
//             tiles[i] = other.tiles[i];
//             positions[i] = other.positions[i];
//         }
//     }
//     return *this;
// }
// 
// TileRack::~TileRack()
// {
//     delete[] tiles;
//     delete[] positions;
//     tiles = nullptr;
//     positions = nullptr;
// }
// 
// void TileRack::addTile(const Clue& clue)
// {
//     if (isFull())
//     {
//         throw RackFullException(capacity);
//     }
//     if (clue.clueLetter < 'A' || clue.clueLetter > 'Z')
//     {
//         throw InvalidLetterException(clue.clueLetter, clue.cluePosition);
//     }
//     if (clue.cluePosition < 1)
//     {
//         throw InvalidPositionException(clue.cluePosition, 8);
//     }
//     if (hasPosition(clue.cluePosition))
//     {
//         throw SlotFilledException(clue.cluePosition);
//     }
// 
//     tiles[count].tileLetter = clue.clueLetter;
//     tiles[count].filled     = false;
//     positions[count]        = clue.cluePosition;
//     ++count;
// }
// 
// void TileRack::removeTile(int index)
// {
//     if (isEmpty())
//     {
//         throw RackEmptyException();
//     }
//     if (index < 0 || index >= count)
//     {
//         throw InvalidPositionException(index, count - 1);
//     }
// 
//     for (int i = index; i < count - 1; ++i)
//     {
//         tiles[i]     = tiles[i + 1];
//         positions[i] = positions[i + 1];
//     }
//     --count;
//     tiles[count]     = Tile{};
//     positions[count] = 0;
// }
// 
// void TileRack::removeLast()
// {
//     if (isEmpty())
//     {
//         throw RackEmptyException();
//     }
//     removeTile(count - 1);
// }
// 
// void TileRack::clear()
// {
//     count = 0;
//     for (int i = 0; i < capacity; ++i)
//     {
//         tiles[i] = Tile{};
//         positions[i] = 0;
//     }
// }
// 
// int TileRack::getCount() const
// {
//     return count;
// }
// 
// int TileRack::getCapacity() const
// {
//     return capacity;
// }
// 
// bool TileRack::isFull() const
// {
//     return count >= capacity;
// }
// 
// bool TileRack::isEmpty() const
// {
//     return count == 0;
// }
// 
// char TileRack::getLetterAt(int index) const
// {
//     if (index < 0 || index >= count)
//     {
//         throw InvalidPositionException(index, count - 1);
//     }
//     return tiles[index].tileLetter;
// }
// 
// int TileRack::getPositionAt(int index) const
// {
//     if (index < 0 || index >= count)
//     {
//         throw InvalidPositionException(index, count - 1);
//     }
//     return positions[index];
// }
// 
// bool TileRack::hasPosition(int position) const
// {
//     for (int i = 0; i < count; ++i)
//     {
//         if (positions[i] == position)
//         {
//             return true;
//         }
//     }
//     return false;
// }
// 
// void TileRack::markPlaced(int position)
// {
//     for (int i = 0; i < count; ++i)
//     {
//         if (positions[i] == position)
//         {
//             tiles[i].filled = true;
//             return;
//         }
//     }
//     throw InvalidPositionException(position, 8);
// }
// 
// bool TileRack::isPlaced(int position) const
// {
//     for (int i = 0; i < count; ++i)
//     {
//         if (positions[i] == position)
//         {
//             return tiles[i].filled;
//         }
//     }
//     return false;
// }
// 
// void TileRack::display(std::ostream& out) const
// {
//     if (isEmpty())
//     {
//         out << "Rack: (empty)";
//         return;
//     }
//     out << "Rack: ";
//     for (int i = 0; i < count; ++i)
//     {
//         out << "[" << tiles[i].tileLetter << "@" << positions[i];
//         if (tiles[i].filled)
//         {
//             out << "*";
//         }
//         out << "] ";
//     }
// }

// ------------------------------------------------------------
// REFACTORED TILERACK (Module 8) - using DynamicArray<T>
// ------------------------------------------------------------
TileRack::TileRack(int cap)
    : tiles(cap > 0 ? cap : 4),
      positions(cap > 0 ? cap : 4),
      capacity(cap)
{
    if (capacity < 1)
    {
        throw InvalidInputException("TileRack capacity must be at least 1; requested: " +
                                    StringUtils::intToString(cap));
    }
}

TileRack::TileRack(const TileRack& other)
    : tiles(other.tiles),
      positions(other.positions),
      capacity(other.capacity)
{
}

TileRack& TileRack::operator=(const TileRack& other)
{
    if (this != &other)
    {
        tiles     = other.tiles;
        positions = other.positions;
        capacity  = other.capacity;
    }
    return *this;
}

TileRack::~TileRack()
{
    // DynamicArray members automatically release their own dynamic heap memory.
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

    Tile t;
    t.tileLetter = clue.clueLetter;
    t.filled     = false;

    tiles.push(t);
    positions.push(clue.cluePosition);
}

void TileRack::removeTile(int index)
{
    if (isEmpty())
    {
        throw RackEmptyException();
    }
    if (index < 0 || index >= tiles.getSize())
    {
        throw InvalidPositionException(index, tiles.getSize() - 1);
    }
    tiles.removeAt(index);
    positions.removeAt(index);
}

void TileRack::removeLast()
{
    if (isEmpty())
    {
        throw RackEmptyException();
    }
    removeTile(tiles.getSize() - 1);
}

void TileRack::clear()
{
    tiles.clear();
    positions.clear();
}

int TileRack::getCount() const
{
    return tiles.getSize();
}

int TileRack::getCapacity() const
{
    return capacity;
}

bool TileRack::isFull() const
{
    return tiles.getSize() >= capacity;
}

bool TileRack::isEmpty() const
{
    return tiles.isEmpty();
}

char TileRack::getLetterAt(int index) const
{
    if (index < 0 || index >= tiles.getSize())
    {
        throw InvalidPositionException(index, tiles.getSize() - 1);
    }
    return tiles[index].tileLetter;
}

int TileRack::getPositionAt(int index) const
{
    if (index < 0 || index >= positions.getSize())
    {
        throw InvalidPositionException(index, positions.getSize() - 1);
    }
    return positions[index];
}

bool TileRack::hasPosition(int position) const
{
    for (int i = 0; i < positions.getSize(); ++i)
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
    for (int i = 0; i < positions.getSize(); ++i)
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
    for (int i = 0; i < positions.getSize(); ++i)
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
    for (int i = 0; i < tiles.getSize(); ++i)
    {
        out << "[" << tiles[i].tileLetter << "@" << positions[i];
        if (tiles[i].filled)
        {
            out << "*";
        }
        out << "] ";
    }
}

// Private raw tile helper: bypasses clue position validation to allow separator tiles (position 0)
void TileRack::addRawTile(char letter, int position, bool isFilled)
{
    Tile t;
    t.tileLetter = letter;
    t.filled     = isFilled;
    tiles.push(t);
    positions.push(position);
}

// Merges two racks into a new rack with capacity (this->capacity + other.capacity + 1),
// sorting tiles by board position with an intermediate separator tile (space, position 0).
TileRack TileRack::operator+(const TileRack& other) const
{
    int newCap = this->capacity + other.capacity + 1;
    TileRack result(newCap);

    struct TilePos
    {
        char letter = ' ';
        int  pos    = 0;
        bool filled = false;
    };

    // Sort this rack's tiles by board position using selection sort (no STL)
    int nA = this->tiles.getSize();
    DynamicArray<TilePos> arrA(nA > 0 ? nA : 1);
    for (int i = 0; i < nA; ++i)
    {
        TilePos item;
        item.letter = this->tiles[i].tileLetter;
        item.pos    = this->positions[i];
        item.filled = this->tiles[i].filled;
        arrA.push(item);
    }

    for (int i = 0; i < nA - 1; ++i)
    {
        int minIdx = i;
        for (int j = i + 1; j < nA; ++j)
        {
            if (arrA[j].pos < arrA[minIdx].pos)
            {
                minIdx = j;
            }
        }
        if (minIdx != i)
        {
            TilePos temp = arrA[i];
            arrA[i] = arrA[minIdx];
            arrA[minIdx] = temp;
        }
    }

    for (int i = 0; i < nA; ++i)
    {
        result.addRawTile(arrA[i].letter, arrA[i].pos, arrA[i].filled);
    }

    // Add separator tile (space character, position 0) via private helper
    result.addRawTile(' ', 0, false);

    // Sort other rack's tiles by board position using selection sort (no STL)
    int nB = other.tiles.getSize();
    DynamicArray<TilePos> arrB(nB > 0 ? nB : 1);
    for (int i = 0; i < nB; ++i)
    {
        TilePos item;
        item.letter = other.tiles[i].tileLetter;
        item.pos    = other.positions[i];
        item.filled = other.tiles[i].filled;
        arrB.push(item);
    }

    for (int i = 0; i < nB - 1; ++i)
    {
        int minIdx = i;
        for (int j = i + 1; j < nB; ++j)
        {
            if (arrB[j].pos < arrB[minIdx].pos)
            {
                minIdx = j;
            }
        }
        if (minIdx != i)
        {
            TilePos temp = arrB[i];
            arrB[i] = arrB[minIdx];
            arrB[minIdx] = temp;
        }
    }

    for (int i = 0; i < nB; ++i)
    {
        result.addRawTile(arrB[i].letter, arrB[i].pos, arrB[i].filled);
    }

    return result;
}

// Returns all stored letters in order as unified clue text (Module 11)
std::string TileRack::getClueText() const
{
    std::string text = "";
    int sz = tiles.getSize();
    for (int i = 0; i < sz; ++i)
    {
        text += tiles[i].tileLetter;
    }
    return text;
}


// ------------------------------------------------------------
// RAW DMA VERSION (Modules 4 and 5) - kept for reference, replaced by DynamicArray<T> in Module 8
// ------------------------------------------------------------
// Scrabble::Scrabble()
//     : targetsLoaded(false)
// {
//     rowLength[0] = 7;
//     rowLength[1] = 8;
//     targetWord[0] = "";
//     targetWord[1] = "";
// 
//     grid = new Tile*[2];
//     grid[0] = new Tile[7];
//     grid[1] = new Tile[8];
// }
// 
// Scrabble::Scrabble(const std::string& word1, const std::string& word2)
//     : targetsLoaded(false)
// {
//     rowLength[0] = 7;
//     rowLength[1] = 8;
//     targetWord[0] = "";
//     targetWord[1] = "";
// 
//     grid = new Tile*[2];
//     grid[0] = new Tile[7];
//     grid[1] = new Tile[8];
// 
//     loadTargetWords(word1, word2);
// }
// 
// Scrabble::Scrabble(const Scrabble& other)
//     : targetsLoaded(other.targetsLoaded)
// {
//     rowLength[0]  = other.rowLength[0];
//     rowLength[1]  = other.rowLength[1];
//     targetWord[0] = other.targetWord[0];
//     targetWord[1] = other.targetWord[1];
// 
//     grid = new Tile*[2];
//     grid[0] = new Tile[rowLength[0]];
//     grid[1] = new Tile[rowLength[1]];
// 
//     for (int i = 0; i < rowLength[0]; ++i)
//     {
//         grid[0][i] = other.grid[0][i];
//     }
//     for (int i = 0; i < rowLength[1]; ++i)
//     {
//         grid[1][i] = other.grid[1][i];
//     }
// }
// 
// Scrabble& Scrabble::operator=(const Scrabble& other)
// {
//     if (this != &other)
//     {
//         delete[] grid[0];
//         delete[] grid[1];
//         delete[] grid;
// 
//         rowLength[0]  = other.rowLength[0];
//         rowLength[1]  = other.rowLength[1];
//         targetWord[0] = other.targetWord[0];
//         targetWord[1] = other.targetWord[1];
//         targetsLoaded = other.targetsLoaded;
// 
//         grid = new Tile*[2];
//         grid[0] = new Tile[rowLength[0]];
//         grid[1] = new Tile[rowLength[1]];
// 
//         for (int i = 0; i < rowLength[0]; ++i)
//         {
//             grid[0][i] = other.grid[0][i];
//         }
//         for (int i = 0; i < rowLength[1]; ++i)
//         {
//             grid[1][i] = other.grid[1][i];
//         }
//     }
//     return *this;
// }
// 
// Scrabble::~Scrabble()
// {
//     delete[] grid[0];
//     delete[] grid[1];
//     delete[] grid;
//     grid = nullptr;
// }
// 
// void Scrabble::loadTargetWords(const std::string& word1, const std::string& word2)
// {
//     if (word1.length() != 7 || !StringUtils::isAllLetters(word1))
//     {
//         throw InvalidWordException("Word 1 must be 7 uppercase letters: " + word1);
//     }
//     for (size_t i = 0; i < word1.length(); ++i)
//     {
//         if (word1[i] < 'A' || word1[i] > 'Z')
//         {
//             throw InvalidWordException("Word 1 contains non-uppercase letter: " + word1);
//         }
//     }
// 
//     if (word2.length() != 8 || !StringUtils::isAllLetters(word2))
//     {
//         throw InvalidWordException("Word 2 must be 8 uppercase letters: " + word2);
//     }
//     for (size_t i = 0; i < word2.length(); ++i)
//     {
//         if (word2[i] < 'A' || word2[i] > 'Z')
//         {
//             throw InvalidWordException("Word 2 contains non-uppercase letter: " + word2);
//         }
//     }
// 
//     targetWord[0] = word1;
//     targetWord[1] = word2;
//     targetsLoaded = true;
// }
// 
// void Scrabble::placeLetter(const Clue& clue)
// {
//     if (!targetsLoaded)
//     {
//         throw InvalidWordException("Target words are not loaded on Scrabble board");
//     }
// 
//     int r = static_cast<int>(clue.clueRow);
//     if (r < 0 || r > 1)
//     {
//         throw InvalidPositionException(r, 1);
//     }
// 
//     if (clue.cluePosition < 1 || clue.cluePosition > rowLength[r])
//     {
//         throw InvalidPositionException(clue.cluePosition, rowLength[r]);
//     }
// 
//     int idx = clue.cluePosition - 1;
//     if (grid[r][idx].filled)
//     {
//         throw SlotFilledException(clue.cluePosition);
//     }
// 
//     if (clue.clueLetter != targetWord[r][idx])
//     {
//         throw InvalidLetterException(clue.clueLetter, clue.cluePosition);
//     }
// 
//     grid[r][idx].tileLetter = clue.clueLetter;
//     grid[r][idx].filled     = true;
// }
// 
// char Scrabble::revealLetter(WordRow row, int position) const
// {
//     int r = static_cast<int>(row);
//     if (position < 1 || position > rowLength[r])
//     {
//         throw InvalidPositionException(position, rowLength[r]);
//     }
// 
//     int idx = position - 1;
//     return grid[r][idx].filled ? grid[r][idx].tileLetter : ' ';
// }
// 
// bool Scrabble::isRowComplete(WordRow row) const
// {
//     int r = static_cast<int>(row);
//     for (int i = 0; i < rowLength[r]; ++i)
//     {
//         if (!grid[r][i].filled)
//         {
//             return false;
//         }
//     }
//     return true;
// }
// 
// bool Scrabble::isComplete() const
// {
//     return isRowComplete(WordRow::Word1) && isRowComplete(WordRow::Word2);
// }
// 
// int Scrabble::getRowLength(WordRow row) const
// {
//     return rowLength[static_cast<int>(row)];
// }
// 
// int Scrabble::getFilledCount(WordRow row) const
// {
//     int r = static_cast<int>(row);
//     int filledCount = 0;
//     for (int i = 0; i < rowLength[r]; ++i)
//     {
//         if (grid[r][i].filled)
//         {
//             ++filledCount;
//         }
//     }
//     return filledCount;
// }
// 
// std::string Scrabble::getRowString(WordRow row) const
// {
//     int r = static_cast<int>(row);
//     std::string s = "";
//     for (int i = 0; i < rowLength[r]; ++i)
//     {
//         s += (grid[r][i].filled ? grid[r][i].tileLetter : '_');
//     }
//     return s;
// }
// 
// std::string Scrabble::getTargetWord(WordRow row) const
// {
//     return targetWord[static_cast<int>(row)];
// }
// 
// void Scrabble::verifyWord(WordRow row, const std::string& typedWord) const
// {
//     int r = static_cast<int>(row);
//     if (!isRowComplete(row))
//     {
//         throw WordIncompleteException(r == 0 ? "Word 1" : "Word 2");
//     }
// 
//     if (!StringUtils::equalsIgnoreCase(typedWord, targetWord[r]))
//     {
//         throw InvalidWordException(typedWord);
//     }
// }
// 
// void Scrabble::display(std::ostream& out) const
// {
//     out << "Word 1: ";
//     for (int i = 0; i < rowLength[0]; ++i)
//     {
//         if (grid[0][i].filled)
//         {
//             out << "[" << grid[0][i].tileLetter << "] ";
//         }
//         else
//         {
//             out << "[ ] ";
//         }
//     }
//     if (isRowComplete(WordRow::Word1))
//     {
//         out << "COMPLETE";
//     }
// 
//     out << "\nWord 2: ";
//     for (int i = 0; i < rowLength[1]; ++i)
//     {
//         if (grid[1][i].filled)
//         {
//             out << "[" << grid[1][i].tileLetter << "] ";
//         }
//         else
//         {
//             out << "[ ] ";
//         }
//     }
//     if (isRowComplete(WordRow::Word2))
//     {
//         out << "COMPLETE";
//     }
//     out << "\n";
// }

// ------------------------------------------------------------
// REFACTORED SCRABBLE (Module 8) - using DynamicArray<Tile>
// ------------------------------------------------------------
Scrabble::Scrabble()
    : targetsLoaded(false)
{
    rowLength[0] = 7;
    rowLength[1] = 8;
    targetWord[0] = "";
    targetWord[1] = "";

    rows[0].resize(7);
    rows[1].resize(8);
}

Scrabble::Scrabble(const std::string& word1, const std::string& word2)
    : targetsLoaded(false)
{
    rowLength[0] = 7;
    rowLength[1] = 8;
    targetWord[0] = "";
    targetWord[1] = "";

    rows[0].resize(7);
    rows[1].resize(8);

    loadTargetWords(word1, word2);
}

Scrabble::Scrabble(const Scrabble& other)
    : targetsLoaded(other.targetsLoaded)
{
    rowLength[0]  = other.rowLength[0];
    rowLength[1]  = other.rowLength[1];
    targetWord[0] = other.targetWord[0];
    targetWord[1] = other.targetWord[1];

    rows[0] = other.rows[0];
    rows[1] = other.rows[1];
}

Scrabble& Scrabble::operator=(const Scrabble& other)
{
    if (this != &other)
    {
        targetsLoaded = other.targetsLoaded;
        rowLength[0]  = other.rowLength[0];
        rowLength[1]  = other.rowLength[1];
        targetWord[0] = other.targetWord[0];
        targetWord[1] = other.targetWord[1];

        rows[0] = other.rows[0];
        rows[1] = other.rows[1];
    }
    return *this;
}

Scrabble::~Scrabble()
{
    // DynamicArray members automatically release their own dynamic heap memory.
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

    // Reset grid rows
    rows[0].clear();
    rows[1].clear();
    rows[0].resize(7);
    rows[1].resize(8);
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
    if (rows[r][idx].filled)
    {
        throw SlotFilledException(clue.cluePosition);
    }

    // Verify letter matching using Tile::operator== (Module 11)
    Tile providedTile{clue.clueLetter, true};
    Tile expectedTile{targetWord[r][idx], true};
    if (providedTile != expectedTile)
    {
        throw InvalidLetterException(clue.clueLetter, clue.cluePosition);
    }

    rows[r][idx].tileLetter = clue.clueLetter;
    rows[r][idx].filled     = true;
}

char Scrabble::revealLetter(WordRow row, int position) const
{
    int r = static_cast<int>(row);
    if (position < 1 || position > rowLength[r])
    {
        throw InvalidPositionException(position, rowLength[r]);
    }

    int idx = position - 1;
    return rows[r][idx].filled ? rows[r][idx].tileLetter : ' ';
}

bool Scrabble::isRowComplete(WordRow row) const
{
    int r = static_cast<int>(row);
    for (int i = 0; i < rowLength[r]; ++i)
    {
        if (!rows[r][i].filled)
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
        if (rows[r][i].filled)
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
        s += (rows[r][i].filled ? rows[r][i].tileLetter : '_');
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
        if (rows[0][i].filled)
        {
            out << "[" << rows[0][i].tileLetter << "] ";
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
        if (rows[1][i].filled)
        {
            out << "[" << rows[1][i].tileLetter << "] ";
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


// ------------------------------------------------------------
// RAW DMA VERSION (Modules 4 and 5) - kept for reference, replaced by DynamicArray<T> in Module 8
// ------------------------------------------------------------
// Cottage::Cottage()
//     : locations(nullptr),
//       locationCount(0)
// {
// }
// 
// Cottage::Cottage(const LocationRecord records[], int count, int culpritColumn)
//     : locations(nullptr),
//       locationCount(0)
// {
//     if (count < 0)
//     {
//         throw InvalidInputException("Location count cannot be negative");
//     }
//     if (culpritColumn < 0 || culpritColumn >= DataLoader::ROLE_COUNT)
//     {
//         throw InvalidInputException("Invalid culprit column index: " + StringUtils::intToString(culpritColumn));
//     }
// 
//     if (count > 0)
//     {
//         locations = new Location*[count];
//         locationCount = count;
//         for (int i = 0; i < count; ++i)
//         {
//             locations[i] = new Location;
//             locations[i]->locName       = records[i].placeName;
//             locations[i]->letterClueLoc = records[i].letters[culpritColumn];
//             locations[i]->boardPosLoc   = records[i].word1Pos;
//             locations[i]->locRow        = WordRow::Word1;
//             locations[i]->locSearchFlag = false;
//         }
//     }
// }
// 
// Cottage::Cottage(const Cottage& other)
//     : locations(nullptr),
//       locationCount(other.locationCount)
// {
//     if (other.locationCount > 0 && other.locations != nullptr)
//     {
//         locations = new Location*[other.locationCount];
//         for (int i = 0; i < other.locationCount; ++i)
//         {
//             locations[i] = new Location;
//             *(locations[i]) = *(other.locations[i]);
//         }
//     }
//     else
//     {
//         locations = nullptr;
//         locationCount = 0;
//     }
// }
// 
// Cottage& Cottage::operator=(const Cottage& other)
// {
//     if (this != &other)
//     {
//         // Delete current dynamically allocated locations
//         if (locations != nullptr)
//         {
//             for (int i = 0; i < locationCount; ++i)
//             {
//                 delete locations[i];
//             }
//             delete[] locations;
//             locations = nullptr;
//         }
//         locationCount = 0;
// 
//         // Perform deep copy from other
//         if (other.locationCount > 0 && other.locations != nullptr)
//         {
//             locations = new Location*[other.locationCount];
//             locationCount = other.locationCount;
//             for (int i = 0; i < other.locationCount; ++i)
//             {
//                 locations[i] = new Location;
//                 *(locations[i]) = *(other.locations[i]);
//             }
//         }
//     }
//     return *this;
// }
// 
// Cottage::~Cottage()
// {
//     if (locations != nullptr)
//     {
//         for (int i = 0; i < locationCount; ++i)
//         {
//             delete locations[i];
//         }
//         delete[] locations;
//         locations = nullptr;
//     }
//     locationCount = 0;
// }
// 
// int Cottage::getLocationCount() const
// {
//     return locationCount;
// }
// 
// bool Cottage::isLocationSearched(int number) const
// {
//     if (number < 1 || number > locationCount)
//     {
//         throw InvalidPositionException(number, locationCount);
//     }
//     return locations[number - 1]->locSearchFlag;
// }
// 
// Clue Cottage::searchLocation(int number)
// {
//     if (number < 1 || number > locationCount)
//     {
//         throw InvalidPositionException(number, locationCount);
//     }
//     locations[number - 1]->locSearchFlag = true;
// 
//     Clue c;
//     c.clueLetter   = locations[number - 1]->letterClueLoc;
//     c.cluePosition = locations[number - 1]->boardPosLoc;
//     c.clueRow      = locations[number - 1]->locRow;
//     return c;
// }
// 
// Clue Cottage::getLocationClue(int number) const
// {
//     if (number < 1 || number > locationCount)
//     {
//         throw InvalidPositionException(number, locationCount);
//     }
//     Clue c;
//     c.clueLetter   = locations[number - 1]->letterClueLoc;
//     c.cluePosition = locations[number - 1]->boardPosLoc;
//     c.clueRow      = locations[number - 1]->locRow;
//     return c;
// }
// 
// std::string Cottage::getLocationName(int number) const
// {
//     if (number < 1 || number > locationCount)
//     {
//         throw InvalidPositionException(number, locationCount);
//     }
//     return StringUtils::underscoreToSpace(locations[number - 1]->locName);
// }
// 
// int Cottage::getSearchedCount() const
// {
//     int searched = 0;
//     for (int i = 0; i < locationCount; ++i)
//     {
//         if (locations[i] != nullptr && locations[i]->locSearchFlag)
//         {
//             ++searched;
//         }
//     }
//     return searched;
// }
// 
// bool Cottage::allSearched() const
// {
//     return (locationCount > 0 && getSearchedCount() == locationCount);
// }
// 
// void Cottage::displayLocations(std::ostream& out) const
// {
//     for (int i = 1; i <= locationCount; ++i)
//     {
//         out << "[" << i << "] " << getLocationName(i) << " - "
//             << (isLocationSearched(i) ? "Searched" : "NOT Searched") << "\n";
//     }
// }

// ------------------------------------------------------------
// REFACTORED COTTAGE (Module 8) - using DynamicArray<Location>
// ------------------------------------------------------------
Cottage::Cottage()
    : locations(4)
{
}

Cottage::Cottage(const LocationRecord records[], int count, int culpritColumn)
    : locations(count > 0 ? count : 4)
{
    if (count < 0)
    {
        throw InvalidInputException("Location count cannot be negative");
    }
    if (culpritColumn < 0 || culpritColumn >= DataLoader::ROLE_COUNT)
    {
        throw InvalidInputException("Invalid culprit column index: " + StringUtils::intToString(culpritColumn));
    }

    for (int i = 0; i < count; ++i)
    {
        Location loc;
        loc.locName       = records[i].placeName;
        loc.letterClueLoc = records[i].letters[culpritColumn];
        loc.boardPosLoc   = records[i].word1Pos;
        loc.locRow        = WordRow::Word1;
        loc.locSearchFlag = false;
        locations.push(loc);
    }
}

Cottage::Cottage(const Cottage& other)
    : locations(other.locations)
{
}

Cottage& Cottage::operator=(const Cottage& other)
{
    if (this != &other)
    {
        locations = other.locations;
    }
    return *this;
}

Cottage::~Cottage()
{
    // DynamicArray members automatically release their own dynamic heap memory.
}

int Cottage::getLocationCount() const
{
    return locations.getSize();
}

bool Cottage::isLocationSearched(int number) const
{
    if (number < 1 || number > locations.getSize())
    {
        throw InvalidPositionException(number, locations.getSize());
    }
    return locations[number - 1].locSearchFlag;
}

Clue Cottage::searchLocation(int number)
{
    if (number < 1 || number > locations.getSize())
    {
        throw InvalidPositionException(number, locations.getSize());
    }
    locations[number - 1].locSearchFlag = true;

    Clue c;
    c.clueLetter   = locations[number - 1].letterClueLoc;
    c.cluePosition = locations[number - 1].boardPosLoc;
    c.clueRow      = locations[number - 1].locRow;
    return c;
}

Clue Cottage::getLocationClue(int number) const
{
    if (number < 1 || number > locations.getSize())
    {
        throw InvalidPositionException(number, locations.getSize());
    }
    Clue c;
    c.clueLetter   = locations[number - 1].letterClueLoc;
    c.cluePosition = locations[number - 1].boardPosLoc;
    c.clueRow      = locations[number - 1].locRow;
    return c;
}

std::string Cottage::getLocationName(int number) const
{
    if (number < 1 || number > locations.getSize())
    {
        throw InvalidPositionException(number, locations.getSize());
    }
    return StringUtils::underscoreToSpace(locations[number - 1].locName);
}

int Cottage::getSearchedCount() const
{
    int searched = 0;
    for (int i = 0; i < locations.getSize(); ++i)
    {
        if (locations[i].locSearchFlag)
        {
            ++searched;
        }
    }
    return searched;
}

bool Cottage::allSearched() const
{
    return (locations.getSize() > 0 && getSearchedCount() == locations.getSize());
}

void Cottage::displayLocations(std::ostream& out) const
{
    for (int i = 1; i <= locations.getSize(); ++i)
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


// ============================================================
// SECTION 9 -- CONSOLE UTILITIES (Commit 8)
// ============================================================

// Static member definition: starts false; set to true when cin hits EOF.
bool ConsoleUtils::inputClosed = false;

// clearScreen() calls the Windows system command to wipe the terminal.
// Using system("cls") from <cstdlib> which is already included.
void ConsoleUtils::clearScreen()
{
    system("cls");
}

// readLine() shows a prompt then reads one full line using std::getline.
// If getline fails (EOF on a piped test script), it sets inputClosed
// so callers can break out of their loops instead of blocking forever.
std::string ConsoleUtils::readLine(const std::string& prompt)
{
    std::string line;
    if (!prompt.empty())
    {
        std::cout << prompt;
    }
    if (!std::getline(std::cin, line))
    {
        inputClosed = true;
        return "";
    }
    return StringUtils::trim(line);
}

// readIntInRange() keeps asking until the player enters a valid integer
// that falls within [minVal, maxVal].  Internally catches parse errors
// and out-of-range values and re-prompts rather than propagating them.
// Returns minVal when inputClosed becomes true so the caller can exit cleanly.
int ConsoleUtils::readIntInRange(const std::string& prompt, int minVal, int maxVal)
{
    while (true)
    {
        if (inputClosed)
        {
            return minVal; // safe sentinel so the caller can detect EOF via isInputClosed()
        }

        std::string raw = readLine(prompt);

        if (inputClosed)
        {
            return minVal;
        }

        try
        {
            int value = StringUtils::parseInt(raw);
            if (value < minVal || value > maxVal)
            {
                std::cout << "  Please enter a number between " << minVal
                          << " and " << maxVal << ".\n";
                continue;
            }
            return value;
        }
        catch (const InvalidInputException&)
        {
            std::cout << "  Invalid input. Please enter a number between "
                      << minVal << " and " << maxVal << ".\n";
        }
    }
}

// readYesNo() loops until the player types y/Y/n/N or stdin closes.
// Returns true for yes, false for no or EOF -- never blocks forever.
bool ConsoleUtils::readYesNo(const std::string& prompt)
{
    while (true)
    {
        if (inputClosed)
        {
            return false; // treat EOF as "no"
        }

        std::string raw = readLine(prompt + " (y/n): ");

        if (inputClosed)
        {
            return false;
        }

        if (!raw.empty())
        {
            char ch = raw[0];
            if (ch == 'y' || ch == 'Y') { return true;  }
            if (ch == 'n' || ch == 'N') { return false; }
        }

        std::cout << "  Please type y or n.\n";
    }
}

// isInputClosed() lets game loops check whether stdin has ended
// without calling any blocking I/O.
bool ConsoleUtils::isInputClosed()
{
    return inputClosed;
}


// ============================================================
// SECTION 10 -- GAME ENGINE SINGLETON (Commit 8)
// ============================================================

// Static instance pointer starts null; created on first getInstance() call.
GameEngine* GameEngine::instance = nullptr;

// GameEngine constructor seeds the random number generator, zeroes the
// cast array, and sets the initial state before any round is started.
GameEngine::GameEngine()
    : detective(),
      cottage(),
      scrabble(),
      combinedClue(16),    // capacity = 7 (Word1) + 1 (separator) + 8 (Word2)
      scoreBoard(nullptr),
      rules(),
      state(GameState::Menu),
      culpritColumn(0),
      forcedCulpritColumn(-1)  // -1 means pick at random each round
{
    // Seed the global random number generator once per engine lifetime.
    // Every subsequent rand() call in this session uses this seed.
    srand(static_cast<unsigned int>(std::time(nullptr)));

    // Initialise all cast slots to null so the destructor can safely
    // skip slots that were never filled.
    for (int i = 0; i < CAST_SIZE; ++i)
    {
        cast[i] = nullptr;
    }
}

// GameEngine destructor frees all Characters that the engine created via
// CharacterFactory.  ScoreBoard is NOT deleted here because main() owns it.
GameEngine::~GameEngine()
{
    for (int i = 0; i < CAST_SIZE; ++i)
    {
        delete cast[i];
        cast[i] = nullptr;
    }
}

// getInstance() creates the singleton on the first call and returns a
// reference thereafter.  Using a static pointer (not a local static) so
// destroyInstance() can fully release the memory.
GameEngine& GameEngine::getInstance()
{
    if (instance == nullptr)
    {
        instance = new GameEngine();
    }
    return *instance;
}

// destroyInstance() deletes the singleton and resets the pointer to null.
// Always call this in main() before the ScoreBoard goes out of scope.
void GameEngine::destroyInstance()
{
    delete instance;
    instance = nullptr;
}

// attachScoreBoard() gives the engine a pointer to the ScoreBoard created
// in main().  The engine will call displayAll() and addScore() on it but
// will never delete it (ASSOCIATION, not COMPOSITION).
void GameEngine::attachScoreBoard(ScoreBoard* sb)
{
    scoreBoard = sb;
}

// setForcedCulprit() lets the --culprit command-line flag pre-select the
// culprit for testing without randomness.  oneToSeven maps 1=Cook...7=Child.
// Values outside [1,7] are invalid and throw to prevent bad test setups.
void GameEngine::setForcedCulprit(int oneToSeven)
{
    if (oneToSeven < 1 || oneToSeven > 7)
    {
        throw InvalidInputException(
            "Culprit column must be 1 (Cook) to 7 (Child); received: " +
            StringUtils::intToString(oneToSeven));
    }
    forcedCulpritColumn = oneToSeven - 1; // convert to 0-based index
}

// sanitiseName() validates and cleans a raw detective name entered by the player.
// Rules: not empty, not longer than 20 chars, no pipe '|' or control chars (<32),
// and spaces get replaced by the chosen replacement character (_ or .).
// Throws InvalidInputException describing exactly which rule was broken.
std::string GameEngine::sanitiseName(const std::string& raw, char spaceReplacement)
{
    // Reject empty names immediately -- the player must provide something
    if (raw.empty())
    {
        throw InvalidInputException("Detective name cannot be empty.");
    }

    // Reject names that are too long for the scoreboard column
    if (static_cast<int>(raw.size()) > MAX_DETECTIVE_NAME_LEN)
    {
        throw InvalidInputException(
            "Detective name must be at most " +
            StringUtils::intToString(MAX_DETECTIVE_NAME_LEN) +
            " characters long.");
    }

    // Build the sanitised result, checking each character along the way
    std::string result = "";
    for (int i = 0; i < static_cast<int>(raw.size()); ++i)
    {
        char c = raw[i];

        // Pipe is the scoreBoard field delimiter -- absolutely forbidden in names
        if (c == '|')
        {
            throw InvalidInputException(
                "Detective name must not contain the '|' character.");
        }

        // Control characters (ASCII < 32) would corrupt the display
        if (static_cast<int>(c) < 32)
        {
            throw InvalidInputException(
                "Detective name must not contain control characters.");
        }

        // Replace spaces with the user-chosen separator
        if (c == ' ')
        {
            result += spaceReplacement;
        }
        else
        {
            result += c;
        }
    }

    return result;
}

// loadRulesOrDefaults() tries to load rules from the given path.
// If the file is missing or corrupt it silently falls back to defaults
// and sets usedDefaults = true so the caller can warn the player.
RulesConfig GameEngine::loadRulesOrDefaults(const std::string& path, bool& usedDefaults)
{
    try
    {
        RulesConfig loaded = DataLoader::loadRules(path);
        usedDefaults = false;
        return loaded;
    }
    catch (const FileNotFoundException&)
    {
        usedDefaults = true;
        return RulesConfig::defaultRules();
    }
    catch (const FileCorruptException&)
    {
        usedDefaults = true;
        return RulesConfig::defaultRules();
    }
}

// beginRound() sets up a complete game session: loads data files, selects
// the culprit, builds the full cast via CharacterFactory, constructs the
// Cottage and Scrabble board for the culprit's words, and resets the Detective.
// Fatal data errors (FileNotFound, FileCorrupt) propagate up to run() which
// reports them and returns to the main menu.
void GameEngine::beginRound(const std::string& detectiveName)
{
    // 1. Load rules, falling back to defaults if the file is missing or corrupt.
    bool usedDefaults = false;
    rules = loadRulesOrDefaults(DataLoader::FILE_RULES, usedDefaults);
    if (usedDefaults)
    {
        std::cout << "\n  [WARNING] rules.txt could not be loaded -- using default rules.\n\n";
    }

    // 2. Load clue profiles, locations and characters from their data files.
    //    validateConsistency throws if any file is internally inconsistent.
    DataLoader::loadClueProfiles(DataLoader::FILE_CLUE, profiles, DataLoader::ROLE_COUNT);
    DataLoader::loadLocations(DataLoader::FILE_LOCATIONS, locationRecords, DataLoader::LOCATION_COUNT);
    DataLoader::loadCharacters(DataLoader::FILE_CHARACTERS, characterRecords, DataLoader::CHARACTER_COUNT);
    DataLoader::validateConsistency(profiles, locationRecords, characterRecords);

    // 3. Pick the culprit -- use forced column if set, otherwise random.
    if (forcedCulpritColumn >= 0)
    {
        culpritColumn = forcedCulpritColumn;
    }
    else
    {
        culpritColumn = rand() % DataLoader::ROLE_COUNT;
    }

    // 4. Delete any characters from a previous round, then rebuild the full cast.
    for (int i = 0; i < CAST_SIZE; ++i)
    {
        delete cast[i];
        cast[i] = nullptr;
    }

    // The culprit's role in the profile may use underscores (e.g. "Partner_1"),
    // while the character records may use spaces ("Partner 1").  Normalise both
    // sides to spaces for a reliable match.
    std::string culpritRoleNorm =
        StringUtils::replaceChar(profiles[culpritColumn].culpritRole, '_', ' ');

    for (int i = 0; i < DataLoader::CHARACTER_COUNT; ++i)
    {
        // CharacterFactory reads characterRecords[i].isCulprit but we override
        // it ourselves after creation so the factory always gets isCulprit=false
        // from the data file (which is correct -- the data file never sets it).
        cast[i] = CharacterFactory::create(characterRecords[i], culpritColumn);

        // Compare normalised role strings to identify the culprit character.
        // GameEngine is a friend of Character so it can directly set the private field.
        std::string charRoleNorm =
            StringUtils::replaceChar(characterRecords[i].role, '_', ' ');

        if (StringUtils::equalsIgnoreCase(charRoleNorm, culpritRoleNorm))
        {
            cast[i]->isCulprit = true;
        }
    }

    // 5. Build the Cottage for this round using the culprit's column.
    cottage = Cottage(locationRecords, DataLoader::LOCATION_COUNT, culpritColumn);

    // 6. Build the Scrabble board with the culprit's two secret words.
    scrabble = Scrabble(profiles[culpritColumn].word1, profiles[culpritColumn].word2);

    // 7. Assign-construct a fresh Detective (exercises deep-copy assignment operator).
    detective = Detective(detectiveName, rules.startingCredibility);

    // 8. Clear the combined clue rack from any previous round.
    combinedClue.clear();

    // 9. Mark the game as active.
    state = GameState::Playing;

    // 10. Confirm the round has started without revealing the culprit's identity.
    std::cout << "\n";
    std::cout << "+============================================================+\n";
    std::cout << "|   A culprit has been secretly chosen. The game begins!     |\n";
    std::cout << "+============================================================+\n";
    std::cout << "\n";
}

// askDetectiveName() handles the full name-input loop for the New Game flow.
// It enforces the naming rules and asks the player how to handle spaces.
std::string GameEngine::askDetectiveName()
{
    while (true)
    {
        if (ConsoleUtils::isInputClosed()) { return "Detective"; }

        std::string raw = ConsoleUtils::readLine("  Enter your detective name: ");

        if (ConsoleUtils::isInputClosed()) { return "Detective"; }

        // Reject blank or whitespace-only input before any other check
        std::string trimmed = StringUtils::trim(raw);
        if (trimmed.empty())
        {
            std::cout << "  Name cannot be empty. Please try again.\n";
            continue;
        }

        // Check for forbidden characters before asking about spaces
        bool hasPipe    = false;
        bool hasControl = false;
        bool hasTooLong = (static_cast<int>(trimmed.size()) > MAX_DETECTIVE_NAME_LEN);

        for (int i = 0; i < static_cast<int>(trimmed.size()); ++i)
        {
            if (trimmed[i] == '|') { hasPipe = true; break; }
            if (static_cast<int>(trimmed[i]) < 32) { hasControl = true; break; }
        }

        if (hasPipe)
        {
            std::cout << "  Name must not contain '|'. Please try again.\n";
            continue;
        }
        if (hasControl)
        {
            std::cout << "  Name must not contain control characters. Please try again.\n";
            continue;
        }
        if (hasTooLong)
        {
            std::cout << "  Name must be at most " << MAX_DETECTIVE_NAME_LEN
                      << " characters. Please try again.\n";
            continue;
        }

        // If the name contains spaces, ask the player how to represent them.
        bool hasSpaces = false;
        for (int i = 0; i < static_cast<int>(trimmed.size()); ++i)
        {
            if (trimmed[i] == ' ') { hasSpaces = true; break; }
        }

        if (hasSpaces)
        {
            std::cout << "  Your name contains spaces. Replace with underscore or dot?\n";
            std::cout << "    [1] Underscore (_)\n";
            std::cout << "    [2] Dot (.)\n";
            int sepChoice = ConsoleUtils::readIntInRange("  Choice: ", 1, 2);
            if (ConsoleUtils::isInputClosed()) { return "Detective"; }

            char sep = (sepChoice == 1) ? '_' : '.';
            return sanitiseName(trimmed, sep);
        }

        // No spaces -- accept as-is
        return trimmed;
    }
}

// showAbout() reads about.txt and prints its entire contents to the console.
// If the file is missing it prints a brief inline message instead of throwing.
void GameEngine::showAbout()
{
    std::cout << "\n";
    try
    {
        std::string text = DataLoader::loadAboutText(DataLoader::FILE_ABOUT);
        std::cout << text << "\n";
    }
    catch (const FileNotFoundException&)
    {
        std::cout << "  About file not found.\n\n";
    }
}

// showLeaderboard() displays the persistent score table.
// File exceptions are caught and reported so the player can continue playing.
void GameEngine::showLeaderboard()
{
    std::cout << "\n";
    if (scoreBoard == nullptr)
    {
        std::cout << "  Leaderboard not available.\n\n";
        return;
    }

    try
    {
        scoreBoard->displayAll(std::cout);
    }
    catch (const FileNotFoundException& ex)
    {
        std::cout << "  [NOTE] " << ex.getMessage() << " -- no scores recorded yet.\n\n";
    }
    catch (const FileCorruptException& ex)
    {
        std::cout << "  [WARNING] Scoreboard file is corrupt: " << ex.getMessage() << "\n\n";
    }
}

// showRules() prints all rule parameters from the loaded RulesConfig together
// with the interview-credibility thresholds taken directly from the static
// constants on each class so there is never a discrepancy between code and display.
void GameEngine::showRules()
{
    std::cout << "\n";
    std::cout << "+============================================================+\n";
    std::cout << "|                      GAME RULES                            |\n";
    std::cout << "+============================================================+\n";
    std::cout << "\n";
    std::cout << "  Max turns allowed  : " << rules.maxTurns << "\n";
    std::cout << "  Starting credibility: " << rules.startingCredibility << "\n";
    std::cout << "  Wrong turn penalty : " << (rules.wrongTurnPenalty * -1) << " credibility points\n";
    std::cout << "  Wrong word penalty : " << (rules.wrongWordPenalty  * -1) << " credibility points\n";
    std::cout << "  Win condition      : " << rules.winCondition  << "\n";
    std::cout << "  Lose condition     : " << rules.loseCondition << "\n";
    std::cout << "\n";
    std::cout << "  Interview credibility thresholds:\n";
    std::cout << "    Victim (Mr. Leonardo)  : " << Victim::MIN_CREDIBILITY_INTERVIEW  << "\n";
    std::cout << "    Partner (Ms./Mr.)      : " << Partner::MIN_CREDIBILITY_INTERVIEW << "\n";
    std::cout << "    Personal Assistant     : " << PA::MIN_CREDIBILITY_INTERVIEW      << "\n";
    std::cout << "    Child (Ms. Celine)     : " << Child::MIN_CREDIBILITY_INTERVIEW   << "\n";
    std::cout << "    House Help (Cook/Guard/Servant): no minimum\n";
    std::cout << "\n";
    ConsoleUtils::readLine("  Press Enter to continue...");
}

// runInGameLoop() is the active-investigation loop for Commit 8.
// Only options [3] View Scrabble Board and [6] Quit are available here;
// all remaining in-game features are added in Commits 9-11.
void GameEngine::runInGameLoop()
{
    while (state == GameState::Playing && !ConsoleUtils::isInputClosed())
    {
        std::cout << "\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << "  Credibility: " << detective.getCredibility()
                  << "  |  Turns used: " << detective.getTurnsUsed() << "\n";
        std::cout << "------------------------------------------------------------\n";

        // Show the detective's collected clue racks
        detective.displayRacks(std::cout);

        // Show the crossword board
        std::cout << "\n  Scrabble Board:\n";
        scrabble.display(std::cout);

        // Show only the two options available in this commit
        std::cout << "\n  What would you like to do?\n";
        std::cout << "    [3] View Scrabble Board\n";
        std::cout << "    [6] Quit to main menu\n";

        int choice = ConsoleUtils::readIntInRange("\n  Choice: ", 1, 9);

        if (ConsoleUtils::isInputClosed()) { break; }

        switch (choice)
        {
            case 3:
                // Re-display the board on explicit player request
                std::cout << "\n  Current Scrabble Board:\n";
                scrabble.display(std::cout);
                break;

            case 6:
            {
                bool confirmed = ConsoleUtils::readYesNo("  Quit to main menu? No score will be recorded.");
                if (confirmed)
                {
                    state = GameState::Menu;
                }
                break;
            }

            default:
                // Any other number is not a valid option in this commit
                std::cout << "  That option is not available yet. Use [3] or [6].\n";
                break;
        }
    }
}

// handleNewGame() runs the full New Game flow: get a name, call beginRound(),
// then hand control to the in-game loop.
void GameEngine::handleNewGame()
{
    std::cout << "\n";
    std::cout << "+============================================================+\n";
    std::cout << "|                      NEW GAME                              |\n";
    std::cout << "+============================================================+\n";

    // Collect and validate the detective's name
    std::string detName = askDetectiveName();

    if (ConsoleUtils::isInputClosed()) { return; }

    // Set up the round -- this may throw GameException on data file errors
    beginRound(detName);

    // Enter the investigation loop
    runInGameLoop();
}

// showMainMenu() prints the five menu options.
void GameEngine::showMainMenu()
{
    std::cout << "\n";
    std::cout << "+============================================================+\n";
    std::cout << "|                      MAIN MENU                             |\n";
    std::cout << "+============================================================+\n";
    std::cout << "    [1] New Game\n";
    std::cout << "    [2] View Leaderboard\n";
    std::cout << "    [3] About\n";
    std::cout << "    [4] Rules\n";
    std::cout << "    [5] Exit\n";
}

// run() is the top-level program loop.  It shows the title + scenario text
// once, then cycles through the main menu until the player chooses Exit.
void GameEngine::run()
{
    // Title and scenario banner shown once per program run
    std::cout << "\n";
    std::cout << "+============================================================+\n";
    std::cout << "|              THE LOCKER HEIST                              |\n";
    std::cout << "+============================================================+\n";
    std::cout << "\n";
    std::cout << "  A priceless artefact has been stolen from the mansion of\n";
    std::cout << "  Mr. Leonardo. Seven suspects live under the same roof.\n";
    std::cout << "  Only one of them is the culprit -- and you must find out who.\n";
    std::cout << "\n";

    state = GameState::Menu;

    while (state != GameState::Exit && !ConsoleUtils::isInputClosed())
    {
        showMainMenu();

        int raw = ConsoleUtils::readIntInRange("\n  Choice: ", 1, 5);

        if (ConsoleUtils::isInputClosed()) { break; }

        MenuChoice choice = static_cast<MenuChoice>(raw);

        try
        {
            switch (choice)
            {
                case MenuChoice::NewGame:
                    handleNewGame();
                    // After returning from a game, reset state to Menu
                    if (state != GameState::Exit)
                    {
                        state = GameState::Menu;
                    }
                    break;

                case MenuChoice::ViewLeaderBoard:
                    showLeaderboard();
                    break;

                case MenuChoice::About:
                    showAbout();
                    break;

                case MenuChoice::Rules:
                    showRules();
                    break;

                case MenuChoice::Exit:
                    state = GameState::Exit;
                    std::cout << "\n  Thank you for playing The Locker Heist. Goodbye!\n\n";
                    break;
            }
        }
        catch (const GameException& ex)
        {
            // Data-load failures inside handleNewGame() are non-fatal at the menu level
            std::cout << "\n  [Error] " << ex.getCategory() << ": " << ex.getMessage() << "\n";
            state = GameState::Menu;
        }
    }
}

// ------------------------------------------------------------------
// Read-only getter implementations
// ------------------------------------------------------------------

int GameEngine::getCulpritColumn() const
{
    return culpritColumn;
}

// getCharacter() returns a const pointer to the character at 0-based index i.
// Throws InvalidPositionException if i is outside [0, CAST_SIZE).
const Character* GameEngine::getCharacter(int i) const
{
    if (i < 0 || i >= CAST_SIZE)
    {
        throw InvalidPositionException(i + 1, CAST_SIZE);
    }
    return cast[i];
}

const Scrabble& GameEngine::getScrabble() const
{
    return scrabble;
}

const Cottage& GameEngine::getCottage() const
{
    return cottage;
}

const Detective& GameEngine::getDetective() const
{
    return detective;
}

const RulesConfig& GameEngine::getRules() const
{
    return rules;
}
