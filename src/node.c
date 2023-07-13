#include "compiler.h"

static mound *node = NULL;
static mound *node_root = NULL;

void set_mound(mound *node_set, mound *node_root_set)
{
	node = node_set;
	node_root = node_root_set;
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
		return data;
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

Node *node_creat(Node *_node)
{
	Node *node = calloc(1, sizeof(Node));
	memcpy(node, _node, sizeof(Node));
	push_node(node);
	return node;
}
