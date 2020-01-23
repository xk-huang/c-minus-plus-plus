/*
# encoding=utf-8
# author: xiaoke huang
# date: 2019/10/12
*/

#include "scanner.h"

std::string ReservedKeywordList[] =
    {
        "auto", "double", "int", "struct",
        "break", "else", "long", "switch",
        "case", "enum", "register", "typedef",
        "char", "extern", "return", "union",
        "const", "float", "short", "unsigned",
        "continue", "for", "signed", "void",
        "default", "goto", "sizeof", "volatile",
        "do", "if", "static", "while",
        "read", "write", "printf",

        // operations
        "=", "+", "-", "*", "/", "%", "&", "|", "~", "!", "<", ">",

        // interpunctions
        "(", ")", "{", "}", "[", "]", ",", ".", ";", ":"};

CScanner::CScanner(std::string &str) : CToken(str)
{
    bPushedBack = false;
    token.type = _ID;

    MapKeyword();
}

CScanner::~CScanner()
{
}

// map keyword to TokenType
void CScanner::MapKeyword()
{
    // KeyIndex.RemoveAll();
    // KeyIndex.InitHashTable( MAX_KW );
    KeyIndex.clear();

    for (int i = 0; i < sizeof(ReservedKeywordList) / sizeof(*ReservedKeywordList); i++)
    {
        KeyIndex[ReservedKeywordList[i]] = (enum TokenType)i;
        // std::printf("DEBUG: %s --- %d\n", ReservedKeywordList[i].c_str(), i);//debug
    }
}

void CScanner::Flush()
{
    // if( fTraceFile.hFile == CFile::hFileNull ) return;
    // fTraceFile.Flush();
}

// get the next token
TOKEN &CScanner::NextToken()
{
    if (bPushedBack)
    {
        bPushedBack = false;
        return token;
    }
    if (token.type == _EOF)
        return token;

    enum TokenType type;
    int val = CToken::NextToken();
    int lineno = LineNo(); // the actual line the token is got from

    token.type = _ERROR;
    token.str = "error";

    if (val == TT_EOF)
    {
        token.type = _EOF;
        token.str = "EOF";
        return token;
    }

    if (val == TT_WORD)
    {
        // if( KeyIndex.Lookup( sVal, type ) )
        // std::cout<<"test:"<< KeyIndex[sVal];
        if (KeyIndex.count(sVal))
            token.type = KeyIndex[sVal];
        else
            token.type = _ID;
        token.str = sVal;
    }
    else if (val == TT_INTEGER || val == TT_REAL)
    {
        token.type = _NUM;
        token.str = GetStrValue();
    }
    else if (val == TT_STRING)
    {
        token.type = _STRING;
        token.str = sVal;
    }
    else if (val == TT_CHAR)
    {
        token.type = _CHARACTER;
        // token.str = (sVal.IsEmpty()) ? " " : std::string( sVal[0] );
        token.str = (sVal.empty()) ? " " : std::string(std::to_string(sVal[0]));
    }
    else if (val == TT_EOL)
    {
        return NextToken();
    }
    else if (::strchr("=+-*/&|~!<>(){}[],.;:", val))
    {
        token.str = (char)val;
        token.type = KeyIndex[token.str];

        // complex operations
        switch (val)
        {
        case '=':
            if (CToken::NextToken() == '=')
            {
                token.str = "==";
                token.type = EQ;
            }
            else
                CToken::PushBack();
            break;
        case '!':
            if (CToken::NextToken() == '=')
            {
                token.str = "!=";
                token.type = NEQ;
            }
            else
                CToken::PushBack();
            break;
        case '+':
            if (CToken::NextToken() == '=')
            {
                token.str = "+=";
                token.type = PLUS_ASSIGN;
            }
            else
            {
                CToken::PushBack();
                if (CToken::NextToken() == '+')
                {
                    token.str = "++";
                    token.type = PLUS_PLUS;
                }
                else
                    CToken::PushBack();
            }
            break;
        case '-':
            if (CToken::NextToken() == '=')
            {
                token.str = "-=";
                token.type = MINUS_ASSIGN;
            }
            else
            {
                CToken::PushBack();
                if (CToken::NextToken() == '-')
                {
                    token.str = "--";
                    token.type = MINUS_MINUS;
                }
                else
                    CToken::PushBack();
            }
            break;
        case '*':
            if (CToken::NextToken() == '=')
            {
                token.str = "*=";
                token.type = TIMES_ASSIGN;
            }
            else
                CToken::PushBack();
            break;
        case '/':
            if (CToken::NextToken() == '=')
            {
                token.str = "/=";
                token.type = DIV_ASSIGN;
            }
            else
                CToken::PushBack();
            break;
        case '<':
            if (CToken::NextToken() == '>')
            {
                token.str = "<>";
                token.type = NEQ;
            }
            else
            {
                CToken::PushBack();
                if (CToken::NextToken() == '=')
                {
                    token.str = "<=";
                    token.type = NGT;
                }
                else
                    CToken::PushBack();
            }
            break;
        case '>':
            if (CToken::NextToken() == '=')
            {
                token.str = ">=";
                token.type = NLT;
            }
            else
                CToken::PushBack();
            break;
        case '&':
            if (CToken::NextToken() == '&')
            {
                token.str = "&&";
                token.type = LOGICAL_AND;
            }
            else
                CToken::PushBack();
            break;
        case '|':
            if (CToken::NextToken() == '|')
            {
                token.str = "||";
                token.type = LOGICAL_OR;
            }
            else
                CToken::PushBack();
            break;
        }
    }
    else
    {
        std::printf("error in line %d: syntax error '%c'", LineNo(), (char)val);
        error_flag = true;
    }
    // std::printf("DEBUG: %s --- %d\n", token.str.c_str(), token.type);//debug
    return token;
}

