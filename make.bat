set OBJECTS= ./build/compiler.obj ./build/process.obj ./build/lex_process.obj ./build/lexer.obj ./build/token.obj ./build/string_buffer.obj ./build/mound.obj ./build/parser.obj ./build/node.obj ./build/expressionable.obj ./build/datatype.obj ./build/scope.obj ./build/symresolver.obj ./build/array.obj
set INCLUDE= -I./src/
set CODE_LOCATION=./src/
set COMPILE_METHOR= -g

:all
    gcc %CODE_LOCATION%./compiler.c %COMPILE_METHOR% -c -o ./build/compiler.obj
    gcc %CODE_LOCATION%./lex_process.c %COMPILE_METHOR% -c -o ./build/lex_process.obj
    gcc %CODE_LOCATION%./process.c %COMPILE_METHOR% -c -o ./build/process.obj
    gcc %CODE_LOCATION%./token.c %COMPILE_METHOR% -c -o ./build/token.obj
    gcc %CODE_LOCATION%./lexer.c %COMPILE_METHOR% -c -o ./build/lexer.obj
    gcc %CODE_LOCATION%./string_buffer.c  %COMPILE_METHOR% -c -o ./build/string_buffer.obj
    gcc %CODE_LOCATION%./mound.c %INCLUDE% %COMPILE_METHOR% -c -o ./build/mound.obj
    gcc %CODE_LOCATION%./parser.c %INCLUDE% %COMPILE_METHOR% -c -o ./build/parser.obj
    gcc %CODE_LOCATION%./node.c %INCLUDE% %COMPILE_METHOR% -c -o ./build/node.obj
    gcc %CODE_LOCATION%./expressionable.c %INCLUDE% %COMPILE_METHOR% -c -o ./build/expressionable.obj
    gcc %CODE_LOCATION%./datatype.c %INCLUDE% %COMPILE_METHOR% -o ./build/datatype.obj -c
    gcc %CODE_LOCATION%./scope.c %INCLUDE% %COMPILE_METHOR% -o ./build/scope.obj -c
    gcc %CODE_LOCATION%./symresolver.c %INCLUDE% %COMPILE_METHOR% -o ./build/symresolver.obj -c
    gcc %CODE_LOCATION%./array.c %INCLUDE% %COMPILE_METHOR% -o ./build/array.obj -c
    gcc %CODE_LOCATION%./main.c %OBJECTS% %LIB_FILE% -o main
