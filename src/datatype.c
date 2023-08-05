#include "compiler.h"

bool data_type_is_struct_or_union(DataType *datatype)
{
	return datatype->type == DATA_TYPE_STRUCT || datatype->type == DATA_TYPE_UNION;
}

bool datatype_is_struct_or_union_for_name(Token *token)
{
	char *type = token->sval;
	return S_EQ(type, "union") || S_EQ(type, "struct");
}

DataType *last = NULL;

void free_datatype_in_heap(DataType *datatype)
{
	if (last != datatype)
	{
		free_datatype(datatype);
		free(datatype);
		last = datatype;
	}
}

void free_datatype(DataType *datatype)
{
	if (datatype->secondary)
		free_datatype_in_heap(datatype->secondary);

	if (datatype->type == DATA_TYPE_STRUCT)
	{
		free_node(datatype->struct_node);
	}
	else if (datatype->type == DATA_TYPE_UNION)
	{
		free_node(datatype->union_node);
	}
	if (datatype->flag & DATATYPE_FLAG_ARRAY)
	{
		free_array_brackets(datatype->array.brackets);
	}
}

size_t datatype_element_size(DataType *datatype)
{
	if (datatype->flag & DATATYPE_FLAG_POINTER)
	{
		return QWORD;
	}

	return datatype->size;
}

size_t datatype_size_for_array_access(DataType *datatype)
{
	if (data_type_is_struct_or_union(datatype) && datatype->flag & DATATYPE_FLAG_POINTER && datatype->pointer_depth == 1)
	{
		return datatype->size;
	}

	return datatype_size(datatype);
}

size_t datatype_size(DataType *datatype)
{
	if (datatype->flag & DATATYPE_FLAG_POINTER && datatype->pointer_depth > 0)
	{
		return QWORD;
	}

	if (datatype->flag & DATATYPE_FLAG_ARRAY)
	{
		return datatype->array.size;
	}

	return datatype->size;
}
