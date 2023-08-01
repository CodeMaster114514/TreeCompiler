#include "compiler.h"

ArrayBrackets *new_array_brackets()
{
	ArrayBrackets *brackets = calloc(1, sizeof(ArrayBrackets));
	brackets->n_brackets = creat_mound(sizeof(Node *));
	return brackets;
}

void free_array_brackets(ArrayBrackets *brackets)
{
	set_peek(brackets->n_brackets, 0);
	Node *node = next_ptr(brackets->n_brackets);
	while(node)
	{
		free_node(node);
		node = next_ptr(brackets->n_brackets);
	}
	free(brackets);
}

void array_brackets_add(ArrayBrackets *brackets, Node *node)
{
	assert(brackets && node && node->type == NODE_TYPE_BRACKET);
	push(brackets->n_brackets, &node);
}

size_t array_brackets_calculate_size_from_index(DataType *datatype, int index)
{
	assert(datatype && (datatype->flag & DATATYPE_FLAG_ARRAY) && datatype->array.brackets);
	size_t type_size = datatype->size;
	set_peek(datatype->array.brackets->n_brackets, 0);
	int depth = get_count(datatype->array.brackets->n_brackets);
	if(index > depth)
	{
		return type_size;
	}
	Node *data = next_ptr(datatype->array.brackets->n_brackets);
	if(!data)
	{
		return 0;
	}
	size_t all = data->brackets.inner->lnum;
	for(int i = 1;i < depth;++i)
	{
		data = next_ptr(datatype->array.brackets->n_brackets);
		all = all * data->brackets.inner->lnum;
	}
	return all * datatype->size;
}

size_t array_brackets_calculate_size(DataType *datatype)
{
	return array_brackets_calculate_size_from_index(datatype, 0);
}

int array_total_indexes(DataType *datatype)
{
	assert(datatype->flag & DATATYPE_FLAG_ARRAY);
	return get_count(datatype->array.brackets->n_brackets);
}
