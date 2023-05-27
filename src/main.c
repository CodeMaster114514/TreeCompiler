#include <stdio.h>
#include "compiler.h"

int main(){
//	printf("hello world!\n");
	int res = compile_file("./test.c","./test",0);
	if(res == COMPILER_FAILED_WITH_ERROR){
		printf("Compile failed or compile error\n");
	}else if(res == COMPILER_FILE_COMPILE_OK){
		printf("Everything compile fine\n");
	}else{
		printf("Unknow error occurred\n");
	}
	return 0;
}
