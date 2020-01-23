/*
# encoding=utf-8
# author: xiaoke huang
# date: 2019/11/19
*/

#include "parser.h"

// construction
CParser::CParser(std::string &str)
{
    pScaner = new CScanner(str);
    pProgram = NULL;
    indent = -1;
}

// destruction
CParser::~CParser()
{
    delete pScaner;
    if (pProgram)
        delete pProgram;
}

// build the parse tree
CTreeNode *CParser::BuildSyntaxTree()
{
    return (pProgram = program());
}

// construct a new node
CTreeNode *CParser::newNode(NodeKind kind, enum TokenType type, std::string &ID)
{
    CTreeNode *t = new CTreeNode();
    t->lineno = pScaner->LineNo();
    t->nodekind = kind;
    t->type = type;
    t->szName = ID;
    t->szScope = szScope;
    return t;
}

// construct a new statment node
CTreeNode *CParser::newStmtNode(StmtKind kind, const std::string &ID)
{
    CTreeNode *t = new CTreeNode();
    t->lineno = pScaner->LineNo();
    t->nodekind = kStmt;
    t->kind.stmt = kind;
    t->type = _NONE;
    t->szName = ID;
    t->szScope = szScope;
    return t;
}

// construct a new expression node
CTreeNode *CParser::newExpNode(ExpKind kind, enum TokenType type, std::string &ID)
{
    CTreeNode *t = new CTreeNode();
    t->lineno = pScaner->LineNo();
    t->nodekind = kExp;
    t->kind.exp = kind;
    t->type = type;
    t->szName = ID;
    t->szScope = szScope;
    return t;
}

// get the next token, check if its type is expected
bool CParser::match(enum TokenType type)
{
    token = pScaner->NextToken();
    return (token.type == type);
}

// for error recovery
void CParser::ConsumeUntil(enum TokenType type)
{
    while (token.type != type && token.type != _EOF)
        token = pScaner->NextToken();
}

void CParser::ConsumeUntil(enum TokenType type1, enum TokenType type2)
{
    while (token.type != type1 && token.type != type2 && token.type != _EOF)
        token = pScaner->NextToken();
}

// Grammar:
// 1. program->declaration_list
CTreeNode *CParser::program()
{
    return declaration_list();
}

// Grammar:
// 2. declaration_list->declaration_list declaration | declaration
CTreeNode *CParser::declaration_list()
{
    CTreeNode *first = NULL, *last = NULL, *temp = NULL;
    token = pScaner->NextToken();
    while (token.type != _EOF)
    {
        if (token.type != _CHAR && token.type != _INT &&
            token.type != _VOID && token.type != _FLOAT)
        {
            //throw_error( ERROR_INVALID_TYPE, pScaner->LineNo(), token.str );
            error_flag = true, std::printf("error in line %d: invalid type '%s'",
                                           pScaner->LineNo(), token.str.c_str());
            // std::printf("DEBUG: %s --- %d\n", token.str.c_str(), token.type);//debug
            ConsumeUntil(SEMI /* ';' */, RBRACE /* '}' */); // error recovery
        }
        else if ((temp = declaration()) != NULL)
        {
            // link all declarations together
            if (!first)
            {
                first = temp;
                last = temp->LastSibling();
            }
            else
            {
                last->sibling = temp;
                last = temp->LastSibling();
            }
        }
        // read the next token
        token = pScaner->NextToken();
    }
    return first;
}

// Grammar:
// 3. declaration->var_declaration | fun_declaration
// token is a supported type-identifier token
CTreeNode *CParser::declaration()
{
    szScope = ("global"); // global function or variable declaration
    CTreeNode *temp = NULL;

    TypeToken = token;
    IDToken = token = pScaner->NextToken();
    if (IDToken.type != _ID)
    {
        //throw _error( ERROR_DECLARATION, pScaner->LineNo(), IDToken.str );
        error_flag = true, std::printf("error in line %d: \"%s\" is a reserved token",
                                       pScaner->LineNo(), IDToken.str.c_str());
        ConsumeUntil(SEMI, RBRACE);
    }
    else
    {
        token = pScaner->NextToken(); // '(', ';', '[', ',' or error
        if (token.type == LPARAN)
            temp = fun_declaration();
        else if (token.type == SEMI || token.type == LSQUARE || token.type == COMMA)
            temp = var_declaration();
        else
        {
            // throw _error( ERROR_SEMICOLON_MISS, pScaner->LineNo(), IDToken.str );
            error_flag = true, std::printf("error in line %d: missing ';' after identifier \"%s\"",
                                           pScaner->LineNo(), IDToken.str.c_str());
            ConsumeUntil(SEMI, RBRACE);
        }
    }

    return temp;
}

