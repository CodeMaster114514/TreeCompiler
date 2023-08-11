METHOR= debug
OBJECTS= ./build/compiler.o ./build/process.o ./build/lex_process.o ./build/lexer.o ./build/token.o ./build/mound.o ./build/parser.o ./build/string_buffer.o ./build/node.o ./build/expressionable.o ./build/datatype.o ./build/scope.o ./build/symresolver.o ./build/array.o ./build/helper.o
INCLUDE= -I./src/
CODE_LOCATION=./src/
#LIB_FILE= ./build/libparentheses_buffer.so
#LIB= -L./build -lparentheses_buffer
#LIB_INSTALL= /usr/lib/
MAIN_INSTALL= /usr/bin/

COMPILER= gcc

ifeq ($(METHOR),debug)
	COMPILE_METHOR= -g
else ifeq ($(METHOR),releas)
	COMPILE_METHOR= 
else
	COMPILE_METHOR= drvrvgg#乱码使其报错
endif

main: $(OBJECTS) $(LIB_FILE) $(CODE_LOCATION)./main.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)main.c ${INCLUDE} $(COMPILE_METHOR) ${OBJECTS} -o main

./build/compiler.o: $(CODE_LOCATION)./compiler.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./compiler.c ${INCLUDE} $(COMPILE_METHOR) -o ./build/compiler.o -c

./build/process.o: $(CODE_LOCATION)./process.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./process.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/process.o -c

./build/lex_process.o: $(CODE_LOCATION)./lex_process.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./lex_process.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/lex_process.o -c

./build/lexer.o: $(CODE_LOCATION)./lexer.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./lexer.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/lexer.o -c

./build/token.o: $(CODE_LOCATION)./token.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./token.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/token.o -c

./build/mound.o: $(CODE_LOCATION)./mound.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./mound.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/mound.o -c

./build/parser.o: $(CODE_LOCATION)./parser.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./parser.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/parser.o -c

./build/string_buffer.o: $(CODE_LOCATION)./string_buffer.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./string_buffer.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/string_buffer.o -c

./build/node.o: $(CODE_LOCATION)./node.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./node.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/node.o -c

./build/expressionable.o: $(CODE_LOCATION)./expressionable.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./expressionable.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/expressionable.o -c

./build/datatype.o: $(CODE_LOCATION)./datatype.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./datatype.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/datatype.o -c

./build/scope.o: $(CODE_LOCATION)./scope.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./scope.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/scope.o -c

./build/symresolver.o: $(CODE_LOCATION)./symresolver.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./symresolver.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/symresolver.o -c

./build/array.o: $(CODE_LOCATION)./array.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./array.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/array.o -c

./build/helper.o: $(CODE_LOCATION)./helper.c $(CODE_LOCATION)./compiler.h
	$(COMPILER) $(CODE_LOCATION)./helper.c $(INCLUDE) $(COMPILE_METHOR) -o ./build/helper.o -c


clean:
	rm ${OBJECTS} ${LIB_FILE} -rf
	rm ./main

install: main
	rm $(MAIN_INSTALL)tcc -rf
	ln main $(MAIN_INSTALL)tcc
#	cp $(LIB_FILE) $(LIB_INSTALL)
