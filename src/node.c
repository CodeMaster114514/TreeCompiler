#include "compiler.h"

static mound *node = NULL;
static mound *node_root = NULL;
extern compile_process *current_process;
Node *parse_current_body = NULL;
Node *parse_current_function = NULL;

void set_mound(mound *node_set, mound *node_root_set)
{
	node = node_set;
	node_root = node_root_set;
}

void free_node(Node *data);

void free_variable_list_node(Node *data)
{
	set_peek(data->var_list.list, 0);
	Node *node = next_ptr(data->var_list.list);
	while (node)
	{
		free_node(node);
		node = next_ptr(data->var_list.list);
	}
	free_mound(data->var_list.list);
}

void free_body_node(Node *data)
{
	set_peek(data->body.statements, 0);
	Node *node = next_ptr(data->body.statements);
	while (node)
	{
		free_node(node);
		node = next_ptr(data->body.statements);
	}
	free_mound(data->body.statements);
}

void free_function_node(Node *data)
{
	mound *variables = data->function.args.variables;
	if (variables)
		set_peek(variables, 0);
	while (variables && peek_ptr(variables))
	{
		free_node(next_ptr(variables));
	}
	if (variables)
		free_mound(variables);
	if (data->function.body_node)
		free_node(data->function.body_node);
}

void free_for_statement_node(Node *data)
{
	Node *init_node = data->statement.for_statement.init_node;
	Node *condition_node = data->statement.for_statement.condition_node;
	Node *loop_node = data->statement.for_statement.loop_node;
	Node *body_node = data->statement.for_statement.body_node;

	if (init_node)
		free_node(init_node);
	if (condition_node)
		free_node(condition_node);
	if (loop_node)
		free_node(loop_node);
}

void free_switch_node(Node *data)
{
	mound *cases = data->statement.switch_statement.cases;
	free_node(data->statement.switch_statement.body_node);
	free_node(data->statement.switch_statement.exp_node);
	set_peek(cases, 0);
	Node *case_node = next_ptr(cases);
	while (case_node)
	{
		free_node(case_node);
		case_node = next_ptr(cases);
	}
}

void free_node(Node *data)
{
	switch (data->type)
	{
	case NODE_TYPE_NUMBER:
	case NODE_TYPE_IDENTIFIER:
	case NODE_TYPE_STATEMENT_BREAK:
	case NODE_TYPE_STATEMENT_CONTINUE:
		break;

	case NODE_TYPE_VARIABLE:
		free_datatype(&data->var.datatype);
		if (data->var.value)
			free_node(data->var.value);
		break;

	case NODE_TYPE_VARIABLE_LIST:
		free_variable_list_node(data);
		break;

	case NODE_TYPE_EXPRESSION:
		if (data->exp.node_left)
			free_node(data->exp.node_left);
		if (data->exp.node_right)
			free_node(data->exp.node_right);
		break;

	case NODE_TYPE_TENARY:
		free_node(data->tenary.true_node);
		free_node(data->tenary.false_node);
		break;

	case NODE_TYPE_EXPRESSION_PARENTHESES:
		free_node(data->parenthesis.exp);
		break;

	case NODE_TYPE_BRACKET:
		free_node(data->brackets.inner);
		break;

	case NODE_TYPE_STRUCT:
		if (data->_struct.body_node)
			free_node(data->_struct.body_node);
		break;

	case NODE_TYPE_UNION:
		if (data->_union.body_node)
			;
		free_node(data->_union.body_node);
		break;

	case NODE_TYPE_BODY:
		free_body_node(data);
		break;

	case NODE_TYPE_FUNCTION:
		free_function_node(data);
		break;

	case NODE_TYPE_STATEMENT_IF:
		free_node(data->statement.if_statement.condition_node);
		free_node(data->statement.if_statement.body_node);

		if (data->statement.if_statement.next)
			free_node(data->statement.if_statement.next);
		break;

	case NODE_TYPE_STATEMENT_ELSE:
		free_node(data->statement.else_statement.body_node);
		break;

	case NODE_TYPE_STATEMENT_RETURN:
		free_node(data->statement.return_statement.exp);
		break;

	case NODE_TYPE_STATEMENT_FOR:
		free_for_statement_node(data);
		break;

	case NODE_TYPE_STATEMENT_WHILE:
		free_node(data->statement.while_statement.condition_node);
		free_node(data->statement.while_statement.body_node);
		break;
	
	case NODE_TYPE_STATEMENT_DO_WHILE:
		free_node(data->statement.do_while_statement.condition_node);
		free_node(data->statement.do_while_statement.body_node);
		break;
	
	case NODE_TYPE_LABEL:
		free_node(data->label.name_node);
		break;
	
	case NODE_TYPE_STATEMENT_GOTO:
		free_node(data->statement.goto_statement.label_node);
		break;

	case NODE_TYPE_STATEMENT_SWITCH:
		free_switch_node(data);
		break;

	case NODE_TYPE_STATEMENT_CASE:
		free_node(data->statement.case_statement.exp_node);
		break;
	}
	free(data);
}

