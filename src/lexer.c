#include "compiler.h"

static lex_process *LexProcess;
static Token tmp_token;

Token *make_special_number_hexadcimal();
Token *make_special_number_binary();

bool lex_is_in_expression();

char get_escaped_char(char c)
{
	char data;
	switch (c)
	{
	case 'n':
		data = '\n';
		break;
	case 't':
		data = '\t';
		break;
	case 'a':
		data = '\a';
		break;
	case 'b':
		data = '\b';
		break;
	case 'f':
		data = '\f';
		break;
	case 'r':
		data = '\r';
		break;
	case 'v':
		data = '\v';
		break;
	case '\'':
		data = '\'';
		break;
	case '\"':
		data = '\"';
		break;
	case '\?':
		data = '\?';
		break;
	case '0':
		data = '\0';
		break;
	case '\\':
		data = '\\';
	default:
		compile_error(
			LexProcess->cprocess,
			"E escape character compilation error\n");
	}
	return data;
}

Token *read_next_token();

static char peekc()
{
	return LexProcess->process_functions->peek_char(LexProcess);
}

static char nextc()
{
	char c = LexProcess->process_functions->next_char(LexProcess);
	++LexProcess->pos.col;
	if (lex_is_in_expression())
	{
		write(LexProcess->expression.buffer_info[LexProcess->expression
													 .string_buffer_count -
												 1],
			  c);
	}
	if (c == '\n')
	{
		++LexProcess->pos.line;
		LexProcess->pos.col = 1;
	}
	return c;
}

static char assert_next_char(char c)
{
	char next_char = nextc();
	assert(c == next_char);
	return next_char;
}

static void pushc(char c)
{
	LexProcess->process_functions->push_char(LexProcess, c);
}

static Token *token_creat(Token *_token)
{
	memcpy(&tmp_token, _token, sizeof(Token));
	if (lex_is_in_expression())
	{
		tmp_token.between_brackets =
			get_buffer(
				LexProcess->expression.buffer_info[LexProcess->expression.string_buffer_count - 1]);
	}
	tmp_token.pos = LexProcess->pos;
	return &tmp_token;
}

char *read_number_str()
{
	char *str_ret = NULL;
	string_buffer *buffer = creat_string_buffer();
	while(peekc() >= '0' && peekc() <= '9')
	{
		write(buffer, nextc());
	}
	write(buffer, '\0');
	str_ret = get_buffer(buffer);
	free_buffer_without_string(buffer);
	return str_ret;
}

Token *lexer_last_token()
{
	Token *token = last_data(LexProcess->tokens);
	if(!token){
		return NULL;
	}
	pop(LexProcess->tokens);
	return token;
}

Token *handle_whitespace()
{
	Token *token = lexer_last_token();
	if (token)
	{
		token->whitespace = true;
	}
	push(LexProcess->tokens, token);
	nextc();
	return read_next_token();
}

unsigned long long read_number()
{
	char *s = read_number_str();
	unsigned long long ret = atoll(s);
	free(s);
	return ret;
}

int lexer_number_type(char c)
{
	int res = NUMEBR_TYPE_NORMAL;
	if (c == 'L')
	{
		res = NUMBER_TYPE_LONG;
	}
	else if (c == 'f')
	{
		res = NUMBER_TYPE_FLOAT;
	}
	return res;
}

Token *make_number_token_for_value(unsigned long number)
{
	int type = lexer_number_type(peekc());
	if (type != NUMEBR_TYPE_NORMAL)
	{
		nextc();
	}
	Token token = {
		.type = TOKEN_TYPE_NUMBER,
		.llnum = number,
		.num.type = type};
	return token_creat(&token);
}

Token *make_number_token()
{
	Token *number;
	char now = peekc();//防止丢失第一个数字
	if (peekc() == '0')
	{
		nextc();
		switch(peekc())
		{
			case 'x':
				number = make_special_number_hexadcimal();
				break;

			case 'b':
				number = make_special_number_binary();
				break;

			default:
				pushc(now);
				number = make_number_token_for_value((read_number()));
		}
	}
	else
	{
		number = make_number_token_for_value(read_number());
	}
	return number;
}

