#include "compiler.h"

lex_process *lex_process_create(
	compile_process *cprocess,
	lex_process_functions *lex_process_function,
	void *private)
{
	lex_process *process = (lex_process *)calloc(1, sizeof(lex_process));
	process->tokens = creat_mound(sizeof(Token));
	process->cprocess = cprocess;
	process->private = private;
	process->process_functions = lex_process_function;
	process->pos.line = 1;
	process->pos.col = 1;
	return process;
}

void free_tokens(lex_process *process)
{
	set_peek(process->tokens, 0);
	while(1)
	{
		Token *current = next(process->tokens);
		if(!current)
		{
			break;
		}
		free_token(current);
	}
}

void lex_process_free(lex_process *process)
{
	// free(process->cprocess);
	for (int i = 0; i < process->expression.string_buffer_count; ++i)
	{
		free_buffer(process->expression.buffer_info[i]);
	}
	free(process->expression.buffer_info);
	free_tokens(process);
	free_mound(process->tokens);
	if (process->private)
		free(process->private);
	free(process);
}