void free_nodes()
{
	int node_count = get_count(node);
	int node_root_count = get_count(node_root);
	set_peek(node, 0);
	set_peek(node_root, 0);
	bool nodeIsAllOk = false, rootIsAllOk = false;
	Node *data, *root;
	for (;;)
	{
		if (!nodeIsAllOk)
		{
			data = peek_ptr(node);
			if (!data)
			{
				nodeIsAllOk = true;
			}
			else
			{
				free_node(data);
				next(node);
			}
		}
		if (!rootIsAllOk)
		{
			root = peek_ptr(node_root);
			if (!root)
			{
				rootIsAllOk = true;
			}
			else
			{
				if (root == data)
				{
					next(node_root);
				}
				else
				{
					free_node(root);
					next(node_root);
				}
			}
		}
		if (rootIsAllOk && nodeIsAllOk)
			break;
	}
}

void push_node(Node *data)
{
	push(node, &data);
}

Node *next_node()
{
	Node *data = next_ptr(node);
	if (!data)
		return NULL;
	return data;
}

Node *peek_node()
{
	Node *data = peek_ptr(node);
	if (!data)
		return NULL;
	return data;
}

Node *pop_node()
{
	Node *last_node = last_data_ptr(node);
	Node *last_node_root;
	if (!isEmpty(node_root))
	{
		last_node_root = last_data_ptr(node_root);
	}
	else
	{
		last_node_root = NULL;
	}
	pop(node);
	if (last_node == last_node_root)
	{
		pop(node_root);
	}
	return last_node;
}

bool node_is_expressionable(Node *node)
{
	return node->type == NODE_TYPE_EXPRESSION || node->type == NODE_TYPE_EXPRESSION_PARENTHESES || node->type == NODE_TYPE_UNARY || node->type == NODE_TYPE_IDENTIFIER || node->type == NODE_TYPE_NUMBER || node->type == NODE_TYPE_STRING;
}

Node *node_peek_expressionable()
{
	Node *node = peek_node();
	if (!node)
	{
		return NULL;
	}
	return node_is_expressionable(node) ? node : NULL;
}

Node *make_exp_node(Node *left, Node *right, char *op)
{
	assert(left || right);
	return node_creat(&(Node){.type = NODE_TYPE_EXPRESSION, .exp.node_left = left, .exp.node_right = right, .exp.op = op});
}

Node *make_variable_list_node(mound *var_list)
{
	return node_creat(&(Node){.type = NODE_TYPE_VARIABLE_LIST, .var_list.list = var_list});
}

Node *make_bracket_node(Node *node)
{
	return node_creat(&(Node){.type = NODE_TYPE_BRACKET, .brackets.inner = node});
}

Node *make_body_node(size_t size, mound *body, bool padding, Node *largest_variable)
{
	return node_creat(&(Node){.type = NODE_TYPE_BODY, .body.statements = body, .body.size = size, .body.padding = padding, .body.largest_variable = largest_variable});
}

