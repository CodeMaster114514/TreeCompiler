#include "compiler.h"

Scope *scope_alloc()
{
	Scope* scope = calloc(1,sizeof(Scope));
	scope->entities = creat_mound(sizeof(void *));
	scope->sub = creat_mound(sizeof(Scope *));
	set_peek_in_end(scope->entities);
	set_flag(scope->entities,MOUND_FLAG_PEEK_DECREMENT);
	return scope;
}

void free_scope(Scope *scope)
{
	free_mound(scope->entities);
	set_peek(scope->sub, 0);
	Scope *sub = next_ptr(scope->sub);
	while(sub)
	{
		free_scope(sub);
		sub = next_ptr(scope->sub);
	}
	free_mound(scope->sub);
}

void free_scope_with_root(Scope *root, Scope *current)
{
	bool RootIsInCurrent = false;
	for(Scope *parent = current->parent;parent;parent = parent->parent)
	{
		if(parent == root)
		{
			RootIsInCurrent = true;
		}
	}
	if(RootIsInCurrent)
	{
		free_scope(current);
	}
	else
	{
		free_scope(current);
		free_scope(root);
	}
}

Scope *scope_creat_root(compile_process *process)
{
	assert(!process->scope.root);
	assert(!process->scope.current);

	Scope *root = scope_alloc();
	process->scope.root = root;
	process->scope.current = root;
	return root;
}

void scope_free_root(compile_process *process)
{
	free_scope_with_root(process->scope.root, process->scope.current);
	process->scope.root = NULL;
	process->scope.current = NULL;
}

Scope *scope_new(compile_process *process, int flag)
{
	assert(process->scope.root);
	assert(process->scope.current);

	Scope *new = scope_alloc();
	new->flag |= flag;
	new->parent = process->scope.current;
	push(process->scope.current->sub, new);
	process->scope.current = new;
	return new;
}

void scope_iteration_start(Scope *scope)
{
	set_peek(scope->entities,0);
	if(scope->entities->flag & MOUND_FLAG_PEEK_DECREMENT)
	{
		set_peek_in_end(scope->entities);
	}
}

void scope_iteration_end(Scope *scope)
{
}

void *scope_iteration_back(Scope *scope)
{
	if(!get_count(scope->entities))
		return NULL;
	return next_ptr(scope->entities);
}

void *scope_last_entity_at_scope(Scope *scope)
{
	if(!get_count(scope->entities))
		return NULL;
	return last_data_ptr(scope->entities);
}

void *scope_last_entity_from_scope_stop_at(Scope *scope, Scope *stop)
{
	if(scope == stop)
		return NULL;

	void * last = scope_last_entity_at_scope(scope);
	if(last)
		return last;

	Scope* parent = scope->parent;
	if(parent)
		return scope_last_entity_from_scope_stop_at(parent, stop);

	return NULL;
}

void *scope_last_entity_stop_at(compile_process *process, Scope *stop)
{
	return scope_last_entity_from_scope_stop_at(process->scope.current, stop);
}

void *scope_last_entity(compile_process *process)
{
	return scope_last_entity_stop_at(process,NULL);
}

void scope_push(compile_process *process, void *ptr, size_t size)
{
	push(process->scope.current->entities, &ptr);
	process->scope.current->total += size;
}

void scope_finish(compile_process *process)
{
	Scope *new_scope = process->scope.current->parent;
	free_scope(process->scope.current);
	process->scope.current = new_scope;
	if(!process->scope.current && process->scope.root)
		process->scope.root =  NULL;
}

Scope *scope_current(compile_process *process)
{
	return process->scope.current;
}