Token *make_string_token(char start_delim, char end_delim)
{
	int i = 0;
	char c = nextc();
	int count = 0;
	if (lex_is_in_expression())
	{
		count = LexProcess->expression.current_expression_count;
		LexProcess->expression.current_expression_count = 0;
	}
	Pos pos_of_LexProcess = LexProcess->pos;
	Pos pos_of_cprocess = LexProcess->cprocess->pos;
	long fp_now = ftell(LexProcess->cprocess->in_fp.fp);
	for (c = nextc(); c != end_delim && c != EOF; c = nextc())
	{
		if (c == '\n')
		{
			compile_error(LexProcess->cprocess, "There is no corresponding symbol\n");
		}
		if (c == '\\')
		{
			continue;
		}
		i++;
	}
	if (count != 0)
		LexProcess->expression.current_expression_count = count;
	LexProcess->pos = pos_of_LexProcess;
	LexProcess->cprocess->pos = pos_of_cprocess;
	fseek(LexProcess->cprocess->in_fp.fp, fp_now, SEEK_SET);
	char *str = calloc(i + 1, sizeof(char));
	for (int j = 0; j < i; ++j)
	{
		char data = nextc();
		if (data == '\\')
		{
			data = get_escaped_char(nextc());
		}
		str[j] = data;
	}
	nextc();
	Token token = {
		.type = TOKEN_TYPE_STRING,
		.sval = str};
	return token_creat(&token);
}

static bool is_single_operator(char op)
{
	return op == '+' ||
		   op == '-' ||
		   op == '/' ||
		   op == '*' ||
		   op == '=' ||
		   op == '<' ||
		   op == '>' ||
		   op == '|' ||
		   op == '&' ||
		   op == '^' ||
		   op == '%' ||
		   op == '!' ||
		   op == '(' ||
		   op == '[' ||
		   op == ',' ||
		   op == '.' ||
		   op == '~' ||
		   op == '?';
}

static bool op_treated_as_one(char op)
{
	return op == '(' ||
		   op == '[' ||
		   op == ',' ||
		   //		op == '.' ||
		   op == '*' ||
		   op == '?';
}

bool op_valid(const char *op)
{
	return S_EQ(op, "++") ||
		   S_EQ(op, "--") ||
		   S_EQ(op, "+=") ||
		   S_EQ(op, "-=") ||
		   S_EQ(op, "+") ||
		   S_EQ(op, "-") ||
		   S_EQ(op, "*") ||
		   S_EQ(op, "/") ||
		   S_EQ(op, "<=") ||
		   S_EQ(op, ">=") ||
		   S_EQ(op, "==") ||
		   S_EQ(op, "^") ||
		   S_EQ(op, ">>") ||
		   S_EQ(op, "<<") ||
		   S_EQ(op, "*=") ||
		   S_EQ(op, "/=") ||
		   S_EQ(op, "<<=") ||
		   S_EQ(op, ">>=") ||
		   S_EQ(op, "^=") ||
		   S_EQ(op, "&&") ||
		   S_EQ(op, "||") ||
		   S_EQ(op, "|") ||
		   S_EQ(op, "&") ||
		   S_EQ(op, "=") ||
		   S_EQ(op, "!=") ||
		   S_EQ(op, "->") ||
		   S_EQ(op, ".") ||
		   S_EQ(op, ",") ||
		   S_EQ(op, "(") ||
		   S_EQ(op, "[") ||
		   S_EQ(op, "...") ||
		   S_EQ(op, "~") ||
		   S_EQ(op, "?") ||
		   S_EQ(op, "<") ||
		   S_EQ(op, ">") ||
		   S_EQ(op, "%") ||
		   S_EQ(op, "!");
}

char *read_op()
{
	bool single_operator = true;
	char c = nextc();
	int len = 1;
	char *op = calloc(128, sizeof(char));
	op[0] = c;
	if (!op_treated_as_one(c))
	{
		c = peekc();
		int i = 1;
		if (is_single_operator(c))
		{
		again:
			op[i] = c;
			++len;
			single_operator = false;
			nextc();
			c = peekc();
			if (is_single_operator(c))
			{
				++i;
				goto again;
			}
		}
	}
	char *op_ret = calloc(len + 1, sizeof(char));
	for (int i = 0; i < len; i++)
	{
		op_ret[i] = op[i];
	}
	free(op);
	op_ret[len] = '\0';
	if (!op_valid(op_ret))
	{
		compile_error(
			LexProcess->cprocess,
			"The operator %s isn\'t valid\n",
			op_ret);
	}
	return op_ret;
}