Node *make_struct_node(char *name, Node *body_node)
{
	int flags = 0;
	if (!body_node)
	{
		flags |= NODE_FLAG_IS_FORWARD_DECLARATION;
	}

	return node_creat(&(Node){.type = NODE_TYPE_STRUCT, .flags = flags, ._struct.name = name, ._struct.body_node = body_node});
}

Node *make_function_node(DataType *ret_datatype, char *name, mound *input_variables, Node *body_node)
{
	return node_creat(&(Node){.type = NODE_TYPE_FUNCTION, .function.return_datatype = *ret_datatype, .function.name = name, .function.args.variables = input_variables, .function.body_node = body_node, .function.args.stack_offset = current_process->flags & COMPILE_PROCESS_FLAG_OUT_X86 ? DQWORD : ZERO});
}

Node *make_exp_parentheses_node(Node *exp)
{
	return node_creat(&(Node){.type = NODE_TYPE_EXPRESSION_PARENTHESES, .parenthesis.exp = exp});
}

Node *make_if_node(Node *condition, Node *body, size_t var_size, Node *next)
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_IF, .statement.if_statement.condition_node = condition, .statement.if_statement.body_node = body, .statement.if_statement.variable_size = var_size, .statement.if_statement.next = next});
}

Node *make_else_node(Node *body)
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_ELSE, .statement.else_statement.body_node = body});
}

Node *make_union_node(char *name, Node *body_node)
{
	int flags = 0;
	if (!body_node)
	{
		flags |= NODE_FLAG_IS_FORWARD_DECLARATION;
	}

	return node_creat(&(Node){.type = NODE_TYPE_UNION, .flags = flags, ._union.name = name, ._union.body_node = body_node});
}

Node *make_return_node(Node *exp)
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_GOTO, .statement.return_statement.exp = exp});
}

Node *make_for_node(Node *init_node, Node *condition_node, Node *loop_node, Node *body_node)
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_FOR, .statement.for_statement = {.init_node = init_node, .condition_node = condition_node, .loop_node = loop_node, .body_node = body_node}});
}

Node *make_while_node(Node *condition_node, Node *body_ndoe)
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_WHILE, .statement.while_statement = {.condition_node = condition_node, .body_node = body_ndoe}});
}

Node *make_do_while_node(Node *condition_node, Node *body_node)
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_DO_WHILE, .statement.do_while_statement = {.condition_node = condition_node, .body_node = body_node}});
}

Node *make_switch_node(Node *exp_node, Node *body_node, mound *cases, bool has_default_case)
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_SWITCH, .statement.switch_statement = {.exp_node = exp_node, .body_node = body_node, .cases = cases, .has_default_case = has_default_case}});
}

Node *make_braek_node()
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_BREAK});
}

Node *make_continue_node()
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_CONTINUE});
}

Node *make_label_node(Node *label_name_node)
{
	return node_creat(&(Node){.type = NODE_TYPE_LABEL, .label.name_node = label_name_node});
}

Node *make_goto_node(Node *label_node)
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_GOTO, .statement.goto_statement.label_node = label_node});
}

Node *make_case_node(Node *exp_node)
{
	return node_creat(&(Node){.type = NODE_TYPE_STATEMENT_CASE, .statement.case_statement.exp_node = exp_node});
}

Node *make_tenary_node(Node *true_node, Node *false_node)
{
	return node_creat(&(Node){.type = NODE_TYPE_TENARY, .tenary = {.true_node = true_node, .false_node = false_node}});
}

Node *node_creat(Node *_node)
{
	Node *node = calloc(1, sizeof(Node));
	memcpy(node, _node, sizeof(Node));
	node->binded.owner = parse_current_body;
	node->binded.function = parse_current_function;
	node->pos = current_process->pos;
	push_node(node);
	return node;
}

bool node_is_struct_or_union(Node *node)
{
	if (node->type != NODE_TYPE_VARIABLE)
	{
		return false;
	}

	return data_type_is_struct_or_union(&node->var.datatype);
}

