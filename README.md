# TreeCompiler
The HongMuOS’s C compiler

## 更新日志(update the log)

创建函数lex_token_build_for_string

调用示例

```C
lex_process* process = lex_token_build_for_string(compile_process,"int a = 0xb8000;")//compile_process已提前创建
```

将parentheses_buffer.c编译后的内容移动到动态链接库中
