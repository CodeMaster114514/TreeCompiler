#ifndef MOUND_H
#define MOUND_H

typedef struct
{
    void *buffer;
    size_t len;
    size_t place;
    size_t count;
    int peek;
    int write_p;
    int flags;
} mound;

enum
{
	MOUND_FLAG_PEEK_DECREMENT = 0b00000001
};

mound *creat_mound(size_t len);
mound *creat_mound_with_data(void *data, size_t len, size_t count); // 调用后释放传入数据
void *read(mound *this, size_t p);
void push(mound *this, void *data);
void pop(mound *this);
void set_peek(mound *this, size_t p);
size_t get_count(mound *this);
void free_mound(mound *this);
void *peek(mound *this);
void *peek_ptr(mound *this);
void *next(mound *this);
void *next_ptr(mound *this);
void *last_data(mound *this);
void *last_data_ptr(mound *this);
bool isEmpty(mound *this);
void set_flag(mound *this, int flags);
void unset_flag(mound *this, int flags);
void set_peek_in_end(mound *this);

#endif