bool variable_node_is_primitive(Node *node)
{
	switch (node->type)
	{
	case NODE_TYPE_VARIABLE:
		return data_type_is_primitive(&node->var.datatype);
		break;

	case NODE_TYPE_VARIABLE_LIST:
		return data_type_is_primitive(&variable_in_var_list(node)->var.datatype);
		break;

	default:
		assert(0);
		break;
	}
}

Node *variable_node(Node *node)
{
	Node *var = NULL;

	switch (node->type)
	{
	case NODE_TYPE_VARIABLE:
		var = node;
		break;
	default:
		break;
	}

	return var;
}

Node *variable_node_or_list(Node *node)
{
	if (node->type == NODE_TYPE_VARIABLE_LIST)
	{
		return node;
	}

	return variable_node(node);
}

bool node_is_variables(Node *node)
{
	return node->type == NODE_TYPE_VARIABLE || NODE_TYPE_VARIABLE_LIST;
}

Node *variables_node(Node *node)
{
	if (node->type == NODE_TYPE_VARIABLE || node->type == NODE_TYPE_VARIABLE_LIST)
	{
		return variable_node_or_list(node);
	}

	return NULL;
}

int variable_list_size(Node *node)
{
	set_peek(node->var_list.list, 0);
	Node *var = peek_ptr(node->var_list.list);

	return var->var.datatype.size;
}

Node *variable_in_var_list(Node *node)
{
	set_peek(node->var_list.list, 0);
	Node *var = peek_ptr(node->var_list.list);

	return var;
}

bool node_have_body(Node *node)
{
	return node->type == NODE_TYPE_BODY || node->type == NODE_TYPE_STATEMENT_IF || node->type == NODE_TYPE_STATEMENT_ELSE || node->type == NODE_TYPE_STATEMENT_WHILE || node->type == NODE_TYPE_STATEMENT_SWITCH;
}

int node_body_size(Node *node)
{
	int ret = 0;
	switch (node->type)
	{
	case NODE_TYPE_BODY:
		ret = node->body.size;
		break;

	case NODE_TYPE_STATEMENT_IF:
		ret = node->statement.if_statement.variable_size;
		break;

	case NODE_TYPE_STATEMENT_ELSE:
		ret = node->statement.else_statement.body_node->body.size;
		break;
	case NODE_TYPE_STATEMENT_FOR:
		ret = node->statement.for_statement.var_size;
		break;
	case NODE_TYPE_STATEMENT_WHILE:
	case NODE_TYPE_STATEMENT_SWITCH:

	default:
		ret = 0;
		break;
	}

	return ret;
}

Node *node_from_sym(Symble *symble)
{
	if (symble->type != SYMBLE_TYPE_NODE)
	{
		return NULL;
	}

	return symble->data;
}

Node *node_from_symble(compile_process *process, char *name)
{
	Symble *symble = symresolver_get_symble_by_name(process, name);

	if (!symble)
	{
		return NULL;
	}

	return node_from_sym(symble);
}

Node *struct_node_for_name(compile_process *process, char *name)
{
	Node *node = node_from_symble(process, name);

	if (node && node->type != NODE_TYPE_STRUCT)
	{
		return NULL;
	}

	return node;
}

Node *union_node_for_name(compile_process *process, char *name)
{
	Node *node = node_from_symble(process, name);

	if (node && node->type != NODE_TYPE_UNION)
	{
		return NULL;
	}

	return node;
}

size_t function_node_args_stack_offset(Node *node)
{
	assert(node->type == NODE_TYPE_FUNCTION);
	return node->function.args.stack_offset;
}

bool node_is_expression_or_parentheses(Node *node)
{
	return node->type == NODE_TYPE_EXPRESSION || node->type == NODE_TYPE_EXPRESSION_PARENTHESES;
}

bool node_is_value_type(Node *node)
{
	return node_is_expression_or_parentheses(node) || node->type == NODE_TYPE_IDENTIFIER || node->type == NODE_TYPE_NUMBER || node->type == NODE_TYPE_UNION || node->type == NODE_TYPE_TENARY || node->type == NODE_TYPE_STRING;
}
