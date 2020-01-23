#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdarg.h>
#include <iostream>
#include "scanner.h"

extern bool error_flag;
typedef enum
{
    kVarDec,
    kFunDec,
    kParam,
    kStmt,
    kExp
} NodeKind;

typedef enum
{
    kIf,
    kWhile,
    kGoto,
    kBreak,
    kContinue,
    kRead,
    kWrite,
    kPrintf,
    kReturn,
    kLabel,
    kCall
} StmtKind;

typedef enum
{
    kOp,
    kConst,
    kID
} ExpKind;

// parse-tree node
#define MAX_CHILDREN 10

class CTreeNode
{
public:
    CTreeNode() : father(NULL), sibling(NULL), lineno(0), bArray(false)
    {
        for (int i = 0; i < MAX_CHILDREN; i++)
            child[i] = NULL;
    }
    ~CTreeNode()
    { // for convenient self destruction
        for (int i = 0; i < MAX_CHILDREN; i++)
            if (child[i])
                delete child[i];
        if (sibling)
            delete sibling;
        /* DO NOT DELETE father */
    }

    CTreeNode *LastSibling()
    { // get last sibling
        CTreeNode *last = this;
        while (last->sibling)
            last = last->sibling;
        return last;
    }

public:
    CTreeNode *child[MAX_CHILDREN]; // point to child node
    CTreeNode *father;              // point to father node
    CTreeNode *sibling;             // point to sibling node
    int lineno;
    NodeKind nodekind;
    union {
        StmtKind stmt;
        ExpKind exp;
    } kind;
    enum TokenType type;
    std::string szName;
    std::string szScope; // node function scope
    bool bArray;         // is this an array declaration
    int iArraySize;      // array size
};

// class CParser
class CParser
{
public:
    CParser(std::string &str);
    virtual ~CParser();

    CTreeNode *BuildSyntaxTree();
    void PrintTree();
    void PrintTree(CTreeNode *pNode);

    // Attributes
public:
    CTreeNode *pProgram;

    // Operations
private:
    // help routines
    CTreeNode *newNode(NodeKind kind, enum TokenType type, std::string &ID);
    CTreeNode *newStmtNode(StmtKind kind, const std::string &ID);
    CTreeNode *newExpNode(ExpKind kind, enum TokenType type, std::string &ID);

    bool match(enum TokenType type);
    void ConsumeUntil(enum TokenType type);
    void ConsumeUntil(enum TokenType type1, enum TokenType type2);

    // Grammar functions
    CTreeNode *program();
    CTreeNode *declaration_list();
    CTreeNode *declaration();
    CTreeNode *var_declaration();
    CTreeNode *fun_declaration();
    CTreeNode *params();
    CTreeNode *compound_stmt();
    CTreeNode *local_declarations();
    CTreeNode *read_stmt();
    CTreeNode *write_stmt();
    CTreeNode *printf_stmt();
    CTreeNode *expression_stmt();
    CTreeNode *subcompound_stmt();
    CTreeNode *if_stmt();
    CTreeNode *while_stmt();
    CTreeNode *for_stmt();
    CTreeNode *goto_stmt();
    CTreeNode *break_stmt();
    CTreeNode *continue_stmt();
    CTreeNode *return_stmt();
    CTreeNode *expression();
    CTreeNode *logic1_expression();
    CTreeNode *logic2_expression();
    CTreeNode *simple_expression();
    CTreeNode *additive_expression();
    CTreeNode *term();
    CTreeNode *logic3_expression();
    CTreeNode *factor();
    CTreeNode *var();
    CTreeNode *call();
    CTreeNode *args();

    void print_buf(char *format, ...);
    void print_buf(std::string &str);

private:
    CScanner *pScaner;
    struct TOKEN token, TypeToken, IDToken; // the latter two are for temporary use
    std::string szScope;

    int indent; // for indent of output
};

#endif // _PARSER_H_