void lex_new_expression()
{
	++LexProcess->expression.current_expression_count;
	if (LexProcess->expression.current_expression_count == 1)
	{
		if (LexProcess->expression.buffer_info == NULL)
		{
			LexProcess->expression.buffer_info = calloc(1, sizeof(string_buffer *));
			++LexProcess->expression.string_buffer_count;
		}
		else
		{
			string_buffer **buffer = calloc(LexProcess->expression.string_buffer_count + 1, sizeof(string_buffer *));
			for (int i = 0; i < LexProcess->expression.string_buffer_count; ++i)
			{
				buffer[i] = LexProcess->expression.buffer_info[i];
			}
			++LexProcess->expression.string_buffer_count;
			free(LexProcess->expression.buffer_info);
			LexProcess->expression.buffer_info = buffer;
		}
		LexProcess->expression.buffer_info[LexProcess->expression.string_buffer_count - 1] = creat_string_buffer();
	}
}

void lex_finish_expression()
{
	--LexProcess->expression.current_expression_count;
	if (LexProcess->expression.current_expression_count <
		0)
	{
		compile_error(LexProcess->cprocess, "You can't close an expression that doesn' exist!\n");
	}
}

bool lex_is_in_expression()
{
	return LexProcess->expression.current_expression_count > 0;
}

bool keyword_is_datatype(const char *str)
{
	return S_EQ(str, "char") ||
		   S_EQ(str, "short") ||
		   S_EQ(str, "int") ||
		   S_EQ(str, "long") ||
		   S_EQ(str, "float") ||
		   S_EQ(str, "double") ||
		   S_EQ(str, "struct") ||
		   S_EQ(str, "union");
}

bool is_keyword(const char *str)
{
	return S_EQ(str, "unsigned") ||
		   S_EQ(str, "signed") ||
		   S_EQ(str, "char") ||
		   S_EQ(str, "short") ||
		   S_EQ(str, "int") ||
		   S_EQ(str, "long") ||
		   S_EQ(str, "float") ||
		   S_EQ(str, "double") ||
		   S_EQ(str, "void") ||
		   S_EQ(str, "typedef") ||
		   S_EQ(str, "struct") ||
		   S_EQ(str, "static") ||
		   S_EQ(str, "union") ||
		   S_EQ(str, "return") ||
		   S_EQ(str, "include") ||
		   S_EQ(str, "sizeof") ||
		   S_EQ(str, "if") ||
		   S_EQ(str, "else") ||
		   S_EQ(str, "while") ||
		   S_EQ(str, "for") ||
		   S_EQ(str, "do") ||
		   S_EQ(str, "break") ||
		   S_EQ(str, "switch") ||
		   S_EQ(str, "continue") ||
		   S_EQ(str, "case") ||
		   S_EQ(str, "default") ||
		   S_EQ(str, "goto") ||
		   S_EQ(str, "const") ||
		   S_EQ(str, "extern") ||
		   S_EQ(str, "restrict");
}

Token *make_operator_or_string_token()
{
	char c = peekc();
	if (c == '<')
	{
		Token *token = last_data(LexProcess->tokens);
		if (token_is_keyword(token, "include"))
		{
			return make_string_token('<', '>');
		}
	}
	Token *token = token_creat(&(Token){
		.type = TOKEN_TYPE_OPERATOR,
		.sval = read_op()});
	if (c == '(')
	{
		lex_new_expression();
	}
	return token;
}

Token *make_symbol_token()
{
	char c = nextc();
	if (c == ')')
	{
		lex_finish_expression();
	}
	return token_creat(&(Token){
		.type = TOKEN_TYPE_SYMBOL,
		.cval = c});
}

