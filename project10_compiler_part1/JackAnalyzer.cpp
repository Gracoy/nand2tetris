#include <iostream>
#include <filesystem>
#include <vector>
#include "CompilationEngine.h"
#include "JackTokenizer.h"

/* 
   Provide the directory of ".jack" files to use this program. Ex: ./JackAnalyzer.exe ArrayTest
   Tokens are updated and parsed by methon "advance() / setToeknType()" in "JackTokenizer.cpp".
   "CompileEngine.cpp" handles different conditions of compilation.
   The method "updateToken()" is called every time when the program gets a token from tokenizer.
*/


void fileOrdir(std::string &s, std::vector<std::string> &files);
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Please provide proper file name or directory." << std::endl;
        exit(1) ;
    }
    
    std::string fileordir = argv[1];
    std::vector<std::string> files;
    fileOrdir(fileordir, files);
    if (files.size() == 0)
    {
        std::cerr << "No \"xxx.jack\" file in directory." << std::endl;
        exit(1);
    }

    for (int i = 0; i < files.size(); i++)
    {
        int slashPos = files[i].find('\\');
        std::string outfileName = files[i].substr(slashPos + 1, files[i].length() - slashPos - 5 - 1) + ".xml";
        JackTokenizer *tokenizer = new JackTokenizer(files[i]);
        CompileEngine *compileEngine = new CompileEngine(tokenizer, outfileName);
        delete tokenizer;
        delete compileEngine;
    }

    return 0;
}
void fileOrdir(std::string &fileordir, std::vector<std::string> &files)
{
    std::string fileExtension = ".jack";
    std::filesystem::path directory(fileordir);

    for (const auto &entry: std::filesystem::directory_iterator(directory))
    {
        if (entry.path().extension().string() == fileExtension)
            files.push_back(entry.path().string());
    }
    return;
}