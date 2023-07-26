#include "compiler.h"

bool datatype_is_struct_or_union_for_name(Token *token)
{
	char* type = token->sval;
	return S_EQ(type,"union") || S_EQ(type,"struct");
}
