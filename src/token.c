#include "compiler.h"

#define PRIMITIVE_TYPE_TOTAL 7

char *primitive[PRIMITIVE_TYPE_TOTAL] =
	{"char", "short", "int", "long", "struct", "union"};

bool token_is_keyword(Token *token,
					  const char *keyword)
{
	return (token->type == TOKEN_TYPE_KEYWORDS) && S_EQ(token->sval, keyword);
}

bool token_is_symbol(Token *token, char c)
{
	return token->type == TOKEN_TYPE_SYMBOL &&
		   token->cval == c;
}

bool token_is_nl_or_comment_or_new_line(Token *token)
{
	return token->type == TOKEN_TYPE_NEWLINE || token->type == TOKEN_TYPE_COMMENT || token_is_symbol(token, '\\');
}

bool token_is_primitive(Token *token)
{
	const char *keyword = token->sval;
	for (int i = 0; i < PRIMITIVE_TYPE_TOTAL; ++i)
	{
		if(S_EQ(primitive[i],keyword))
		{
			return true;
		}
	}
	return false;
}
