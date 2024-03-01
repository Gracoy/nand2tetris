#ifndef COMPILE_ENGINE_H
#define COMPILE_ENGINE_H

#include <string>
#include "JackTokenizer.h"

class CompileEngine {
private:
    JackTokenizer *tokenizer;
    std::ofstream outfile;
public:
    CompileEngine(JackTokenizer *tok, std::string &outfileName);
    virtual ~CompileEngine();
    
    void compileClass();
    void compileClassVarDec();
    void compileSubroutine();
    void compileParameterList();
    void compileSubroutineBody();
    void compileVarDec();
    void compileStatements();
    void compileDo();
    void compileLet();
    void compileWhile();
    void compileReturn();
    void compileIf();
    void compileExpression();
    void compileTerm();
    void compileExpressionList();
};


#endif