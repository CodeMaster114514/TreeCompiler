#include "compiler.h"

compile_process *compile_process_create(
	const char *input_file,
	const char *output_file,
	int flags)
{
	FILE *input = fopen(input_file, "r");
	if (!input)
	{
		return NULL;
	}
	FILE *output = NULL;
	if (output_file)
	{
		output = fopen(output_file, "w");
		if (!output)
		{
			return NULL;
		}
	}
	compile_process *process = calloc(1, sizeof(compile_process));
	process->flags = flags;
	process->in_fp.fp = input;
	process->out_fp = output;
	process->pos.line = 1;
	process->pos.col = 1;
	process->node = creat_mound(sizeof(Node *));
	process->node_tree = creat_mound(sizeof(Node *));
	scope_creat_root(process);
	set_mound(process->node,process->node_tree);
	return process;
}

void free_compile_process(compile_process *process)
{
	fclose(process->in_fp.fp);
	if (process->out_fp != NULL)
		fclose(process->out_fp);
	free_nodes();
	scope_free_root(process);
	free_mound(process->node);
	free_mound(process->node_tree);
	free(process);
}

char compile_process_next_char(lex_process *lex_process)
{
	compile_process *cprocess = lex_process->cprocess;
	cprocess->pos.col += 1;
	char c = getc(cprocess->in_fp.fp);
	if (c == '\n')
	{
		cprocess->pos.line += 1;
		cprocess->pos.col = 1;
	}
	return c;
}

char compile_process_peek_char(lex_process *lex_process)
{
	compile_process *cprocess = lex_process->cprocess;
	char c = getc(cprocess->in_fp.fp);
	ungetc(c, cprocess->in_fp.fp);
	return c;
}

void compile_process_push_char(lex_process *lex_process, char c)
{
	compile_process *cprocess = lex_process->cprocess;
	ungetc(c, cprocess->in_fp.fp);
}
