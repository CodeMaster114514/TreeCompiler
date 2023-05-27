#include "compiler.h"

static lex_process* LexProcess;
static Token tmp_token;

Token* read_next_token();

static char peekc()
{
	return LexProcess->process_functions->peek_char(LexProcess);
}

static char nextc()
{
	char c = LexProcess->process_functions->next_char(LexProcess);
	++LexProcess->pos.col;
	if(c == '\n')
	{
		++LexProcess->pos.line;
		LexProcess->pos.col = 1;
	}
	return c;
}

static void pushc(char c)
{
	LexProcess->process_functions->push_char(LexProcess,c);
}

static Token* token_creat(Token* _token)
{
	memcpy(&tmp_token,_token,sizeof(Token));
	tmp_token.pos = LexProcess->pos;
	return &tmp_token;
}

const char* read_number_str()
{
	const char* str_ret = NULL;
	int i = 0;
	Pos pos_of_Lex = LexProcess->pos;
	Pos pos_of_compiler = LexProcess->cprocess->pos;
	long fp_now = ftell(LexProcess->cprocess->in_fp.fp);
	while(peekc() >= '0' && peekc() <= '9')
	{
		nextc();
		++i;
	}
	LexProcess->pos = pos_of_Lex;
	LexProcess->cprocess->pos = pos_of_compiler;
	fseek(LexProcess->cprocess->in_fp.fp,fp_now,SEEK_SET);
	char* str = calloc(i+1,sizeof(char));
	for(int j = 0;j < i;++j)
	{
		str[j] = nextc();
	}
	str[i] = '\0';
	str_ret = str;
	return str_ret;
}

Token* lexer_last_token()
{
	return &LexProcess->tokens[LexProcess->token_count-1];
}

Token* handle_whitespace()
{
	Token* token = lexer_last_token();
	if(token)
	{
		token->whitespace = true;
	}
	nextc();
	return read_next_token();
}

unsigned long long read_number()
{
	const char* s = read_number_str();
	return atoll(s);
}

Token* make_number_token_for_value(unsigned long number)
{
	Token token = {
		.type=TOKEN_TYPE_NUMBER,
		.llnum = number
	};
	return token_creat(&token);
}

Token* make_number_token()
{
	return make_number_token_for_value(read_number());
}

Token* make_string_token(char start_delim,char end_delim)
{
	int i = 0;
	char c = nextc();
	Pos pos_of_LexProcess = LexProcess->pos;
	Pos pos_of_cprocess = LexProcess->cprocess->pos;
	long fp_now = ftell(LexProcess->cprocess->in_fp.fp);
	for(c = nextc();c != end_delim && c != EOF;c = nextc())
	{
		if(c == '\n')
		{
			compile_error(LexProcess->cprocess,"There is no corresponding symbol\n");
		}
		if(c == '\\')
		{
			continue;
		}
		i++;
	}
	LexProcess->pos = pos_of_LexProcess;
	LexProcess->cprocess->pos = pos_of_cprocess;
	fseek(LexProcess->cprocess->in_fp.fp,fp_now,SEEK_SET);
	char* str = calloc(i+1,sizeof(char));
	for(int j = 0;j < i;++j)
	{
		char data = nextc();
		if(data == '\\')
		{
			data = nextc();
			switch(data)
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
			default:
				compile_error(
					LexProcess->cprocess,
					"E escape character compilation error\n"
				);
			}
		}
		str[j] = data;
	}
	nextc();
	Token token = {
		.type = TOKEN_TYPE_STRING,
		.sval = str
	};
	return token_creat(&token);
}

Token* read_next_token()
{
	Token* token = NULL;
	char c = peekc();
	switch(c)
	{
		NUMERIC_CASE:
			token = make_number_token();
			break;
		case '\n':
		case ' ':
		case '\t':
			token = handle_whitespace();
			break;
		case '"':
			token = make_string_token('"','"');
			break;
		case EOF:
			break;

		default:
			compile_error(LexProcess->cprocess,"Unexpexted token\n");
	}
	return token;
}

void add_token(Token** tokens_in,Token* token,int token_count)
{
	Token* tokens = tokens_in[0];
	Token* tokens_next = calloc(token_count+1,sizeof(Token));
	for(int i = 0;i < token_count;i++)
	{
		tokens_next[i] = tokens[i];
	}
	free(tokens);
	tokens = tokens_next;
	tokens_in[0] = tokens;
	tokens[token_count] = token[0];
}

int lex(lex_process* process){
	process->current_expression_count = 0;
	process->parentheses_buffer = NULL;
	LexProcess = process;
	Token* token = read_next_token();
	process->tokens[0] = token[0];
	++process->token_count;
	while(token)
	{
		token = read_next_token();
		if(token){
			add_token(&process->tokens,token,process->token_count);
			++process->token_count;
		}
	}
	return LEX_ANALYSIS_ALL_OK;
}
