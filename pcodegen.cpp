/*
# encoding=utf-8
# author: xiaoke huang
# date: 2019/12/09
*/

#include "pcodegen.h"
#include <cassert>

extern bool error_flag;

CPCodeGen::CPCodeGen(std::string &str)
{
    pAnalyzer = new CAnalyzer(str);
    pProgram = NULL;
    label = 1;
}

CPCodeGen::~CPCodeGen()
{
    if (pAnalyzer)
        delete pAnalyzer;
}

void CPCodeGen::GeneratePCode()
{
    pAnalyzer->TraceTypeCheck();
    if (error_flag)
    {
        puts("\r\nerrors occur, stop generating code!");
        return;
    }
    pProgram = pAnalyzer->pProgram;
    assert(pProgram != NULL);

    // generating p-code
    puts("\r\ngenerating p-code...");
    emitComment("--------- c-++ P-code generating ---------");
    genPCode(pProgram);
}

// write comments
void CPCodeGen::emitComment(char *format, ...)
{
    va_list params;
    static char buf[1024];
    buf[0] = ';';

    va_start(params, format);
    vsnprintf(buf + 1, 1020, format, params);
    strcat(buf, "\r\n");
    va_end(params);
    std::printf(buf);
}

// write codes
void CPCodeGen::emitCode(char *format, ...)
{
    va_list params;
    static char buf[1024];
    buf[0] = '\t';

    va_start(params, format);
    vsnprintf(buf + 1, 1020, format, params);
    strcat(buf, "\r\n");
    va_end(params);
    std::printf(buf);
}

// add beginning "\t" & ending "/r/n" manually
void CPCodeGen::emitCode(std::string &code)
{
    printf("%s", code.c_str());
}

