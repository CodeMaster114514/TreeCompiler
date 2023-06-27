typedef struct{
    void* buffer;
    size_t len;
    size_t count;
    size_t write_p;
} mound;

mound* creat_mound(size_t len);
void* read(mound* this,size_t p);
void push(mound* this,void* data);
void* pop(mound* this);
void set(mound* this,size_t p,void* data);