// Grammar:
// 4. var_declaration->type_specifier ID(, ...)`;` | type_specifier ID `[` NUM `]`(, ...)`;`
// 5. type_specifier->`int` | `void` | `char`, actually this step is in declaration_list()
// token.str == ";" "," or "["
CTreeNode *CParser::var_declaration()
{
    CTreeNode *temp = newNode(kVarDec, TypeToken.type, IDToken.str);

    if (token.type == LSQUARE)
    {                                 // '['
        token = pScaner->NextToken(); // NUM
        if (token.type != _NUM)
        {
            error_flag = true, std::printf("error in line %d: syntax error in declaration of array %s[], missing array size",
                                           pScaner->LineNo(), IDToken.str.c_str());
            delete temp;
            ConsumeUntil(SEMI, RBRACE); // error recovery
            return NULL;
        }

        temp->bArray = true;
        temp->iArraySize = pScaner->GetIntNumValue();

        if (!match(RSQUARE))
        { // `]`
            error_flag = true, std::printf("error in line %d: syntax error in declaration of array %s[], missing ']'",
                                           pScaner->LineNo(), IDToken.str.c_str());
            pScaner->PushBack(); // error recovery
        }
        token = pScaner->NextToken(); // should be ';' or ','
    }
    if (token.type == COMMA)
    {
        IDToken = token = pScaner->NextToken(); // ID or error
        if (IDToken.type != _ID)
        {
            error_flag = true, std::printf("error in line %d: \"%s\" is a reserved token",
                                           pScaner->LineNo(), IDToken.str.c_str());
            ConsumeUntil(SEMI, RBRACE); // error recovery
            return temp;
        }
        token = pScaner->NextToken(); // ';', '[', ',' or error
        if (token.type == SEMI || token.type == LSQUARE || token.type == COMMA)
            temp->sibling = var_declaration(); // link following variable declarations
        else
        {
            error_flag = true, std::printf("error in line %d: missing ';' after identifier \"%s\"",
                                           pScaner->LineNo(), IDToken.str.c_str());
            pScaner->PushBack(); // error recovery
            return temp;
        }
    }
    else if (token.type != SEMI)
    { // token should be ';' now
        error_flag = true, std::printf("error in line %d: bad declaration sequence, missing ';'", pScaner->LineNo());
        ConsumeUntil(SEMI, RBRACE);
    }

    return temp;
}

// Grammar:
// 6. fun_declaration->type_specifier ID `(` params `)` compound_stmt
// token.str == "(", TypeToken contains type_specifier, IDToken contains ID
CTreeNode *CParser::fun_declaration()
{
    CTreeNode *temp = newNode(kFunDec, TypeToken.type, IDToken.str);

    // update function scope
    szScope = IDToken.str;

    // params
    CTreeNode *p = temp->child[0] = params();
    if (p)
        p->father = temp;
    while (p && p->sibling)
    {
        p = p->sibling;
        p->father = temp;
    }

    if (!match(RPARAN))
    {
        error_flag = true, std::printf("error in line %d: missing ')' in function \"%s\"(...) declaration",
                                       pScaner->LineNo(), token.str.c_str());
        pScaner->PushBack();
    }
    // compound statements
    p = temp->child[1] = compound_stmt();
    if (p)
        p->father = temp;
    while (p && p->sibling)
    {
        p = p->sibling;
        p->father = temp;
    }

    return temp;
}

// Grammar:
// 7. params->paralist | `void` | empty, `void` is thought as empty
// 8. paralist->paralist `,` param | param
// 9. param->type_specifier ID | type_specifier ID `[` `]`
// token.str == "("
CTreeNode *CParser::params()
{
    CTreeNode *first = NULL, *temp = NULL;

    TypeToken = token = pScaner->NextToken(); // type-specifier or ')'
    if (token.type == RPARAN)
    {
        pScaner->PushBack();
        return NULL;
    }
    if (TypeToken.type == _VOID)
        if (match(RPARAN))
        {
            pScaner->PushBack();
            return NULL;
        }
        else
            pScaner->PushBack(); // is not ')', push it back
    while (TypeToken.type == _INT || TypeToken.type == _CHAR ||
           TypeToken.type == _VOID || TypeToken.type == _FLOAT)
    {
        IDToken = token = pScaner->NextToken();
        if (IDToken.type != _ID)
        {
            error_flag = true, std::printf("error in line %d: invalid parameter \"%s\"",
                                           pScaner->LineNo(), IDToken.str.c_str());
        }
        else
        {
            temp = newNode(kParam, TypeToken.type, IDToken.str);
            temp->sibling = first; // the FIRST parameter is the LAST sibling node
            first = temp;
        }
        token = pScaner->NextToken();
        if (token.type == LSQUARE)
        { // '['
            temp->bArray = true;
            if (!match(RSQUARE))
            { //']'
                error_flag = true, std::printf("error in line %d: bad array parameter, missing ']'", pScaner->LineNo());
                ConsumeUntil(COMMA, RPARAN); // error recovery
            }
            else
                token = pScaner->NextToken(); // should be ',' or ')'
        }
        if (token.type == RPARAN)
            break;                    // ')'
        else if (token.type == COMMA) // ','
            TypeToken = token = pScaner->NextToken();
        else
        { // just break
            //error_flag = true, std::printf( "error in line %d: bad function parameters", pScaner->LineNo() );
            break;
        }
    }
    pScaner->PushBack(); // the next token should be ')'

    return first;
}

