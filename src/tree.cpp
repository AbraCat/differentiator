#include <stdlib.h>
#include <string.h>

#include <tree.h>
#include <utils.h>
#include <logs.h>

int dump_cnt = 0;
static const int n_ops = 4, buffer_size = 300;
static const char tree_path[] = "./txt/tree.txt";

#define OP_STR_CASE(op, str) {OP_ ## op, str, sizeof str - 1}
static OpDescr op_strs[] = {
    OP_STR_CASE(ADD, "+"),
    OP_STR_CASE(SUB, "-"),
    OP_STR_CASE(MUL, "*"),
    OP_STR_CASE(DIV, "/")
};
#undef OP_STR_CASE

ErrEnum nodeCtor(Node** node, NodeType type, NodeVal val, Node* parent, Node* lft, Node* rgt)
{
    myAssert(node != NULL);

    if (*node == NULL) 
    {
        *node = (Node*)calloc(1, sizeof(Node));
        if (*node == NULL) return ERR_MEM;
    }

    (*node)->type = type;
    (*node)->val = val;
    (*node)->parent = parent;
    (*node)->lft = lft;
    (*node)->rgt = rgt;

    (*node)->n_nodes = 0;
    if (lft != NULL)
    {
        if (lft->parent != NULL) return ERR_PARENT_DISCARDED;
        (*node)->n_nodes += lft->n_nodes;
    }
    if (rgt != NULL)
    {
        if (rgt->parent != NULL) return ERR_PARENT_DISCARDED;
        (*node)->n_nodes += rgt->n_nodes;
    }

    return ERR_OK;
}

void nodeDtor(Node* node)
{
    if (node == NULL) return;
    nodeDtor(node->lft);
    nodeDtor(node->rgt);
    free(node);
}

void nNodes(Node* node, int* ans)
{
    myAssert(ans != NULL);

    if (node == NULL)
    {
        *ans = 0;
        return;
    }
    int ans_lft = 0, ans_rgt = 0;
    nNodes(node->lft, &ans_lft);
    nNodes(node->rgt, &ans_rgt);
    *ans = ans_lft + ans_rgt + 1;
}

ErrEnum nodeVerify(Node* node)
{
    if (node == NULL) return ERR_OK;
    if (node->lft != NULL && node->lft->parent != node) return ERR_INVAL_CONNECT;
    if (node->rgt != NULL && node->rgt->parent != node) return ERR_INVAL_CONNECT;

    int n_nodes = 0;
    nNodes(node, &n_nodes);
    if (node->n_nodes != n_nodes) return ERR_INVAL_NNODES;

    returnErr(nodeVerify(node->lft));
    returnErr(nodeVerify(node->lft));
    return ERR_OK;
}

ErrEnum getOpByCode(OpEnum op_code, OpDescr** ans)
{
    myAssert(ans != NULL);

    for (int ind = 0; ind < n_ops; ++ind)
    {
        if (op_strs[ind].op_code == op_code)
        {
            *ans = op_strs + ind;
            return ERR_OK;
        }
    }
    *ans = NULL;
    return ERR_INVAL_OP_CODE;
}

ErrEnum getOpByStr(const char* op_str, OpDescr** ans)
{
    myAssert(op_str != NULL && ans != NULL);

    for (int ind = 0; ind < n_ops; ++ind)
    {
        if (strcmpToBracket(op_str, op_strs[ind].op_str) == 0)
        {
            *ans = op_strs + ind;
            return ERR_OK;
        }
    }
    *ans = NULL;
    return ERR_INVAL_OP_STR;
}

