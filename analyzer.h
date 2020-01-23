#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#include <cstdio>
#include <iostream>
#include <string>
#include "symboltable.h"
#include "typecheck.h"

// build a syntax tree, then do semantic analysis,
// besides construct a symbol table
extern bool error_flag;
class CAnalyzer
{
public:
    CAnalyzer(std::string &str);
    ~CAnalyzer();

    // Attributes
public:
    CTreeNode *pProgram;
    CParser *pParser;
    static CSymbolTable SymbolTable;
    static TypeCheck FunArgs;

    // Operations
public:
    void BuildSymbolTable(CTreeNode *pNode);
    void typeCheck(CTreeNode *pNode);

    void Trace();
    void TraceTypeCheck();

    // help routines
private:
    void traverse(CTreeNode *t,
                  void (*preProc)(CTreeNode *), void (*postProc)(CTreeNode *));
    static void nullProc(CTreeNode *t);
    static void insertNode(CTreeNode *t);
    static void checkNode(CTreeNode *t);

private:
    static int location;
};

#endif // _ANALYZER_H_