Token *make_identifier_or_keyword_token()
{
	int count = 0;
	if (lex_is_in_expression())
	{
		count = LexProcess->expression.current_expression_count;
		LexProcess->expression.current_expression_count = 0;
	}
	int i = 0;
	Pos pos_of_LexProcess = LexProcess->pos;
	Pos pos_of_cprocess = LexProcess->cprocess->pos;
	long fp_now = ftell(LexProcess->cprocess->in_fp.fp);
	for (char c = nextc();
		 (c >= 'a' && c <= 'z') ||
		 (c >= 'A' && c <= 'Z') ||
		 (c >= '0' && c <= '9') ||
		 (c == '_');
		 c = nextc())
		++i;
	if (count != 0)
		LexProcess->expression.current_expression_count = count;
	LexProcess->pos = pos_of_LexProcess;
	LexProcess->cprocess->pos = pos_of_cprocess;
	fseek(LexProcess->cprocess->in_fp.fp, fp_now, SEEK_SET);
	char *str = calloc(i + 1, sizeof(char));
	for (int j = 0; j < i; ++j)
	{
		str[j] = nextc();
	}
	str[i] = '\0';
	if (is_keyword(str))
	{
		return token_creat(&(Token){
			.type = TOKEN_TYPE_KEYWORDS,
			.sval = str});
	}
	return token_creat(&(Token){
		.type = TOKEN_TYPE_IDENTIFIER,
		.sval = str});
}

Token *read_special_token()
{
	char c = peekc();
	if (isalpha(c) || c == '_')
	{
		return make_identifier_or_keyword_token();
	}
	return NULL;
}

Token *make_new_line_token()
{
	char c = nextc();
	return token_creat(&(Token){
		.type = TOKEN_TYPE_NEWLINE});
}

Token *make_one_line_comment_token()
{
	int count = 0;
	if (lex_is_in_expression())
	{
		count = LexProcess->expression.current_expression_count;
		LexProcess->expression.current_expression_count = 0;
	}
	int i = 0;
	Pos pos_of_LexProcess = LexProcess->pos;
	Pos pos_of_cprocess = LexProcess->cprocess->pos;
	long fp_now = ftell(LexProcess->cprocess->in_fp.fp);
	char c;
	for (c = nextc();
		 c != '\n' && c != '\377';
		 c = nextc())
	{
		++i;
	}
	if (count != 0)
		LexProcess->expression.current_expression_count = count;
	char *str = calloc(i + 1, sizeof(char));
	LexProcess->pos = pos_of_LexProcess;
	LexProcess->cprocess->pos = pos_of_cprocess;
	fseek(LexProcess->cprocess->in_fp.fp, fp_now, SEEK_SET);
	for (int j = 0; j < i; ++j)
	{
		str[j] = nextc();
	}
	return token_creat(&(Token){
		.type = TOKEN_TYPE_COMMENT,
		.sval = str});
}

Token *make_multiline_comment_token()
{
	int count = 0;
	if (lex_is_in_expression())
	{
		count = LexProcess->expression.current_expression_count;
		LexProcess->expression.current_expression_count = 0;
	}
	int i = 0;
	Pos pos_of_LexProcess = LexProcess->pos;
	Pos pos_of_cprocess = LexProcess->cprocess->pos;
	long fp_now = ftell(LexProcess->cprocess->in_fp.fp);
	char c;
	while (1)
	{
		for (c = peekc();
			 c != '*' && c != '\377';
			 c = peekc())
		{
			++i;
			nextc();
		}
		if (c == '\377')
		{
			compile_error(LexProcess->cprocess, "you didn't close the mutiline_comment\n");
		}
		else if (c == '*')
		{
			nextc();
			++i;
			if (peekc() == '/')
			{
				++i;
				nextc();
				break;
			}
		}
	}
	if (count != 0)
		LexProcess->expression.current_expression_count = count;
	LexProcess->pos = pos_of_LexProcess;
	LexProcess->cprocess->pos = pos_of_cprocess;
	fseek(LexProcess->cprocess->in_fp.fp, fp_now, SEEK_SET);
	char *str = calloc(i + 1, sizeof(char *));
	for (int j = 0; j < i; ++j)
	{
		str[j] = nextc();
	}
	return token_creat(&(Token){
		.type = TOKEN_TYPE_COMMENT,
		.sval = str});
}

Token *handle_comment()
{
	char c = peekc();
	if (c == '/')
	{
		nextc();
		if (peekc() == '/')
		{
			nextc();
			return make_one_line_comment_token();
		}
		else if (peekc() == '*')
		{
			nextc();
			return make_multiline_comment_token();
		}
		pushc('/');
		return make_operator_or_string_token();
	}
	return NULL;
}

