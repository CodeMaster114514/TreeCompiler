#include "compiler.h"

static compile_process *current_process;
static Token *parse_last_token;
extern expressionable_operator_precedence_group
	operator_precendence[TOTAL_OPERATOR_GROUPS];

typedef struct
{
	int flag;
} History;

void parse_expressionable(History *history);

History *history_begin(int flag)
{
	History *history = calloc(1, sizeof(History));
	history->flag = flag;
	return history;
}

History *history_down(History *_history, int flag)
{
	History *history = calloc(1, sizeof(History));
	memcpy(history, _history, sizeof(History));
	history->flag = flag;
	return history;
}

void free_history(History *history)
{
	free(history);
}

static void parse_nl_or_comment(Token *token)
{
	while (token && token_is_nl_or_comment_or_new_line(token))
	{
		next(current_process->tokens);
		token = peek(current_process->tokens);
	}
}

static Token *next_token()
{
	Token *next_token = peek(current_process->tokens);
	parse_nl_or_comment(next_token);
	current_process->pos = next_token->pos;
	parse_last_token = next_token;
	return next(current_process->tokens);
}

static Token *peek_token()
{
	Token *next_token = peek(current_process->tokens);
	parse_nl_or_comment(next_token);
	return peek(current_process->tokens);
}

bool this_token_is_operator(char* op)
{
	return token_is_operator(peek_token(),op);
}

void parse_single_token_to_node()
{
	Token *token = next_token();
	Node *node = NULL;
	switch (token->type)
	{
	case TOKEN_TYPE_NUMBER:
		node = node_creat(&(Node){.type = NODE_TYPE_NUMBER, .llnum = token->llnum});
		break;
	case TOKEN_TYPE_IDENTIFIER:
		node = node_creat(&(Node){.type = NODE_TYPE_IDENTIFIER, .savl = token->sval});
		break;
	case TOKEN_TYPE_STRING:
		node = node_creat(&(Node){.type = NODE_TYPE_STRING, .savl = token->sval});
		break;
	default:
		compile_error(current_process, "This isn't a single token that can be converted to a node.\n");
	}
}

void parse_expressionable_for_op(History *history, const char *op)
{
	parse_expressionable(history);
}

static int parse_get_precedence_for_operator(char *op, expressionable_operator_precedence_group **group)
{
	int i;
	for (i = 0; i < TOTAL_OPERATOR_GROUPS; ++i)
	{
		for (int b = 0; operator_precendence[i].operators[b]; ++b)
		{
			if (S_EQ(operator_precendence[i].operators[b], op))
			{
				*group = &operator_precendence[i];
				return i;
			}
		}
	}
	return -1;
}

bool parse_left_operator_has_priority(char *left, char *right)
{
	expressionable_operator_precedence_group
		*left_group = NULL,
		*right_group = NULL;
	if (S_EQ(left, right))
		return false;
	int left_lever = parse_get_precedence_for_operator(left, &left_group);
	int right_lever = parse_get_precedence_for_operator(right, &right_group);
	if (left_group->associativity == RIGHT_TO_LEFT)
	{
		return false;
	}
	return left_lever <= right_lever;
}

void parse_shift_children_node_left(Node *node)
{
	assert(node->type == NODE_TYPE_EXPRESSION);
	assert(node->exp.node_right->type == NODE_TYPE_EXPRESSION);
	char *right_op = node->exp.node_right->exp.op;
	Node *new_left_node = node->exp.node_left;
	Node *new_right_node = node->exp.node_right->exp.node_left;
	Node *now_right_node = node->exp.node_right;
	make_exp_node(new_left_node, new_right_node, node->exp.op);

	Node *new_left_exp = pop_node();
	Node *new_right_exp = node->exp.node_right->exp.node_right;

	// 释放右边表达式节点
	free(now_right_node);

	node->exp.node_left = new_left_exp;
	node->exp.node_right = new_right_exp;
	node->exp.op = right_op;
}

