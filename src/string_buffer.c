#include "compiler.h"

string_buffer *creat_string_buffer()
{
	string_buffer *buffer = calloc(
		1,
		sizeof(string_buffer));
	buffer->buffer_len = 1;
	buffer->buffer = calloc(1, sizeof(char));
}

void write(string_buffer *buffer, char c)
{
	char *str = realloc(buffer->buffer, (buffer->buffer_len + 1)*sizeof(char));
	str[buffer->buffer_len - 1] = c;
	buffer->buffer = str;
	++buffer->buffer_len;
}

char *get_buffer(string_buffer *buffer)
{
	return buffer->buffer;
}

void free_buffer_without_string(string_buffer *buffer)
{
	free(buffer);
}

void free_buffer(string_buffer *buffer_p)
{
	free(buffer_p->buffer);
	free_buffer_without_string(buffer_p);
}