// Grammar:
// 10. compound_stmt->`{` loal_declarations statement_list `}` | expression_stmt
// the next token should be '{'
CTreeNode *CParser::compound_stmt()
{
    CTreeNode *first = NULL, *last = NULL, *temp = NULL;
    bool bHasNoBraces = false;

    if (!match(LBRACE))
    { // match'{'
        // error_flag = true, std::printf( "error in line %d: missing '{'", pScaner->LineNo() );
        bHasNoBraces = true;
        pScaner->PushBack(); // error recovery
    }

    // local_declarations
    while (1)
    {
        TypeToken = token = pScaner->NextToken();
        if (token.type == _CHAR || token.type == _INT ||
            token.type == _VOID || token.type == _FLOAT)
            temp = local_declarations();
        else
        {
            pScaner->PushBack();
            break;
        }
        if (bHasNoBraces)
            return temp; // has no braces, return when reach the first ';'
        if (temp)
            // link all local_declarations together
            if (!first)
            {
                first = temp;
                last = temp->LastSibling();
            }
            else
            {
                last->sibling = temp;
                last = temp->LastSibling();
            }
    }

    // statement_list
    // token contains the first token of statement_list
    token = pScaner->NextToken();
    while (1)
    {
        temp = NULL;
        if (token.type == RBRACE)
        {
            if (bHasNoBraces)
                error_flag = true, std::printf("error in line %d: unpaired '}'", pScaner->LineNo());
            break; // '}'
        }
        if (token.type == _EOF)
        {
            error_flag = true, std::printf("error in line %d: missing '}'", pScaner->LineNo());
            pScaner->PushBack();
            break;
        }
        switch (token.type)
        {
        case _READ:
            temp = read_stmt();
            break;
        case _WRITE:
            temp = write_stmt();
            break;
        case _PRINTF:
            temp = printf_stmt();
            break;
        case SEMI: // ';'
        case _NUM:
        case _CHARACTER:
        case LOGICAL_NOT:
        case LPARAN:
            temp = expression_stmt();
            break;
        case _ID:
            temp = subcompound_stmt();
            break;
        case _IF:
            temp = if_stmt();
            break;
        case _WHILE:
            temp = while_stmt();
            break;
        case _FOR:
            temp = for_stmt();
            break;
        case _GOTO:
            temp = goto_stmt();
            break;
        case _BREAK:
            temp = break_stmt();
            break;
        case _CONTINUE:
            temp = continue_stmt();
            break;
        case _RETURN:
            temp = return_stmt();
            break;
        case _ELSE:
            error_flag = true, std::printf("error in line %d: unpaired 'else' statement", pScaner->LineNo());
            ConsumeUntil(SEMI, RBRACE);
            break;
        default:
            error_flag = true, std::printf("error in line %d: undefined symbol \"%s\"",
                                           pScaner->LineNo(), token.str.c_str());
            ConsumeUntil(SEMI, RBRACE);
        }
        if (bHasNoBraces)
            return temp; // has no braces, return when reach the first ';'
        if (temp)
            // link all statements together
            if (!first)
            {
                first = temp;
                last = temp->LastSibling();
            }
            else
            {
                last->sibling = temp;
                last = temp->LastSibling();
            }
        token = pScaner->NextToken();
    }

    return first;
}

// Grammar:
// 11. local_declarations->local_declarations var_declaration | var_declaration
// token is a supported type-specifier token
CTreeNode *CParser::local_declarations()
{
    CTreeNode *temp = NULL;

    IDToken = token = pScaner->NextToken(); // ID or error
    if (IDToken.type != _ID)
    {
        error_flag = true, std::printf("error in line %d: \"%s\" is a reserved token",
                                       pScaner->LineNo(), IDToken.str.c_str());
        ConsumeUntil(SEMI); // error recovery
        return NULL;
    }
    token = pScaner->NextToken(); // ';', '[', ',' or error
    if (token.type == SEMI || token.type == LSQUARE || token.type == COMMA)
        temp = var_declaration();
    else
    {
        error_flag = true, std::printf("error in line %d: missing ';' after identifier \"%s\"",
                                       pScaner->LineNo(), IDToken.str.c_str());
        pScaner->PushBack(); // error recovery
    }
    return temp;
}

