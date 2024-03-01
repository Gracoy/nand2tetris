#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include <iostream>
#include <fstream>
#include <string>
#include "SymbolTable.h"

namespace CW {
    enum segment  { CONST, ARG, LOCAL, STATIC, THIS, THAT, POINTER, TEMP };
    enum oper     { MUL, DIV, MOD, ADD, SUB, EQ, GT, LT, AND, OR, NEG, NOT };
};

class CodeWriter {
private:
    std::ofstream outfile;
public:
    CodeWriter(std::string &outifleName);
    virtual ~CodeWriter();
    
    void writePush(const CW::segment seg, const int idx, bool isWriting);
    void writePop(const CW::segment seg, const int idx, bool isWriting);
    void writeArithmetic(const CW::oper op, bool isWriting);
    void writeLabel(const std::string &label, bool isWriting);
    void writeGoto(const std::string &label, bool isWriting);
    void writeIf(const std::string &label, bool isWriting);
    void writeCall(const std::string &funcName, const int argCnt, bool isWriting);
    void writeFunction(const std::string &funcName, const int varCnt, bool isWriting);
    void writeReturn(bool isWriting);
};

#endif
