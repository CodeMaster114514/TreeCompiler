#include "compiler.h"

compile_process *current_process;
static Token *parse_last_token;
extern expressionable_operator_precedence_group operator_precendence[TOTAL_OPERATOR_GROUPS];
extern Node *parse_current_body;
extern Node *parse_current_function;

enum
{
	PARSE_SCOPE_ENTITY_ON_STACK = 0b0001,
	PARSE_SCOPE_ENTITY_STRUCT_SCOPE = 0b0010
};

typedef struct
{
	int flags;

	// 在栈或结构体中的偏移
	int offset;

	// 变量节点
	Node *node;
} parse_scope_entity;

parse_scope_entity *parse_new_scope_entity(Node *node, int flags, int offset)
{
	parse_scope_entity *scope_entity = calloc(1, sizeof(parse_scope_entity));
	scope_entity->flags = flags;
	scope_entity->offset = offset;
	scope_entity->node = node;
	return scope_entity;
}

parse_scope_entity *parse_scope_last_entity_stop_global_scope()
{
	return scope_last_entity_stop_at(current_process, current_process->scope.root);
}

parse_scope_entity *parse_scope_last_entity_stop_parent()
{
	return scope_last_entity_stop_at(current_process, current_process->scope.current->parent);
}

enum
{
	HISTORY_FLAG_INSIDE_UNION = 0b00000001,
	HISTORY_FLAG_INSIDE_STRUCT = 0b00000010,
	HISTORY_FLAG_IS_PARAMETER_STACK = 0b00000100,
	HISTORY_FLAG_IS_GLOBAL_SCOPE = 0b00001000,
	HISTORY_FlAG_INSIDE_FUNCTION_BODY = 0b00100000,
	HISTORY_FLAG_FUNCTION_HAVE_VARIABLE = 0b01000000,
	HISTORY_FLAG_INSIDE_EXPRESSION = 0b10000000
};

typedef struct
{
	int flags;
} History;

void parse_expressionable(History *history);

History *history_begin(int flags)
{
	History *history = calloc(1, sizeof(History));
	history->flags = flags;
	return history;
}

