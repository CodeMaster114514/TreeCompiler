#include "compiler.h"

static compile_process* current_process;
static Token* parse_last_token;

static void parse_nl_or_comment(Token* token)
{
	while(token && token_is_nl_or_comment_or_new_line(token))
	{
		next(current_process->tokens);
		token = peek(current_process->tokens);
	}
}

static Token* next_token()
{
	Token* next_token = peek(current_process->tokens);
	parse_nl_or_comment(next_token);
	current_process->pos = next_token->pos;
	parse_last_token = next_token;
	return next(current_process->tokens);
}

static Token* peek_token()
{
	Token* next_token = peek(current_process->tokens);
	parse_nl_or_comment(next_token);
	return peek(current_process->tokens);
}

int test = 0;

void parse_single_token_to_node()
{
	Token* token = next_token();
	Node* node = NULL;
	switch(token->type)
	{
		case TOKEN_TYPE_NUMBER:
			node = creat
	}
}

int parse_next()
{
	Token* token = peek_token();
	if(!token)
	{
		return -1;
	}
	int res = 0;
	switch(token->type)
	{
		case TOKEN_TYPE_NUMBER:
		case TOKEN_TYPE_IDENTIFIER:
		case TOKEN_TYPE_STRING:
			parse_single_token_to_node();
			break;
	}
	return test++;
}

int parse(compile_process* process)
{
	current_process = process;
	set_peek(process->tokens,0);
	Node* node = NULL;
	while(!parse_next())
	{
		push(current_process->node_tree,&node);
	}
	return PARSE_ALL_OK;
}