void nodeWrite(FILE* fout, Node* node)
{
    #define OP_CASE(op)       \
        case OP_ ## op:       \
            fputs(#op, fout); \
            break


    if (node == NULL) return;
    fputc('(', fout);
    nodeWrite(fout, node->lft);
    
    switch (node->type)
    {
        case TYPE_NUM:
            fprintf(fout, "%lf", node->val.num);
            break;
        case TYPE_VAR:
            fputc('x', fout);
            break;
        case TYPE_OP:
        {
            OpDescr* op_descr = NULL;
            fputs(getOpByCode(node->val.op_code, &op_descr) == ERR_OK ? op_descr->op_str : "BAD_OP", fout);
            break;
        }
        default:
            fputs("BAD_TYPE", fout);
            break;
    }

    nodeWrite(fout, node->rgt);
    fputc(')', fout);

    #undef OP_CASE
}

ErrEnum treeWrite(Node* node)
{
    FILE* fout = fopen(tree_path, "w");
    if (fout == NULL) return ERR_OPEN_FILE;

    nodeWrite(fout, node);

    fclose(fout);
    return ERR_OK;
}

ErrEnum nodeRead(char* buf, int* buf_pos, Node* node, int* n_nodes, int buf_size)
{
    #define cur_buf (buf + *buf_pos)

    #define INCR_BUF_POS(incr)    \
        *buf_pos += incr;         \
        if (*buf_pos >= buf_size) \
            return ERR_BUF_BOUND;

    ++(*n_nodes);
    int pos_incr = 0;

    if (cur_buf[0] != '(') return ERR_TREE_FMT;
    INCR_BUF_POS(1);
    if (cur_buf[0] == '(')
    {
        returnErr(nodeCtor(&node->lft, TYPE_NUM, {.num = 0}, node, NULL, NULL));
        returnErr(nodeRead(buf, buf_pos, node->lft, n_nodes, buf_size));
    }

    if (sscanf(cur_buf, "%lf%n", &node->val.num, &pos_incr) == 1)
    {
        INCR_BUF_POS(pos_incr);
        node->type = TYPE_NUM;
    }
    else if (cur_buf[0] == 'x')
    {
        INCR_BUF_POS(1);
        node->type = TYPE_VAR;
        node->val.var_id = 0;
    }
    else
    {
        OpDescr *op_descr = NULL;
        returnErr(getOpByStr(cur_buf, &op_descr));
        node->type = TYPE_OP;
        node->val.op_code = op_descr->op_code;
        INCR_BUF_POS(op_descr->op_str_len);
    }

    if (cur_buf[0] == '(')
    {
        returnErr(nodeCtor(&node->rgt, TYPE_NUM, {.num = 0}, node, NULL, NULL));
        returnErr(nodeRead(buf, buf_pos, node->rgt, n_nodes, buf_size));
    }
    if (*cur_buf != ')') return ERR_TREE_FMT;
    INCR_BUF_POS(1);

    return ERR_OK;
}

ErrEnum treeRead(Node** tree)
{
    myAssert(tree != NULL && *tree == NULL);

    FILE* fin = fopen(tree_path, "r");
    if (fin == NULL) return ERR_OPEN_FILE;

    int buf_size = 0, buf_pos = 0;
    returnErr(fileSize(fin, (long*)&buf_size));
    ++buf_size; // for '\0'

    char* buf = NULL;
    CALLOC_BUF(buf, buf_size);
    fread(buf, sizeof(char), buf_size, fin);
    fclose(fin);
    buf[buf_size - 1] = '\0';

    returnErr(nodeCtor(tree, TYPE_NUM, {.num = 0}, NULL, NULL, NULL));
    (*tree)->n_nodes = 0;
    returnErr(nodeRead(buf, &buf_pos, *tree, &((*tree)->n_nodes), buf_size));

    free(buf);
    return ERR_OK;
}

void nodeWriteTex(FILE* fout, Node* node)
{
    #define OP_CASE(op)       \
        case OP_ ## op:       \
            fputs(#op, fout); \
            break

    if (node == NULL) return;
    if (node->type == TYPE_NUM)
    {
        fprintf(fout, "%lf", node->val.num);
        return;
    }
    if (node->type == TYPE_VAR)
    {
        fputc('x', fout);
        return;
    }
    myAssert(node->type == TYPE_OP);
    myAssert(node->lft != NULL && node->rgt != NULL);
    
    // brackets ?
    
    if (node->val.op_code == OP_DIV)
    {
        fputs("\\frac{", fout);
        nodeWriteTex(fout, node->lft);
        fputs("}{", fout);
        nodeWriteTex(fout, node->rgt);
        fputc('}', fout);
        return;
    }

    if (node->val.op_code == OP_ADD || node->val.op_code == OP_SUB || node->val.op_code == OP_MUL)
    {
        OpDescr* op_descr = NULL;
        getOpByCode(node->val.op_code, &op_descr);
        myAssert(op_descr != NULL);

        int priority_brackets = 0;
        if ((node->val.op_code == OP_ADD || node->val.op_code == OP_SUB) && 
        node->parent != NULL && node->parent->val.op_code == OP_MUL)
            priority_brackets = 1;

        if (priority_brackets) fputc('(', fout);
        nodeWriteTex(fout, node->lft);
        fputs(op_descr->op_str, fout);
        nodeWriteTex(fout, node->rgt);
        if (priority_brackets) fputc(')', fout);
        return;
    }

    myAssert("nodeWriteTex(): invalid node" && 0);
    #undef OP_CASE
}

ErrEnum treeWriteTex(Node* node)
{
    char tex_path[buffer_size] = "";
    sprintf(tex_path, "%s/tex/tree.txt", log_path);
    FILE* fout = fopen(tex_path, "a");
    if (fout == NULL) return ERR_OPEN_FILE;

    fputc('$', fout);
    nodeWriteTex(fout, node);
    fputs("$\n", fout);

    fclose(fout);
    return ERR_OK;
}

void printNodeDot(FILE* fout, Node* node)
{
    #define TYPE_CASE(type, ...)    \
        case TYPE_ ## type:         \
            fputs(#type "|", fout); \
            __VA_ARGS__             \
            break;

    #define OP_CASE(op)       \
        case OP_ ## op:       \
            fputs(#op, fout); \
            break

    // node123 [shape = Mrecord, label = "{type | val | { lft | rgt }}"]
    fprintf(fout, "node%p [shape = Mrecord , label = \"{node %p|", node, node);
    switch (node->type)
    {
        TYPE_CASE(OP, 
            switch(node->val.op_code)
            {
                OP_CASE(ADD);
                OP_CASE(SUB);
                OP_CASE(MUL);
                OP_CASE(DIV);
                default: fprintf(fout, "BAD_OP");
            }
        );

        TYPE_CASE(VAR, fputc('x', fout););
        TYPE_CASE(NUM, fprintf(fout, "%lf", node->val.num););

        default:
            fputs("BAD_TYPE", fout);
            break;
    }
    fprintf(fout, "|<parent>parent %p|{<lft>lft %p|<rgt>rgt %p}}\"]\n", node->parent, node->lft, node->rgt);

    if (node->lft != NULL)
    {
        printNodeDot(fout, node->lft);
        fprintf(fout, "node%p:<lft> -> node%p[color = red]\n", node, node->lft);
    }
    if (node->rgt != NULL)
    {
        printNodeDot(fout, node->rgt);
        fprintf(fout, "node%p:<rgt> -> node%p[color = green]\n", node, node->rgt);
    }

    #undef TYPE_CASE
    #undef OP_CASE
}

ErrEnum treeMakeGraph(Node* tree)
{
    myAssert(tree != NULL);

    char buf[buffer_size] = "";

    sprintf(buf, "%s/dot-src/dot-src.txt", log_path);
    FILE *fout = fopen(buf, "w");
    if (fout == NULL) return ERR_OPEN_FILE;

    fputs("digraph Tree\n{\nrankdir = TB\n", fout);
    if (tree == NULL) fputs("NULL [shape = Mrecord]\n", fout);
    else printNodeDot(fout, tree);
    fputs("}\n", fout);

    fclose(fout);

    sprintf(buf, "dot %s/dot-src/dot-src.txt -Tpng -o%s/dot-img/dot-img-%d.png", 
    log_path, log_path, dump_cnt);
    system(buf);

    return ERR_OK;
}

ErrEnum treeDump(Node* tree)
{
    myAssert(tree != NULL);

    char buf[buffer_size] = "";
    openDumpFile();

    // dump text

    sprintf(buf, "../dot-img/dot-img-%d.png", dump_cnt);
    fprintf(fdump, "\n\n<img src=\"%s\" alt=\"graph image\"/>\n\n", buf);

    treeMakeGraph(tree);
    ++dump_cnt;
    return ERR_OK;
}

ErrEnum nodeCopy(Node* src, Node** dest)
{
    myAssert(src != NULL && dest != NULL && *dest == NULL);

    Node *lft_copy = NULL, *rgt_copy = NULL;
    if (src->lft != NULL) 
    {
        returnErr(nodeCopy(src->lft, &lft_copy));
    }
    if (src->rgt != NULL) 
    {
        returnErr(nodeCopy(src->rgt, &rgt_copy));
    }
    returnErr(nodeCtor(dest, src->type, src->val, NULL, lft_copy, rgt_copy));
    if (lft_copy != NULL) lft_copy->parent = *dest;
    if (rgt_copy != NULL) rgt_copy->parent = *dest;

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
    myAssert(node->lft != NULL && node->rgt != NULL);
    
    #define OP_OPERATOR_CASE(op, operator)  \
        if (node->val.op_code == OP_ ## op) \
        {                                   \
            double y1 = 0, y2 = 0;          \
            evaluate(node->lft, x, &y1);    \
            evaluate(node->rgt, x, &y2);    \
            *ans = y1 operator y2;          \
            return;                         \
        }

    OP_OPERATOR_CASE(ADD, +);
    OP_OPERATOR_CASE(SUB, -);
    OP_OPERATOR_CASE(MUL, *);
    OP_OPERATOR_CASE(DIV, /);

    #undef OP_OPERATOR_CASE
    return;
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
    switch (node->val.op_code)
    {
        case OP_ADD:
            returnErr(diff(node->lft, &deriv_lft));
            returnErr(diff(node->rgt, &deriv_rgt));
            returnErr(nodeCtor(deriv, TYPE_OP, {.op_code = OP_ADD}, NULL, deriv_lft, deriv_rgt));
            deriv_lft->parent = deriv_rgt->parent = *deriv;
            return ERR_OK;
        case OP_SUB:
            returnErr(diff(node->lft, &deriv_lft));
            returnErr(diff(node->rgt, &deriv_rgt));
            returnErr(nodeCtor(deriv, TYPE_OP, {.op_code = OP_SUB}, NULL, deriv_lft, deriv_rgt));
            deriv_lft->parent = deriv_rgt->parent = *deriv;
            return ERR_OK;
        case OP_MUL:
        {
            Node *lft_copy = NULL, *rgt_copy = NULL;
            returnErr(nodeCopy(node->lft, &lft_copy));
            returnErr(nodeCopy(node->rgt, &rgt_copy));
            returnErr(diff(node->lft, &deriv_lft));
            returnErr(diff(node->rgt, &deriv_rgt));

            Node *gdf = NULL, *fdg = NULL;
            returnErr(nodeCtor(&gdf, TYPE_OP, {.op_code = OP_MUL}, NULL, deriv_lft, rgt_copy));
            deriv_lft->parent = rgt_copy->parent = gdf;
            returnErr(nodeCtor(&fdg, TYPE_OP, {.op_code = OP_MUL}, NULL, lft_copy, deriv_rgt));
            lft_copy->parent = deriv_rgt->parent = fdg;
            returnErr(nodeCtor(deriv, TYPE_OP, {.op_code = OP_ADD}, NULL, gdf, fdg));
            gdf->parent = fdg->parent = *deriv;
            return ERR_OK;
        }
        case OP_DIV:
        {
            return ERR_OK;
        }
        default: return ERR_INVAL_OP_CODE;
    }
}