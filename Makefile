METHOR= releas
OBJECTS= ./build/compiler.o ./build/process.o ./build/lex_process.o ./build/lexer.o ./build/token.o ./build/mound.o
INCLUDE= -I./src/
CODE_LOCATION=./src/
LIB_FILE= ./build/libparentheses_buffer.so
LIB= -L./build -lparentheses_buffer
LIB_INSTALL= /usr/lib/
MAIN_INSTALL= /usr/bin/

ifeq ($(METHOR),debug)
	COMPILE_METHOR= -g
else ifeq ($(METHOR),releas)
	COMPILE_METHOR= 
else
	COMPILE_METHOR= drvrvgg#乱码使其报错
endif

main: $(OBJECTS) $(LIB_FILE)
	gcc $(CODE_LOCATION)main.c ${INCLUDE} ${OBJECTS} $(COMPILE_METHOR) $(LIB) -o main

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

./build/mound.o: $(CODE_LOCATION)./mound.c
	gcc $(CODE_LOCATION)./mound.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/mound.o

./build/libparentheses_buffer.so: $(CODE_LOCATION)./parentheses_buffer.c
	gcc $(CODE_LOCATION)./parentheses_buffer.c $(INCLUDE) $(COMPILE_METHOR) -fPIC -shared -o ./build/libparentheses_buffer.so

clear:
	rm ${OBJECTS} ${LIB_FILE} -rf
	rm ./main

install: main
	cp main $(MAIN_INSTALL)
	cp $(LIB_FILE) $(LIB_INSTALL)
