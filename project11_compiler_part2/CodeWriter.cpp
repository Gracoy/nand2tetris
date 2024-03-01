#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include "CodeWriter.h"

std::unordered_map<CW::segment, std::string> segmentMap  { {CW::CONST, "constant"}, {CW::ARG, "argument"}, {CW::LOCAL, "local"}, {CW::STATIC, "static"},
                                                           {CW::THIS, "this"}, {CW::THAT, "that"}, {CW::POINTER, "pointer"}, {CW::TEMP, "temp"}};
std::unordered_map<CW::oper, std::string>    operatorMap { {CW::ADD, "add"}, {CW::SUB, "sub"}, {CW::EQ, "eq"}, {CW::GT, "gt"}, {CW::LT, "lt"}, {CW::AND, "and"}, {CW::OR, "or"}, 
                                                           {CW::NEG, "neg"}, {CW::NOT, "not"}};

CodeWriter::CodeWriter(std::string &outfileName)
{
    outfile.open(outfileName);
    if (!outfile)
    {
        std::cerr << "Open output file \"" << outfileName << "\" fail." << std::endl;
        exit(1);
    }
}
CodeWriter::~CodeWriter()
{
    if (outfile)
    {
        outfile.close();
        std::cout << "Delete code writer and close output file success !" << std::endl;
    }
}
void CodeWriter::writePush(const CW::segment seg, const int idx, bool isWriting)
{
    if (isWriting)
        outfile << "push " << segmentMap[seg] << ' ' << idx << std::endl;

    return;
}
void CodeWriter::writePop(const CW::segment seg, const int idx, bool isWriting)
{
    if (isWriting)
        outfile << "pop " << segmentMap[seg] << ' ' << idx << std::endl;

    return;
}
void CodeWriter::writeArithmetic(const CW::oper op, bool isWriting)
{
    if (isWriting)
        outfile << operatorMap[op] << std::endl;

    return;
}
void CodeWriter::writeLabel(const std::string &label, bool isWriting)
{
    if (isWriting)
        outfile << "label " << label << std::endl;

    return;
}
void CodeWriter::writeGoto(const std::string &label, bool isWriting)
{
    if (isWriting)
        outfile << "goto " << label << std::endl;

    return;
}
void CodeWriter::writeIf(const std::string &label, bool isWriting)
{
    if (isWriting)
        outfile << "if-goto " << label << std::endl;

    return;
}
void CodeWriter::writeCall(const std::string &funName, const int argCnt, bool isWriting)
{
    if (isWriting)
        outfile << "call " << funName << ' ' << argCnt << std::endl;

    return;
}
void CodeWriter::writeFunction(const std::string &funName, const int varCnt, bool isWriting)
{
    if (isWriting)
        outfile << "function " << funName << ' ' << varCnt << std::endl;

    return;
}
void CodeWriter::writeReturn(bool isWriting)
{
    if (isWriting)
        outfile << "return" << std::endl;

    return;
}
