#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mound.h"

mound* creat_mound(size_t len)
{
    mound* ret = calloc(1,sizeof(mound));
    ret->buffer = calloc(20,len);
    ret->len = len;
    ret->place = 20;
    ret->count = 0;
    ret->write_p = 0;
    return ret;
}

mound* creat_mound_with_data(void* data,size_t len,size_t count){
    mound* ret = calloc(1,sizeof(mound));
    ret->buffer = calloc(count,len);
    ret->len = len;
    ret->count = count;
    ret->write_p = 0;
    return ret;
}

void* read(mound* this,size_t p)
{
    void* ret = calloc(1,this->len);
    memcpy(ret,this->buffer + this->len*p,this->len);
    return ret;
}

void push(mound* this,void* data){
    size_t write = this->write_p++;
    if(this->write_p >= this->count){
        this->buffer = realloc(this->buffer,this->write_p*this->len);
        ++this->count;
    }
    memcpy(this->buffer + write*this->len,data,this->len);
}

void* pop(mound* this){
    void* ret = calloc(1,this->len);
    memcpy(ret,this->buffer + (this->write_p - 1) * this->len,this->len);
    --this->write_p;
    return ret;
}

void set(mound* this,size_t p,void* data){
    memcpy(this->buffer + p*this->len,data,this->len);
}

size_t get_count(mound* this){
    return this->count;
}

void free_mound(mound* this){
    free(this->buffer);
    free(this);
}