#include <string>
#include <assert.h>
#include <unordered_map>
#include <unordered_set>
#include "CompilationEngine.h"

std::unordered_map<Tok::keyword, ST::symbolKind> token_symbol_Map    { {Tok::STATIC, ST::STATIC}, {Tok::FIELD, ST::FIELD}, {Tok::VAR, ST::VAR} };
std::unordered_map<ST::symbolKind, CW::segment>  symbol_segmemt_Map  { {ST::STATIC, CW::STATIC}, {ST::FIELD, CW::THIS}, {ST::ARG, CW::ARG}, {ST::VAR, CW::LOCAL} };
std::unordered_map<std::string, CW::oper>        unary_operator_Map  { {"-", CW::NEG}, {"~", CW::NOT}};
std::unordered_map<std::string, CW::oper>        binary_operator_Map { {"*", CW::MUL}, {"/", CW::DIV}, {"%", CW::MOD}, {"+", CW::ADD}, {"-", CW::SUB}, {"=", CW::EQ}, {">", CW::GT}, {"<", CW::LT}, {"&", CW::AND}, {"|", CW::OR}};
std::unordered_set<Tok::keyword> statement     { Tok::LET, Tok::IF, Tok::WHILE, Tok::DO, Tok::RETURN };
std::unordered_set<char>         terminalPunct { ';', '}', ')', ']', ',' };

