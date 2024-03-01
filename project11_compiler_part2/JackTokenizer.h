#ifndef JACKTOKENIZER_H
#define JACKTOKENIZER_H

#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>

namespace Tok {
enum token   { KEYWORD, SYMBOL, INT_CONST, STR_CONST, IDENTIFIER };
enum keyword { CLASS, CONSTRUCTOR, FUNCTION, METHOD, FIELD, STATIC, VAR, INT, CHAR, 
               BOOLEAN, VOID, TRUE, FALSE, null, THIS, LET, DO, IF, WHILE, ELSE, RETURN };
};

class JackTokenizer {
private:
    int startofToken;               // Used for tracing the new position that the token should start when calling updateToken()
    Tok::token tokenType;
    Tok::keyword kwdType;
    std::string returnString;       // Store the content of string to be accessed by compile engine
    std::string curToken;
    std::ifstream infile;
public:
    JackTokenizer(std::string &infile_name);
    virtual ~JackTokenizer();

    // Methods to manipulate infile stream and parse token
    // setTokenType() defines and set the new value of startofToken
    // updateToken() was called when compile engine get a return value from tokenizer
    void restartFileStream();
    bool hasMoreTokens() const;
    bool isComment1() const;
    bool isComment2() const;
    void advance();
    void setTokenType();

    // Access methods, return private variables to compile engine
    Tok::token       getTokenType() const;
    Tok::keyword     getKeywordType() const;
    std::string getKeyword();
    std::string getSymbol();
    int         getIntVal();
    std::string getStringVal();
    std::string getIdentifier();
    std::string getCurToken() const;

    // Update the current token by curToken = curToken.substr(startofToken)
    void updateToken();
};

#endif
