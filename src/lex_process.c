#include "compiler.h"

lex_process *lex_process_create(
	compile_process *cprocess,
	lex_process_functions *lex_process_function,
	void *private)
{
	lex_process *process = (lex_process *)calloc(1, sizeof(lex_process));
	process->tokens = create_mound(sizeof(Token));
	process->cprocess = cprocess;
	process->private = private;
	process->process_functions = lex_process_function;
	process->pos.line = 1;
	process->pos.col = 1;
	return process;
}

void lex_process_free(lex_process *process)
{
	// free(process->cprocess);
	for (int i = 0; i < process->expression.parentheses_buffer_count; ++i)
	{
		free_buffer(process->expression.buffer_info[i]);
	}
	free(process->expression.buffer_info);
	size_t count = get_count(process->tokens);
	for (size_t i = 0; i < count; ++i)
	{
		Token *the = read(process->tokens, i);
		if (the->type ==
				TOKEN_TYPE_STRING ||
			the->type ==
				TOKEN_TYPE_OPERATOR ||
			the->type ==
				TOKEN_TYPE_KEYWORDS ||
			the->type ==
				TOKEN_TYPE_COMMENT)
		{
			free(the->sval);
		}
	}
	free_mound(process->tokens);
	if (!process->private)
		free(process->private);
	free(process);
}