Token *make_quote_token()
{
	assert_next_char('\'');
	char c = nextc();
	if (c == '\\')
	{
		c = nextc();
		c = get_escaped_char(c);
	}
	if (nextc() != '\'')
	{
		compile_error(
			LexProcess->cprocess,
			"You didn't close the quote!\n");
	}
	return token_creat(&(Token){
		.type = TOKEN_TYPE_NUMBER,
		.cval = c});
}

bool is_hex_number(char c)
{
	c = tolower(c);
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

char *read_hex_number_str()
{
	string_buffer *buffer = creat_string_buffer();
	while(is_hex_number(peekc()))
	{
		write(buffer,nextc());
	}
	write(buffer,'\0');
	char *str = get_buffer(buffer);
	free_buffer_without_string(buffer);
	return str;
}

Token *make_special_number_hexadcimal()
{
	nextc();
	// 跳过“x”
	char *number_str = read_hex_number_str();
	unsigned long long llnum = 0;
	llnum = strtol(number_str, 0, 16);
	free(number_str);
	return token_creat(&(Token){
		.type = TOKEN_TYPE_NUMBER,
		.llnum = llnum});
}

bool is_bin_number(char c)
{
	return c == '1' ||
		   c == '0';
}

char *read_bin_number_str()
{
	string_buffer *buffer = creat_string_buffer();
	while(is_bin_number(peekc()))
	{
		write(buffer,nextc());
	}
	char* str = get_buffer(buffer);
	free_buffer_without_string(buffer);
	return str;
}

Token *make_special_number_binary()
{
	nextc();
	char *number_str = read_bin_number_str();
	unsigned long long llnum = 0;
	llnum = strtol(number_str, 0, 2);
	free(number_str);
	return token_creat(&(Token){
		.type = TOKEN_TYPE_NUMBER,
		.llnum = llnum});
}

Token *read_next_token()
{
	Token *token = NULL;
	char c = peekc();
	token = handle_comment();
	if (token)
	{
		return token;
	}
	switch (c)
	{
	NUMERIC_CASE:
		token = make_number_token();
		break;
	OPERATOR_CASE_EXCLUDING_DIVISION:
		token = make_operator_or_string_token();
		break;

	SYMBOL_CASE:
		token = make_symbol_token();
		break;
	case ' ':
	case '\t':
		token = handle_whitespace();
		break;

	case '\n':
	case '\r':
		token = make_new_line_token();
		break;

	case '"':
		token = make_string_token('"', '"');
		break;

	case '\'':
		token = make_quote_token();
		break;

	case '\377':
		break;

	default:
		token = read_special_token();
		if (!token)
			compile_error(LexProcess->cprocess, "Unexpected token\n");
	}
	return token;
}

int lex(lex_process *process)
{
	process->expression.current_expression_count = 0;
	process->expression.buffer_info = NULL;
	LexProcess = process;
	Token *token = read_next_token();
	while (token)
	{
		push(LexProcess->tokens, token);
		token = read_next_token();
	}
	return LEX_ANALYSIS_ALL_OK;
}

char string_nextc(lex_process *process)
{
	string_read *p = process->private;
	return p->str[p->index++];
}
char string_peekc(lex_process *process)
{
	return ((string_read *)process->private)->str[((string_read *)process->private)->index];
}
void string_pushc(lex_process *process, char c)
{
	string_read *p = process->private;
	p->str[--p->index] = c;
}

lex_process_functions functions_of_string =
	{
		.next_char = string_nextc,
		.peek_char = string_peekc,
		.push_char = string_pushc};

lex_process *lex_token_build_for_string(compile_process *cprocess, const char *str)
{
	size_t len = strlen(str);
	char *str_new = calloc(len, sizeof(char));
	for (size_t i = 0; i < len; ++i)
	{
		str_new[i] = str[i];
	}
	string_read *string_read_p = calloc(1, sizeof(string_read));
	string_read_p->str = str_new;
	string_read_p->len = len;
	lex_process *process = lex_process_create(cprocess, &functions_of_string, string_read_p);
	if (!process)
	{
		free(str_new);
		free(string_read_p);
		return NULL;
	}
	if (lex(process) != LEX_ANALYSIS_ALL_OK)
	{
		lex_process_free(process);
		free(str_new);
		free(string_read_p);
		return NULL;
	}
	free(str_new);
	//string_read_p->str = str;
	free(string_read_p);
	return process;
}
