#include <string>
#include <assert.h>
#include <unordered_set>
#include "CompilationEngine.h"

std::unordered_set<keyword> classVarDec {STATIC, FIELD};
std::unordered_set<keyword> subroutineDec {CONSTRUCTOR, FUNCTION, METHOD};
std::unordered_set<keyword> statement {LET, IF, WHILE, DO, RETURN};
std::unordered_set<char>    terminalPunct {';', '}', ')', ']', ','};

CompileEngine::CompileEngine(JackTokenizer *tok, std::string &outfileName)
{
    tokenizer = tok;
    outfile.open(outfileName);
    if (!outfile)
    {
        std::cerr << "CompileEngine: Open output file error !" << std::endl;
        exit(1);
    }
    compileClass();
}
CompileEngine::~CompileEngine()
{
    if (outfile)
    {
        outfile.close();
        std::cout << "Compile sucess !" << std::endl; 
    }
}
void CompileEngine::compileClass()
{
    tokenizer->advance();
    while (tokenizer->getKeywordType() == CLASS)
    {
        outfile << "<class>" << std::endl;

        // class Main {classVarDec* subroutine*}
        outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
        tokenizer->advance();
        outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
        tokenizer->advance();
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();

        while ((tokenizer->getTokenType() == KEYWORD) && (classVarDec.find(tokenizer->getKeywordType()) != classVarDec.end() || subroutineDec.find(tokenizer->getKeywordType()) != subroutineDec.end()))
        {
            switch (tokenizer->getKeywordType())
            {
                case (STATIC):
                case (FIELD):
                    compileClassVarDec();   
                    break;
                case (CONSTRUCTOR):
                case (FUNCTION):
                case (METHOD):
                    compileSubroutine();
                    break;
                default:
                    break;
            }
        }

        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        outfile << "</class>" << std::endl;;
    }
    return;
}
void CompileEngine::compileClassVarDec()
{
    outfile << "<classVarDec>" << std::endl;

    // static / field
    outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
    tokenizer->advance();

    // if declared type is primitive
    if (tokenizer->getTokenType() == KEYWORD)
    {
        outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
        tokenizer->advance();
    }
    // else if declared type is self-defined
    else if (tokenizer->getTokenType() == IDENTIFIER)
    {
        outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
        tokenizer->advance();
    }

    // static int n;
    // field char c;
    while (tokenizer->getCurToken() != ";")
    {
        switch (tokenizer->getTokenType())
        {
            case (SYMBOL):
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                break;
            case (IDENTIFIER):
                outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
                tokenizer->advance();
                break;
        }
    }

    // ";" at the end of declaration
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    outfile << "</classVarDec>" << std::endl;
    return;
}
void CompileEngine::compileSubroutine()
{
    outfile << "<subroutineDec>" << std::endl;

    // method / constructor / function
    outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
    tokenizer->advance();

    // if declared type is primitive: int, char, boolean
    if (tokenizer->getTokenType() == KEYWORD)
    {
        outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
        tokenizer->advance();
    }
    // else if declared type is self-defined
    else if (tokenizer->getTokenType() == IDENTIFIER)
    {
        outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
        tokenizer->advance();
    }

    // function void main() {subroutine body;}
    // constructor Square new(int Ax, int Ay, int Asize) {subroutinebody;}
    outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
    tokenizer->advance();
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    compileParameterList();
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    compileSubroutineBody();

    outfile << "</subroutineDec>" << std::endl;

    return;
}
void CompileEngine::compileParameterList()
{
    outfile << "<parameterList>" << std::endl;

    // ()
    // (int Ax, int Ay, int Asize)
    // (myclass a, char b, boolean c)
    while (tokenizer->getCurToken() != ")")
    {
        switch (tokenizer->getTokenType())
        {
            case (KEYWORD):
                outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
                tokenizer->advance();
                break;
            case (IDENTIFIER):
                outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
                tokenizer->advance();
                break;
            case (SYMBOL):
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                break;
            default:
                std::cerr << "Compile parameterList error: Token type not defined" << std::endl;
                exit(1);
                break;
        }
    }

    outfile << "</parameterList>" << std::endl;
    return;
}
void CompileEngine::compileSubroutineBody()
{
    outfile << "<subroutineBody>" << std::endl;

    // "{ varDec* statemets }"
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();

    while (tokenizer->getCurToken()[0] != '}')
    {
        // var int a = 10
        if (tokenizer->getKeywordType() == VAR)
            compileVarDec();
        // let a = 100 + 200
        // while (a < 500)
        else if (statement.find(tokenizer->getKeywordType()) != statement.end())
            compileStatements();
    }

    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    
    outfile << "</subroutineBody>" << std::endl;
    return;
}
void CompileEngine::compileVarDec()
{
    outfile << "<varDec>" << std::endl;
    
    // "var type varName (, varName)*;"
    // var int i, j
    // var mytype n
    while (tokenizer->getCurToken()[0] != ';')
    {
        switch (tokenizer->getTokenType())
        {
            case (KEYWORD):
                outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
                tokenizer->advance();
                break;
            case (IDENTIFIER):
                outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
                tokenizer->advance();
                break;
            case (SYMBOL):
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                break;
            default:
                std::cerr << "Compile varDec error: Token type not defined" << std::endl;
                exit(1);
                break;   
        }
    }
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    outfile << "</varDec>" << std::endl;
    return;
}
void CompileEngine::compileStatements()
{
    outfile << "<statements>" << std::endl;

    while (tokenizer->getTokenType() == KEYWORD)
    {
        switch (tokenizer->getKeywordType())
        {
            case (LET):
                compileLet();
                break;
            case (DO):
                compileDo();
                break;
            case (WHILE):
                compileWhile();
                break;
            case (IF):
                compileIf();
                break;
            case (RETURN):
                compileReturn();
                break;
            default:
                std::cerr << "Compile statements error: Keyword type not defined" << std::endl;
                exit(1);
                break;   
        }
    }

    outfile << "</statements>" << std::endl;
    return;
}
void CompileEngine::compileLet()
{
    outfile << "<letStatement>" << std::endl;

    // "let varName ([expression])? = expression;"
    // let arr[2] = 200
    // let game = SquareGame.new();
    // let x = 100
    outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
    tokenizer->advance();
    outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
    tokenizer->advance();

    if (tokenizer->getCurToken()[0] == '[')
    {
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
        compileExpression();
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
    }
    if (tokenizer->getCurToken()[0] == '=')
    {
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
        compileExpression();
    }

    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();

    outfile << "</letStatement>" << std::endl;
}
void CompileEngine::compileDo()
{
    outfile << "<doStatement>" << std::endl;

    // "do subroutineCall;"
    // do draw();
    // do square.dispose();
    // do Memory.deAlloc(this);
    outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
    tokenizer->advance();
    outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
    tokenizer->advance();

    if (tokenizer->getCurToken()[0] == '(')
    {
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
        compileExpressionList();
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
    }
    else if (tokenizer->getCurToken()[0] == '.')
    {
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
        outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
        tokenizer->advance();
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
        compileExpressionList();
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
    }

    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    outfile << "</doStatement>" << std::endl;
    return;
}
void CompileEngine::compileWhile()
{
    outfile << "<whileStatement>" << std::endl;

    // "while (expression) {statements}"
    outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
    tokenizer->advance();

    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    compileExpression();
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();

    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    compileStatements();
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();

    outfile << "</whileStatement>" << std::endl;
    return;
}
void CompileEngine::compileReturn()
{
    outfile << "<returnStatement>" << std::endl;

    // "return expression?;"
    outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
    tokenizer->advance();

    if (tokenizer->getCurToken()[0] != ';')
        compileExpression();
    
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();

    outfile << "</returnStatement>" << std::endl;
    return;
}
void CompileEngine::compileIf()
{
    outfile << "<ifStatement>" << std::endl;

    // "if (expression) {statements} (else {statements})?"
    outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
    tokenizer->advance();

    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    compileExpression();
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();

    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();
    compileStatements();
    outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
    tokenizer->advance();

    if (tokenizer->getKeywordType() == ELSE)
    {
        outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
        tokenizer->advance();
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
        compileStatements();
        outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
        tokenizer->advance();
    }

    outfile << "</ifStatement>" << std::endl;
    return;
}
void CompileEngine::compileExpression()
{
    outfile << "<expression>" << std::endl;

    compileTerm();
    while (terminalPunct.find(tokenizer->getCurToken()[0]) == terminalPunct.end())
    {
        switch (tokenizer->getTokenType())
        {
            case (SYMBOL):
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                compileTerm();
                break;
            default:
                compileTerm();
                break;
        }
    }

    outfile << "</expression>" << std::endl;
    return;
}
void CompileEngine::compileTerm()
{
    outfile << "<term>" << std::endl;

    switch (tokenizer->getTokenType())
    {
        case (KEYWORD):
            // Keyword constant: true, flase, this, null
            outfile << "<keyword> " << tokenizer->getKeyword() << " </keyword>" << std::endl;
            tokenizer->advance();
            break;
        case (SYMBOL):
            // (expression)
            if (tokenizer->getCurToken()[0] == '(')
            {
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                compileExpression();
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
            }
            // Operator: binary: (+, -, *, /, &, |, <, >, =), unary: (-, ~)
            else
            {
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                compileTerm();
            }
            break;
        case(INT_CONST):
            // Integer constant
            outfile << "<integerConstant> " << tokenizer->getIntVal() << " </integerConstant>" << std::endl;
            tokenizer->advance();
            break;
        case (STR_CONST):
            // String constant
            outfile << "<stringConstant> " << tokenizer->getStringVal() << " </stringConstant>" << std::endl;
            tokenizer->advance();
            break;
        case (IDENTIFIER):
            outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
            tokenizer->advance();
            // Subroutine call --> identifier(expressionList)
            if (tokenizer->getCurToken()[0] == '(')
            {
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                compileExpressionList();
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
            }
            // Subroutine call --> identifier.subroutineName(expressionList)
            else if (tokenizer->getCurToken()[0] == '.')
            {
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                outfile << "<identifier> " << tokenizer->getIdentifier() << " </identifier>" << std::endl;
                tokenizer->advance();
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                compileExpressionList();
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
            }
            // Index operation --> indentifier[expression]
            else if (tokenizer->getCurToken()[0] == '[')
            {
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
                compileExpression();
                outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
                tokenizer->advance();
            }
            break;
        default:
            std::cerr << "Compile term error: Token type not defined" << std::endl;
            exit(1);
            break;   
    }

    outfile << "</term>" << std::endl;
    return;
}
void CompileEngine::compileExpressionList()
{
    outfile << "<expressionList>" << std::endl;

    while (tokenizer->getCurToken()[0] != ')')
    {
        compileExpression();
        if (tokenizer->getCurToken()[0] == ',')
        {
            outfile << "<symbol> " << tokenizer->getSymbol() << " </symbol>" << std::endl;
            tokenizer->advance();
        }
    }

    outfile << "</expressionList>" << std::endl;
    return;
}
