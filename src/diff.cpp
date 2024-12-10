#include <math.h>

#include <diff.h>
#include <logs.h>
#include <utils.h>
#include <tree-dsl.h>

static const int buffer_size = 300;

ErrEnum nodeWriteTex(FILE* fout, Node* node, int parent_priority)
{
    if (node == NULL) return ERR_OK;
    if (node->type == TYPE_NUM)
    {
        if (parent_priority != 0 && node->val.num < 0) fputc('(', fout);
        printDouble(fout, node->val.num);
        if (parent_priority != 0 && node->val.num < 0) fputc(')', fout);
        return ERR_OK;
    }
    if (node->type == TYPE_VAR)
    {
        fputc('x', fout);
        return ERR_OK;
    }
    myAssert(node->type == TYPE_OP);

    OpInfo* op_info = NULL;
    returnErr(getOpByCode(node->val.op_code, &op_info));

    int priority_brackets = node->val.op_code == OP_DIV && node->parent != NULL &&
        node->parent->val.op_code == OP_POW && node == node->parent->lft ||
        op_info->priority != 0 && parent_priority > op_info->priority;
    if (priority_brackets) fputc('(', fout);

    for (const char* fmt = op_info->tex_fmt; *fmt != '\0'; ++fmt)    
    {
        if (*fmt == '%')
        {
            ++fmt;
            if (*fmt == 'l')
            {
                if (node->val.op_code == OP_POW) nodeWriteTex(fout, node->lft, 4);
                else nodeWriteTex(fout, node->lft, op_info->priority);
            }
            else if (*fmt == 'r')
            {
                if (node->val.op_code == OP_POW) nodeWriteTex(fout, node->rgt, 0);
                else nodeWriteTex(fout, node->rgt, op_info->priority);
            }
            else myAssert("Invalid format specifier" && 0);
        }
        else fputc(*fmt, fout);
    }

    if (priority_brackets) fputc(')', fout);
    return ERR_OK;
}

ErrEnum treeWriteTex(FILE* fout, Node* node)
{
    myAssert(fout != NULL);

    fputs("\\[", fout);
    returnErr(nodeWriteTex(fout, node, 0));
    fputs("\\]\n", fout);

    return ERR_OK;
}

ErrEnum makeArticle(Node* node, const char* fout_path)
{
    myAssert(fout_path != NULL);

    FILE* fout = fopen(fout_path, "w");
    if (fout == NULL) return ERR_OPEN_FILE;

    fputs("\\documentclass{article}\n\\usepackage[english]{babel}\n\n"
    "\\begin{document}\nFunction:\n", fout);
    treeWriteTex(fout, node);

    // simplify(node);
    // fputs("Simplified function:\n", fout);
    // treeWriteTex(fout, node);

    Node* deriv = NULL;
    returnErr(diff(node, &deriv));
    simplify(deriv);
    fputs("Derivative:\n", fout);
    treeWriteTex(fout, deriv);
    fputs("\\end{document}\n", fout);

    fclose(fout);
    return ERR_OK;
}

void evaluate(Node* node, double x, double* ans)
{
    myAssert(node != NULL);

    if (node->type == TYPE_NUM)
    {
        *ans = node->val.num;
        return;
    }
    if (node->type == TYPE_VAR)
    {
        *ans = x;
        return;
    }
    myAssert(node->type == TYPE_OP);

    #define DIFF_OP(name, n_operands, eval, priority, text, tex_fmt) \
    case OP_ ## name:                                                \
    {                                                                \
        double arg1 = 0, arg2 = 0;                                   \
        evaluate(node->lft, x, &arg1);                               \
        if (n_operands == 2) evaluate(node->rgt, x, &arg2);          \
        *ans = eval;                                                 \
        return;                                                      \
    }

    switch (node->val.op_code)
    {
        #include <operations.h>
        default: myAssert(0);
    }

    return;
    #undef DIFF_OP
}

void nodeIsConst(Node* node, int* ans)
{
    myAssert(ans != NULL);
    if (node == NULL)
    {
        *ans = 1;
        return;
    }
    int ans_child = 0;
    nodeIsConst(node->lft, &ans_child);
    if (!ans_child)
    {
        *ans = 0;
        return;
    }
    nodeIsConst(node->rgt, &ans_child);
    if (!ans_child)
    {
        *ans = 0;
        return;
    }
    if (node->type == TYPE_VAR)
    {
        *ans = 0;
        return;
    }
    *ans = 1;
}

