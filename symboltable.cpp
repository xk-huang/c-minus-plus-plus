/*
# encoding=utf-8
# author: xiaoke huang
# date: 2019/11/26
*/

#include "symboltable.h"

CSymbolTable::CSymbolTable() { initHashTable(); }

CSymbolTable::~CSymbolTable()
{
    for (int i = 0; i < SIZE; i++)
        if (hashTable[i])
            delete hashTable[i];
}

// hash_ function
int CSymbolTable::hash_(std::string key)
{
    int temp, i;

    for (temp = 0, i = 0; key[i] != '\0'; i++)
        temp = ((temp << SHIFT) + key[i]) % SIZE;

    return temp;
}

void CSymbolTable::initHashTable()
{
    for (int i = 0; i < SIZE; i++)
        hashTable[i] = NULL;
}

void CSymbolTable::deleteHashTable()
{
    for (int i = 0; i < SIZE; i++)
        if (hashTable[i])
        {
            delete hashTable[i];
            hashTable[i] = NULL;
        }
}

// print a string
void CSymbolTable::print_buf(char *format, ...)
{
    va_list params;
    static char buf[1024];

    va_start(params, format);
    std::vsnprintf(buf, 1020, format, params);
    va_end(params);
    printf(buf);
}

// insert a node into the hash_ table,
// memloc is inserted only the first time, otherwise ignored
void CSymbolTable::st_insert(std::string &name, std::string &scope,
                             enum TokenType type, int lineno, int memloc,
                             bool bArray)
{
    int h = hash_(name);
    HashList *l = hashTable[h];

    while (l && ((l->name != name) || (l->scope != scope)))
        l = l->next;
    if (l == NULL)
    {
        // variable not yet in table
        l = new HashList();
        l->name = name;
        l->scope = scope;
        l->type = type;
        l->memloc = memloc;
        l->bArray = bArray;
        l->lineno = new LineList();
        l->lineno->lineno = lineno;
        l->next = hashTable[h];
        hashTable[h] = l;
    }
    else
    {
        // found in table, so just add line number
        LineList *t = l->lineno;
        while (t->next)
            t = t->next;
        t->next = new LineList();
        t->next->lineno = lineno;
    }
}

// lookup a node with specified name and scope from the hash_ table,
// return the memloc
int CSymbolTable::st_lookup(const std::string &name, const std::string &scope)
{
    int h = hash_(name);
    HashList *l = hashTable[h];

    while (l && ((l->name != name) || (l->scope != scope)))
        l = l->next;
    return (l == NULL) ? -1 : l->memloc;
}

// check if it is array
bool CSymbolTable::st_lookup_isarray(const std::string &name,
                                     const std::string &scope)
{
    int h = hash_(name);
    HashList *l = hashTable[h];

    while (l && ((l->name != name) || (l->scope != scope)))
        l = l->next;
    return (l == NULL) ? false : l->bArray;
}

// return the type of the specified table node
enum TokenType CSymbolTable::st_lookup_type(const std::string &name,
                                            const std::string &scope)
{
    int h = hash_(name);
    HashList *l = hashTable[h];

    while (l && ((l->name != name) || (l->scope != scope)))
        l = l->next;
    return (l == NULL) ? _ERROR : l->type;
}

// print a formatted listing of the symbol table contents to lpszPathName
void CSymbolTable::PrintSymbolTable()
{
    print_buf(
        "           Scope        Variable Name     Type     Location    Line "
        "Numbers\r\n");
    print_buf(
        "       -------------    -------------    ------    --------    "
        "------------\r\n");
    for (int i = 0; i < SIZE; i++)
        if (hashTable[i] != NULL)
        {
            HashList *l = hashTable[i];
            while (l)
            {
                print_buf("%18s", (l->scope).c_str());
                print_buf("%17s", (l->name).c_str());
                if (l->type == _LABEL)
                    print_buf("%11s", "label");
                else
                    print_buf("%11s",
                              (ReservedKeywordList[l->type] + ((l->bArray) ? "[]" : ""))
                                  .c_str());
                print_buf("%12d     ", l->memloc);
                LineList *t = l->lineno;
                while (t)
                {
                    print_buf("%3d ", t->lineno);
                    t = t->next;
                }
                print_buf("\r\n");
                l = l->next;
            }
        }
}
