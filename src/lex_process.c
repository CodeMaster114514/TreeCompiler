#include "compiler.h"

lex_process* lex_process_create(
	compile_process* cprocess,
	lex_process_functions* lex_process_function,
	void* private
){
	lex_process* process = (lex_process*) calloc(1,sizeof(lex_process));
	process->tokens = (Token*) calloc(1,sizeof(Token));
	process->token_count = 0;
	process->cprocess = cprocess;
	process->private = private;
	process->process_functions = lex_process_function;
	process->pos.line = 1;
	process->pos.col = 1;
	return process;
}

void lex_process_free(lex_process* process){
	for(unsigned long long i = 0;i < process->token_count;++i)
	{
		if(process->tokens[i].type
		==
		TOKEN_TYPE_STRING
		||
		process->tokens[i].type
		==
		TOKEN_TYPE_OPERATOR
		||
		process->tokens[i].type
		==
		TOKEN_TYPE_KEYWORDS)
		{
			free(process->tokens[i].sval);
		}
	}
	free(process->tokens);
	free(process);
}
