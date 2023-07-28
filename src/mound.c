#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "mound.h"

bool noCross(mound *this)
{
    return (this->peek >= 0 && this->write_p >= 0 && this->peek <= this->place && this->write_p <= this->place);
}

mound *creat_mound(size_t len)
{
    mound *ret = calloc(1, sizeof(mound));
    ret->buffer = calloc(20, len);
    ret->len = len;
    ret->place = 20;
    ret->count = 0;
    ret->write_p = 0;
    return ret;
}

mound *creat_mound_with_data(void *data, size_t len, size_t count)
{
    mound *ret = calloc(1, sizeof(mound));
    ret->buffer = calloc(count, len);
    ret->len = len;
    ret->count = count;
    ret->peek = 0;
    ret->write_p = 0;
    free(data);
    return ret;
}

void *read(mound *this, size_t p)
{
    return (void *)(this->buffer + p * this->len);
}

void push(mound *this, void *data)
{
    size_t write = this->write_p++;
    if (this->write_p >= this->place)
    {
        this->buffer = realloc(this->buffer, this->write_p * this->len);
        ++this->place;
    }
    memcpy(this->buffer + write * this->len, data, this->len);
    ++this->count;
    assert(noCross(this));
}

void pop(mound *this)
{
    --this->count;
    --this->write_p;
    --this->peek;
    if (this->peek < 0)
    {
        this->peek = 0;
    }
    assert(noCross(this));
}

void set_peek(mound *this, size_t p)
{
    if (p >= this->count)
    {
        this->peek = get_count(this) - 1;
    }
    else
    {
        this->peek = p;
    }
}

size_t get_count(mound *this)
{
	if(!this)
		return 0;
	return this->count;
}

void free_mound(mound *this)
{
    free(this->buffer);
    free(this);
}

void *peek(mound *this)
{
    if (this->peek >= this->count)
    {
        return NULL;
    }
    return (void *)(this->buffer + this->peek * this->len);
}

void *next(mound *this)
{
    void *data = peek(this);
    if (!data)
    {
        return data;
    }
    if(this->flag & MOUND_FLAG_PEEK_DECREMENT)
	    --this->peek;
    else
	    ++this->peek;
    assert(noCross(this));
    return data;
}

void *last_data(mound *this)
{
	if(this->count <= 0)
		return NULL;
	return (void *)(this->buffer + (this->count - 1) * this->len);
}

bool isEmpty(mound *this)
{
    return this->count == 0;
}

void set_flag(mound *this, int flag)
{
	this->flag |= flag;
}

void unset_flag(mound *this, int flag)
{
	this->flag &= ~flag;
}

void set_peek_in_end(mound *this)
{
	set_peek(this, this->count-1);
}

void *peek_ptr(mound *this)
{
	void **ptr = peek(this);
	if(!ptr)
		return NULL;
	return *ptr;
}

void *next_ptr(mound *this)
{
	void **ptr = next(this);
	if(!ptr)
		return NULL;
	return *ptr;
}

void *last_data_ptr(mound *this)
{
	void **ptr = last_data(this);
	if(!ptr)
		return NULL;
	return *ptr;
}