void parse_reorder_expression(Node **out)
{
	Node *node = *out;
	if (node->type != NODE_TYPE_EXPRESSION)
		return;

	// 不是表达式节点，不用做任何事
	if (node->exp.node_left->type != NODE_TYPE_EXPRESSION &&
		node->exp.node_right && node->exp.node_right->type != NODE_TYPE_EXPRESSION)
		return;

	// 只有右边是表达式
	if (node->exp.node_left->type != NODE_TYPE_EXPRESSION &&
		node->exp.node_right && node->exp.node_right->type == NODE_TYPE_EXPRESSION)
	{
		char *right_op = node->exp.node_right->exp.op;
		if (parse_left_operator_has_priority(node->exp.op, right_op))
		{
			parse_shift_children_node_left(node);
			parse_reorder_expression(&node->exp.node_left);
			parse_reorder_expression(&node->exp.node_right);
		}
	}
}

void parse_exp_normal(History *history)
{
	Token *token = peek_token();
	char *op = token->sval;
	Node *node_left = node_peek_expressionable();
	if (!node_left)
		return;
	// 跳到下一个token
	next_token();
	// 弹出左节点
	pop_node();
	node_left->flag |= NODE_FLAG_INSIDE_EXPRESSION;
	parse_expressionable_for_op(history_down(history, history->flag), op);
	Node *node_right = pop_node();
	node_right->flag |= NODE_FLAG_INSIDE_EXPRESSION;
	make_exp_node(node_left, node_right, op);
	Node *exp_node = pop_node();

	// 重新排序
	parse_reorder_expression(&exp_node);

	push_node(exp_node);
}

int parse_exp(History *history)
{
	parse_exp_normal(history);
	return 0;
}

void parse_identifier(History *history)
{
	assert(peek_token()->type == TOKEN_TYPE_IDENTIFIER);
	parse_single_token_to_node();
}

static bool is_keyword_variable_modifier(const char *var)
{
	return S_EQ(var, "unsigned") ||
		   S_EQ(var, "static") ||
		   S_EQ(var, "signed") ||
		   S_EQ(var, "const") ||
		   S_EQ(var, "extern");
}

void parse_datatype_modeifier(datatype *DataType)
{
	Token *token = peek_token();
	while (token && token->type == TOKEN_TYPE_KEYWORDS)
	{
		if (!is_keyword_variable_modifier(token->sval))
			break;

		if (S_EQ(token->sval, "signed"))
		{
			DataType->flag |= DATATYPE_FLAG_SIGNED;
		}
		else if (S_EQ(token->sval, "unsigned"))
		{
			DataType->flag &= ~DATATYPE_FLAG_SIGNED;
		}
		else if (S_EQ(token->sval, "static"))
		{
			DataType->flag |= DATATYPE_FLAG_STATIC;
		}
		else if (S_EQ(token->sval, "const"))
		{
			DataType->flag |= DATATYPE_FLAG_CONST;
		}
		else if (S_EQ(token->sval, "extern"))
		{
			DataType->flag |= DATATYPE_FLAG_EXTERN;
		}

		next_token();
		token = peek_token();
	}
}

void parse_get_token_data_type(Token **type_token, Token **secondary_token)
{
	*type_token = next_token();
	Token *token = peek_token();
	if (token_is_primitive(token))
	{
		*secondary_token = token;
		next_token();
	}
}

int parse_datatype_expected_for_type(Token *token)
{
	if (S_EQ(token->sval, "union"))
	{
		return DATA_TYPE_EXPECT_UNION;
	}
	else if (S_EQ(token->sval, "struct"))
	{
		return DATA_TYPE_EXPECT_STRUCT;
	}
	return DATA_TYPE_EXPECT_PRIMITIVE;
}

int parse_get_pointer_depth()
{
	int depth = 0;
	while(this_token_is_operator("*"))
	{
		++depth;
		next_token();
	}
	return depth;
}

int get_id()
{
	static int x = 0;
	return x++;
}

