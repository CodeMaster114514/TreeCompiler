typedef struct{
    void* buffer;
    size_t len;
    size_t place;
    size_t count;
    size_t write_p;
} mound;

mound* creat_mound(size_t len);
mound* creat_mound_with_data(void* data,size_t len,size_t count);//调用后释放传入数据
void* read(mound* this,size_t p);
void push(mound* this,void* data);
void* pop(mound* this);
void set(mound* this,size_t p,void* data);
size_t get_count(mound* this);
void free_mound(mound* this);