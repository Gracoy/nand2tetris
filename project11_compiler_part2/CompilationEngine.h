#ifndef COMPILATION_ENGINE_H
#define COMPILATION_ENGINE_H

#include <string>
#include <functional>
#include "JackTokenizer.h"
#include "SymbolTable.h"
#include "CodeWriter.h"

class CompilationEngine {
private:
    JackTokenizer *tokenizer;
    SymbolTable   *symboltable;
    CodeWriter    *codewriter;
    std::string    className;
    Tok::keyword   subroutineType;
    std::string    subroutineName;
    int            nArgs;
    int            nLabels;
    bool           isWriting;
public:
    CompilationEngine(JackTokenizer *tok, SymbolTable *st, CodeWriter *cw);
    virtual ~CompilationEngine();
    
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
    void compileExpression(std::function<void()> compileSubExp, std::function<bool(char)> isOper);
    void compileTerm();
    void compileExpressionList();
    void compileOr();
    void compileAnd();
    void compileCmp();
    void compileAddSub();
    void compileMulDivMod();
    bool isOr(char op);
    bool isAnd(char op);
    bool isCmp(char op);
    bool isAddSub(char op);
    bool isMulDivMod(char op);
};

#endif