char* random_id()
{
	char id[25];
	sprintf(id,"id_of_%i",get_id());
	char* ret_id = malloc(sizeof(id));
	memcpy(ret_id,id,sizeof(id));
	return ret_id;
}

Token* parse_random_id_for_struct_or_union()
{
	Token* token = calloc(1,sizeof(Token));
	token->type = TOKEN_TYPE_IDENTIFIER;
	token->flags |= TOKEN_FLAG_FROM_PARSER;
	token->sval = random_id();
	return token;
}

bool parse_datatype_is_secondary_allow(int expected_type)
{
	return expected_type == DATA_TYPE_EXPECT_PRIMITIVE;
}

bool parse_datatype_is_secondary_allow_for_type(Token* type)
{
	return S_EQ(type->sval,"long") || S_EQ(type->sval,"short") || S_EQ(type->sval,"double");
}

size_t parse_datatype_get_the_size_of_this_combination(int type1,int type2)
{
	size_t out;
	switch(type1+type2)
	{
		case DATA_TYPE_SHORT+DATA_TYPE_INT:
			out = WORD;
			break;
		case DATA_TYPE_LONG+DATA_TYPE_INT:
			out = DWORD;
			break;
		case DATA_TYPE_LONG+DATA_TYPE_LONG:
			out = QWORD;
			break;
		case DATA_TYPE_DOUBLE+DATA_TYPE_LONG:
			out = DQWORD;
			break;
		default:
			compile_error(current_process,"There isn’t have such combination\n");
	}
	return out;
}

void parse_datatype_init_type_and_size_for_primitive(Token* type,Token* secondary,datatype* out);

void parse_datatype_adjust_size_for_primitive(datatype* out)
{
	out->size = parse_datatype_get_the_size_of_this_combination(out->type,out->secondary->type);
}

void parse_datatype_adjust_type_and_size_for_primitive(Token* secondary,datatype* out)
{
	if(!secondary)
	{
		return;
	}
	datatype* secondary_data_type = calloc(1,sizeof(datatype));
	parse_datatype_init_type_and_size_for_primitive(secondary,NULL,secondary_data_type);
	out->secondary = secondary_data_type;
	parse_datatype_adjust_size_for_primitive(out);
}

void parse_datatype_init_type_and_size_for_primitive(Token* type,Token* secondary,datatype* out)
{
	if(!parse_datatype_is_secondary_allow_for_type(type) && secondary)
	{
		compile_error(current_process,"this data type can‘t have secondary type\n");
	}
	if(S_EQ(type->sval,"void"))
	{
		out->type = DATA_TYPE_VOID;
		out->size = ZERO;
	}
	else if(S_EQ(type->sval,"char"))
	{
		out->type = DATA_TYPE_CHAR;
		out->size = BYTE;
	}
	else if(S_EQ(type->sval,"short"))
	{
		out->type = DATA_TYPE_SHORT;
		out->size = WORD;
	}
	else if(S_EQ(type->sval,"int"))
	{
		out->type = DATA_TYPE_INT;
		out->size = DWORD;
	}
	else if(S_EQ(type->sval,"long"))
	{
		out->type = DATA_TYPE_LONG;
		out->size = DWORD;
	}
	else if(S_EQ(type->sval,"float"))
	{
		out->type = DATA_TYPE_FLOAT;
		out->size = DWORD;
	}
	else if(S_EQ(type->sval,"double"))
	{
		out->type = DATA_TYPE_DOUBLE;
		out->size = QWORD;
	}
	else
	{
		compile_error(current_process,"BUG: Other keywords have entered this function\n");
	}

	parse_datatype_adjust_type_and_size_for_primitive(secondary,out);
}

