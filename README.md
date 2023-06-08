# TreeCompiler
The HongMuOS’s C compiler

##更新日志(update the log)
```
Token* make_symbol_token()
{
	char c = nextc();
	if(c == ')')
	{
		lex_finish_expression();
	}
	return token_creat(&(Token)
	{
		.type = TOKEN_TYPE_SYMBOL,
		.cval = c
	});
}
```
to
```
Token* make_symbol_token()
{
	char c = nextc();
	if(c == ')')
	{
		lex_finish_expression();
	}
	return token_creat(&(Token)
	{
		.type = TOKEN_TYPE_SYMBOL,
		.cval = c
	});
}
```
---
修复多行注释解析的bug
---
添加在括号内的内容写入缓冲区