// Grammar:
// 12. `read` `(` var `)` `;`
// token.str == "read"
CTreeNode *CParser::read_stmt()
{
    CTreeNode *temp = NULL;

    if (!match(LPARAN))
    { // '('
        error_flag = true, std::printf("error in line %d: syntax error, missing '('", pScaner->LineNo());
        ConsumeUntil(SEMI, RBRACE); // error recovery
        return NULL;
    }
    IDToken = token = pScaner->NextToken();
    if (token.type != _ID)
    {
        error_flag = true, std::printf("error in line %d: \"%s\" bad arguments", pScaner->LineNo(), token.str.c_str());
        ConsumeUntil(SEMI, RBRACE); // error recovery
        return NULL;
    }
    temp = newStmtNode(kRead, std::string("read"));
    if ((temp->child[0] = var()) != NULL)
        temp->child[0]->father = temp;
    if (!match(RPARAN))
    { // ')'
        error_flag = true, std::printf("error in line %d: syntax error, missing ')'", pScaner->LineNo());
        ConsumeUntil(SEMI, RBRACE); // error recovery
        return temp;
    }
    if (!match(SEMI))
    { // ';'
        error_flag = true, std::printf("error in line %d: syntax error, missing ';'", pScaner->LineNo());
        pScaner->PushBack(); // error recovery
    }
    return temp;
}

// Grammar:
// 13. `write` `(` expression `)` `;`
// token.str == "write"
CTreeNode *CParser::write_stmt()
{
    CTreeNode *temp = NULL;

    if (!match(LPARAN))
    { // '('
        error_flag = true, std::printf("error in line %d: syntax error, missing '('", pScaner->LineNo());
        ConsumeUntil(SEMI, RBRACE); // error recovery
        return NULL;
    }
    temp = newStmtNode(kWrite, std::string("write"));
    token = pScaner->NextToken();
    // token contains the first token of expression
    if ((temp->child[0] = expression()) != NULL)
        temp->child[0]->father = temp;
    if (!match(RPARAN))
    { // ')'
        error_flag = true, std::printf("error in line %d: syntax error, missing ')'", pScaner->LineNo());
        ConsumeUntil(SEMI, RBRACE); // error recovery
        return temp;
    }
    if (!match(SEMI))
    { // ';'
        error_flag = true, std::printf("error in line %d: syntax error, missing ';'", pScaner->LineNo());
        pScaner->PushBack(); // error recovery
    }
    return temp;
}

// Grammar:
// 14. `printf` `(` `"` std::string `"` `)` `;`
// token.str == "printf"
CTreeNode *CParser::printf_stmt()
{
    CTreeNode *temp = NULL;

    if (!match(LPARAN))
    { // '('
        error_flag = true, std::printf("error in line %d: syntax error, missing '('", pScaner->LineNo());
        ConsumeUntil(SEMI, RBRACE); // error recovery
        return NULL;
    }
    token = pScaner->NextToken();
    if (token.type != _STRING)
    {
        error_flag = true, std::printf("error in line %d: arguments should be strings", pScaner->LineNo());
        ConsumeUntil(SEMI, RBRACE); // error recovery
        return NULL;
    }
    temp = newStmtNode(kPrintf, token.str);
    if (!match(RPARAN))
    { // ')'
        error_flag = true, std::printf("error in line %d: syntax error, missing ')'", pScaner->LineNo());
        ConsumeUntil(SEMI, RBRACE); // error recovery
        return temp;
    }
    if (!match(SEMI))
    { // ';'
        error_flag = true, std::printf("error in line %d: syntax error, missing ';'", pScaner->LineNo());
        pScaner->PushBack(); // error recovery
    }
    return temp;
}

// Grammar:
// 15. expression_stmt->expression `;` | `;`
// token is '!', '(', ID, NUM, CHARACTER or ';'
CTreeNode *CParser::expression_stmt()
{
    if (token.type == SEMI)
        return NULL;
    CTreeNode *temp = expression();
    if (!match(SEMI))
    {
        error_flag = true, std::printf("error in line %d: missing ';'", pScaner->LineNo());
        pScaner->PushBack(); // error recovery
    }

    return temp;
}

// Grammar:
// 16. expression->var `=` expression | logic1_expression
// FIRST( expression ) = { `!`, `(`, ID, NUM, CHARACTER }
// token contains the first token of expression
CTreeNode *CParser::expression()
{
    CTreeNode *temp = logic1_expression(), *p = temp;
    token = pScaner->NextToken();
    if (token.type == ASSIGN)
    {
        if (temp->type != _ID && temp->type != ASSIGN)
        { // left of '=' should be a ID
            error_flag = true, std::printf("error in line %d: left of '=' syntax error", pScaner->LineNo());
            ConsumeUntil(SEMI, RPARAN);
            delete temp;
            return NULL;
        }
        p = newExpNode(kOp, token.type, token.str);
        p->child[0] = temp;
        if (p->child[0])
            p->child[0]->father = p;
        token = pScaner->NextToken();
        p->child[1] = expression();
        if (p->child[1])
            p->child[1]->father = p;
    }
    else
        pScaner->PushBack();

    return p;
}

