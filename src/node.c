#include "compiler.h"

static mound *node = NULL;
static mound *node_root = NULL;
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

void free_node(Node *data)
{
	if (data->type == NODE_TYPE_NUMBER || data->type == NODE_TYPE_IDENTIFIER)
	{
		free(data);
		return;
	}
	if (data->type == NODE_TYPE_VARIABLE)
	{
		free_datatype(&data->var.datatype);
		if (data->var.value)
			free_node(data->var.value);
	}
	if (data->type == NODE_TYPE_VARIABLE_LIST)
	{
		free_variable_list_node(data);
	}
	if (data->type == NODE_TYPE_EXPRESSION)
	{
		free_node(data->exp.node_left);
		free_node(data->exp.node_right);
	}
	if (data->type == NODE_TYPE_EXPRESSION_PARENTHESES)
	{
		free_node(data->parenthesis.exp);
	}
	if (data->type == NODE_TYPE_BRACKET)
	{
		free_node(data->brackets.inner);
	}
	if (data->type == NODE_TYPE_STRUCT)
	{
		if (data->flags & DATATYPE_FLAG_STRUCT_OR_UNION_NO_NAME)
		{
			free(data->_struct.name);
		}
		if (data->_struct.body_node)
			free_node(data->_struct.body_node);
	}
	if (data->type == NODE_TYPE_BODY)
	{
		free_body_node(data);
	}
	if (data->type == NODE_TYPE_FUNCTION)
	{
		free_function_node(data);
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
	Node *node = next_node();
	if (!node)
	{
		return NULL;
	}
	return node_is_expressionable(node) ? node : NULL;
}

Node *make_exp_node(Node *left, Node *right, char *op)
{
	assert(left);
	assert(right);
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

Node *make_function_node(DataType *ret_datatype, char *name, mound *variables, Node *body_node)
{
	return node_creat(&(Node){.type = NODE_TYPE_FUNCTION, .function.return_datatype = *ret_datatype, .function.name = name, .function.args.variables = variables, .function.body_node = body_node, .function.args.stack_addition = DQWORD});
}

Node *make_exp_parentheses_node(Node *exp)
{
	return node_creat(&(Node){.type = NODE_TYPE_EXPRESSION_PARENTHESES, .parenthesis.exp = exp});
}

Node *node_creat(Node *_node)
{
	Node *node = calloc(1, sizeof(Node));
	memcpy(node, _node, sizeof(Node));
	node->binded.owner = parse_current_body;
	node->binded.function = parse_current_function;
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
	assert(node->type == NODE_TYPE_VARIABLE);

	return data_type_is_primitive(&node->var.datatype);
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

size_t function_node_args_stack_addition(Node *node)
{
	assert(node->type == NODE_TYPE_FUNCTION);
	return node->function.args.stack_addition;
}

bool node_is_expression_or_parentheses(Node *node)
{
	return node->type == NODE_TYPE_EXPRESSION || node->type == NODE_TYPE_EXPRESSION_PARENTHESES;
}

bool node_is_value_type(Node *node)
{
	return node_is_expression_or_parentheses(node) || node->type == NODE_TYPE_IDENTIFIER || node->type == NODE_TYPE_NUMBER || node->type == NODE_TYPE_UNION || node->type == NODE_TYPE_TENARY || node->type == NODE_TYPE_STRING;
}
