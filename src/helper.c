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

Node *variable_struct_or_union_node(Node *node)
{
    if (!node_is_struct_or_union(node))
    {
        return NULL;
    }

    if (node->var.datatype.type == DATA_TYPE_STRUCT)
    {
        return node->var.datatype.struct_node->_struct.n_body;
    }

#warning "Don't remember to imlement unions"

}

int padding(int value, int to)
{
    if (to <= 0)
    {
        return 0;
    }

    if ((value % to) == 0)
    {
        return 0;
    }

    return to - (value % to) % to;
}

int align_value(int value, int to)
{
    if (value && to)
    {
        value += padding(value, to);
    }

    return value;
}

int align_value_treat_positive(int value, int to)
{
    assert(to >= 0);
    if (value < 0)
    {
        to = -to;
    }

    return align_value(value, to);
}

int compute_sum_padding(mound *nodes)
{
    int padding = 0, last_type = -1;
    bool isMixedType = false;
    set_peek(nodes, 0);
    Node *current_node = next_ptr(nodes), *last_node = NULL;
    while (current_node)
    {
        if (current_node->type != NODE_TYPE_VARIABLE)
        {
            current_node = next_ptr(nodes);
            continue;
        }
        else if (current_node->type == NODE_TYPE_VARIABLE_LIST)
        {
            padding += compute_sum_padding(current_node->var_list.list);
            last_type = -1;
            goto over;
        }

        padding += current_node->var.padding;
        last_type = current_node->var.datatype.type;
    over:
        last_node = current_node;
        current_node = next_ptr(nodes);
    }
    return padding;
}
