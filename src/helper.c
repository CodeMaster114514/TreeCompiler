#include "compiler.h"

size_t variable_size(Node *node)
{
    assert(node && node->type == NODE_TYPE_VARIABLE);
    return datatype_size(&node->var.datatype);
}

size_t variable_size_for_list(Node *node)
{
    assert(node && node->type == NODE_TYPE_VARIABLE_LIST);
    size_t size = 0;
    set_peek(node->var_list.list, 0);
    Node *var = peek(node->var_list.list);
    while (var)
    {
        size += datatype_size(&var->var.datatype);
        var = peek(node->var_list.list);
    }

    return size;
}