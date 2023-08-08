#include "compiler.h"

#define PRIMITIVE_TYPE_TOTAL 7

char *primitive[PRIMITIVE_TYPE_TOTAL] =
	{"char", "short", "int", "long", "float", "double"};

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
	if (!token)
	{
		return false;
	}
	const char *keyword = token->sval;
	for (int i = 0; i < PRIMITIVE_TYPE_TOTAL; ++i)
	{
		if (S_EQ(primitive[i], keyword))
		{
			return true;
		}
	}
	return false;
}

bool token_is_operator(Token *token, char *op)
{
	return token && op && token->type == TOKEN_TYPE_OPERATOR && S_EQ(token->sval, op);
}

void free_token(Token *token)
{
	if (token->type == TOKEN_TYPE_IDENTIFIER && token->flags & TOKEN_FLAG_FROM_PARSER)
	{
		free(token);
		goto over;
	}
	if (token->type ==
			TOKEN_TYPE_STRING ||
		token->type ==
			TOKEN_TYPE_OPERATOR ||
		token->type ==
			TOKEN_TYPE_KEYWORDS ||
		token->type ==
			TOKEN_TYPE_IDENTIFIER ||
		token->type ==
			TOKEN_TYPE_COMMENT)
	{
		free(token->sval);
	}
over:
}
