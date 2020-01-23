#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <cstdio>
#include "parser.h"

// SIZE is the size of the hash_ table
#define SIZE 250

// SHIFT is the power of two used as multiplier in hash_ function
#define SHIFT 4

extern bool error_flag;

// the list of line numbers of the source code in which a variable is referenced
class LineList
{
public:
    int lineno;
    LineList *next;

public:
    // initiation
    LineList() : lineno(0), next(NULL) {}
    // for convenient self-destruction
    ~LineList()
    {
        if (next)
            delete next;
    }
};

// The record in the bucket lists for each variable, including name,
// assigned memory location, and the list of line numbers in which
// it appears in the source code
class HashList
{
public:
    std::string name;  // variable name
    std::string scope; // function scope
    enum TokenType type;
    int memloc;  // memory location for variable
    bool bArray; // for array checking
    LineList *lineno;
    HashList *next;

public:
    // initiation
    HashList() : memloc(0), lineno(NULL), next(NULL) {}
    // for convenient self-destruction
    ~HashList()
    {
        if (lineno)
            delete lineno;
        if (next)
            delete next;
    }
};

class CSymbolTable
{
public:
    CSymbolTable();
    ~CSymbolTable();

    // Operations
public:
    void deleteHashTable();

    void st_insert(std::string &name, std::string &scope, enum TokenType type,
                   int lineno, int memloc, bool bArray = false);
    int st_lookup(const std::string &name, const std::string &scope);
    bool st_lookup_isarray(const std::string &name, const std::string &scope);
    enum TokenType st_lookup_type(const std::string &name,
                                  const std::string &scope);
    void PrintSymbolTable();

    // help routines
private:
    inline int hash_(std::string key);
    void initHashTable();

    void print_buf(char *format, ...);

private:
    HashList *hashTable[SIZE];
};

#endif // _SYMBOLTABLE_H_