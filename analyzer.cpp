/*
# encoding=utf-8
# author: xiaoke huang
# date: 2019/11/28
*/

#include "analyzer.h"
#include <cassert>

// static member variables initiation
CSymbolTable CAnalyzer::SymbolTable;
TypeCheck CAnalyzer::FunArgs;
int CAnalyzer::location;

CAnalyzer::CAnalyzer(std::string &str)
{
    pParser = new CParser(str);
    pProgram = NULL;
    SymbolTable.deleteHashTable();
    FunArgs.deleteList();

    location = 0;
}

CAnalyzer::~CAnalyzer()
{
    if (pParser)
        delete pParser;
    // DO NOT DELETE pProgram, it points to the tree in the pParser,
    // which has been destructed above
}

// build the symbol table
void CAnalyzer::BuildSymbolTable(CTreeNode *pNode)
{
    traverse(pNode, insertNode, nullProc);
}

// Procedure typeCheck performs type checking by a postorder syntax tree traversal
void CAnalyzer::typeCheck(CTreeNode *pNode)
{
    traverse(pNode, nullProc, checkNode);
    // after semantic analysis, check if main() exists
    if (SymbolTable.st_lookup("main", "global") == -1)
        std::printf("\r\nUnresolved external symbol _main");
}

// trace the symbol table
void CAnalyzer::Trace()
{
    // ClearErrFlag();

    std::puts("building syntax tree...");
    pProgram = pParser->BuildSyntaxTree();
    pParser->PrintTree(pProgram);
    if (error_flag)
    {
        std::puts("\r\nerrors occur while parsing, stop constructing symbol table!");
        return;
    }
    else
        std::puts("successfully build the syntax tree!");
    std::puts("\r\nconstructing symbol table...");
    BuildSymbolTable(pProgram);
    // ignore errors
    SymbolTable.PrintSymbolTable();
}

// trace type checking
void CAnalyzer::TraceTypeCheck()
{
    std::puts("building syntax tree...");
    pProgram = pParser->BuildSyntaxTree();

    //print AST
    pParser->PrintTree();

    if (error_flag)
    {
        std::puts("\r\nerrors occur while parsing, stop constructing symbol table!");
        return;
    }
    std::puts("\r\nconstructing symbol table...");
    BuildSymbolTable(pProgram);
    if (error_flag)
    {
        std::puts("\r\nerrors occur while constructing symbol table, stop type checking!");
        return;
    }
    std::puts("\r\ntype checking...");
    typeCheck(pProgram);
    if (error_flag)
        std::puts("\r\nerrors occur while type checking!");
    else
        std::puts("All OK!");
    SymbolTable.PrintSymbolTable();
}

// Procedure traverse is a generic recursive syntax tree traversal routine:
// it applies preProc in preorder and postProc in postorder to tree pointed to by t
void CAnalyzer::traverse(CTreeNode *t,
                         void (*preProc)(CTreeNode *), void (*postProc)(CTreeNode *))
{
    if (t)
    {
        preProc(t);
        for (int i = 0; i < MAX_CHILDREN; i++)
            traverse(t->child[i], preProc, postProc);
        postProc(t);
        traverse(t->sibling, preProc, postProc);
    }
}

// nullProc is a do-nothing procedure to generate preorder-only or
// postorder-only traversals from traverse
void CAnalyzer::nullProc(CTreeNode *t)
{
    return;
}

// Procedure insertNode inserts identifiers stored in t into
// the symbol table
void CAnalyzer::insertNode(CTreeNode *t)
{
    assert(t);
    switch (t->nodekind)
    {
    case kFunDec:
        if (SymbolTable.st_lookup(t->szName, t->szScope) == -1)
        {
            // not defined, so add it to the symbol table
            SymbolTable.st_insert(t->szName, t->szScope, t->type, t->lineno, location++);
            // add it to function declaration list
            FunArgs.fa_insert(t);
        }
        else // redefinition
            error_flag = true, std::printf("error in line %d: function '%s()' redefinition", t->lineno, (t->szName).c_str());
        break;
    case kVarDec:
    case kParam:
        if (SymbolTable.st_lookup(t->szName, t->szScope) == -1)
            // not defined, so add it to the symbol table
            SymbolTable.st_insert(t->szName, t->szScope, t->type, t->lineno, location++, t->bArray);
        else // redefinition
            error_flag = true, std::printf("error in line %d: variable '%s' redefinition", t->lineno, (t->szName).c_str());
        break;
    case kStmt:
        switch (t->kind.stmt)
        {
        case kLabel:
            if (SymbolTable.st_lookup(t->szName, t->szScope) == -1)
                // first time encountered in the scope, add it to the symbol table
                SymbolTable.st_insert(t->szName, t->szScope, _LABEL, t->lineno, location++);
            else // label redifition
                error_flag = true, std::printf("error in line %d: lable '%s' in scope '%s' has already defined",
                                               t->lineno, (t->szName).c_str(), (t->szScope).c_str());
            break;
        case kGoto:
            if (SymbolTable.st_lookup(t->szName, t->szScope) == -1)
                // label undeclared
                error_flag = true, std::printf("error in line %d: lable '%s' in scope '%s' undeclared",
                                               t->lineno, (t->szName).c_str(), (t->szScope).c_str());
            else
                SymbolTable.st_insert(t->szName, t->szScope, _LABEL, t->lineno, 0);
            break;
        case kCall:
            if (SymbolTable.st_lookup(t->szName, t->szScope) == -1)
                error_flag = true, std::printf("error in line %d: unresolved external symbol %s", t->lineno, (t->szName).c_str());
            else
                SymbolTable.st_insert(t->szName, t->szScope, _ID, t->lineno, 0);
        default:
            break;
        }
        break;
    case kExp:
        switch (t->kind.exp)
        {
        case kID:
            if (SymbolTable.st_lookup(t->szName, t->szScope) == -1 &&
                SymbolTable.st_lookup(t->szName, "global") == -1)
                // undeclared
                error_flag = true, std::printf("error in line %d: '%s': undeclared identifier", t->lineno, (t->szName).c_str());
            else if (SymbolTable.st_lookup(t->szName, t->szScope) != -1)
            {
                // local variable
                if (t->father && (t->father->nodekind != kStmt || t->father->kind.stmt != kCall) /* not in call statement */ &&
                    t->bArray != SymbolTable.st_lookup_isarray(t->szName, t->szScope))
                {
                    // one is array but the other is not
                    error_flag = true, std::printf("error in line %d: '%s' is%sdeclared as array", t->lineno,
                                                   (t->szName).c_str(), t->bArray ? " not " : " ");
                    break;
                }
                SymbolTable.st_insert(t->szName, t->szScope, t->type, t->lineno, 0);
            }
            else
            { // global variable
                t->szScope = ("global");
                if (t->father && (t->father->nodekind != kStmt || t->father->kind.stmt != kCall) /* not in call statement */ &&
                    t->bArray != SymbolTable.st_lookup_isarray(t->szName, t->szScope))
                {
                    // one is array but the other is not
                    error_flag = true, std::printf("error in line %d: '%s' is%sdeclared as array", t->lineno,
                                                   (t->szName).c_str(), t->bArray ? " not " : " ");
                    break;
                }
                SymbolTable.st_insert(t->szName, t->szScope, t->type, t->lineno, 0);
            }
            break;
        default:
            break;
        }
    default:
        break;
    }
}

