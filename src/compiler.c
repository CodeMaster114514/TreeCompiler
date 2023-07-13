#include "compiler.h"

lex_process_functions lex_process_function = {
	.next_char = compile_process_next_char,
	.peek_char = compile_process_peek_char,
	.push_char = compile_process_push_char};

lex_process *process;

void freeAll()
{
	free_compile_process(process->cprocess);
	lex_process_free(process);
}

void compile_error(compile_process *CompileProcess, const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);

	fprintf(stderr, "on line %i,col %i in file %s\n",
			CompileProcess->pos.line,
			CompileProcess->pos.col,
			CompileProcess->in_fp.file_path);
	freeAll();
	exit(-1);
}

void compile_warning(compile_process *CompileProcess, const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);

	fprintf(stderr, "on line %i,col %i in file %s",
			CompileProcess->pos.line,
			CompileProcess->pos.col,
			CompileProcess->in_fp.file_path);
}

int compile_file(
	const char *input_file,
	const char *output_file,
	int flags)
{
	compile_process *cprocess =
		compile_process_create(
			input_file,
			output_file,
			flags);
	if (!cprocess)
	{
		return COMPILER_FAILED_WITH_ERROR;
	}
	cprocess->in_fp.file_path = input_file;
	/*long long p = 0;
	fseek(cprocess->in_fp.fp,0,SEEK_END);
	char* text = (char*) malloc(5+ftell(cprocess->in_fp.fp));
	fseek(cprocess->in_fp.fp,0,SEEK_SET);
	while(!feof(cprocess->in_fp.fp)){
		text[p] = fgetc(cprocess->in_fp.fp);
		++p;
	}
	text[--p] = '\0';
	printf("%s",text);
	free(text);
	fseek(cprocess->in_fp.fp,0,SEEK_SET);*/

	lex_process *lex_process = lex_process_create(
		cprocess,
		&lex_process_function,
		NULL);
	if (!lex_process)
	{
		free_compile_process(cprocess);
		return COMPILER_FAILED_WITH_ERROR;
	}
	process = lex_process;
	if (lex(lex_process) != LEX_ANALYSIS_ALL_OK)
	{
		free_compile_process(cprocess);
		lex_process_free(lex_process);
		return COMPILER_FAILED_WITH_ERROR;
	}
	cprocess->tokens = lex_process->tokens;

	if (parse(cprocess) != PARSE_ALL_OK)
	{
		return COMPILER_FAILED_WITH_ERROR;
	}

	free_compile_process(cprocess);
	lex_process_free(lex_process);
	return COMPILER_FILE_COMPILE_OK;
}
