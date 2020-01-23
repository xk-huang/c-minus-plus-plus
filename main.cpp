/*
# encoding=utf-8
# author: xiaoke huang
# date: 2019/12/14
*/

#include <iostream>
#include "analyzer.h"
#include "parser.h"
#include "pcodegen.h"
#include "scanner.h"
#include "symboltable.h"
#include "typecheck.h"

bool error_flag = false;

int main()
{
    std::string s = "", line;
    while (std::getline(std::cin, line))
    {
        if (!s.empty())
            s += "\n";
        s += line;
    }

    // CScanner cs = CScanner(s);
    // for (int i=0;; i++){
    //     TOKEN _tmp = cs.NextToken();
    //     std::string tmp = _tmp.str;
    //     if (_tmp.type == _EOF)
    //         break;
    //     std::cout<< tmp << std::endl;
    // }

    // CParser cp = CParser(s);
    // CTreeNode* cptr = cp.BuildSyntaxTree();
    // cp.PrintTree();

    // CAnalyzer ca = CAnalyzer(s);
    // ca.TraceTypeCheck();

    CPCodeGen pg = CPCodeGen(s);
    pg.GeneratePCode();

    return 0;
}