void parse_datatype_init_type_and_size(Token* type,Token* secondary,datatype* out,int pointer_depth,int expected_type)
{
	if(!parse_datatype_is_secondary_allow(expected_type) && secondary)
	{
		compile_error(current_process,"this data type can’t have secondary type\n");
	}
	switch(expected_type)
	{
		case DATA_TYPE_EXPECT_PRIMITIVE:
			parse_datatype_init_type_and_size_for_primitive(type,secondary,out);
			break;
		case DATA_TYPE_EXPECT_UNION:
		case DATA_TYPE_EXPECT_STRUCT:
			if(type->flags & TOKEN_FLAG_FROM_PARSER)
			{
				free(type->sval);
				free(type);
			}
			compile_error(current_process,"Structures and unions are not supported just now.\n");
			break;
	}
}

void parse_datatype_init(Token* type,Token* secondary,datatype* out,int pointer_depth,int expected_type)
{
	parse_datatype_init_type_and_size(type,secondary,out,pointer_depth,expected_type);
	out->type_str = type->sval;
}

void parse_datatype_type(datatype *DataType)
{
	Token *type_token = NULL, *secondary_token = NULL;
	parse_get_token_data_type(&type_token, &secondary_token);
	int expected_type = parse_datatype_expected_for_type(type_token);
	if(datatype_is_struct_or_union_for_name(type_token))
	{
		if(peek_token()->type == TOKEN_TYPE_IDENTIFIER)
		{
			type_token = next_token();
		}
		else
		{
			type_token = parse_random_id_for_struct_or_union();
			DataType->flag |= DATATYPE_FLAG_STRUCT_OR_UNION_NO_NAME;
		}
	}
	int pointer_depth = parse_get_pointer_depth();
	parse_datatype_init(type_token,secondary_token,DataType,pointer_depth,expected_type);
}

void parse_datatype(datatype **DataType)
{
	*DataType = calloc(1, sizeof(datatype));
	(*DataType)->flag |= DATATYPE_FLAG_SIGNED;

	parse_datatype_modeifier(*DataType);
	parse_datatype_type(*DataType);
	parse_datatype_modeifier(*DataType);
}

void parse_variable_function_or_struct_union(History *history)
{
	datatype *DataType;
	parse_datatype(&DataType);
	#warning "The data type isn’t into node and don’t pushed"
	free(DataType);
}

void parse_keyword(History *history)
{
	Token *token = peek_token();
	if (is_keyword_variable_modifier(token->sval) || keyword_is_datatype(token->sval))
	{
		parse_variable_function_or_struct_union(history);
		return;
	}
}

int parse_expressionable_single(History *history)
{
	Token *token = peek_token();
	if (!token)
	{
		return -1;
	}

	history->flag |= NODE_FLAG_INSIDE_EXPRESSION;
	int res = -1;
	switch (token->type)
	{
	case TOKEN_TYPE_IDENTIFIER:
		parse_identifier(history);
		res = 0;
		break;
	case TOKEN_TYPE_NUMBER:
		parse_single_token_to_node();
		res = 0;
		break;
	case TOKEN_TYPE_OPERATOR:
		res = parse_exp(history);
		break;
	}
	return res;
}

void parse_expressionable(History *history)
{
	while (parse_expressionable_single(history) == 0)
		;
	free_history(history);
}

void parse_keyword_for_global()
{
	parse_keyword(history_begin(0));
	Node *node = pop_node();
}

int parse_next()
{
	Token *token = peek_token();
	if (!token)
	{
		return -1;
	}
	int res = 0;
	switch (token->type)
	{
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_IDENTIFIER:
	case TOKEN_TYPE_STRING:
		parse_expressionable(history_begin(0));
		break;
	case TOKEN_TYPE_NEWLINE:
	case TOKEN_TYPE_COMMENT:
		next_token();
		res = parse_next();
		break;
	case TOKEN_TYPE_KEYWORDS:
		parse_keyword_for_global();
	}
	return res;
}

int parse(compile_process *process)
{
	current_process = process;
	set_peek(process->tokens, 0);
	set_mound(current_process->node, current_process->node_tree);
	get_count(process->tokens);
	Node *node = NULL;
	while (!parse_next())
	{
		node = next_node();
		push(current_process->node_tree, &node);
	}
	return PARSE_ALL_OK;
}

