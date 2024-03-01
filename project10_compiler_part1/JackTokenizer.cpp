#include "JackTokenizer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

std::unordered_map<std::string, keyword> kwds { {"class", CLASS}, {"constructor", CONSTRUCTOR}, {"function", FUNCTION}, {"method", METHOD}, {"field", FIELD}, {"static", STATIC},
                                                {"var", VAR}, {"int", INT}, {"char", CHAR}, {"boolean", BOOLEAN}, {"void", VOID}, {"true", TRUE}, {"false", FALSE}, {"null", null},
                                                {"this", THIS}, {"let", LET}, {"do", DO}, {"if", IF}, {"while", WHILE}, {"else", ELSE}, {"return", RETURN} };
std::unordered_set<char> sybls { '{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '~' };
std::unordered_set<char> kwdTerminals {';', ')', ','};
char punct[7] = { '.', '(', ')', '[', ']', ',', ';' };

JackTokenizer::JackTokenizer(std::string &infile_name): startofToken(0), tokenType(KEYWORD), kwdType(CLASS), returnString("") 
                                                        // symString(""), constInt(0), constString(""), idtString(""), curToken("")
{
    infile.open(infile_name);
    if (!infile)
    {
        std::cerr << "JackTokenizer: Open input file error !" << std::endl;
        exit(1);
    }
}
JackTokenizer::~JackTokenizer()
{
    if (infile)
    {
        infile.close();
        std::cout << "Tokenize success !" << std::endl;
    }
}
bool JackTokenizer::hasMoreTokens() const
{
    return !infile.eof();
}
bool JackTokenizer::isComment1() const
{
    if (curToken.size() < 2)
        return false;
    return curToken[0] == '/' && curToken[1] == '/';
}
bool JackTokenizer::isComment2() const
{
    if (curToken.size() < 2)
        return false;
    return curToken[0] == '/' && curToken[1] == '*';
}
void JackTokenizer::advance()
{
    if (hasMoreTokens())
    {
        if (curToken == "")             // Get new token from inputfile if token is empty, otherwise update tokentype only
        {
            infile >> curToken;
            while (isComment1() || isComment2())                    
            {
                if (isComment1())       // Handle single line comment
                {
                    if (infile.peek() != '/')
                    {
                        std::string line;
                        getline(infile, line);
                        curToken += line;
                    }       
                }
                else                    /* Handle block comment */
                {
                    std::string line;
                    while (curToken.find("*/") == -1)
                    {
                        getline(infile, line);
                        curToken += line;
                    }
                }
                if (hasMoreTokens())
                    infile >> curToken;
            }
        }
        setTokenType();                 // Update current tokentype
    }
    return;
}
void JackTokenizer::setTokenType()
{
    // If current token starts with char in "sybls set":
    // 1. Update current token: curToken[1:]
    // 2. Tokentype = SYMBOL
    if (sybls.find(curToken[0]) != sybls.end())
    {
        returnString = curToken[0];
        startofToken = 1;
        tokenType = SYMBOL;
        return;
    }
    // If current token starts with digit:
    // 1. Construct intVal from string
    // 2. Find the position of last digit and update curToken: curToken[pos:]
    // 3. Tokentype = INT_CONST
    else if (isdigit(curToken[0]))
    {
        returnString = "";
        int pos = 0;
        while (isdigit(curToken[pos]))
            returnString += curToken[pos++];

        startofToken = pos;
        tokenType = INT_CONST;
        return;
    }
    // If current token starts with '"':
    // 1. Readline from infile stream and append to curToken
    // 2. Find the position of last '"' (pos = string.rfind('"'))
    // 3. strVal = curToken.substr(1, pos - 1) Ex: "How are you_? " --> How are you ?_
    // 4. Move the pointer to closest non-space position after last '"' and update curToken: curToken[pos:]
    // 5. Tokentype = STR_CONST
    else if (curToken[0] == '"')
    {
        std::string restline = "";
        getline(infile, restline);
        curToken += restline;
        int pos = curToken.rfind('"');
        returnString = curToken.substr(1, pos - 1);
        startofToken = pos + 1;
        for (int i = startofToken; i < curToken.length(); i++)
        {
            if (curToken[i] == ' ')
                startofToken++;
            else
                break;
        }
        tokenType = STR_CONST;
        return;
    }
    // If current token starts with string in "kwds map"
    // Keyword might be: "keyword (,?) (;?) ()?)"
    // 1. Set keyword string to the found string in kwds map
    // 2. Update curToken: curToken[keyword.length():]
    // 3. Set keyword type to kwds[keyword string]
    // 4. Tokentype = KEYWORD
    for (auto &pair: kwds)
    {
        if (curToken == pair.first || (curToken.find(pair.first) == 0 && kwdTerminals.find(curToken[pair.first.length()]) != kwdTerminals.end()))
        {
            returnString = pair.first;
            startofToken = returnString.length();
            kwdType = kwds[returnString];
            tokenType = KEYWORD;
            return;
        }
    }
    // If condition 1 ~ 4 not meet, curToken type is identifier
    // If any punctuation in token, find the left most position of it:
    // 1. Update identifier string to curToken.substr(0, pos)
    // 2. Update curToken to curToken[pos:]
    // If no puctuation in token:
    // 1. Update identifier string = curToken
    // 2. Update curToken = "" (curToken = curToken.substr(curToken.length()))
    tokenType = IDENTIFIER;
    for (int i = 0; i < 7; i++)
    {
        int pos = curToken.find(punct[i]);
        if (pos != -1)
        {
            returnString = curToken.substr(0, pos);
            startofToken = pos;
            return;
        }
    }
    returnString = curToken;
    startofToken = returnString.length();
    return;
}

token JackTokenizer::getTokenType() const
{
    return tokenType;
}
keyword JackTokenizer::getKeywordType() const
{
    return kwdType;
}
std::string JackTokenizer::getKeyword()
{
    updateToken();
    return returnString;
}
std::string JackTokenizer::getSymbol()
{
    updateToken();
    if (returnString == "<")
        return "&lt;";
    else if (returnString == ">")
        return "&gt;";
    else if (returnString == "&")
        return "&amp;";
    return returnString;
}
int JackTokenizer::getIntVal()
{
    updateToken();
    int res = std::stoi(returnString);
    return res;
}
std::string JackTokenizer::getStringVal()
{
    updateToken();
    return returnString;
}
std::string JackTokenizer::getIdentifier()
{
    updateToken();
    return returnString;
}
void JackTokenizer::updateToken()
{
    curToken = curToken.substr(startofToken, std::string::npos);
    return;
}
std::string JackTokenizer::getCurToken() const
{
    return curToken;
}