// Grammar:
// 17. logic1_expression->logic1_expression `||` logic2_expression | logic2_expression
// token contains the first token of logic1_expression
CTreeNode *CParser::logic1_expression()
{
    CTreeNode *p = logic2_expression();

    token = pScaner->NextToken();
    while (token.type == LOGICAL_OR)
    {
        CTreeNode *temp = newExpNode(kOp, token.type, token.str);
        temp->child[0] = p;
        p = temp;
        if (p->child[0])
            p->child[0]->father = p;
        token = pScaner->NextToken();
        if ((p->child[1] = logic2_expression()))
            p->child[1]->father = p;
        token = pScaner->NextToken();
    }
    pScaner->PushBack(); // put the next token back

    return p;
}

// Grammar:
// 18. logic2_expression-> logic2_expression `&&` simple_expression | simple_expression
// token contains the first token of logic2_expression
CTreeNode *CParser::logic2_expression()
{
    CTreeNode *p = simple_expression();

    token = pScaner->NextToken();
    while (token.type == LOGICAL_AND)
    {
        CTreeNode *temp = newExpNode(kOp, token.type, token.str);
        temp->child[0] = p;
        p = temp;
        if (p->child[0])
            p->child[0]->father = p;
        token = pScaner->NextToken();
        if ((p->child[1] = simple_expression()))
            p->child[1]->father = p;
        token = pScaner->NextToken();
    }
    pScaner->PushBack(); // put the next token back

    return p;
}

// Grammar:
// 19. simple_expression->additive_expression relop additive_expression | additive_expression
// 20. relop-> `<=` | `<` | `>` | `>=` | `==` | `!=`
// token contains the first token of simple_expression
CTreeNode *CParser::simple_expression()
{
    CTreeNode *p = additive_expression();

    token = pScaner->NextToken();
    if (token.type == NGT || token.type == LT || token.type == GT ||
        token.type == NLT || token.type == EQ || token.type == NEQ)
    {
        CTreeNode *temp = newExpNode(kOp, token.type, token.str);
        temp->child[0] = p;
        p = temp;
        if (p->child[0])
            p->child[0]->father = p;
        token = pScaner->NextToken();
        if ((p->child[1] = additive_expression()))
            p->child[1]->father = p;
    }
    else
        pScaner->PushBack();

    return p;
}

// Grammar:
// 21. additive_expression -> additive_expression addop term | term
// 22. addop-> `+` | `-`
// token contains the first token of add_expression
CTreeNode *CParser::additive_expression()
{
    CTreeNode *p = term();

    token = pScaner->NextToken();
    while (token.type == PLUS || token.type == MINUS)
    {
        CTreeNode *temp = newExpNode(kOp, token.type, token.str);
        temp->child[0] = p;
        p = temp;
        if (p->child[0])
            p->child[0]->father = p;
        token = pScaner->NextToken();
        if ((p->child[1] = term()))
            p->child[1]->father = p;
        token = pScaner->NextToken();
    }
    pScaner->PushBack(); // put the next token back

    return p;
}

// Grammar:
// 23. term->term mulop logic3_expression | logic3_expression
// 24. mulop-> `*` | `/` | `%`
// token contains the first token of term
CTreeNode *CParser::term()
{
    CTreeNode *p = logic3_expression();

    token = pScaner->NextToken();
    while (token.type == TIMES || token.type == DIV || token.type == MOD)
    {
        CTreeNode *temp = newExpNode(kOp, token.type, token.str);
        temp->child[0] = p;
        p = temp;
        if (p->child[0])
            p->child[0]->father = p;
        token = pScaner->NextToken();
        if ((p->child[1] = logic3_expression()))
            p->child[1]->father = p;
        token = pScaner->NextToken();
    }
    pScaner->PushBack(); // put the next token back

    return p;
}

// Grammar:
// 25. logic3_expression-> `!` logic3_expression | factor
// token contains the first token of logic3_expression
CTreeNode *CParser::logic3_expression()
{
    CTreeNode *p = NULL, *temp = NULL;

    if (token.type == LOGICAL_NOT)
    {
        p = newExpNode(kOp, token.type, token.str);
        token = pScaner->NextToken();
        if ((temp = factor()))
        {
            p->child[0] = temp;
            p->child[0]->father = p;
        }
    }
    else
        p = factor();

    return p;
}