void CPCodeGen::genPCode(CTreeNode *t, bool addr, int lab1, int lab2)
{
    CTreeNode *p = NULL;
    int i, _lab1, _lab2;
    std::string temp = "";

    if (t == NULL)
        return;

    switch (t->nodekind)
    {
    case kFunDec:
        emitComment("start of function '%s %s(...)' declaration",
                    (ReservedKeywordList[(int)t->type]).c_str(), (t->szName).c_str());
        emitCode("ent \t%s", (t->szName).c_str());
        genPCode(t->child[0], addr, lab1, lab2);
        genPCode(t->child[1], addr, lab1, lab2);
        emitCode("ret");
        emitComment("end of function '%s %s(...)' declaration",
                    (ReservedKeywordList[(int)t->type]).c_str(), (t->szName).c_str());
        break;
    case kVarDec:
        if (t->bArray)
        {
            char tmp_[20];
            sprintf(tmp_, "[%d]", t->iArraySize);
            temp = std::string(tmp_);
        }
        emitComment("declaration: %s %s%s",
                    (ReservedKeywordList[(int)t->type]).c_str(), (t->szName).c_str(), (temp).c_str());
        //		if( t->szScope == "global" )
        //			emitCode( "global\t%s%s:%s\t;global variable declaration",
        //			          (t->szName).c_str(), (temp).c_str(), (ReservedKeywordList[(int)t->type]).c_str() );
        //		else
        //			emitCode( "local\t%s%s:%s\t;local variable declaration",
        //			          (t->szName).c_str(), (temp).c_str(), (ReservedKeywordList[(int)t->type]).c_str() );
        break;
    case kParam:
        emitComment("parameter: '%s %s%s'",
                    (ReservedKeywordList[(int)t->type]).c_str(), (t->szName).c_str(), (t->bArray ? "[]" : ""));
        //		emitCode( "param\t%s%s:%s",
        //			      (t->szName).c_str(), (t->bArray ? "[]" : ""), (ReservedKeywordList[(int)t->type]).c_str() );
        break;
    case kStmt:
        switch (t->kind.stmt)
        {
        case kRead:
            emitComment("read");
            genPCode(t->child[0], true, lab1, lab2);
            switch (t->type)
            {
            case _CHAR:
                emitCode("rdc\t;read a character from terminal");
                break;
            case _INT:
                emitCode("rdi\t;read an integer from terminal");
                break;
            case _FLOAT:
                emitCode("rdf\t;read a float number from terminal");
            }
            break;
        case kWrite:
            emitComment("write");
            genPCode(t->child[0], addr, lab1, lab2);
            switch (t->type)
            {
            case _CHAR:
                emitCode("wrc\t;output a character to terminal");
                break;
            case _INT:
                emitCode("wri\t;output an integer to terminal");
                break;
            case _FLOAT:
                emitCode("wrf\t;output a float number to terminal");
            }
            break;
        case kPrintf:
            emitComment("print \"%s\"", (t->szName).c_str());
            for (i = 0; i < t->szName.length(); i++)
            {
                if (t->szName[i] == '\r')
                    temp = "\\r";
                else if (t->szName[i] == '\n')
                    temp = "\\n";
                else if (t->szName[i] == '\t')
                    temp = "\\t";
                else if (t->szName[i] == '\a')
                    temp = "\\a";
                else if (t->szName[i] == '\b')
                    temp = "\\b";
                else if (t->szName[i] == '\f')
                    temp = "\\f";
                else if (t->szName[i] == '\v')
                    temp = "\\v";
                else
                {
                    char tmp_[3];
                    sprintf(tmp_, "%c", t->szName[i]);
                    temp = std::string(tmp_);
                }
                emitCode("ldcc\t'%s'", (temp).c_str());
                emitCode("wrc\t;output a character to terminal");
            }
            break;
        case kLabel:
            emitComment("lab %s@%s", (t->szScope).c_str(), (t->szName).c_str());
            emitCode("lab \t%s@%s", (t->szScope).c_str(), (t->szName).c_str());
            break;
        case kGoto:
            emitComment("goto %s@%s", (t->szScope).c_str(), (t->szName).c_str());
            emitCode("ujp \t%s@%s", (t->szScope).c_str(), (t->szName).c_str());
            break;
        case kIf:
            emitComment("start of if statement");
            emitComment("if conditions");
            genPCode(t->child[0], addr, lab1, lab2);
            _lab1 = label++;
            emitCode("fjp \tL%d", _lab1);
            emitComment("if statements");
            genPCode(t->child[1], addr, lab1, lab2);
            if (t->child[2] != NULL)
            {
                _lab2 = label++;
                emitCode("ujp \tL%d", _lab2);
            }
            emitCode("lab \tL%d", _lab1);
            if (t->child[2] != NULL)
            {
                emitComment("else statements");
                genPCode(t->child[2], addr, lab1, lab2);
                emitCode("lab \tL%d", _lab2);
            }
            emitComment("end of if statement");
            break;
        case kWhile:
            emitComment("start of while statement");
            _lab1 = label++;
            emitCode("lab \tL%d", _lab1);
            emitComment("while conditions");
            genPCode(t->child[0], addr, lab1, lab2);
            _lab2 = label++;
            emitCode("fjp \tL%d", _lab2);
            emitComment("while statements");
            genPCode(t->child[1], addr, _lab1, _lab2);
            emitCode("ujp \tL%d", _lab1);
            emitCode("lab \tL%d", _lab2);
            emitComment("end of while statement");
            break;
        case kBreak:
            emitComment("break statement");
            emitCode("ujp \tL%d", lab2);
            break;
        case kContinue:
            emitComment("continue statement");
            emitCode("ujp \tL%d", lab1);
            break;
        case kReturn:
            emitComment("return statement");
            if (t->child[0]) // return a value
                genPCode(t->child[0], addr, lab1, lab2);
            else // void
                emitComment("return void");
            if (t->sibling ||
                (t->father && t->father->nodekind != kFunDec))
                emitCode("ret"); // not at the end of the routine, add "ret"
            break;
        case kCall:
            emitComment("call '%s(...)'", (t->szName).c_str());
            emitCode("mst");
            if (t->child[0]) // generate code of expressions in arguments
                genPCode(t->child[0], addr, lab1, lab2);
            emitCode("cup \t%s", (t->szName).c_str());
            emitComment("end of call '%s(...)'", (t->szName).c_str());
        }
        break;
    case kExp:
        switch (t->kind.exp)
        {
        case kConst:
            if (t->type == _CHARACTER)
                emitCode("ldcc\t%s", (t->szName).c_str());
            else
            {
                if (t->szName.find('.') == std::string::npos) // integer
                    emitCode("ldci\t%s", (t->szName).c_str());
                else // is float number
                    emitCode("ldcf\t%s", (t->szName).c_str());
            }
            break;
        case kID:
            if (t->bArray)
            {
                emitCode("lda \t%s", (t->szName).c_str());
                if (t->father && t->father->nodekind == kStmt && t->father->kind.stmt == kCall)
                    break; // passing its base-address to the call function is OK
                genPCode(t->child[0], false, lab1, lab2);
                emitCode("ixa \telesize(%s)", (t->szName).c_str());
                if (!addr) // load value
                    emitCode("ind \t0");
            }
            else
            {
                if (addr) // load address
                    emitCode("lda \t%s", (t->szName).c_str());
                else // load value
                    emitCode("lod \t%s", (t->szName).c_str());
            }
            break;
        case kOp:
            if (t->szName == "=")
            {
                genPCode(t->child[0], true, lab1, lab2);
                genPCode(t->child[1], false, lab1, lab2);
                p = t->father;
                if ((p && p->nodekind == kStmt && t == p->child[0]) ||
                    (p && p->nodekind == kExp))
                    // t is a condition expression, or is in a expression,
                    // keep the result in stack
                    emitCode("stn");
                else // otherwise, pop it
                    emitCode("sto");
            }
            else if (t->szName == "!")
            {
                genPCode(t->child[0], false, lab1, lab2);
                emitCode("not");
            }
            else
            {
                genPCode(t->child[0], false, lab1, lab2);
                genPCode(t->child[1], false, lab1, lab2);
                if (t->szName == "==")
                    emitCode("equ \t;equal expression");
                else if (t->szName == "!=")
                    emitCode("neq \t;not equal expression");
                else if (t->szName == "+")
                {
                    if (t->type == _FLOAT)
                        emitCode("adr \t;float add"); // float add
                    else
                        emitCode("adi \t;integer add"); // integer add
                }
                else if (t->szName == "-")
                {
                    if (t->type == _FLOAT)
                        emitCode("sbr \t;float sub"); // float sub
                    else
                        emitCode("sbi \t;integer sub"); // integer sub
                }
                else if (t->szName == "*")
                {
                    if (t->type == _FLOAT)
                        emitCode("mpr \t;float mul"); // float mul
                    else
                        emitCode("mpi \t;integer mul"); // integer mul
                }
                else if (t->szName == "/")
                {
                    if (t->type == _FLOAT)
                        emitCode("dvr \t;float div"); // float div
                    else
                        emitCode("dvi \t;integer div"); // integer div
                }
                else if (t->szName == "%")
                    emitCode("mod \t;mod expression");
                else if (t->szName == "<")
                    emitCode("les \t;less than expression");
                else if (t->szName == "<=")
                    emitCode("leq \t;less than or equal expression");
                else if (t->szName == ">")
                    emitCode("grt \t;greater than expression");
                else if (t->szName == ">=")
                    emitCode("geq \t;greater than or equal expression");
                else if (t->szName == "&&")
                    emitCode("and \t;logical and expression");
                else if (t->szName == "||")
                    emitCode("or  \t;logical or expression");
                else
                    emitCode("error");
            }
        }
        break;
    default:
        emitCode("error");
    }

    if (t->sibling)
        genPCode(t->sibling, addr, lab1, lab2);
}
