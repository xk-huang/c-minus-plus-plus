#ifndef _SCANER_H_
#define _SCANER_H_

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

// now should be defined the token types
#define TT_EOL '\n'
#define TT_EOF -1
#define TT_INTEGER -2
#define TT_REAL -3
#define TT_WORD -4
#define TT_STRING '"'
#define TT_CHAR '\''

extern bool error_flag;
extern std::string ReservedKeywordList[];

extern bool error_flag;

enum TokenType
{
    // reserved Keyword
    _AUTO,
    _DOUBLE,
    _INT,
    _STRUCT,
    _BREAK,
    _ELSE,
    _LONG,
    _SWITCH,
    _CASE,
    _ENUM,
    _REGISTER,
    _TYPEDEF,
    _CHAR,
    _EXTERN,
    _RETURN,
    _UNION,
    _CONST,
    _FLOAT,
    _SHORT,
    _UNSIGNED,
    _CONTINUE,
    _FOR,
    _SIGNED,
    _VOID,
    _DEFAULT,
    _GOTO,
    _SIZEOF,
    _VOLATILE,
    _DO,
    _IF,
    _STATIC,
    _WHILE,
    _READ,
    _WRITE,
    _PRINTF,

    // operations
    ASSIGN,
    PLUS,
    MINUS,
    TIMES,
    DIV,
    MOD,
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_NOT,
    LOGICAL_NOT,
    LT,
    GT,

    // interpunctions
    LPARAN,
    RPARAN,
    LBRACE,
    RBRACE,
    LSQUARE,
    RSQUARE,
    COMMA,
    DOT,
    SEMI,
    COLON,

    // complex operations
    EQ /* == */,
    NEQ /* != */,
    PLUS_PLUS /* ++ */,
    MINUS_MINUS /* -- */,
    PLUS_ASSIGN /* += */,
    MINUS_ASSIGN /* -= */,
    TIMES_ASSIGN /* *= */,
    DIV_ASSIGN /* /= */,
    NGT /* <= */,
    NLT /* >= */,
    LOGICAL_AND /* && */,
    LOGICAL_OR /* || */,

    // others
    _EOF,
    _ID,
    _NUM,
    _STRING,
    _CHARACTER,
    _LABEL,
    _ERROR,
    _NONE
};

struct TOKEN
{
    enum TokenType type;
    std::string str;
};

class CToken
{
public:
    CToken(std::string &_string); // Constructor
    virtual ~CToken();            // Destructor

private:
    // inline function
    inline char GetChar()
    {
        // std::printf("char %c ptr:%d\n", sString[iChar], iChar);
        return sString[iChar++];
    }

private:
    std::string sString; // the tokenized string

    bool bSlSlComments;     // Slash slash comments enabled
    bool bSlStComments;     // Slash star comments enabled
    bool bEolIsSignificant; // Specifies that EOL is significant or not
    bool bForceLower;       // Enable / disable case sensitivity
    bool bPushedBack;       //Enable Pushed Back

    int peekc;
    int iLineNo;
    int tType; // The last read token type

protected:
    std::string sVal; // the value of the token
    double dVal;
    int iChar; // the index of the current character

public:
    double GetDoubleNumValue();
    int GetIntNumValue();
    virtual std::string GetStrValue();

    int LineNo();
    void PushBack();
    int NextToken(); // gives the next Token, returns the token type
};

// class CScanner
#define MAX_KW 100

class CScanner : public CToken
{
public:
    CScanner(std::string &str);
    virtual ~CScanner();

    void MapKeyword();
    void Flush();

    void PushBack();
    TOKEN &NextToken();

private:
    bool bPushedBack;

    TOKEN token;

    std::map<std::string, enum TokenType> KeyIndex;
};

#endif