// Procedure checkNode performs type checking at a single tree node
void CAnalyzer::checkNode(CTreeNode *t)
{
    CTreeNode *p = t;
    int ret;

    switch (t->nodekind)
    {
    case kStmt:
        switch (t->kind.stmt)
        {
        case kRead:
            if (t->child[0] == NULL)
            {
                std::printf("fatal error: compiler error!");
                break;
            }
            t->type = t->child[0]->type;
            if (t->type != _CHAR && t->type != _INT && t->type != _FLOAT)
                error_flag = true, std::printf("error in line %d: read a character, int or float number", t->lineno);
            break;
        case kWrite:
            if (t->child[0] == NULL)
            {
                std::printf("fatal error: compiler error!");
                break;
            }
            t->type = t->child[0]->type;
            if (t->type != _CHAR && t->type != _INT && t->type != _FLOAT)
                error_flag = true, std::printf("error in line %d: read a character, int or float number", t->lineno);
            break;
        case kReturn:
            if (t->child[0] == NULL)
            { // 'return' returns 'void'
                if (SymbolTable.st_lookup_type(t->szName, "global") != _VOID)
                    error_flag = true, std::printf("error in line %d : function '%s' must return a value",
                                                   t->lineno, (t->szName).c_str());
            }
            break;
        case kBreak:
            while (p->father && (p->father->nodekind != kStmt || p->father->kind.stmt != kWhile))
                p = p->father;
            if (p->father == NULL)
                // 'break' is not within a while statment
                error_flag = true, std::printf("error in line %d: illegal 'break'", t->lineno);
            break;
        case kContinue: // treat it like kBreak
            while (p->father && (p->father->nodekind != kStmt || p->father->kind.stmt != kWhile))
                p = p->father;
            if (p->father == NULL)
                // 'continue' is not within a while statment
                error_flag = true, std::printf("error in line %d: illegal 'continue'", t->lineno);
            break;
        case kCall:
            // check if its arguments match declaration
            ret = FunArgs.fa_check(t);
            if (ret != -3)
            { // errors
                if (ret >= 0)
                    error_flag = true, std::printf("error in line %d: function '%s()' takes %d parameters", t->lineno, (t->szName).c_str(), ret);
                else if (ret == -1)
                    error_flag = true, std::printf("error in line %d: function '%s()' is not declared", t->lineno, (t->szName).c_str());
                else
                    error_flag = true, std::printf("error in line %d: arguments type not match with function '%s()' declaration", t->lineno, (t->szName).c_str());
                break;
            }
            t->type = SymbolTable.st_lookup_type(t->szName, t->szScope);
            break;
        default:
            break;
        }
        break;
    case kExp:
        switch (t->kind.exp)
        {
        case kOp: // assign a type to this op node
            if (t->type == LOGICAL_NOT || t->type == ASSIGN)
                t->type = t->child[0]->type;
            else
            {
                // the others are binary operations,
                // and the type should be the subexpression with higher precision
                if (t->child[0]->type == _VOID || t->child[1]->type == _VOID)
                    error_flag = true, std::printf("error in line %d: illegal use of type 'void'", t->lineno);
                else if (t->child[0]->type == _FLOAT || t->child[1]->type == _FLOAT)
                    t->type = _FLOAT;
                else if (t->child[0]->type == _INT || t->child[1]->type == _INT)
                    t->type = _INT;
                else
                    t->type = _CHAR;
            }
            break;
        case kID:
            // find the type in the symbol table, assign it to the node
            t->type = SymbolTable.st_lookup_type(t->szName, t->szScope);
            t->bArray = SymbolTable.st_lookup_isarray(t->szName, t->szScope);
            break;
        default:
            break;
        }
    default:
        break;
    }
}
