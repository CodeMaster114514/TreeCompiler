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
	TOKEN_FLAG_FROM_PARSER = 0b00000001
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

	char *between_brackets;
} Token;

enum
{
	COMPILER_FAILED_WITH_ERROR,
	COMPILER_FILE_COMPILE_OK
};

struct scope;
typedef struct scope Scope;

enum
{
	SYMBLE_TYPE_NODE,
	SYMBLE_TYPE_NATIVE_FUNCTION,
	SYMBLE_TYPE_UNKOWN
};

typedef struct
{
	char* name;
	int type;
	void* data;
} Symble;

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

	struct
	{
		Scope *root;
		Scope *current;
	} scope;

	struct
	{
		mound *current_table,
		      *tables;
	} symbles;
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

enum
{
	NODE_FLAG_INSIDE_EXPRESSION = 0x01
};

struct DataType;
typedef struct DataType DataType;

struct Node;
typedef struct Node Node;

struct DataType
{
	int type;

	int flag;

	char *type_str;

	DataType *secondary;

	size_t size;

	int pointer_depth;

	union
	{
		Node *struct_node;
		Node *union_node;
	};
};

struct Node
{
	int type;
	int flag;

	Pos pos;

	union
	{
		struct
		{
			Node *node_left;
			Node *node_right;
			char *op;
		} exp;

		struct
		{
			DataType datatype;
			char *name;
			Node *value;
		}var;
	};

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

enum
{
	DATATYPE_FLAG_STATIC = 0b00000000000000000000000000000001,
	DATATYPE_FLAG_SIGNED = 0b00000000000000000000000000000010,
	DATATYPE_FLAG_CONST = 0b00000000000000000000000000000100,
	DATATYPE_FLAG_POINTER = 0b00000000000000000000000000001000,
	DATATYPE_FLAG_ARRAY = 0b00000000000000000000000000010000,
	DATATYPE_FLAG_EXTERN = 0b00000000000000000000000000100000,
	DATATYPE_FLAG_SECONDARY = 0b00000000000000000000000001000000,
	DATATYPE_FLAG_STRUCT_OR_UNION_NO_NAME = 0b00000000000000000000000010000000,
	DATATYPE_FLAG_LITERAL = 0b00000000000000000000000100000000
};

enum
{
	DATA_TYPE_CHAR,
	DATA_TYPE_VOID,
	DATA_TYPE_INT,
	DATA_TYPE_SHORT,
	DATA_TYPE_LONG,
	DATA_TYPE_FLOAT,
	DATA_TYPE_DOUBLE,
	DATA_TYPE_STRUCT,
	DATA_TYPE_UNION,
	DATA_TYPE_UNKNOW
};

enum
{
	DATA_TYPE_EXPECT_PRIMITIVE,
	DATA_TYPE_EXPECT_UNION,
	DATA_TYPE_EXPECT_STRUCT
};

enum
{
	ZERO = 0,
	BYTE = 1,
	WORD = 2,
	DWORD = 4,
	QWORD = 8,
	DQWORD = 16
};

struct scope
{
	int flag;

	mound* entities;//存储数据信息

	size_t total;

	Scope* parent;
};

#define TOTAL_OPERATOR_GROUPS 14
#define MAX_OPERATORS_IN_GROUP 14

enum
{
	LEFT_TO_RIGHT,
	RIGHT_TO_LEFT
};

typedef struct
{
	char *operators[MAX_OPERATORS_IN_GROUP];
	int associativity;
} expressionable_operator_precedence_group;

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
} string_buffer;

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
			string_buffer_count;
		string_buffer **buffer_info;
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
bool keyword_is_datatype(const char *str);
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
bool token_is_primitive(Token *token);
bool token_is_operator(Token *token, char *op);
void free_token(Token *token);

// in file string_buffer.c
string_buffer *creat_string_buffer();
void write(string_buffer *, char);
char *get_buffer(string_buffer *);
void free_buffer(string_buffer *);
void free_buffer_without_string(string_buffer *buffer);

// in file parser.c
int parse(compile_process *);

// in file node.c
void set_mound(mound *node_set, mound *node_root_set);
void free_node(Node *data);
void free_nodes();
void push_node(Node *data);
Node *next_node();
Node *peek_node();
Node *pop_node();
Node *node_creat(Node *_node);
Node *node_peek_expressionable();
bool node_is_expressionable(Node *node);
Node *make_exp_node(Node *left, Node *right, char *op);

//in file datatype.c
bool datatype_is_struct_or_union_for_name(Token *token);
void free_datatype(DataType* datatype);
void free_datatype_in_heap(DataType* datatype);

//in file scope.c
Scope *scope_alloc();
void free_scope(Scope *scope);
void free_scope_with_root(Scope *root, Scope *current);
Scope *scope_creat_root(compile_process *process);
void scope_free_root(compile_process *process);
Scope *scope_new(compile_process *process, int flag);
void scope_iteration_start(Scope *scope);
void scope_iteration_end(Scope *scope);
void *scope_iteration_back(Scope *scope);
void *scope_last_entity_at_scope(Scope *scope);
void *scope_last_entity_from_scope_stop_at(Scope *scope, Scope *stop);
void *scope_last_entity_stop_at(compile_process *process, Scope *stop);
void *scope_last_entity(compile_process *process);
void scope_push(compile_process *process, void *ptr, size_t size);
void scope_finish(compile_process *process);
Scope *scope_current(compile_process *process);

//in file symresolver.c
void symresolver_push(compile_process *process, Symble *symble);
void free_symble(Symble *symble);
void free_table(compile_process *process);
void free_tables(compile_process *process);
void symresolver_initialize(compile_process *process);
void symresolver_new_table(compile_process *process);
void symresolver_end_table(compile_process *process);
Symble *symresolver_get_symble_by_name(compile_process *process,const char* name);
Symble *symresolver_get_symble_for_native_function_by_name(compile_process *process, const char* name);

#endif