// Grammar:
// 26. factor->`(` expression `)` | var | call | NUM
// token contains the first token of factor
CTreeNode *CParser::factor()
{
    CTreeNode *temp = NULL;

    switch (token.type)
    {
    case _NUM:
    case _CHARACTER:
        temp = newExpNode(kConst, token.type, token.str);
        break;
    case _ID:
        IDToken = token;
        token = pScaner->NextToken();
        if (token.type == LPARAN)
            temp = call();
        else
        {
            pScaner->PushBack(); // push the next token back
            temp = var();
        }
        break;
    case LPARAN:
        token = pScaner->NextToken(); // token contain the first token of expression
        temp = expression();
        if (!match(RPARAN))
        { // match ')'
            error_flag = true, std::printf("error in line %d: missing ')'", pScaner->LineNo());
            pScaner->PushBack(); // error recovery
        }
        break;
    default:
        error_flag = true, std::printf("error in line %d: '%s' expression syntax error",
                                       pScaner->LineNo(), token.str.c_str());
        ConsumeUntil(SEMI, RBRACE); // error recovery
    }

    return temp;
}

// Grammar:
// 27. var->ID | ID `[` expression `]`
// IDToken contains ID
CTreeNode *CParser::var()
{
    CTreeNode *temp = newExpNode(kID, IDToken.type, IDToken.str);
    token = pScaner->NextToken(); // should be `[` or just push back
    if (token.type == LSQUARE)
    {
        temp->bArray = true;
        token = pScaner->NextToken();
        temp->child[0] = expression();
        if (!match(RSQUARE))
        {
            error_flag = true, std::printf("error in line %d: missing ']'", pScaner->LineNo());
            pScaner->PushBack(); // error recovery
        }
    }
    else
        pScaner->PushBack();

    return temp;
}

// Grammar:
// 28. call->ID `(` args `)`
// token.str == "(", IDToken contains ID
CTreeNode *CParser::call()
{
    CTreeNode *p = newStmtNode(kCall, IDToken.str), *temp = NULL;
    p->szScope = ("global");
    //	CTreeNode* temp = newExpNode( kID, IDToken.type, IDToken.str );
    //	p->child[0] = temp;
    //	p->child[0]->father = p;
    if ((p->child[0] = args()))
        p->child[0]->father = p;
    temp = p->child[0];
    while (temp && temp->sibling)
    {
        temp = temp->sibling;
        temp->father = p;
    }
    if (!match(RPARAN))
    { // match ')'
        error_flag = true, std::printf("error in line %d: missing ')'", pScaner->LineNo());
        pScaner->PushBack(); // error recovery
    }

    return p;
}

// Grammar:
// 29. args->args_list | empty
// 30. args_list->args_list `,` expression | expression
// token.str == "("
CTreeNode *CParser::args()
{
    CTreeNode *first = NULL, *temp = NULL;

    token = pScaner->NextToken();
    if (token.type == RPARAN)
    {
        pScaner->PushBack(); // push the next token back
        return NULL;
    }
    while (1)
    {
        if ((temp = expression()) != NULL)
            // link all args together, the LAST argument is the FIRST in the list
            if (!first)
                first = temp;
            else
            {
                temp->sibling = first;
                first = temp;
            }
        token = pScaner->NextToken();
        if (token.type == COMMA)
            token = pScaner->NextToken();
        else
            break;
    }
    pScaner->PushBack();

    return first;
}

// Grammar:
// 31: sub_compoundstmt->ID `:` | call `;` | expression_stmt
// token contains the first token of sub_compoundstmt
CTreeNode *CParser::subcompound_stmt()
{
    CTreeNode *temp = NULL;

    IDToken = token;
    token = pScaner->NextToken();
    if (token.type == COLON)
    { // label
        temp = newStmtNode(kLabel, IDToken.str);
    }
    else if (token.type == LPARAN)
    { // call statement
        temp = call();
        if (!match(SEMI))
        {
            error_flag = true, std::printf("error in line %d: missing ';'", pScaner->LineNo());
            pScaner->PushBack(); // error recovery
        }
    }
    else
    {
        pScaner->PushBack();
        token = IDToken;
        temp = expression_stmt();
    }

    return temp;
}

// Grammar:
// 32: if_stmt->`if` `(` expression `)` compound_stmt
//              | `if` `(` expression `)` compound_stmt `else` compound_stmt
// token.str == "if"
CTreeNode *CParser::if_stmt()
{
    CTreeNode *temp = newStmtNode(kIf, token.str), *p = NULL;

    if (!match(LPARAN)) // match '('
        error_flag = true, std::printf("error in line %d: missing '(' in 'if' statement", pScaner->LineNo());
    else
        token = pScaner->NextToken();
    // token should be the first token of expression
    temp->child[0] = expression();
    if (temp->child[0])
        temp->child[0]->father = temp;
    if (!match(RPARAN))
    { // match ')'
        error_flag = true, std::printf("error in line %d: missing ')' in 'if' statement", pScaner->LineNo());
        pScaner->PushBack();
    }
    p = temp->child[1] = compound_stmt();
    if (p)
        p->father = temp;
    while (p && p->sibling)
    {
        p = p->sibling;
        p->father = temp;
    }
    token = pScaner->NextToken();
    if (token.type == _ELSE)
    {
        // ::puts("DEBUG: else");//debug
        p = temp->child[2] = compound_stmt();
        if (p)
            p->father = temp;
        while (p && p->sibling)
        {
            p = p->sibling;
            p->father = temp;
        }
    }
    else
        pScaner->PushBack(); // push the next token back

    return temp;
}

