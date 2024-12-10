#include <stdlib.h>

#include <parser.h>
#include <utils.h>
#include <colors.h>
#include <tree-dsl.h>

#define cur_chr (pars->s[pars->p])
#define INCR_P ++(pars->p)
#define SYNT_ERR(exp) {pars->synt_err = {1, pars->p, exp, cur_chr}; return ERR_OK;}
#define NO_SYNT_ERR_RET {pars->synt_err.err = 0; return ERR_OK;}
#define RET_IF_SYNT_ERR if (pars->synt_err.err) return ERR_OK
#define CALL_SYNT_ERR if (pars->synt_err.err) syntaxErr(&pars->synt_err)

static void syntaxErr(SyntaxErr* synt_err);

static ErrEnum getG(Parser* pars, Node** node);
static ErrEnum getE(Parser* pars, Node** node, int rule_num);
static ErrEnum getP(Parser* pars, Node** node);
static ErrEnum getF(Parser* pars, Node** node);
static ErrEnum getN(Parser* pars, Node** node);
static ErrEnum getV(Parser* pars, Node** node);

ErrEnum treeParse(Node** node, const char *expr_txt_path)
{
    myAssert(node != NULL && *node == NULL);

    Parser pars = {};
    returnErr(readFile(expr_txt_path, (void**)(&(pars.s)), &pars.buf_size));

    returnErr(getG(&pars, node));
    return ERR_OK;
}

void syntaxErr(SyntaxErr* synt_err)
{
    myAssert(synt_err != NULL && synt_err->err);

    printf("%sSyntax error at pos %d: expected %s, got %c%s\n", 
    RED_STR, synt_err->pos, synt_err->exp, synt_err->got, DEFAULT_STR);
    exit(0);
}

ErrEnum getG(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);  

    returnErr(getE(pars, node, 0));
    CALL_SYNT_ERR;
    if (cur_chr != '$')
    {
        SYNT_ERR("$");
        CALL_SYNT_ERR;
    }
    INCR_P;
    return ERR_OK;
}

ErrEnum getE(Parser* pars, Node** node, int rule_num)
{
    const int max_rule_num = 2;
    myAssert(pars != NULL && node != NULL && *node == NULL);
    myAssert(0 <= rule_num || rule_num <= max_rule_num);

    if (rule_num == max_rule_num) returnErr(getP(pars, node))
    else returnErr(getE(pars, node, rule_num + 1));
    RET_IF_SYNT_ERR;

    while (rule_num == 0 && (cur_chr == '+' || cur_chr == '-') || 
           rule_num == 1 && (cur_chr == '*' || cur_chr == '/') || 
           rule_num == 2 && cur_chr == '^')
    {
        OpEnum op_code = OP_ADD;
        switch (cur_chr)
        {
            case '+': op_code = OP_ADD; break; // copypaste
            case '-': op_code = OP_SUB; break;
            case '*': op_code = OP_MUL; break;
            case '/': op_code = OP_DIV; break;
            case '^': op_code = OP_POW; break;
            default: myAssert("Invalid operator" && 0);
        }
        (*node)->parent = _NODE(TYPE_OP, {.op_code = op_code}, NULL, *node, NULL);
        *node = (*node)->parent;
        INCR_P;

        if (rule_num == max_rule_num) returnErr(getP(pars, &(*node)->rgt))
        else returnErr(getE(pars, &(*node)->rgt, rule_num + 1));
        RET_IF_SYNT_ERR;

        (*node)->rgt->parent = *node;
    }
    NO_SYNT_ERR_RET;
}

ErrEnum getP(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);

    if (cur_chr == '(')
    {
        INCR_P;
        returnErr(getE(pars, node, 0));
        RET_IF_SYNT_ERR;
        if (cur_chr != ')') SYNT_ERR(")");
        INCR_P;
        NO_SYNT_ERR_RET;
    }
    returnErr(getN(pars, node));
    if (pars->synt_err.err == 0) return ERR_OK;
    returnErr(getV(pars, node));
    if (pars->synt_err.err == 0) return ERR_OK;
    returnErr(getF(pars, node));
    return ERR_OK;
}

ErrEnum getF(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);

    int old_p = pars->p;
    OpInfo *op_info = NULL;
    if (getOpByStr(&cur_chr, &op_info) != ERR_OK) SYNT_ERR("FUNCTION NAME");

    #define GET_F_CASE(name) case OP_ ## name: *node = _ ## name (NULL); break;
    switch (op_info->op_code)
    {
        GET_F_CASE(EXP); // copypaste
        GET_F_CASE(LN);
        GET_F_CASE(SIN);
        GET_F_CASE(COS);
        GET_F_CASE(TAN);
        GET_F_CASE(ARCSIN);
        GET_F_CASE(ARCCOS);
        GET_F_CASE(ARCTAN);
        default: SYNT_ERR("FUNCTION NAME");
    }
    pars->p += op_info->op_str_len;

    if (cur_chr != '(')
    {
        nodeDtor(*node);
        *node = NULL;
        pars->p = old_p;
        SYNT_ERR("(");
    }
    INCR_P;

    returnErr(getE(pars, &(*node)->lft, 0));
    RET_IF_SYNT_ERR;
    (*node)->lft->parent = *node;

    if (cur_chr != ')')
    {
        nodeDtor(*node);
        *node = NULL;
        pars->p = old_p;
        SYNT_ERR(")");
    }
    INCR_P;

    NO_SYNT_ERR_RET;
}

ErrEnum getN(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);

    // int old_p = pars->p;
    // long long val = 0;
    // while (cur_chr >= '0' && cur_chr <= '9')
    // {
    //     val = val * 10 + cur_chr - '0';
    //     INCR_P;
    // }
    // if (pars->p == old_p) SYNT_ERR("[0-9]");

    // *node = _NUM((double)val);
    // NO_SYNT_ERR_RET;

    int pos_incr = 0;
    double val = 0;
    if (sscanf(&cur_chr, "%lf%n", &val, &pos_incr) == 0) SYNT_ERR("double");
    pars->p += pos_incr;
    *node = _NUM(val);
    NO_SYNT_ERR_RET;
}

ErrEnum getV(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);

    if (cur_chr != 'x') SYNT_ERR("x");
    *node = _VAR();
    INCR_P;
    NO_SYNT_ERR_RET;
}