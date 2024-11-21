#include <stdio.h>
#include <string.h>

#include <diff.h>
#include <error.h>
#include <utils.h>

#include <stdlib.h>

int main(int argc, const char* argv[])
{
    Node* root = NULL;
    handleErr(treeRead(&root));
    // handleErr(treeWrite(root));
    handleErr(treeDump(root));
    handleErr(treeWriteTex(root));

    Node* root_deriv = NULL;
    returnErr(diff(root, &root_deriv));
    handleErr(treeDump(root_deriv));
    handleErr(treeWriteTex(root_deriv));

    // double y = 0;
    // evaluate(root, -1, &y);
    // printf("evaluates to %lf\n", y);

    nodeDtor(root);
    return 0;
}