// Grammar:
// 33. while_stmt->`while` `(` expression `)` compound_stmt
// token.str == "while"
CTreeNode *CParser::while_stmt()
{
    CTreeNode *temp = newStmtNode(kWhile, token.str), *p = NULL;

    if (!match(LPARAN)) // match '('
        error_flag = true, std::printf("error in line %d: missing '(' in 'while' statement", pScaner->LineNo());
    else
        token = pScaner->NextToken();
    // token should be the first token of expression
    temp->child[0] = expression();
    if (temp->child[0])
        temp->child[0]->father = temp;
    if (!match(RPARAN))
    { // match ')'
        error_flag = true, std::printf("error in line %d: missing ')' in 'while' statement", pScaner->LineNo());
        pScaner->PushBack();
    }
    // the next token should be '{'
    p = temp->child[1] = compound_stmt();
    if (p)
        p->father = temp;
    while (p && p->sibling)
    {
        p = p->sibling;
        p->father = temp;
    }

    return temp;
}

// Grammar:
// 34. for_stmt->`for` `(` var `=` expression `;` expression `;` var `=` expression `)` compound_stmt
// token.str == "for"
CTreeNode *CParser::for_stmt()
{
    CTreeNode *temp = NULL, *p1 = NULL, *p2 = NULL, *p3 = NULL;

    if (!match(LPARAN)) // match '('
        error_flag = true, std::printf("error in line %d: missing '(' in 'for' statement", pScaner->LineNo());
    else
        token = pScaner->NextToken();
    // token should be var or ';'
    if (token.type == SEMI)
    {
        p1 = temp = newStmtNode(kWhile, std::string("while"));
        token = pScaner->NextToken();
    }
    else
    {
        if ((temp = expression()) == NULL)
        {
            error_flag = true, std::printf("error in line %d: syntax error in the first expression, ignore the whole", pScaner->LineNo());
            ConsumeUntil(RBRACE);
            return NULL;
        }
        else
            p1 = temp->sibling = newStmtNode(kWhile, std::string("while"));

        if (!match(SEMI)) // match ';'
            error_flag = true, std::printf("error in line %d: missing the first ';' in 'for' statement", pScaner->LineNo());
        else
            token = pScaner->NextToken();
    }
    // token should be the first token of expression
    p1->child[0] = expression();
    if (!p1->child[0])
    {
        error_flag = true, std::printf("error in line %d: missing the second parameter in 'for' statement, ignore the whole", pScaner->LineNo());
        ConsumeUntil(RBRACE);
        if (temp)
            delete temp;
        return NULL;
    }
    p1->child[0]->father = p1;
    if (!match(SEMI)) // match ';'
        error_flag = true, std::printf("error in line %d: missing the second ';' in 'for' statement", pScaner->LineNo());
    else
        token = pScaner->NextToken();
    // token should be var
    p2 = expression();
    if (p2)
        p2->father = p1;
    if (!match(RPARAN))
    { // match ')'
        error_flag = true, std::printf("error in line %d: missing ')' in 'for' statement", pScaner->LineNo());
        pScaner->PushBack();
    }
    // the next token should be '{'
    p3 = p1->child[1] = compound_stmt();
    if (p3)
        p3->father = p1;
    if (p3 && p3->sibling)
    {
        p3 = p3->sibling;
        p3->father = p1;
    }
    if (p3)
        p3->sibling = p2;
    else
        p1->child[1] = p2;

    return temp;
}

// Grammar:
// 35. goto_stmt->`goto` ID `;`
// token.str == "goto"
CTreeNode *CParser::goto_stmt()
{
    if (!match(_ID))
    {
        error_flag = true, std::printf("error in line %d: a label should follow 'goto'", pScaner->LineNo());
        ConsumeUntil(SEMI, RBRACE); // error recovery
        return NULL;
    }
    CTreeNode *temp = newStmtNode(kGoto, token.str);
    if (!match(SEMI))
    {
        error_flag = true, std::printf("error in line %d: missing ';' in 'goto' statement", pScaner->LineNo());
        pScaner->PushBack();
    }
    return temp;
}

