#include "compiler.h"

bool token_is_keyword(Token* token,
		const char* keyword)
{
	return (token->type == TOKEN_TYPE_KEYWORDS) && S_EQ(token->sval,keyword);
}
