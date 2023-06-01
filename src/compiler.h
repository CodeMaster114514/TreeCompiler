#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

typedef struct {
	int line;
	int col;
	const char* filename;
} Pos;

#define S_EQ(str0,str1) \
	(str0 && str1 && (strcmp(str0,str1) == 0))

#define NUMERIC_CASE \
	case '1': \
	case '2': \
	case '3': \
	case '4': \
	case '5': \
	case '6': \
	case '7': \
	case '8': \
	case '9'

#define OPERATOR_CASE_EXCLUDING_DIVISION \
	case '+': \
	case '-': \
	case '*': \
	case '/': \
	case '<': \
	case '>': \
	case '%': \
	case '^': \
	case '!': \
	case '=': \
	case '~': \
	case '|': \
	case '(': \
	case '[': \
	case ',': \
	case '.': \
	case '?'

#define SYMBOL_CASE \
	case '{': \
	case '}': \
	case ':': \
	case ';': \
	case '\\':\
	case ')': \
	case ']'

enum {
	LEX_ANALYSIS_ALL_OK,
	LEX_ANALYSIS_INPUT_ERROR
};

enum {
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_KEYWORDS,
	TOKEN_TYPE_OPERATOR,
	TOKEN_TYPE_SYMBOL,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_STRING,
	TOKEN_TYPE_CHAR,
	TOKEN_TYPE_COMMENT,
	TOKEN_TYPE_NEWLINE
};

typedef struct {
	int type;
	int flags;
	Pos pos;

	union {
		char cval;
		char* sval;
		unsigned int inum;
		unsigned long lnum;
		unsigned long long llnum;
		void* any;
	};

	//如果值为ture，那么两个令牌时间是空格
	bool whitespace;

	const char* between_brackets;
} Token;

enum{
	COMPILER_FAILED_WITH_ERROR,
	COMPILER_FILE_COMPILE_OK
};

typedef struct {
	int flags;
	Pos pos;
	struct compile_process_input_file {
		FILE* fp;
		const char* file_path;
	} in_fp;
	FILE* out_fp;
} compile_process;

struct lex_process;
typedef struct lex_process lex_process;

typedef char (*LEX_PROCESS_NEXT_CHAR)(lex_process* process);
typedef char (*LEX_PROCESS_PEEK_CHAR)(lex_process* process);
typedef void (*LEX_PROCESS_PUSH_CHAR)(lex_process* process,char c);

typedef struct {
	LEX_PROCESS_NEXT_CHAR next_char;
	LEX_PROCESS_PEEK_CHAR peek_char;
	LEX_PROCESS_PUSH_CHAR push_char;
} lex_process_functions;

typedef struct {
	char* buffer;
	int buffer_len;
} parentheses_buffer;

struct lex_process{
	Pos pos;
	Token* tokens;
	unsigned long long token_count;
	compile_process* cprocess;

	struct {
		int current_expression_count,
		    parentheses_buffer_count;
		parentheses_buffer** buffer_info;
	} expression;
	lex_process_functions* process_functions;
	void* private;
};

int compile_file(
	const char*,
	const char*,
	int
);

compile_process* compile_process_create(
	const char*,
	const char*,
	int
);

void compile_error(
	compile_process*,
	const char*,
	...
);
void compile_warning(
	compile_process*,
	const char*,
	...
);
char compile_process_next_char(lex_process*);
char compile_process_peek_char(lex_process*);
void compile_process_push_char(lex_process*,char);

lex_process* lex_process_create(
	compile_process*,
	lex_process_functions*,
	void*
);
void lex_process_free(lex_process*);
int lex(lex_process*);

//in file token.c
bool token_is_keyword(Token*,const char*);

//in file parenthese_buffer.c
parentheses_buffer* creat_parentheses_buffer();
void write(parentheses_buffer*,char);

#endif