// Grammar:
// 36. break_stmt->`break` `;`
// token.str == "break"
CTreeNode *CParser::break_stmt()
{
    CTreeNode *temp = newStmtNode(kBreak, token.str);
    if (!match(SEMI))
    { // match ';'
        error_flag = true, std::printf("error in line %d: missing ';' in 'break' statement", pScaner->LineNo());
        pScaner->PushBack();
    }
    return temp;
}

// Grammar:
// 37. continue_stmt->`continue` `;`
// token.str = "continue"
CTreeNode *CParser::continue_stmt()
{
    CTreeNode *temp = newStmtNode(kContinue, token.str);
    if (!match(SEMI))
    {
        error_flag = true, std::printf("error in line %d: missing ';' in 'continue' statement", pScaner->LineNo());
        pScaner->PushBack();
    }
    return temp;
}

// Grammar:
// 38. return_stmt->`return` `;` | `return` expression `;`
// token.str = "return"
CTreeNode *CParser::return_stmt()
{
    CTreeNode *temp = newStmtNode(kReturn, token.str);
    token = pScaner->NextToken();
    if (token.type != SEMI)
    {
        temp->child[0] = expression();
        if (!match(SEMI))
        {
            error_flag = true, std::printf("error in line %d: missing ';' in 'return' statement", pScaner->LineNo());
            pScaner->PushBack();
        }
    }
    return temp;
}

void CParser::PrintTree()
{
    PrintTree(pProgram);
}

void CParser::PrintTree(CTreeNode *pNode)
{
    int i;
    indent++;
    while (pNode != NULL)
    {
        for (i = 0; i < indent; i++)
            print_buf("\t");
        switch (pNode->nodekind)
        {
        case kFunDec:
            print_buf("Function declaration: ");
            print_buf(ReservedKeywordList[(int)pNode->type]);
            print_buf(" ");
            print_buf(pNode->szName);
            print_buf("\r\n");
            break;
        case kVarDec:
            print_buf("Variable declaration: ");
            print_buf(ReservedKeywordList[(int)pNode->type]);
            print_buf(" ");
            print_buf(pNode->szName);
            if (pNode->bArray)
                print_buf("[%d]", pNode->iArraySize);
            print_buf("\r\n");
            break;
        case kParam:
            print_buf("parameter: ");
            print_buf(ReservedKeywordList[(int)pNode->type]);
            print_buf(" ");
            print_buf(pNode->szName);
            if (pNode->bArray)
                print_buf("[]");
            print_buf("\r\n");
            break;
        case kStmt:
            switch (pNode->kind.stmt)
            {
            case kRead:
                print_buf("call read(), args:\r\n");
                break;
            case kWrite:
                print_buf("call write(), args:\r\n");
                break;
            case kPrintf:
                print_buf("call printf( \"%s\" )\r\n", pNode->szName.c_str());
                break;
            case kLabel:
                print_buf("label: \"%s\"\r\n", pNode->szName.c_str());
                break;
            case kGoto:
                print_buf("goto\r\n");
                for (i = 0; i <= indent; i++)
                    print_buf("\t");
                print_buf("label: \"%s\"\r\n", pNode->szName.c_str());
                break;
            case kCall:
                print_buf("call %s(), args:\r\n", pNode->szName.c_str());
                break;
            case kIf:
                print_buf("if\r\n");
                break;
            case kWhile:
                print_buf("while\r\n");
                break;
            case kBreak:
                print_buf("break\r\n");
                break;
            case kContinue:
                print_buf("continue\r\n");
                break;
            case kReturn:
                print_buf("return\r\n");
                break;
            default:
                print_buf("Unknown node kind\r\n");
            }
            break;
        case kExp:
            switch (pNode->kind.exp)
            {
            case kOp:
                print_buf("Op: %s\r\n", pNode->szName.c_str());
                break;
            case kConst:
                print_buf("const: %s\r\n", pNode->szName.c_str());
                break;
            case kID:
                print_buf("ID: %s", pNode->szName.c_str());
                if (pNode->bArray)
                {
                    pNode = pNode->child[0];
                    print_buf("[%s]", pNode->szName.c_str());
                }
                print_buf("\r\n");
                break;
            default:
                print_buf("Unkown node kind\r\n");
            }
            break;
        default:
            print_buf("Unkown node kind\r\n");
        }

        for (i = 0; i < MAX_CHILDREN; i++)
            PrintTree(pNode->child[i]);
        pNode = pNode->sibling;
    }
    indent--;
}

// print the string
void CParser::print_buf(char *format, ...)
{
    va_list params;
    static char buf[1024];

    va_start(params, format);
    // sprintf( buf, format, params );//TODO
    std::vsnprintf(buf, 1020, format, params);
    va_end(params);
    std::printf("%s", buf);
}

void CParser::print_buf(std::string &str)
{
    std::cout << str;
}
