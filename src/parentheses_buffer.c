#include "compiler.h"

parentheses_buffer *creat_parentheses_buffer()
{
	parentheses_buffer *buffer = calloc(
		1,
		sizeof(parentheses_buffer));
	buffer->buffer_len = 1;
	buffer->buffer = calloc(1, sizeof(char));
}

void write(parentheses_buffer *buffer, char c)
{
	char *str = calloc(buffer->buffer_len + 1, sizeof(char));
	for (int i = 0; i < buffer->buffer_len; ++i)
	{
		str[i] = buffer->buffer[i];
	}
	str[buffer->buffer_len - 1] = c;
	free(buffer->buffer);
	buffer->buffer = str;
	++buffer->buffer_len;
}

char **get_buffer(parentheses_buffer *buffer)
{
	return &(buffer->buffer);
}

void free_buffer(parentheses_buffer *buffer_p)
{
	free(buffer_p->buffer);
	free(buffer_p);
}
