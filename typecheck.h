#ifndef _FUNARGSCHECK_H_
#define _FUNARGSCHECK_H_

#include <cassert>
#include "parser.h"

extern bool error_flag;

class ParamList
{
public:
    enum TokenType type;
    bool bArray;
    ParamList *next;

public:
    ParamList() : bArray(false), next(NULL) {}
    ParamList(enum TokenType t, bool b) : type(t), bArray(b), next(NULL) {}
    ~ParamList()
    {
        if (next)
            delete next;
    }
};

class FunDecList
{
public:
    std::string name;
    enum TokenType type;
    int count;
    ParamList *params;
    FunDecList *next;

public:
    FunDecList() : count(0), next(NULL) {}
    FunDecList(std::string &s, enum TokenType t) : name(s), type(t), count(0), next(NULL) {}
    ~FunDecList()
    {
        if (next)
            delete next;
    }
};

class TypeCheck
{
public:
    TypeCheck();
    ~TypeCheck();

public:
    void deleteList();

    void fa_insert(CTreeNode *pNode);
    int fa_check(CTreeNode *pNode);

private:
    FunDecList *first, *last;
};

#endif // _FUNARGSCHECK_H_