void CScanner::PushBack()
{
    bPushedBack = true;
}

char asciitolower(char in)
{
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

CToken::CToken(std::string &_string)
{
    bPushedBack = false;
    bEolIsSignificant = true;
    bSlSlComments = true;
    bSlStComments = true;
    bForceLower = false; // case sensitive
    iLineNo = 1;         // the first line
    iChar = 0;
    peekc = ' ';
    sString = _string;
    sString += -1;
}

CToken::~CToken()
{
}

void CToken::PushBack()
{
    bPushedBack = true;
}

// gives the next Token, returns the token type
int CToken::NextToken()
{
    if (bPushedBack)
    {
        bPushedBack = false;
        return tType;
    }

    int c = peekc;
    sVal = ("");

    if (c == EOF)
        return tType = TT_EOF;
    // std::cout<<"DEBUG: "<<int(c);//debug
    // is this a space
    while (std::isspace(c))
    {
        // std::cout<<"DEBUG: "<<c;//debug
        if (c == '\r')
        {
            iLineNo++;
            c = GetChar();
            if (c == '\n')
                c = GetChar();
            if (bEolIsSignificant)
            {
                peekc = c;
                return tType = TT_EOL;
            }
        }
        else
        {
            if (c == '\n')
            {
                iLineNo++;
                if (bEolIsSignificant)
                {
                    peekc = ' ';
                    return tType = TT_EOL;
                }
            }
            c = GetChar();
        }

        if (c == EOF)
            return tType = TT_EOF;
    }

    // is this a number
    if (std::isdigit(c) || c == '.' || c == '-')
    {
        bool neg = false;
        if (c == '-')
        {
            c = GetChar();
            if (c != '.' && !::isdigit(c))
            {
                peekc = c;
                return tType = '-';
            }
            neg = true;
        }
        double v = 0;
        int decexp = 0;
        int seendot = 0;
        while (true)
        {
            if (c == '.' && seendot == 0)
                seendot = 1;
            else if (::isdigit(c))
            {
                v = v * 10 + (c - '0');
                decexp += seendot;
            }
            else
                break;
            c = GetChar();
        }
        peekc = c;
        if (decexp != 0)
        {
            double denom = 10;
            decexp--;
            while (decexp > 0)
            {
                denom *= 10;
                decexp--;
            }
            v = v / denom;
        }
        else if (seendot == 1)
        {
            iChar--;
            peekc = '.';
            seendot = 0;
        }
        dVal = neg ? -v : v;
        if (seendot == 0)
            return tType = TT_INTEGER;
        else
            return tType = TT_REAL;
    }

    // is this a word
    if (std::isalpha(c) || c == '_')
    {
        int i = 0;
        sVal = ("");
        do
        {
            sVal = sVal + (char)c;
            c = GetChar();
        } while (::isalnum(c) || c == '_');
        peekc = c;
        if (bForceLower)
            // sVal.MakeLower();

            std::transform(sVal.begin(), sVal.end(), sVal.begin(), asciitolower);
        return tType = TT_WORD;
    }

    // now the char & string
    if (c == '\'' || c == '"')
    {
        sVal = ("");
        tType = c;
        peekc = ' ';
        int i = 0, c2;
        while ((c = GetChar()) != EOF && c != tType && c != '\n' && c != '\r')
        {
            if (c == '\\') // escape
                switch (c = GetChar())
                {
                case 'a':
                    c = 0x7;
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'f':
                    c = 0xC;
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'v':
                    c = 0xb;
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    c = c - '0';
                    c2 = GetChar();
                    if (c2 == tType)
                    {
                        sVal += (char)c;
                        return tType;
                    }
                    if ('0' <= c2 && c2 <= '7')
                    { // octal
                        c = (c << 3) + (c2 - '0');
                        c2 = GetChar();
                        if (c2 == tType)
                        {
                            sVal += (char)c;
                            return tType;
                        }
                        if ('0' <= c2 && c2 <= '7')
                            c = (c << 3) + (c2 - '0');
                        else
                        {
                            sVal += (char)c;
                            c = c2;
                        }
                    }
                    else
                    {
                        sVal += (char)c;
                        c = c2;
                    }
                    break;
                default:
                    // warning: 'c' : unrecognized character escape sequence
                    std::printf("warning in line %d: '%c': unrecognized character escape sequence",
                                iLineNo, c);
                    error_flag = true;
                }
            sVal += (char)c;
        }
        if (c == EOF)
        {
            // error msg: syntax error in line %d: missing '"'
            std::printf("error in line %d: syntax error, missing '\"'", iLineNo);
            error_flag = true;
        }
        else if (c == '\r' || c == '\n')
        {
            // error msg: syntax error in line %d: new line in constant
            std::printf("error in line %d: syntax error, new line in constant", iLineNo);
            error_flag = true;
        }
        // std::cout<<"DEBUG: "<<tType;//debug
        return tType;
    }

    // and now the comment
    // "//" or "/*...*/"
    if (c == '/' && (bSlSlComments || bSlStComments))
    {
        c = GetChar();
        if (c == '*' && bSlStComments)
        {
            int prevc = 0;
            while ((c = GetChar()) != '/' || prevc != '*')
            {
                if (c == '\n')
                    iLineNo++;
                if (c == EOF)
                    return tType = TT_EOF;
                prevc = c;
            }
            peekc = ' ';
            return NextToken();
        }
        else
        {
            if (c == '/' && bSlSlComments)
            {
                while ((c = GetChar()) != '\n' && c != '\r')
                    ;
                peekc = c;
                return NextToken();
            }
            else
            {
                peekc = c;
                return tType = '/';
            }
        }
    }

    peekc = ' ';

    return tType = c;
}

int CToken::LineNo()
{
    return iLineNo;
}

std::string CToken::GetStrValue()
{
    std::string ret;
    switch (tType)
    {
    case TT_EOF:
        ret = "EOF";
        break;
    case TT_EOL:
        ret = "EOL";
        break;
    case TT_WORD:
        ret = sVal;
        break;
    case TT_STRING:
        ret = sVal;
        break;
    case TT_INTEGER:
    case TT_REAL:
        // ret.Format("%g",dVal);
        ret = std::to_string(dVal);
        break;
    default:
        // ret.Format ( "\'%c\'",(char)tType) ;
        char _tmp[5];
        std::sprintf(_tmp, "\'%c\'", (char)tType);
        ret = std::string(_tmp);
    }
    return ret;
}

double CToken::GetDoubleNumValue()
{
    return dVal;
}

int CToken::GetIntNumValue()
{
    return (int)dVal;
}