History *history_down(History *_history, int flags)
{
	History *history = calloc(1, sizeof(History));
	memcpy(history, _history, sizeof(History));
	history->flags = flags;
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
	if (!next_token)
	{
		return NULL;
	}
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

void parse_scope_new()
{
	scope_new(current_process, 0);
}

void parse_scope_finish()
{
	scope_finish(current_process);
}

void parse_scope_push(parse_scope_entity *entity, size_t size)
{
	scope_push(current_process, entity, size);
}

static void expect_sym(char sym)
{
	Token *token = next_token();
	if (!token_is_symbol(token, sym))
	{
		compile_error(current_process, "Expect the symbol \"%c\" however, other symbols are provided.\n", sym);
	}
}

static void expect_op(char *op)
{
	Token *token = next_token();
	if (!token_is_operator(token, op))
	{
		compile_error(current_process, "Expect the symbole \"%s\" however, other operator are provided.\n", op);
	}
}

static void expect_keyword(char *keyword)
{
	Token *token = next_token();
	if (!token_is_keyword(token, keyword))
	{
		compile_error(current_process, "Expect the keyword \"%s\" however, other keyword are provided.\n", keyword);
	}
}

bool this_token_is_operator(char *op)
{
	return token_is_operator(peek_token(), op);
}

bool this_token_is_symbol(char sym)
{
	return token_is_symbol(peek_token(), sym);
}

bool this_token_is_keyword(char *keyword)
{
	return token_is_keyword(peek_token(), keyword);
}

bool this_token_is_self_increase_operator()
{
	return this_token_is_operator("++") || this_token_is_operator("--");
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
	Node *new_left_exp = make_exp_node(new_left_node, new_right_node, node->exp.op);
	pop_node();

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
	{
		return;
	}
	// 跳到下一个token
	next_token();
	pop_node();
	node_left->flags |= NODE_FLAG_INSIDE_EXPRESSION;
	parse_expressionable_for_op(history, op);
	Node *node_right = pop_node();
	node_right->flags |= NODE_FLAG_INSIDE_EXPRESSION;
	Node *exp_node = make_exp_node(node_left, node_right, op);
	pop_node();

	// 重新排序
	parse_reorder_expression(&exp_node);

	push_node(exp_node);
}

void parse_deal_with_addition_expression()
{
	if (peek_token && peek_token()->type == TOKEN_TYPE_OPERATOR)
	{
		History *history = history_begin(0);
		parse_expressionable(history);
		free_history(history);
	}
}

void parse_expressionable_root(History *history);

void parse_parentheses(History *history)
{
	expect_op("(");
	Node *left = NULL, *tmp_node = peek_node();
	if (tmp_node && node_is_value_type(tmp_node))
	{
		left = tmp_node;
		pop_node();
	}

	Node *exp_node = NULL;
	if (!this_token_is_symbol(')'))
	{
		History *second = history_begin(0);
		parse_expressionable_root(second);
		free_history(second);
		exp_node = pop_node();
	}
	expect_sym(')');

	make_exp_parentheses_node(exp_node);

	if (left)
	{
		Node *parentheses = pop_node();
		make_exp_node(left, parentheses, "()");
	}

	parse_deal_with_addition_expression();
}

void parse_increase_exp(History *history)
{
	Token *token = next_token();
	char *op = token->sval;
	Node *node = peek_node();
	if (node && node->type == NODE_TYPE_IDENTIFIER)
	{
		pop_node();
		make_exp_node(node, NULL, op);
		return;
	}
	else
	{
		token = peek_token();

		if (token->type != TOKEN_TYPE_IDENTIFIER)
		{
			compile_error(current_process, "The operatoring object of the self-increment and subtraction operator must be a variable\n");
		}

		parse_single_token_to_node();
	}
	node = pop_node();

	make_exp_node(NULL, node, op);
}

int parse_exp(History *history)
{
	if (this_token_is_operator(","))
	{
		return -1;
	}
	else if (this_token_is_operator("("))
	{
		parse_parentheses(history);
	}
	else if (this_token_is_self_increase_operator())
	{
		parse_increase_exp(history);
	}
	else
	{
		parse_exp_normal(history);
	}
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

void parse_datatype_modeifier(DataType *datatype)
{
	Token *token = peek_token();
	while (token && token->type == TOKEN_TYPE_KEYWORDS)
	{
		if (!is_keyword_variable_modifier(token->sval))
			break;

		if (S_EQ(token->sval, "signed"))
		{
			datatype->flags |= DATATYPE_FLAG_SIGNED;
		}
		else if (S_EQ(token->sval, "unsigned"))
		{
			datatype->flags &= ~DATATYPE_FLAG_SIGNED;
		}
		else if (S_EQ(token->sval, "static"))
		{
			datatype->flags |= DATATYPE_FLAG_STATIC;
		}
		else if (S_EQ(token->sval, "const"))
		{
			datatype->flags |= DATATYPE_FLAG_CONST;
		}
		else if (S_EQ(token->sval, "extern"))
		{
			datatype->flags |= DATATYPE_FLAG_EXTERN;
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
	while (this_token_is_operator("*"))
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

char *random_id()
{
	char id[25];
	sprintf(id, "id_of_%i", get_id());
	char *ret_id = malloc(sizeof(id));
	memcpy(ret_id, id, sizeof(id));
	return ret_id;
}

Token *parse_random_id()
{
	Token *token = calloc(1, sizeof(Token));
	token->type = TOKEN_TYPE_IDENTIFIER;
	token->flags |= TOKEN_FLAG_FROM_PARSER;
	token->sval = random_id();
	return token;
}

Token *parse_random_id_for_struct_or_union()
{
	return parse_random_id();
}

Token *parse_random_id_for_function_args()
{
	return parse_random_id();
}

bool parse_datatype_is_secondary_allow(int expected_type)
{
	return expected_type == DATA_TYPE_EXPECT_PRIMITIVE;
}

bool parse_datatype_is_secondary_allow_for_type(Token *type)
{
	return S_EQ(type->sval, "long") || S_EQ(type->sval, "short") || S_EQ(type->sval, "double");
}

void parse_datatype_init_type_and_size_for_primitive(Token *type, Token *secondary, DataType *out);

size_t parse_datatype_get_the_size_of_this_combination(int type1, int type2)
{
	size_t out;
	switch (type1 + type2)
	{
	case DATA_TYPE_LONG + DATA_TYPE_LONG:
		out = QWORD;
		break;
	case DATA_TYPE_DOUBLE + DATA_TYPE_LONG:
		out = DQWORD;
		break;
	default:
		compile_error(current_process, "There isn’t have such combination\n");
	}
	return out;
}

void parse_datatype_adjust_size_for_primitive(DataType *out)
{
	if (out->type == DATA_TYPE_INT && out->type == DATA_TYPE_INT)
	{
		compile_error(current_process, "Doesn’t have data type \"int int\"");
	}
	if (out->secondary && out->secondary->type == DATA_TYPE_INT)
	{
		return;
	}
	out->size = parse_datatype_get_the_size_of_this_combination(out->type, out->secondary->type);
}

void parse_datatype_adjust_type_and_size_for_primitive(Token *secondary, DataType *out)
{
	if (!secondary)
	{
		return;
	}
	DataType *secondary_data_type = calloc(1, sizeof(DataType));
	parse_datatype_init_type_and_size_for_primitive(secondary, NULL, secondary_data_type);
	secondary_data_type->type_str = secondary->sval;
	out->secondary = secondary_data_type;
	parse_datatype_adjust_size_for_primitive(out);
}

void parse_datatype_init_type_and_size_for_primitive(Token *type, Token *secondary, DataType *out)
{
	if (!parse_datatype_is_secondary_allow_for_type(type) && secondary)
	{
		compile_error(current_process, "this data type can‘t have secondary type\n");
	}
	if (S_EQ(type->sval, "void"))
	{
		out->type = DATA_TYPE_VOID;
		out->size = ZERO;
	}
	else if (S_EQ(type->sval, "char"))
	{
		out->type = DATA_TYPE_CHAR;
		out->size = BYTE;
	}
	else if (S_EQ(type->sval, "short"))
	{
		out->type = DATA_TYPE_SHORT;
		out->size = WORD;
	}
	else if (S_EQ(type->sval, "int"))
	{
		out->type = DATA_TYPE_INT;
		out->size = DWORD;
	}
	else if (S_EQ(type->sval, "long"))
	{
		out->type = DATA_TYPE_LONG;
		out->size = DWORD;
	}
	else if (S_EQ(type->sval, "float"))
	{
		out->type = DATA_TYPE_FLOAT;
		out->size = DWORD;
	}
	else if (S_EQ(type->sval, "double"))
	{
		out->type = DATA_TYPE_DOUBLE;
		out->size = QWORD;
	}
	else
	{
		compile_error(current_process, "BUG: Other keywords have entered this function\n");
	}

	parse_datatype_adjust_type_and_size_for_primitive(secondary, out);
}

size_t size_of_struct(char *struct_name)
{
	Symble *sym = symresolver_get_symble_by_name(current_process, struct_name);
	if (!sym)
	{
		return 0;
	}

	assert(sym->type == SYMBLE_TYPE_NODE);
	Node *struct_node = sym->data;
	assert(struct_node && struct_node->type == NODE_TYPE_STRUCT);
	return struct_node->_struct.body_node->body.size;
}

void parse_datatype_init_type_and_size_for_struct(DataType *out, Token *type)
{
	out->type = DATA_TYPE_STRUCT;
	out->size = size_of_struct(type->sval);
	out->struct_node = struct_node_for_name(current_process, type->sval);
	if (out->flags & DATATYPE_FLAG_STRUCT_OR_UNION_NO_NAME)
	{
		free_token(type);
	}
}

size_t size_of_union(char *union_name)
{
	Symble *sym = symresolver_get_symble_by_name(current_process, union_name);
	if (!sym)
	{
		return 0;
	}

	assert(sym->type == SYMBLE_TYPE_NODE);
	Node *union_node = sym->data;
	assert(union_node && union_node->type == NODE_TYPE_UNION);
	return union_node->_union.body_node->body.size;
}

void parse_datatype_init_type_and_size_for_union(DataType *out, Token *type)
{
	out->type = DATA_TYPE_UNION;
	out->size = size_of_union(type->sval);
	out->union_node = union_node_for_name(current_process, type->sval);
	if (out->flags & DATATYPE_FLAG_STRUCT_OR_UNION_NO_NAME)
	{
		free_token(type);
	}
}

void parse_datatype_init_type_and_size(Token *type, Token *secondary, DataType *out, int pointer_depth, int expected_type)
{
	if (!parse_datatype_is_secondary_allow(expected_type) && secondary)
	{
		compile_error(current_process, "this data type can’t have secondary type\n");
	}
	switch (expected_type)
	{
	case DATA_TYPE_EXPECT_PRIMITIVE:
		parse_datatype_init_type_and_size_for_primitive(type, secondary, out);
		break;
	case DATA_TYPE_EXPECT_STRUCT:
		parse_datatype_init_type_and_size_for_struct(out, type);
		break;
	case DATA_TYPE_EXPECT_UNION:
		parse_datatype_init_type_and_size_for_union(out, type);
		break;
	}
	if (pointer_depth > 0)
	{
		out->flags |= DATATYPE_FLAG_POINTER;
		out->pointer_depth = pointer_depth;
	}
}

void parse_datatype_init(Token *type, Token *secondary, DataType *out, int pointer_depth, int expected_type)
{
	out->type_str = type->sval;
	parse_datatype_init_type_and_size(type, secondary, out, pointer_depth, expected_type);
}

void parse_datatype_type(DataType *datatype)
{
	Token *type_token = NULL, *secondary_token = NULL;
	parse_get_token_data_type(&type_token, &secondary_token);
	int expected_type = parse_datatype_expected_for_type(type_token);
	if (datatype_is_struct_or_union_for_name(type_token))
	{
		if (peek_token()->type == TOKEN_TYPE_IDENTIFIER)
		{
			type_token = next_token();
		}
		else
		{
			type_token = parse_random_id_for_struct_or_union();
			datatype->flags |= DATATYPE_FLAG_STRUCT_OR_UNION_NO_NAME;
		}
	}
	int pointer_depth = parse_get_pointer_depth();
	parse_datatype_init(type_token, secondary_token, datatype, pointer_depth, expected_type);
}

void parse_datatype(DataType *datatype)
{
	//*DataType = calloc(1, sizeof(datatype));
	datatype->flags |= DATATYPE_FLAG_SIGNED;

	parse_datatype_modeifier(datatype);
	parse_datatype_type(datatype);
	parse_datatype_modeifier(datatype);
}

void parse_expressionable_root(History *history);

void parse_variable_node(DataType *datatype, Token *token, Node *value)
{
	char *name_str = NULL;
	if (token)
	{
		name_str = token->sval;
	}

	node_creat(&(Node){.type = NODE_TYPE_VARIABLE, .var.datatype = *datatype, .var.name = name_str, .var.value = value});
}

void parse_scope_parameter_offset_on_stack(int *offset, parse_scope_entity *last_entity, Node *variable, History *history)
{
	if (current_process->flags & COMPILE_PROCESS_FLAG_OUT_X86)
	{
		*offset = -(*offset);
	}
	if (last_entity)
	{
		*offset = datatype_size(&variable_node(last_entity->node)->var.datatype);
	}
}

void parse_scope_offset_on_stack(Node *variable, History *history)
{
	parse_scope_entity *last_entity = history->flags & HISTORY_FLAG_IS_PARAMETER_STACK || !(history->flags & HISTORY_FLAG_FUNCTION_HAVE_VARIABLE) ? parse_scope_last_entity_stop_parent() : parse_scope_last_entity_stop_global_scope();
	bool parameter_stack = history->flags & HISTORY_FLAG_IS_PARAMETER_STACK;
	int offset = -variable_size(variable);

	if (parameter_stack)
	{
		parse_scope_parameter_offset_on_stack(&offset, last_entity, variable, history);
	}

	if (last_entity)
	{
		offset += variable_node(last_entity->node)->var.stack_offset;
		if (!variable_node_is_primitive(variable))
		{
			offset += padding(offset, 4);
		}
	}

	variable->var.stack_offset = offset;
}

void parse_scope_offset_global(Node *variable, History *history)
{
}

void parse_scope_offset_for_struct(Node *variable, History *history)
{
	int offset = 0;
	parse_scope_entity *last_entity = parse_scope_last_entity_stop_global_scope();

	if (last_entity)
	{
		offset += last_entity->offset + last_entity->node->var.datatype.size;
		if (variable_node_is_primitive(last_entity->node))
		{
			variable->var.padding = padding(offset, variable->var.datatype.size);
		}

		variable->var.aoffset = offset + variable->var.padding;
	}
}

void parse_scope_offset(Node *variable, History *history)
{
	if (history->flags & HISTORY_FLAG_IS_GLOBAL_SCOPE)
	{
		parse_scope_offset_global(variable, history);
		return;
	}

	if (history->flags & HISTORY_FLAG_INSIDE_STRUCT)
	{
		parse_scope_offset_for_struct(variable, history);
		return;
	}

	parse_scope_offset_on_stack(variable, history);
}

void parse_variable_node_and_register(History *history, DataType *datatype, Token *name, Node *value)
{
	parse_variable_node(datatype, name, value);
	Node *variable_node = pop_node();

	parse_scope_offset(variable_node, history);

	parse_scope_push(parse_new_scope_entity(variable_node, 0, variable_node->var.aoffset), variable_node->var.datatype.size);

	push_node(variable_node);
}

ArrayBrackets *parse_array_brackets(History *history)
{
	ArrayBrackets *bracket = new_array_brackets();
	while (this_token_is_operator("["))
	{
		expect_op("[");
		if (token_is_symbol(peek_token(), ']'))
		{
			expect_sym(']');
			break;
		}

		parse_expressionable_root(history);
		Node *exp_node = pop_node();
		expect_sym(']');

		Node *bracket_node = make_bracket_node(exp_node);
		pop_node();
		array_brackets_add(bracket, bracket_node);
	}

	return bracket;
}

void parse_variable(DataType *datatype, Token *name, History *history)
{
	Node *value_node = NULL;

	ArrayBrackets *brackets = NULL;
	if (this_token_is_operator("["))
	{
		brackets = parse_array_brackets(history);
		datatype->array.brackets = brackets;
		datatype->flags |= DATATYPE_FLAG_ARRAY;
		datatype->array.size = array_brackets_calculate_size(datatype);
	}

	if (datatype->size == 0 && datatype->pointer_depth == 0)
	{
		compile_error(current_process, "The size of variable can't be zero\n");
	}

	if (this_token_is_operator("="))
	{
		next_token();
		parse_expressionable_root(history);
		value_node = pop_node();
	}

	parse_variable_node_and_register(history, datatype, name, value_node);
}

void parse_keyword(History *history);

void parse_symbol();

void parse_statement(History *history)
{
	if (peek_token()->type == TOKEN_TYPE_KEYWORDS)
	{
		parse_keyword(history);
		return;
	}
	if (peek_token()->type == TOKEN_TYPE_SYMBOL && !this_token_is_symbol(';'))
	{
		parse_symbol(history);
		return;
	}
	parse_expressionable_root(history);
	expect_sym(';');
}

void parse_finish_body(History *history, Node *body_node, mound *statement, size_t *size, Node *largest_align_variable_node, Node *largest)
{
	if (history->flags & HISTORY_FLAG_INSIDE_UNION)
	{
		if (largest && largest->type == NODE_TYPE_VARIABLE)
		{
			*size = variable_size(largest);
		}
		else if (largest && largest->type == NODE_TYPE_VARIABLE_LIST)
		{
			*size = variable_size(variable_node_or_list(largest));
		}
	}
	int padding = compute_sum_padding(statement);
	*size += padding;
	if (largest_align_variable_node)
	{
		*size = align_value(*size, variable_size(largest_align_variable_node->type == NODE_TYPE_VARIABLE_LIST ? variable_in_var_list(largest_align_variable_node) : largest_align_variable_node));
	}
	body_node->body.statements = statement;
	body_node->body.size = *size;
	body_node->body.padding = false;
	body_node->body.largest_variable = largest;
}

void parse_append_size_for_struct_or_union(History *history, size_t *size, Node *node)
{
	*size += variable_size(node);
	if (node->var.datatype.flags & DATATYPE_FLAG_POINTER)
	{
		return;
	}

	Node *largest_node = variable_struct_or_union_node(node)->body.largest_variable;

	if (largest_node)
	{
		*size = align_value(*size, largest_node->var.datatype.size);
	}
}

void parse_append_size_for_node(History *history, size_t *size, Node *node);

void parse_append_size_for_variable_list(History *history, size_t *size, mound *list)
{
	Node *node = NULL;
	set_peek(list, 0);
	node = next_ptr(list);
	while (node)
	{
		parse_append_size_for_node(history, size, node);
		node = next_ptr(list);
	}
}

void parse_append_size_for_node(History *history, size_t *size, Node *node)
{
	// compile_warning(current_process, "parse size isn't finish");
	if (!node)
	{
		return;
	}

	if (node->type == NODE_TYPE_VARIABLE)
	{
		if (node_is_struct_or_union(node))
		{
			parse_append_size_for_struct_or_union(history, size, node);
			return;
		}

		*size += variable_size(node);
	}
	else if (node->type == NODE_TYPE_VARIABLE_LIST)
	{
		parse_append_size_for_variable_list(history, size, node->var_list.list);
	}
	else if (node_have_body(node))
	{
		*size += node_body_size(node);
	}
}

void parse_single_statement(size_t *size, mound *body, History *history)
{
	make_body_node(0, NULL, false, NULL);
	Node *node_body = pop_node();
	node_body->binded.owner = parse_current_body;

	parse_current_body = node_body;

	Node *statement = NULL;
	History *secend = history_down(history, history->flags);
	parse_statement(secend);
	free_history(secend);
	statement = pop_node();

	push(body, &statement);

	parse_append_size_for_node(history, size, statement);

	Node *largest = NULL;
	if (statement->type == NODE_TYPE_VARIABLE)
	{
		largest = statement;
	}

	parse_finish_body(history, node_body, body, size, largest, largest);

	parse_current_body = node_body->binded.owner;

	push_node(node_body);
}

void parse_body_multiple_statment(size_t *size, mound *body, History *history)
{
	make_body_node(0, NULL, false, NULL);
	Node *body_node = pop_node();
	body_node->binded.owner = parse_current_body;

	Node *statement_node = NULL, *largest_align_variable_node = NULL, *largest = NULL, *largest_body = NULL;

	parse_current_body = body_node;

	expect_sym('{');

	while (!this_token_is_symbol('}'))
	{
		History *second = history_down(history, history->flags);

		parse_statement(second);
		statement_node = pop_node();

		if (statement_node->type == NODE_TYPE_VARIABLE || statement_node->type == NODE_TYPE_VARIABLE_LIST)
		{
			history->flags |= HISTORY_FLAG_FUNCTION_HAVE_VARIABLE;
		}

		bool isBody = node_have_body(statement_node);

		if (largest_body && isBody)
		{
			int statement_size = node_body_size(statement_node), largest_body_size = node_body_size(largest_body);
			largest_body = statement_size > largest_body_size ? statement_node : largest_body;
		}
		else if (isBody)
		{
			largest_body = statement_node;
		}

		if (statement_node->type == NODE_TYPE_VARIABLE || statement_node->type == NODE_TYPE_VARIABLE_LIST)
		{

			switch (statement_node->type)
			{
			case NODE_TYPE_VARIABLE:
				largest = largest && largest->var.datatype.size > statement_node->var.datatype.size ? largest : statement_node;
				break;

			case NODE_TYPE_VARIABLE_LIST:
				largest = largest && largest->var.datatype.size > variable_list_size(statement_node) ? largest : variable_in_var_list(statement_node);
				break;

			default:
				break;
			}

			if (variable_node_is_primitive(statement_node))
			{
				if (!largest_align_variable_node || largest_align_variable_node->var.datatype.size < statement_node->var.datatype.size)
				{
					largest_align_variable_node = statement_node;
				}
			}
		}

		// 将语句推入body中
		push(body, &statement_node);

		// 计算
		if (node_is_variables(statement_node))
		{
			parse_append_size_for_node(history, size, variables_node(statement_node));
		}

		free_history(second);
	}

	if (largest_body)
	{
		parse_append_size_for_node(history, size, largest_body);
	}

	expect_sym('}');

	parse_finish_body(history, body_node, body, size, largest_align_variable_node, largest);

	parse_current_body = body_node->binded.owner;

	push_node(body_node);
}

void parse_body(size_t *size, History *history)
{
	parse_scope_new();

	size_t tmp_size = 0;
	mound *body = creat_mound(sizeof(Node *));

	if (!size)
		size = &tmp_size;

	if (!this_token_is_symbol('{'))
		parse_single_statement(size, body, history);

	parse_body_multiple_statment(size, body, history);

	parse_scope_finish();
}

void parse_variable_full(History *history)
{
	DataType datatype = {0};
	parse_datatype(&datatype);

	Token *name = NULL;
	if (peek_token() && peek_token()->type == TOKEN_TYPE_IDENTIFIER)
	{
		name = next_token();
	}
	else
	{
		name = parse_random_id_for_function_args();
	}

	parse_variable(&datatype, name, history);

	if (name->flags & TOKEN_FLAG_FROM_PARSER)
	{
		Node *variable = pop_node();
		variable->flags |= NODE_FLAG_FUNCTION_ARGS_NO_NAME;
		push_node(variable);

		free_token(name);
	}
}

void parse_function_body(History *history)
{
	History *second = history_down(history, history->flags | HISTORY_FlAG_INSIDE_FUNCTION_BODY);
	parse_body(NULL, second);
	free_history(second);
}

mound *parse_function_args(History *history)
{
	parse_scope_new();
	mound *args = creat_mound(sizeof(Node *));
	while (!this_token_is_symbol(')'))
	{
		if (this_token_is_operator("..."))
		{
			parse_scope_finish();
			return args;
		}

		History *second = history_down(history, history->flags | HISTORY_FLAG_IS_PARAMETER_STACK);
		parse_variable_full(second);
		free_history(second);

		Node *arg = pop_node();
		push(args, &arg);

		if (!this_token_is_operator(","))
		{
			break;
		}

		next_token();
	}

	return args;
}

void parse_function(DataType *ret_datatype, Token *name_token, History *history)
{
	mound *args = NULL;
	parse_scope_new();
	make_function_node(ret_datatype, name_token->sval, NULL, NULL);
	Node *function_node = pop_node();
	parse_current_function = function_node;
	expect_op("(");
#warning "Parse function args"
	args = parse_function_args(history);
	expect_sym(')');
	function_node->function.args.variables = args;
	if (symresolver_get_symble_for_native_function_by_name(current_process, name_token->sval))
	{
		function_node->function.flags |= FUNCTION_FLAG_IS_NATIVE_FUNCTION;
	}
	if (this_token_is_symbol('{'))
	{
		History *second = history_down(history, history->flags);
		parse_function_body(second);
		free_history(second);
		Node *body_node = pop_node();
		function_node->function.body_node = body_node;
		function_node->function.stack_size = align_value(body_node->body.size, 16);
		if (current_process->flags & COMPILE_PROCESS_FLAG_OUT_X64)
		{
			function_node->function.args.stack_offset = -(function_node->function.stack_size);
		}
	}
	else
	{
		expect_sym(';');
	}

	parse_current_function = NULL;
	push_node(function_node);
	parse_scope_finish();
}

void parse_struct_no_new_scope(DataType *datatype, bool isForwardDeclaration)
{
	Node *node = NULL; // 存储body node或struct node
	size_t size = 0;

	History *history = history_begin(HISTORY_FLAG_INSIDE_STRUCT);
	if (!isForwardDeclaration)
	{
		parse_body(&size, history);
		node = pop_node();
	}

	make_struct_node(datatype->type_str, node);

	if (node)
	{
		datatype->size = node->body.size;
	}

	node = pop_node();
	datatype->struct_node = node;

	push_node(node);

	free_history(history);
}

void parse_struct(DataType *datatype)
{
	bool isForwardDeclaration = !this_token_is_symbol('{');
	if (!isForwardDeclaration)
	{
		parse_scope_new();
	}

	parse_struct_no_new_scope(datatype, isForwardDeclaration);

	if (!isForwardDeclaration)
	{
		parse_scope_finish();
	}
}

void parse_union_no_new_scope(DataType *datatype, bool isForwardDeclaration)
{
	Node *node = NULL; // 存储body node或union node
	size_t size = 0;

	History *history = history_begin(HISTORY_FLAG_INSIDE_UNION);
	if (!isForwardDeclaration)
	{
		parse_body(&size, history);
		node = pop_node();
	}

	make_union_node(datatype->type_str, node);

	if (node)
	{
		datatype->size = node->body.size;
	}

	node = pop_node();
	datatype->union_node = node;

	push_node(node);

	free_history(history);
}

void parse_union(DataType *datatype)
{
	bool isForwardDeclaration = !this_token_is_symbol('{');
	if (!isForwardDeclaration)
	{
		parse_scope_new();
	}

	parse_union_no_new_scope(datatype, isForwardDeclaration);

	if (!isForwardDeclaration)
	{
		parse_scope_finish();
	}
}

void parse_struct_or_union(DataType *datatype)
{
	switch (datatype->type)
	{
	case DATA_TYPE_STRUCT:
		parse_struct(datatype);
		break;

	case DATA_TYPE_UNION:
		parse_union(datatype);
		break;

	default:
		compile_error(current_process, "BUG: Compiler failed creat data type");
		break;
	}
}

void parse_variable_function_or_struct_union(History *history)
{
	DataType datatype = {0};
	parse_datatype(&datatype);

	if (!symresolver_get_symble_by_name(current_process, datatype.type_str) && data_type_is_struct_or_union(&datatype) && this_token_is_symbol('{'))
	{
		parse_struct_or_union(&datatype);
		Node *struct_or_union_node = pop_node();
		symresovler_build_for_node(current_process, struct_or_union_node);
		push_node(struct_or_union_node);
	}
	else if (data_type_is_struct_or_union(&datatype) && this_token_is_symbol('{'))
	{
		compile_error(current_process, "\"%s %s\" has been initiallized\n", datatype.type == DATA_TYPE_STRUCT ? "struct" : "union", datatype.type_str);
	}

	Token *token = peek_token();
	if (!token_is_identifier(token))
	{
		if (data_type_is_struct_or_union(&datatype))
		{
			goto exit;
		}
		compile_error(current_process, "Variable or funvtion name must be identifier");
	}
	next_token();

	if (this_token_is_operator("("))
	{
		parse_function(&datatype, token, history);
		return;
	}

	parse_variable(&datatype, token, history);

	if (this_token_is_operator(","))
	{
		mound *var_list = creat_mound(sizeof(Node *));
		Node *var = pop_node();
		push(var_list, &var);
		while (this_token_is_operator(","))
		{
			next_token();
			token = next_token();
			if (token->type != TOKEN_TYPE_IDENTIFIER)
			{
				compile_error(current_process, "the variable name must is identifier");
			}

			parse_variable(&datatype, token, history);
			var = pop_node();
			push(var_list, &var);
		}

		make_variable_list_node(var_list);
	}

exit:
	expect_sym(';');

	// free(DataType);
}

void parse_if_statement(History *history);

void parse_else_or_else_if_statement(History *history)
{
	expect_keyword("else");

	if (this_token_is_keyword("if"))
	{
		parse_if_statement(history);
		return;
	}

	parse_body(NULL, history);

	make_else_node(pop_node());
}

void parse_if_statement(History *history)
{
	expect_keyword("if");
	expect_op("(");
	parse_expressionable_root(history);
	expect_sym(')');

	Node *condition = pop_node(), *next = NULL;

	size_t var_size = 0;
	parse_body(&var_size, history);
	Node *body = pop_node();

	if (this_token_is_keyword("else"))
	{
		parse_else_or_else_if_statement(history);
		next = pop_node();
		size_t next_size = node_body_size(next);
		var_size = var_size > next_size ? var_size : next_size;
	}

	make_if_node(condition, body, var_size, next);
}

void parse_return_statement(History *history)
{
	expect_keyword("return");
	if (this_token_is_symbol(';'))
	{
		make_return_node(NULL);
		goto over;
	}
	parse_expressionable_root(history);
	Node *exp = pop_node();
	make_return_node(exp);

over:
	expect_sym(';');
}

bool parse_for_loop_part(History *history)
{
	if (this_token_is_symbol(';'))
	{
		next_token();
		return false;
	}

	if (peek_token()->type == TOKEN_TYPE_KEYWORDS)
	{
		parse_keyword(history);
	}
	else
	{
		parse_expressionable_root(history);
		expect_sym(';');
	}
	return true;
}

bool parse_for_loop_part_loop(History *history)
{
	if (this_token_is_symbol(')'))
	{
		next_token();
		return false;
	}

	parse_expressionable_root(history);
	return true;
}

void parse_for_statement(History *history)
{
	expect_keyword("for");

	parse_scope_new();

	Node *init_node = NULL, *condition_node = NULL, *loop_node = NULL, *body_node = NULL;
	size_t var_size = 0;

	expect_op("(");

	if (parse_for_loop_part(history))
	{
		init_node = pop_node();
	}

	if (init_node)
	{
		switch (init_node->type)
		{
		case NODE_TYPE_VARIABLE:
			var_size += variable_size(init_node);
			break;

		case NODE_TYPE_VARIABLE_LIST:
			var_size += variable_list_size(init_node);
			break;

		default:
			break;
		}
	}

	if (parse_for_loop_part(history))
	{
		condition_node = pop_node();
	}

	if (parse_for_loop_part_loop(history))
	{
		loop_node = pop_node();
	}

	expect_sym(')');

	parse_body(&var_size, history);
	body_node = pop_node();

	parse_scope_finish();
	make_for_node(init_node, condition_node, loop_node, body_node);
}

void parse_keyword_parentheses_expression(char *keyword)
{
	expect_keyword(keyword);
	expect_op("(");

	History *history = history_begin(0);
	parse_expressionable_root(history);
	free_history(history);

	expect_sym(')');
}

void parse_while_statement(History *history)
{
	parse_keyword_parentheses_expression("while");
	Node *exp_node = pop_node();

	parse_body(NULL, history);
	Node *body_node = pop_node();

	make_while_node(exp_node, body_node);
}

void parse_do_while_statement(History *history)
{
	expect_keyword("do");

	parse_body(NULL, history);
	Node *body_node = pop_node();

	parse_keyword_parentheses_expression("while");
	Node *condition_node = pop_node();

	make_do_while_node(condition_node, body_node);
}

void parse_keyword(History *history)
{
	Token *token = peek_token();
	if (is_keyword_variable_modifier(token->sval) || keyword_is_datatype(token->sval))
	{
		parse_variable_function_or_struct_union(history);
		return;
	}

	if (this_token_is_keyword("if"))
	{
		parse_if_statement(history);
		return;
	}

	if (this_token_is_keyword("return"))
	{
		parse_return_statement(history);
		return;
	}

	if (this_token_is_keyword("for"))
	{
		parse_for_statement(history);
		return;
	}

	if (this_token_is_keyword("while"))
	{
		parse_while_statement(history);
	}

	if (this_token_is_keyword("do"))
	{
		parse_do_while_statement(history);
	}
}

int parse_expressionable_single(History *history)
{
	Token *token = peek_token();
	if (!token)
	{
		return -1;
	}

	history->flags |= NODE_FLAG_INSIDE_EXPRESSION;
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
}

void parse_expressionable_root(History *history)
{
	History *second = history_down(history, history->flags);
	parse_expressionable(second);
	free_history(second);
	Node *result_node = pop_node();

	push_node(result_node);
}

void parse_keyword_for_global()
{
	History *history = history_begin(0);
	parse_keyword(history);
	free_history(history);
	Node *node = pop_node();

	push_node(node);
}

void parse_symbol(History *history)
{
	if (this_token_is_symbol('{'))
	{
		size_t size = 0;
		History *second = history_down(history, history->flags);
		parse_body(&size, second);
		free_history(second);
	}
}

int parse_next()
{
	Token *token = peek_token();
	if (!token)
	{
		return -1;
	}
	History *history;
	int res = 0;
	switch (token->type)
	{
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_IDENTIFIER:
	case TOKEN_TYPE_STRING:
		history = history_begin(0);
		parse_expressionable(history);
		free_history(history);
		break;
	case TOKEN_TYPE_SYMBOL:
		history = history_begin(HISTORY_FLAG_IS_GLOBAL_SCOPE);
		parse_symbol(history);
		free_history(history);
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
	//	set_mound(current_process->node, current_process->node_tree);
	//	get_count(process->tokens);
	Node *node = NULL;
	while (!parse_next())
	{
		node = next_node();
		push(current_process->node_tree, &node);
	}
	return PARSE_ALL_OK;
}
