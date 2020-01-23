#ifndef _PCODEGENERATOR_H_
#define _PCODEGENERATOR_H_

#include <string>
#include "analyzer.h"

extern bool error_flag;

class CPCodeGen
{
public:
    CPCodeGen(std::string &str);
    ~CPCodeGen();

    // Operations
public:
    void GeneratePCode();

    // help routines
private:
    void emitComment(char *format, ...);
    void emitCode(char *format, ...);
    void emitCode(std::string &code);

    void genPCode(CTreeNode *t, bool addr = false, int lab1 = 0, int lab2 = 0);

private:
    CAnalyzer *pAnalyzer;
    CTreeNode *pProgram;
    int label; // generator unique lable;
};

#endif // _PCODEGENERATOR_H_