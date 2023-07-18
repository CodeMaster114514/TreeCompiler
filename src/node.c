#include "compiler.h"

static mound *node = NULL;
static mound *node_root = NULL;

void set_mound(mound *node_set, mound *node_root_set)
{
	node = node_set;
	node_root = node_root_set;
}

void free_node(Node *data)
{
	if (data->type == NODE_TYPE_NUMBER || data->type == NODE_TYPE_IDENTIFIER)
	{
		free(data);
		return;
	}
	if (data->type == NODE_TYPE_EXPRESSION)
	{
		free_node(data->exp.node_left);
		free_node(data->exp.node_right);
		return;
	}
}

void free_nodes()
{
	int node_count = get_count(node);
	int node_root_count = get_count(node_root);
	set_peek(node, 0);
	set_peek(node_root, 0);
	bool nodeIsAllOk = false, rootIsAllOk = false;
	Node **data, **root;
	for (;;)
	{
		if (!nodeIsAllOk)
		{
			data = peek(node);
			if (!data)
			{
				nodeIsAllOk = true;
			}
			else
			{
				free_node(*data);
				next(node);
			}
		}
		if (!rootIsAllOk)
		{
			root = peek(node_root);
			if (!root)
			{
				rootIsAllOk = true;
			}
			else
			{
				if (*root == *data)
				{
					next(node_root);
				}
				else
				{
					free_node(*root);
					next(node_root);
				}
			}
		}
		if (rootIsAllOk && nodeIsAllOk)
			break;
	}
}

void push_node(Node *data)
{
	push(node, &data);
}

Node *next_node()
{
	Node **data = next(node);
	if (!data)
		return NULL;
	return *data;
}

Node *peek_node()
{
	Node **data = peek(node);
	if (!data)
		return NULL;
	return *(Node **)peek(node);
}

Node *pop_node()
{
	Node *last_node = *(Node **)last_data(node);
	Node *last_node_root;
	if (!isEmpty(node_root))
	{
		last_node_root = *(Node **)last_data(node_root);
	}
	else
	{
		last_node_root = NULL;
	}
	pop(node);
	if (last_node == last_node_root)
	{
		pop(node_root);
	}
	return last_node;
}

bool node_is_expressionable(Node *node)
{
	return node->type == NODE_TYPE_EXPRESSION || node->type == NODE_TYPE_EXPRESSION_PARENTHESES || node->type == NODE_TYPE_UNARY || node->type == NODE_TYPE_IDENTIFIER || node->type == NODE_TYPE_NUMBER || node->type == NODE_TYPE_STRING;
}

Node *node_peek_expressionable()
{
	Node *node = next_node();
	if (!node)
	{
		return NULL;
	}
	return node_is_expressionable(node) ? node : NULL;
}

Node *make_exp_node(Node *left, Node *right, char *op)
{
	assert(left);
	assert(right);
	return node_creat(&(Node){.type = NODE_TYPE_EXPRESSION, .exp.node_left = left, .exp.node_right = right, .exp.op = op});
}

Node *node_creat(Node *_node)
{
	Node *node = calloc(1, sizeof(Node));
	memcpy(node, _node, sizeof(Node));
	push_node(node);
	return node;
}
