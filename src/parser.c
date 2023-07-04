#include "compiler.h"

static compile_process* current_process;

int test = 0;

int parse_next()
{
	return test++;
}

void push_node(Node** node)
{
	Node** data = calloc(1,sizeof(Node*));
	memcpy(data,node,sizeof(Node*));
	push(current_process->node_tree,data);
}

int parse(compile_process* process)
{
	current_process = process;
	set_peek(process->tokens,0);
	Node** node = NULL;
	while(parse_next())
	{
		push_node(node);
	}
	return PARSE_ALL_OK;
}