ErrEnum diffWrite(FILE* fout, Node* node, Node** deriv)
{
    if (fout != NULL)
    {
        fprintf(fout, "Дифференцируя\n");
        returnErr(treeWriteTex(fout, node));
    }
    returnErr(diff(node, deriv));
    if (fout != NULL)
    {
        fprintf(fout, "Получаем:\n");
        returnErr(treeWriteTex(fout, *deriv));
    }
    return ERR_OK;
}

ErrEnum diff(Node* node, Node** deriv)
{
    myAssert(node != NULL && deriv != NULL && *deriv == NULL);

    if (node->type == TYPE_NUM)
    {
        returnErr(nodeCtor(deriv, TYPE_NUM, {.num = 0}, NULL, NULL, NULL));
        return ERR_OK;
    }
    if (node->type == TYPE_VAR)
    {
        returnErr(nodeCtor(deriv, TYPE_NUM, {.num = 1}, NULL, NULL, NULL));
        return ERR_OK;
    }
    myAssert(node->type == TYPE_OP);

    Node *deriv_lft = NULL, *deriv_rgt = NULL;
    diff(node->lft, &deriv_lft);
    if (node->rgt != NULL) diff(node->rgt, &deriv_rgt);
    switch (node->val.op_code)
    {
        case OP_ADD:
            *deriv = _ADD(deriv_lft, deriv_rgt);
            return ERR_OK;
        case OP_SUB:
            *deriv = _SUB(deriv_lft, deriv_rgt);
            return ERR_OK;
        case OP_MUL:
        {
            *deriv = _ADD(_MUL(deriv_lft, _COPY(node->rgt)), _MUL(_COPY(node->lft), deriv_rgt));
            return ERR_OK;
        }
        case OP_DIV:
        {
            *deriv = _DIV(_SUB(_MUL(deriv_lft, _COPY(node->rgt)), _MUL(_COPY(node->lft), deriv_rgt)), _POW(_COPY(node->rgt), _NUM(2)));
            return ERR_OK;
        }
        case OP_POW:
        {
            int f_const = 0, g_const = 0;
            nodeIsConst(node->lft, &f_const);
            nodeIsConst(node->rgt, &g_const);
            if (f_const && g_const)
            {
                // (c1 ^ c2)' = 0
                *deriv = _NUM(0);
                return ERR_OK;
            }
            if (!f_const && g_const)
            {
                // (f ^ c)' = f' * c * f ^ {c - 1}
                double g_val = 0;
                evaluate(node->rgt, 0, &g_val);
                *deriv = _MUL(deriv_lft, _MUL(_NUM(g_val), _POW(_COPY(node->lft), _NUM(g_val - 1))));
                return ERR_OK;
            }
            if (f_const && !g_const)
            {
                // (c ^ g)' = g' * lnc * c ^ g
                double f_val = 0;
                evaluate(node->lft, 0, &f_val);
                *deriv = _MUL(deriv_rgt, _MUL(_NUM(log(f_val)), _POW(_NUM(f_val), _COPY(node->rgt))));
                return ERR_OK;
            }
            // (f ^ g)' = (exp(g * ln(f)))' = f ^ g * (g' * lnf + g * f' / f)
            *deriv = _MUL(_POW(_COPY(node->lft), _COPY(node->rgt)), 
            _ADD(_MUL(deriv_rgt, _LN(_COPY(node->lft))), _DIV(_MUL(_COPY(node->rgt), deriv_lft), _COPY(node->lft))));
            return ERR_OK;
        }
        case OP_EXP:
        {
            *deriv = _MUL(deriv_lft, _COPY(node));
            return ERR_OK;
        }
        case OP_LN:
        {
            *deriv = _DIV(deriv_lft, _COPY(node->lft));
            return ERR_OK;
        }
        case OP_SIN:
        {
            *deriv = _MUL(deriv_lft, _COS(_COPY(node->lft)));
            return ERR_OK;
        }
        case OP_COS:
        {
            *deriv = _MUL(_NUM(-1), _MUL(deriv_lft, _SIN(_COPY(node->lft))));
            return ERR_OK;
        }
        case OP_TAN:
        {  
            Node *f_copy = NULL, *cosf = NULL, *num2 = NULL, *cosf_squared = NULL;
            *deriv = _DIV(deriv_lft, _POW(_COS(_COPY(node->lft)), _NUM(2)));
            return ERR_OK;
        }
        case OP_ARCSIN:
        {
            *deriv = _DIV(deriv_lft, _POW(_SUB(_NUM(1), _POW(_COPY(node->lft), _NUM(2))), _NUM(0.5)));
            return ERR_OK;
        }
        case OP_ARCCOS:
        {
            *deriv = _MUL(_NUM(-1), _DIV(deriv_lft, _POW(_SUB(_NUM(1), _POW(_COPY(node->lft), _NUM(2))), _NUM(0.5))));
            return ERR_OK;
        }
        case OP_ARCTAN:
        {
            *deriv = _DIV(deriv_lft, _ADD(_NUM(1), _POW(_COPY(node->lft), _NUM(2))));
            return ERR_OK;
        }
        default: return ERR_INVAL_OP_CODE;
    }
}

