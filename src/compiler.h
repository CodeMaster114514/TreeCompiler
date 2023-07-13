#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "mound.h"

typedef struct
{
	int line;
	int col;
	const char *filename;
} Pos;

#define S_EQ(str0, str1) \
	(str0 && str1 && (strcmp(str0, str1) == 0))

#define NUMERIC_CASE \
	case '0':        \
	case '1':        \
	case '2':        \
	case '3':        \
	case '4':        \
	case '5':        \
	case '6':        \
	case '7':        \
	case '8':        \
	case '9'

#define OPERATOR_CASE_EXCLUDING_DIVISION \
	case '+':                            \
	case '-':                            \
	case '*':                            \
	case '/':                            \
	case '<':                            \
	case '>':                            \
	case '%':                            \
	case '^':                            \
	case '!':                            \
	case '=':                            \
	case '~':                            \
	case '|':                            \
	case '(':                            \
	case '[':                            \
	case ',':                            \
	case '.':                            \
	case '?'

#define SYMBOL_CASE \
	case '{':       \
	case '}':       \
	case ':':       \
	case ';':       \
	case '#':       \
	case '\\':      \
	case ')':       \
	case ']'

enum
{
	LEX_ANALYSIS_ALL_OK,
	LEX_ANALYSIS_INPUT_ERROR
};

enum
{
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

enum
{
	NUMEBR_TYPE_NORMAL,
	NUMBER_TYPE_LONG,
	NUMBER_TYPE_FLOAT,
	NUMBER_TYPE_DOUBLE
};

typedef struct
{
	int type;
	int flags;
	Pos pos;

	union
	{
		char cval;
		char *sval;
		unsigned int inum;
		unsigned long lnum;
		unsigned long long llnum;
		void *any;
	};

	struct
	{
		int type;
	} num;

	// 如果值为ture，那么两个令牌时间是空格
	bool whitespace;

	char **between_brackets;
} Token;

enum
{
	COMPILER_FAILED_WITH_ERROR,
	COMPILER_FILE_COMPILE_OK
};

typedef struct
{
	int flags;
	Pos pos;
	struct compile_process_input_file
	{
		FILE *fp;
		const char *file_path;
	} in_fp;
	FILE *out_fp;

	mound *tokens;
	mound *node;
	mound *node_tree;
} compile_process;

enum
{
	PARSE_ALL_OK,
	PARSE_GENERAL_ERROR
};

enum
{
	NODE_TYPE_EXPRESSION,
	NODE_TYPE_EXPRESSION_PARENTHESES,
	NODE_TYPE_NUMBER,
	NODE_TYPE_IDENTIFIER,
	NODE_TYPE_STRING,
	NODE_TYPE_VARIABLE,
	NODE_TYPE_VARIABLE_LIST,
	NODE_TYPE_FUNCTION,
	NODE_TYPE_BODY,
	NODE_TYPE_STATEMENT_RETURN,
	NODE_TYPE_STATEMENT_IF,
	NODE_TYPE_STATEMENT_ELSE,
	NODE_TYPE_STATEMENT_WHILE,
	NODE_TYPE_STATEMENT_DO_WHILE,
	NODE_TYPE_STATEMENT_BREAK,
	NODE_TYPE_STATEMENT_CONTINUE,
	NODE_TYPE_STATEMENT_SWITCH,
	NODE_TYPE_STATEMENT_CASE,
	NODE_TYPE_STATEMENT_DEFAULT,
	NODE_TYPE_STATEMENT_GOTO,

	NODE_TYPE_UNARY,
	NODE_TYPE_TENARY,
	NODE_TYPE_LABEL,
	NODE_TYPE_STRUCT,
	NODE_TYPE_UNION,
	NODE_TYPE_BRACKET,
	NODE_TYPE_CAST,
	NODE_TYPE_BLANK
};

struct Node;
typedef struct Node Node;

struct Node
{
	int type;
	int flag;

	Pos pos;

	struct node_binded
	{
		Node *owner;
		Node *function;
	} binded;

	union
	{
		char cavl;
		char *savl;
		unsigned int inum;
		unsigned long lnum;
		unsigned long long llnum;
	};
};

struct lex_process;
typedef struct lex_process lex_process;

typedef char (*LEX_PROCESS_NEXT_CHAR)(lex_process *process);
typedef char (*LEX_PROCESS_PEEK_CHAR)(lex_process *process);
typedef void (*LEX_PROCESS_PUSH_CHAR)(lex_process *process, char c);

typedef struct
{
	LEX_PROCESS_NEXT_CHAR next_char;
	LEX_PROCESS_PEEK_CHAR peek_char;
	LEX_PROCESS_PUSH_CHAR push_char;
} lex_process_functions;

typedef struct
{
	char *buffer;
	int buffer_len;
} parentheses_buffer;

typedef struct
{
	char *str;
	size_t len;
	size_t index;
} string_read;

struct lex_process
{
	Pos pos;
	mound *tokens;
	compile_process *cprocess;

	struct
	{
		int current_expression_count,
			parentheses_buffer_count;
		parentheses_buffer **buffer_info;
	} expression;
	lex_process_functions *process_functions;
	void *private;
};

int compile_file(
	const char *,
	const char *,
	int);

compile_process *compile_process_create(
	const char *,
	const char *,
	int);

void free_compile_process(compile_process *);

void compile_error(
	compile_process *,
	const char *,
	...);
void compile_warning(
	compile_process *,
	const char *,
	...);
char compile_process_next_char(lex_process *);
char compile_process_peek_char(lex_process *);
void compile_process_push_char(lex_process *, char);

// in file lex_process.c
lex_process *lex_process_create(
	compile_process *,
	lex_process_functions *,
	void *);
void lex_process_free(lex_process *);

// in file lexer.c
int lex(lex_process *);
/*
 * 输入字符串，返回构建的token
 */
lex_process *lex_token_build_for_string(
	compile_process *,
	const char *);

// in file token.c
bool token_is_keyword(Token *, const char *);
bool tolen_is_symbol(Token *, char);
bool token_is_nl_or_comment_or_new_line(Token *);

// in file parenthese_buffer.c
parentheses_buffer *creat_parentheses_buffer();
void write(parentheses_buffer *, char);
char **get_buffer(parentheses_buffer *);
void free_buffer(parentheses_buffer *);

// in file parser.c
int parse(compile_process *);

// in file node.c
void set_mound(mound *node_set, mound *node_root_set);
void push_node(Node *data);
Node *next_node();
Node *peek_node();
Node *pop_node();
Node *node_creat(Node *_node);
#endif
