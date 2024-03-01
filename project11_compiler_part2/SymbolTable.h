#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>

namespace ST {
    enum symbolKind { STATIC, FIELD, ARG, VAR, NONE };
    typedef struct {
        std::string symType;
        symbolKind  symKind;
        int         symIdx;
    } symbolInfo;
};

class SymbolTable {
private:
    std::unordered_map<std::string, ST::symbolInfo>                                  classST;
    std::unordered_map<std::string, std::unordered_map<std::string, ST::symbolInfo>> subroutineST;
    std::unordered_map<ST::symbolKind, int>                                          classSymbolCnt;
    std::unordered_map<std::string, std::unordered_map<ST::symbolKind, int>>         subroutineSymbolCnt;

public:
    SymbolTable();
    virtual ~SymbolTable();

    // void resetSubroutineTable();
    void defineSymbol(const std::string &name, const std::string &type, const ST::symbolKind kind, const std::string &subroutineName);
    int getVarCount(const ST::symbolKind kind, const std::string &subroutineName);
    ST::symbolKind getSymKind(const std::string &name, const std::string &subroutineName) const;
    std::string getSymType(const std::string &name, const std::string &subroutineName) const;
    int getSymIndex(const std::string &name, const std::string &subroutineName) const;
};

#endif
