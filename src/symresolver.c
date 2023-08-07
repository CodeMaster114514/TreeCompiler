#include "compiler.h"

void symresolver_push(compile_process *process, Symble *symble)
{
	push(process->symbles.current_table, &symble);
}

void free_symble(Symble *symble)
{
	free(symble->name);
	free(symble->data);
	free(symble);
}

void free_table(compile_process *process)
{
	int count = get_count(process->symbles.current_table);
	for(int i = 0;i < count;++i)
	{
		free_symble(last_data_ptr(process->symbles.current_table));
		pop(process->symbles.current_table);
	}
}

void free_tables(compile_process *process)
{
	int count = get_count(process->symbles.tables);
	if(process->symbles.current_table)
		free_table(process);
	for(int i = 0;i < count;++i)
	{
		process->symbles.current_table = last_data_ptr(process->symbles.current_table);
		free_table(process);
		pop(process->symbles.tables);
	}
}

void symresolver_initialize(compile_process *process)
{
	process->symbles.current_table = creat_mound(sizeof(mound*));
}

void symresolver_new_table(compile_process *process)
{
	push(process->symbles.tables, &process->symbles.current_table);
	process->symbles.current_table = creat_mound(sizeof(Symble*));
}

void symresolver_end_table(compile_process *process)
{
	mound* last_table = last_data_ptr(process->symbles.tables);
	if(process->symbles.current_table)
	{
		free_table(process);
	}
	pop(process->symbles.tables);
}

Symble *symresolver_get_symble_by_name(compile_process *process,const char* name)
{
	set_peek(process->symbles.current_table, 0);
	Symble *symble = next_ptr(process->symbles.current_table);
	while(symble)
	{
		if(S_EQ(symble->name, name))
		{
			break;
		}

		symble = next_ptr(process->symbles.current_table);
	}

	return symble;
}

Symble *symresolver_get_symble_for_native_function_by_name(compile_process *process, const char* name)
{
	Symble *symble = symresolver_get_symble_by_name(process, name);
	if(symble && symble->type == SYMBLE_TYPE_NATIVE_FUNCTION)
	{
		return symble;
	}
	else
	{
		return NULL;
	}
}

Symble *symresolver_register_symble(compile_process *process, char *name, int type, void *data)
{
	if(symresolver_get_symble_by_name(process, name))
	{
		return NULL;
	}
	
	Symble *symble = calloc(1, sizeof(Symble));
	symble->name = name;
	symble->type = type;
	symble->data = data;
	symresolver_push(process, symble);
	return symble;
}

Node *symresolver_node(Symble *symble)
{
	if(symble->type != SYMBLE_TYPE_NODE)
		return NULL;
	return symble->data;
}

void symresolver_build_for_variable(compile_process *process, Node *node)
{
	compile_error(process, "can’t build variable symble");
}

void symresolver_build_for_native_function(compile_process *process, Node *node)
{
	compile_error(process, "can’t build varible symble");
}

void symresolver_build_for_struct(compile_process *process, Node *node)
{
	if (node->flags & NODE_FLAG_IS_FORWARD_DECLARATION)
	{
		return;
	}

	symresolver_register_symble(process, node->_struct.name, SYMBLE_TYPE_NODE, node);
}

void symresolver_build_for_union(compile_process *process, Node *node)
{
	compile_error(process, "can‘t build union symble");
}

void symresovler_build_for_node(compile_process *process, Node *node)
{
	switch(node->type)
	{
		case NODE_TYPE_FUNCTION:
			symresolver_build_for_native_function(process, node);
			break;
		case NODE_TYPE_VARIABLE:
			symresolver_build_for_variable(process, node);
			break;
		case NODE_TYPE_STRUCT:
			symresolver_build_for_struct(process, node);
			break;
		case NODE_TYPE_UNION:
			symresolver_build_for_union(process, node);
			break;
	}
}
