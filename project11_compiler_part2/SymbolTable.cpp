#include <iostream>
#include "SymbolTable.h"

SymbolTable::SymbolTable()
{
    classST.clear();
    subroutineST.clear();
    classSymbolCnt.clear();
    subroutineSymbolCnt.clear();
}
SymbolTable::~SymbolTable()
{
    std::cout << "Delete symbol table done." << std::endl;
}
void SymbolTable::defineSymbol(const std::string &name, const std::string &type, const ST::symbolKind kind, const std::string &subroutineName)
{
    ST::symbolInfo newSymbol;
    newSymbol.symType = type;
    newSymbol.symKind = kind;
    
    if (kind == ST::STATIC || kind == ST::FIELD)
    {
        newSymbol.symIdx = classSymbolCnt[kind]++;
        classST[name] = newSymbol;
    }
    else
    {
        newSymbol.symIdx = subroutineSymbolCnt[subroutineName][kind]++;
        subroutineST[subroutineName][name] = newSymbol;
    }

    return;
}
int SymbolTable::getVarCount(const ST::symbolKind kind, const std::string &subroutineName)
{
    // If input var kind is not in map, which means subroutine needs no such kind of variable --> return 0
    switch (kind)
    {
        case (ST::STATIC):
        case (ST::FIELD):
            return classSymbolCnt[kind];
        case (ST::ARG):
        case (ST::VAR):
            return subroutineSymbolCnt[subroutineName][kind];
    }

    // This function will never reach here
    return -1;
}
ST::symbolKind SymbolTable::getSymKind(const std::string &name, const std::string &subroutineName) const
{
    if (classST.find(name) != classST.end())
    {
        auto it = classST.find(name);
        if (it != classST.end())
            return it->second.symKind;
    }
    else if (subroutineST.find(subroutineName) != subroutineST.end())
    {
        auto it1 = subroutineST.find(subroutineName);
        if (it1->second.find(name) != it1->second.end())
        {
            auto it2 = it1->second.find(name);
            return it2->second.symKind;
        }
    }
    
    return ST::NONE;
}
std::string SymbolTable::getSymType(const std::string &name, const std::string &subroutineName) const
{
    if (classST.find(name) != classST.end())
    {
        auto it = classST.find(name);
        if (it != classST.end())
            return it->second.symType;
    }
    else if (subroutineST.find(subroutineName) != subroutineST.end())
    {
        auto it1 = subroutineST.find(subroutineName);
        if (it1->second.find(name) != it1->second.end())
        {
            auto it2 = it1->second.find(name);
            return it2->second.symType;
        }
    }
    
    return "";
}
int SymbolTable::getSymIndex(const std::string &name, const std::string &subroutineName) const
{
    if (classST.find(name) != classST.end())
    {
        auto it = classST.find(name);
        if (it != classST.end())
            return it->second.symIdx;
    }
    else if (subroutineST.find(subroutineName) != subroutineST.end())
    {
        auto it1 = subroutineST.find(subroutineName);
        if (it1->second.find(name) != it1->second.end())
        {
            auto it2 = it1->second.find(name);
            return it2->second.symIdx;
        }
    }
    
    std::cerr << "Symbol " << "\"" << name << "\"" << " not defined in symbol table !" << std::endl;
    exit(1);
}
