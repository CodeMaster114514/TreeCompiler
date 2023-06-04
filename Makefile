METHOR= debug
OBJECTS= ./build/compiler.o ./build/process.o ./build/lex_process.o ./build/lexer.o ./build/token.o ./build/parentheses_buffer.o
INCLUDE= -I./src/
CODE_LOCATION=./src/

ifeq ($(METHOR),debug)
	COMPILE_METHOR= -g
else ifeq ($(METHOR),releas)
	COMPILE_METHOR= 
else
	COMPILE_METHOR= drvrvgg#乱码使其报错
endif

all: $(OBJECTS)
	gcc $(CODE_LOCATION)main.c ${INCLUDE} ${OBJECTS} $(COMPILE_METHOR) -o main

./build/compiler.o: $(CODE_LOCATION)./compiler.c
	gcc $(CODE_LOCATION)./compiler.c ${INCLUDE} $(COMPILE_METHOR) -o ./build/compiler.o -c

./build/process.o: $(CODE_LOCATION)./process.c
	gcc $(CODE_LOCATION)./process.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/process.o -c

./build/lex_process.o: $(CODE_LOCATION)./lex_process.c
	gcc $(CODE_LOCATION)./lex_process.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/lex_process.o -c

./build/lexer.o: $(CODE_LOCATION)./lexer.c
	gcc $(CODE_LOCATION)./lexer.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/lexer.o -c

./build/token.o: $(CODE_LOCATION)./token.c
	gcc $(CODE_LOCATION)./token.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/token.o -c

./build/parentheses_buffer.o: $(CODE_LOCATION)./parentheses_buffer.c
	gcc $(CODE_LOCATION)./parentheses_buffer.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/parentheses_buffer.o -c

clear:
	rm ${OBJECTS} -rf
	rm ./main