CompilationEngine::CompilationEngine(JackTokenizer *tok, SymbolTable *st, CodeWriter *cw): className(""), subroutineName(""), nArgs(0), nLabels(0), isWriting(false)
{
    tokenizer   = tok;
    symboltable = st;
    codewriter  = cw;

    compileClass();                  // First scan construct symbol table only
    tokenizer->restartFileStream();  // Move pointer of file stream to start of file
    isWriting = true;                // Start writing VM code
    compileClass();
}
CompilationEngine::~CompilationEngine()
{
    delete tokenizer;
    delete symboltable;
    delete codewriter;
    std::cout << "Delete compilation engine and compile sucess !" << std::endl; 
}
void CompilationEngine::compileClass()
{
    tokenizer->advance();
    while (tokenizer->getKeywordType() == Tok::CLASS)
    {
        assert(tokenizer->getKeyword() == "class");
        tokenizer->advance();

        className =  tokenizer->getIdentifier();
        tokenizer->advance();

        assert(tokenizer->getSymbol() == "{");
        tokenizer->advance();
        while ((tokenizer->getTokenType() == Tok::KEYWORD))
        {
            switch (tokenizer->getKeywordType())
            {
                case (Tok::STATIC):
                case (Tok::FIELD):
                    compileClassVarDec();   
                    break;
                case (Tok::CONSTRUCTOR):
                case (Tok::FUNCTION):
                case (Tok::METHOD):
                    compileSubroutine();
                    break;
                default:
                    std::cerr << "Current token " << tokenizer->getCurToken() << " is not static, field, constructor, function or method." << std::endl;
                    exit(1);
            }
        }
        assert(tokenizer->getSymbol() == "}");
    }
    return;
}
void CompilationEngine::compileClassVarDec()
{
    assert(tokenizer->getKeywordType() == Tok::STATIC || tokenizer->getKeywordType() == Tok::FIELD);
    ST::symbolKind varKind = token_symbol_Map[tokenizer->getKeywordType()];
    tokenizer->updateToken();
    
    tokenizer->advance();
    std::string varType = "";
    // If declared type is primitive (int, char, boolean)
    if (tokenizer->getTokenType() == Tok::KEYWORD)
    {
        varType = tokenizer->getKeyword();
        tokenizer->advance();
    }
    // else if declared type is self-defined
    else if (tokenizer->getTokenType() == Tok::IDENTIFIER)
    {
        varType = tokenizer->getIdentifier();
        tokenizer->advance();
    }

    // static int n;
    // field char c;
    while (tokenizer->getCurToken() != ";")
    {
        switch (tokenizer->getTokenType())
        {
            case (Tok::SYMBOL):
                tokenizer->updateToken();
                tokenizer->advance();
                break;
            case (Tok::IDENTIFIER):
                if (!isWriting)
                    symboltable->defineSymbol(tokenizer->getIdentifier(), varType, varKind, "");
                else
                    tokenizer->updateToken();
                tokenizer->advance();
                break;
        }
    }

    // ";" at the end of declaration
    assert(tokenizer->getSymbol() == ";");
    tokenizer->advance();
    return;
}
void CompilationEngine::compileSubroutine()
{
    assert(tokenizer->getKeywordType() == Tok::CONSTRUCTOR || tokenizer->getKeywordType() == Tok::FUNCTION || tokenizer->getKeywordType() == Tok::METHOD);

    // method / constructor / function
    subroutineType = tokenizer->getKeywordType();
    bool isMethod = false;
    if (subroutineType == Tok::METHOD)
        isMethod = true;

    tokenizer->updateToken();
    tokenizer->advance();

    std::string returnType = "";
    // if declared type is primitive: int, char, boolean
    if (tokenizer->getTokenType() == Tok::KEYWORD)
        returnType = tokenizer->getKeyword();
    // else if declared type is self-defined
    else if (tokenizer->getTokenType() == Tok::IDENTIFIER)
        returnType = tokenizer->getIdentifier();
    tokenizer->advance();

    // function void main() {subroutine body;}
    // constructor Square new(int Ax, int Ay, int Asize) {subroutinebody;}
    subroutineName = className + '.' + tokenizer->getIdentifier();

    if (isMethod && !isWriting)
        symboltable->defineSymbol("this", className, ST::ARG, subroutineName);

    tokenizer->advance();
    assert(tokenizer->getSymbol() == "(");
    tokenizer->advance();
    compileParameterList();
    assert(tokenizer->getSymbol() == ")");
    tokenizer->advance();
    compileSubroutineBody();

    return;
}
void CompilationEngine::compileParameterList()
{
    std::string argType  = "";
    bool        isNewArg = true;

    // ()
    // (int Ax, int Ay, int Asize)
    // (myclass a, char b, boolean c)
    while (tokenizer->getCurToken()[0] != ')')
    {
        switch (tokenizer->getTokenType())
        {
            case (Tok::KEYWORD):
                argType = tokenizer->getKeyword();
                tokenizer->advance();
                isNewArg = false;
                break;
            case (Tok::IDENTIFIER):
                if (isNewArg)
                {
                    argType = tokenizer->getIdentifier();
                    isNewArg = false;
                }
                else
                {
                    if (!isWriting)
                        symboltable->defineSymbol(tokenizer->getIdentifier(), argType, ST::ARG, subroutineName);
                    else
                        tokenizer->updateToken();
                    isNewArg = true;
                }
                tokenizer->advance();
                break;
            case (Tok::SYMBOL):
                tokenizer->updateToken();
                tokenizer->advance();
                break;
            default:
                std::cerr << "Compile parameterList error: Token type not defined" << std::endl;
                exit(1);
                break;
        }
    }

    return;
}
void CompilationEngine::compileSubroutineBody()
{
    assert(tokenizer->getSymbol() == "{");
    tokenizer->advance();
    
    codewriter->writeFunction(subroutineName, symboltable->getVarCount(ST::VAR, subroutineName), isWriting);
    if (subroutineType == Tok::CONSTRUCTOR)
    {
        codewriter->writePush(CW::CONST, symboltable->getVarCount(ST::FIELD, ""), isWriting);
        codewriter->writeCall("Memory.alloc", 1, isWriting);
        codewriter->writePop(CW::POINTER, 0, isWriting);
    }
    else if (subroutineType == Tok::METHOD)
    {
        // Update pointer this = argument[0]
        codewriter->writePush(CW::ARG, 0, isWriting);
        codewriter->writePop(CW::POINTER, 0, isWriting);
    }

    while (tokenizer->getCurToken()[0] != '}')
    {
        // var int a = 10
        if (tokenizer->getKeywordType() == Tok::VAR)
            compileVarDec();
        // let a = 100 + 200
        // while (a < 500)
        else if (statement.find(tokenizer->getKeywordType()) != statement.end())
            compileStatements();
    }

    assert(tokenizer->getSymbol() == "}");
    tokenizer->advance();
    
    return;
}
void CompilationEngine::compileVarDec()
{
    assert(tokenizer->getKeyword() == "var");
    tokenizer->advance();

    std::string varType  = "";
    bool        isNewVar = true;
    
    // "var type varName (, varName)*;"
    // var int i, j
    // var mytype n
    while (tokenizer->getCurToken()[0] != ';')
    {
        switch (tokenizer->getTokenType())
        {
            case (Tok::KEYWORD):
                varType = tokenizer->getKeyword();
                tokenizer->advance();
                isNewVar = false;
                break;
            case (Tok::IDENTIFIER):
                if (isNewVar)
                {
                    varType = tokenizer->getIdentifier();
                    isNewVar = false;
                }
                else
                {
                    if (!isWriting)
                        symboltable->defineSymbol(tokenizer->getIdentifier(), varType, ST::VAR, subroutineName);
                    else
                        tokenizer->updateToken();
                }
                tokenizer->advance();
                break;
            case (Tok::SYMBOL):
                tokenizer->updateToken();
                tokenizer->advance();
                break;
            default:
                std::cerr << "Compile varDec error: Token type not defined" << std::endl;
                exit(1);
                break;   
        }
    }

    assert (tokenizer->getSymbol() == ";");
    tokenizer->advance();

    return;
}
void CompilationEngine::compileStatements()
{
    while (tokenizer->getTokenType() == Tok::KEYWORD && statement.find(tokenizer->getKeywordType()) != statement.end())
    {
        switch (tokenizer->getKeywordType())
        {
            case (Tok::LET):
                compileLet();
                break;
            case (Tok::DO):
                compileDo();
                break;
            case (Tok::WHILE):
                compileWhile();
                break;
            case (Tok::IF):
                compileIf();
                break;
            case (Tok::RETURN):
                compileReturn();
                break;
            default:
                std::cerr << "Compile statements error: Keyword type not defined" << std::endl;
                exit(1);
                break;   
        }
    }

    return;
}
void CompilationEngine::compileLet()
{
    assert(tokenizer->getKeyword() == "let");
    tokenizer->advance();
    
    // "let varName ([expression])? = expression;"
    // let arr[2] = 200
    // let game = SquareGame.new();
    // let x = 100
    std::string varName = tokenizer->getIdentifier();
    tokenizer->advance();

    bool isArray = false;
    if (tokenizer->getCurToken()[0] == '[')
    {
        tokenizer->updateToken();
        tokenizer->advance();
        codewriter->writePush(symbol_segmemt_Map[symboltable->getSymKind(varName, subroutineName)], symboltable->getSymIndex(varName, subroutineName), isWriting);
        compileExpression([this](){compileOr();}, [this](char op){return isOr(op);});
        codewriter->writeArithmetic(CW::ADD, isWriting);
        assert(tokenizer->getSymbol() == "]");
        tokenizer->advance();
        isArray = true;
    }

    assert(tokenizer->getSymbol() == "=");
    tokenizer->advance();
    compileExpression([this](){compileOr();}, [this](char op){return isOr(op);});

    if (isArray)
    {
        codewriter->writePop(CW::TEMP, 0, isWriting);
        codewriter->writePop(CW::POINTER, 1, isWriting);
        codewriter->writePush(CW::TEMP, 0, isWriting);
        codewriter->writePop(CW::THAT, 0, isWriting);
    }
    else
        codewriter->writePop(symbol_segmemt_Map[symboltable->getSymKind(varName, subroutineName)], symboltable->getSymIndex(varName, subroutineName), isWriting);

    assert(tokenizer->getSymbol() == ";");
    tokenizer->advance();

    return;
}
void CompilationEngine::compileDo()
{
    // "do subroutineCall;"
    // do draw();
    // do square.dispose();
    // do Memory.deAlloc(this);
    assert(tokenizer->getKeyword() == "do");
    tokenizer->advance();

    std::string tempSTR = tokenizer->getIdentifier();
    tokenizer->advance();

    if (tokenizer->getCurToken()[0] == '(')
    {
        tokenizer->updateToken();
        tokenizer->advance();
        codewriter->writePush(CW::POINTER, 0, isWriting);  // Push this
        compileExpressionList();
        assert(tokenizer->getSymbol() == ")");
        tokenizer->advance();
        codewriter->writeCall(className + '.' + tempSTR, nArgs + 1, isWriting);
        nArgs = 0;
    }
    else if (tokenizer->getCurToken()[0] == '.')
    {
        tokenizer->updateToken();
        tokenizer->advance();
        std::string functionName = tokenizer->getIdentifier();
        tokenizer->advance();
        tokenizer->updateToken();
        tokenizer->advance();
        if (symboltable->getSymKind(tempSTR, subroutineName) == ST::NONE) // Built-in class methods
        {
            compileExpressionList();
            codewriter->writeCall(tempSTR + '.' + functionName, nArgs, isWriting);
        }
        else                                              // Self-defined class methods
        {
            codewriter->writePush(symbol_segmemt_Map[symboltable->getSymKind(tempSTR, subroutineName)], symboltable->getSymIndex(tempSTR, subroutineName), isWriting);
            compileExpressionList();
            codewriter->writeCall(symboltable->getSymType(tempSTR, subroutineName) + '.' + functionName, nArgs + 1, isWriting);
        }
        assert(tokenizer->getSymbol() == ")");
        tokenizer->advance();
        nArgs = 0;
    }

    assert(tokenizer->getSymbol() == ";");
    tokenizer->advance();
    codewriter->writePop(CW::TEMP, 0, isWriting); // Discard the value return from methond or function
    
    return;
}
void CompilationEngine::compileWhile()
{
    std::string trueLabel  = "while_TRUE." + std::to_string(nLabels);
    std::string falseLabel = "while_FALSE." + std::to_string(nLabels);
    nLabels++;

    // "while (expression) {statements}"
    assert(tokenizer->getKeyword() == "while");
    tokenizer->advance();
    tokenizer->updateToken();
    tokenizer->advance();
    // label_True
    codewriter->writeLabel(trueLabel, isWriting);
    // expression
    compileExpression([this](){compileOr();}, [this](char op){return isOr(op);});
    // not
    codewriter->writeArithmetic(CW::NOT, isWriting);
    // If-go label_False
    codewriter->writeIf(falseLabel, isWriting);
    assert(tokenizer->getSymbol() == ")");
    tokenizer->advance();

    tokenizer->updateToken();
    tokenizer->advance();
    // statements
    compileStatements();
    // goto lable_True
    codewriter->writeGoto(trueLabel, isWriting);
    // label_False
    codewriter->writeLabel(falseLabel, isWriting);

    assert(tokenizer->getSymbol() == "}");
    tokenizer->advance();
    return;
}
void CompilationEngine::compileReturn()
{
    assert(tokenizer->getKeyword() == "return");
    tokenizer->advance();

    if (tokenizer->getCurToken()[0] != ';')
        compileExpression([this](){compileOr();}, [this](char op){return isOr(op);});
    else
        codewriter->writePush(CW::CONST, 0, isWriting);
    codewriter->writeReturn(isWriting);
    
    assert(tokenizer->getSymbol() == ";");
    tokenizer->advance();

    return;
}
void CompilationEngine::compileIf()
{
    std::string trueLabel  = "if_TRUE." + std::to_string(nLabels);
    std::string falseLabel = "if_FALSE." + std::to_string(nLabels);
    nLabels++;

    // "if (expression) {statements} (else {statements})?"
    assert(tokenizer->getKeyword() == "if");
    tokenizer->advance();
    tokenizer->updateToken();
    tokenizer->advance();
    // expression
    compileExpression([this](){compileOr();}, [this](char op){return isOr(op);});
    // not
    codewriter->writeArithmetic(CW::NOT, isWriting);
    // if-goto label_False
    codewriter->writeIf(falseLabel, isWriting);
    assert(tokenizer->getSymbol() == ")");
    tokenizer->advance();

    tokenizer->updateToken();
    tokenizer->advance();
    // if statements
    compileStatements();
    assert(tokenizer->getSymbol() == "}");
    tokenizer->advance();
    // goto label_False
    // label_True
    codewriter->writeGoto(trueLabel, isWriting);
    codewriter->writeLabel(falseLabel, isWriting);

    if (tokenizer->getKeywordType() == Tok::ELSE)
    {   
        tokenizer->updateToken();
        tokenizer->advance();
        tokenizer->updateToken();
        tokenizer->advance();
        // else statements
        compileStatements();
        assert(tokenizer->getSymbol() == "}");
        tokenizer->advance();
    }
    // lable_False
    codewriter->writeLabel(trueLabel, isWriting);

    return;
}
void CompilationEngine::compileExpression(std::function<void()> compileSubExp, std::function<bool(char)> isOper)
{
    compileSubExp();
    while (tokenizer->getTokenType() == Tok::SYMBOL && isOper(tokenizer->getCurToken()[0]))
    {
        // Priority: (*, /, %), (+, -), (=, <, >), (&, |)
        std::string op = tokenizer->getSymbol();
        tokenizer->advance();
        compileSubExp();

        CW::oper operType;
        if (binary_operator_Map.find(op) != binary_operator_Map.end())
            operType = binary_operator_Map[op];
        else
        {
            std::cerr << "Operator error: operator " << "\"" << op << "\"" << " not defined !" << std::endl;
            exit(1);
        }
        switch (operType)
        {
            case(CW::ADD):
            case(CW::SUB):
            case(CW::EQ):
            case(CW::GT):
            case(CW::LT):
            case(CW::AND):
            case(CW::OR):
                codewriter->writeArithmetic(operType, isWriting);
                break;
            case(CW::MUL):
                codewriter->writeCall("Math.multiply", 2, isWriting);
                break;
            case(CW::DIV):
                codewriter->writeCall("Math.divide", 2, isWriting);
                break;
            case(CW::MOD):
                codewriter->writeCall("Math.modulo", 2, isWriting);
                break;
            default:
            std::cerr << "Operator error: operator " << "\"" << op << "\"" << " not defined !" << std::endl;
                exit(1);
            }
    }

    return;
}
void CompilationEngine::compileTerm()
{
    std::string tempSTR = "";
    switch (tokenizer->getTokenType())
    {
        case (Tok::KEYWORD):
            // Keyword constant: true, flase, this, null
            tokenizer->updateToken();
            tokenizer->advance();
            switch (tokenizer->getKeywordType())
            {
                case (Tok::TRUE):
                    codewriter->writePush(CW::CONST, 0, isWriting);
                    codewriter->writeArithmetic(CW::NOT, isWriting);
                    break;
                case (Tok::FALSE):
                case (Tok::null):
                    codewriter->writePush(CW::CONST, 0, isWriting);
                    break;
                case (Tok::THIS):
                    codewriter->writePush(CW::POINTER, 0, isWriting);
                    break;
            }
            break;
        case (Tok::SYMBOL):
            // (expression)
            if (tokenizer->getCurToken()[0] == '(')            
            {
                tokenizer->updateToken();
                tokenizer->advance();
                compileExpression([this](){compileOr();}, [this](char op){return isOr(op);});
                assert(tokenizer->getSymbol() == ")");
                tokenizer->advance();
            }
            // Unary operator: (-, ~)
            else
            {
                tempSTR = tokenizer->getSymbol();
                tokenizer->advance();
                compileTerm();
                codewriter->writeArithmetic(unary_operator_Map[tempSTR], isWriting);
            }
            break;
        case (Tok::INT_CONST):
            // Integer constant
            codewriter->writePush(CW::CONST, tokenizer->getIntVal(), isWriting);
            tokenizer->advance();
            break;
        case (Tok::STR_CONST):
            // String constant
            tempSTR = tokenizer->getStringVal();
            tokenizer->advance();
            codewriter->writePush(CW::CONST, tempSTR.length(), isWriting);
            codewriter->writeCall("String.new", 1, isWriting);
            for (int i = 0; i < tempSTR.length(); i++)
            {
                codewriter->writePush(CW::CONST, (int) tempSTR[i], isWriting);
                codewriter->writeCall("String.appendChar", 2, isWriting);
            }
            break;
        case (Tok::IDENTIFIER):
            tempSTR = tokenizer->getIdentifier();
            tokenizer->advance();
            // Subroutine call --> identifier(expressionList)
            if (tokenizer->getCurToken()[0] == '(')
            {
                tokenizer->updateToken();
                tokenizer->advance();
                // codewriter->writePush(CW::POINTER, 0);
                compileExpressionList();
                assert(tokenizer->getSymbol() == ")");
                tokenizer->advance();
                codewriter->writeCall(className + '.' + tempSTR, nArgs, isWriting);
                nArgs = 0;
            }
            // Subroutine call --> identifier.subroutineName(expressionList)
            else if (tokenizer->getCurToken()[0] == '.')
            {
                tokenizer->updateToken();
                tokenizer->advance();
                std::string methodName = tokenizer->getIdentifier();
                tokenizer->advance();
                tokenizer->updateToken();
                tokenizer->advance();
                // Math.sqrt(n) --> push express --> call Math.sqrt 1
                if (symboltable->getSymKind(tempSTR, subroutineName) == ST::NONE) // Built-in class methods
                {
                    compileExpressionList();
                    codewriter->writeCall(tempSTR + '.' + methodName, nArgs, isWriting);
                }
                // p1.distance(p2) --> push p1 --> push express --> call Point.distance 1 + 1;
                else                                              // Self-defined class methods
                {
                    codewriter->writePush(symbol_segmemt_Map[symboltable->getSymKind(tempSTR, subroutineName)], symboltable->getSymIndex(tempSTR, subroutineName), isWriting);
                    compileExpressionList();
                    codewriter->writeCall(symboltable->getSymType(tempSTR, subroutineName) + '.' + methodName, nArgs + 1, isWriting);
                }
                assert(tokenizer->getSymbol() == ")");
                tokenizer->advance();
                nArgs = 0;    
            }
            // Index operation --> indentifier[expression]
            else if (tokenizer->getCurToken()[0] == '[')
            {
                tokenizer->updateToken();
                tokenizer->advance();
                codewriter->writePush(symbol_segmemt_Map[symboltable->getSymKind(tempSTR, subroutineName)], symboltable->getSymIndex(tempSTR, subroutineName), isWriting);
                compileExpression([this](){compileOr();}, [this](char op){return isOr(op);});
                codewriter->writeArithmetic(CW::ADD, isWriting);
                assert(tokenizer->getSymbol() == "]");
                tokenizer->advance();
                codewriter->writePop(CW::POINTER, 1, isWriting);
                codewriter->writePush(CW::THAT, 0, isWriting);
            }
            else
                codewriter->writePush(symbol_segmemt_Map[symboltable->getSymKind(tempSTR, subroutineName)], symboltable->getSymIndex(tempSTR, subroutineName), isWriting);
            break;
        default:
            std::cerr << "Compile term error: Token type not defined" << std::endl;
            exit(1);
            break;   
    }

    return;
}
void CompilationEngine::compileExpressionList()
{
    while (tokenizer->getCurToken()[0] != ')')
    {
        compileExpression([this](){compileOr();}, [this](char op){return isOr(op);});
        nArgs++;
        if (tokenizer->getCurToken()[0] == ',')
        {
            tokenizer->updateToken();
            tokenizer->advance();
        }
    }

    return;
}
void CompilationEngine::compileOr()
{
    compileExpression([this](){compileAnd();}, [this](char op){return isOr(op);});
    return;
}
void CompilationEngine::compileAnd()
{
    compileExpression([this](){compileCmp();}, [this](char op){return isAnd(op);});
    return;
}
void CompilationEngine::compileCmp()
{
    compileExpression([this](){compileAddSub();}, [this](char op){return isCmp(op);});
    return;
}
void CompilationEngine::compileAddSub()
{
    compileExpression([this](){compileMulDivMod();}, [this](char op){return isAddSub(op);});
    return;
}
void CompilationEngine::compileMulDivMod()
{
    compileExpression([this](){compileTerm();}, [this](char op){return isMulDivMod(op);});
    return;
}
bool CompilationEngine::isOr(char op)
{
    return op == '|';
}
bool CompilationEngine::isAnd(char op)
{
    return op == '&';
}
bool CompilationEngine::isCmp(char op)
{
    return op == '=' || op == '<' || op == '>';
}
bool CompilationEngine::isAddSub(char op)
{
    return op == '+' || op == '-';
}
bool CompilationEngine::isMulDivMod(char op)
{
    return op == '*' || op == '/' || op == '%';
}