int simplifyCase(Node* node, NodeChild crit_child, double crit_val, int replace_with_num, int replacement)
{
    myAssert(node != NULL);

    Node *crit_node = NULL, *expr_node = NULL;
    if (crit_child == LFT_NODE)
    {
        crit_node = node->lft;
        expr_node = node->rgt;
    }
    else if (crit_child == RGT_NODE)
    {
        crit_node = node->rgt;
        expr_node = node->lft;
    }
    else myAssert(0);

    if (crit_node->type == TYPE_NUM && isZero(crit_node->val.num - crit_val))
    {
        if (replace_with_num)
        {
            node->type = TYPE_NUM;
            node->val.num = replacement;
            nodeDtor(node->lft);
            nodeDtor(node->rgt);
            node->lft = NULL;
            node->rgt = NULL;
            return 1;
        }
        if (!isZero(replacement - 1))
        {
            node->val.op_code = OP_MUL;
            crit_node->val.num = replacement;
            return 1;
        }
        node->type = expr_node->type;
        node->val = expr_node->val;
        node->lft = expr_node->lft;
        node->rgt = expr_node->rgt;

        expr_node->lft = expr_node->rgt = NULL;
        if (node->lft != NULL) node->lft->parent = node;
        if (node->rgt != NULL) node->rgt->parent = node;
        nodeDtor(expr_node);
        nodeDtor(crit_node);
        return 1;
    }
    return 0;
}

void simplify(Node* node)
{
    myAssert(node != NULL);

    if (node->type != TYPE_OP) return;
    simplify(node->lft);
    if (node->rgt != NULL) simplify(node->rgt);
    
    if (node->lft->type == TYPE_NUM && node->rgt != NULL && node->rgt->type == TYPE_NUM)
    {
        evaluate(node, 0, &(node->val.num));
        node->type = TYPE_NUM;
        nodeDtor(node->lft);
        nodeDtor(node->rgt);
        node->lft = NULL;
        node->rgt = NULL;
        return;
    }

    if (node->val.op_code == OP_ADD)
    {
        if (simplifyCase(node, LFT_NODE, 0, 0, 1)) return;
        if (simplifyCase(node, RGT_NODE, 0, 0, 1)) return;
    }
    else if (node->val.op_code == OP_SUB)
    {
        if (simplifyCase(node, LFT_NODE, 0, 0, -1)) return;
        if (simplifyCase(node, RGT_NODE, 0, 0, 1)) return;
    }
    else if (node->val.op_code == OP_MUL)
    {
        if (simplifyCase(node, LFT_NODE, 0, 1, 0)) return;
        if (simplifyCase(node, RGT_NODE, 0, 1, 0)) return;
        if (simplifyCase(node, LFT_NODE, 1, 0, 1)) return;
        if (simplifyCase(node, RGT_NODE, 1, 0, 1)) return;
    }
    else if (node->val.op_code == OP_DIV)
    {
        if (simplifyCase(node, RGT_NODE, 1, 0, 1)) return;
    }
    else if (node->val.op_code == OP_POW)
    {
        if (simplifyCase(node, RGT_NODE, 0, 1, 1)) return;
        if (simplifyCase(node, RGT_NODE, 1, 0, 1)) return;
        if (simplifyCase(node, LFT_NODE, 0, 1, 0)) return;
        if (simplifyCase(node, LFT_NODE, 1, 1, 1)) return;
    }
}