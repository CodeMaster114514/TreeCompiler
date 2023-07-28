#include "compiler.h"

bool datatype_is_struct_or_union_for_name(Token *token)
{
	char* type = token->sval;
	return S_EQ(type,"union") || S_EQ(type,"struct");
}

void free_datatype_in_heap(DataType *datatype)
{
	free_datatype(datatype);
	free(datatype);
}

void free_datatype(DataType *datatype)
{
	if(datatype->secondary)
		free_datatype_in_heap(datatype->secondary);

	if(datatype->type == DATA_TYPE_STRUCT)
	{
		free_node(datatype->struct_node);
	}
	else if(datatype->type == DATA_TYPE_UNION)
	{
		free_node(datatype->union_node);
	}
}
