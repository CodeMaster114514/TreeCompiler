set OBJECTS= ./build/compiler.obj ./build/process.obj ./build/lex_process.obj ./build/lexer.obj ./build/token.obj ./build/parentheses_buffer.obj ./build/mound.obj ./build/parser.obj ./build/node.obj
set INCLUDE= -I./src/
set CODE_LOCATION=./src/
set COMPILE_METHOR= -g

:all
    gcc %CODE_LOCATION%./compiler.c %COMPILE_METHOR% -c -o ./build/compiler.obj
    gcc %CODE_LOCATION%./lex_process.c %COMPILE_METHOR% -c -o ./build/lex_process.obj
    gcc %CODE_LOCATION%./process.c %COMPILE_METHOR% -c -o ./build/process.obj
    gcc %CODE_LOCATION%./token.c %COMPILE_METHOR% -c -o ./build/token.obj
    gcc %CODE_LOCATION%./lexer.c %COMPILE_METHOR% -c -o ./build/lexer.obj
    gcc %CODE_LOCATION%./parentheses_buffer.c  %COMPILE_METHOR% -c -o ./build/parentheses_buffer.obj
    gcc %CODE_LOCATION%./mound.c %INCLUDE% %COMPILE_METHOR% -c -o ./build/mound.obj
    gcc %CODE_LOCATION%./parser.c %INCLUDE% %COMPILE_METHOR% -c -o ./build/parser.obj
    gcc %CODE_LOCATION%./node.c %INCLUDE% %COMPILE_METHOR% -c -o ./build/node.obj
    gcc %CODE_LOCATION%./main.c %OBJECTS% %LIB_FILE%  %COMPILE_METHOR% -o main
