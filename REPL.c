
//here's my REPL Version 0.1

#include<stdio.h>
#include<stdlib.h>


#ifdef _WIN32
#include <string.h>

static char buffer[2048];

//fputs prints null-terminated string to any named output stream file
//puts prints a null-terminated string to stdout output stream  and ||appends a new line char|| and we new to be in a new line each time 
//fgets is used because it can get any type of input 
//stdin /stdout are files declared in stdio library we used them because fputs and fgets they need to read/write from/to a given file

//fake readline function (similar to the one in linux to read input from prompt by allowing editing it )

char* readline(char* prompt){
	fputs(prompt,stdout);
	fgets(buffer,2048,stdin);
	char* x = malloc(strlen(buffer)+1);
	strcpy(x,buffer);
	x[strlen(x)-1]= '\0';
	return x;
}

//fake add_history function (used in linux to record the history of the input so that we can use up down arrows and use it again)
void add_history(char* c){}

#else 
//in case the operating system isn't windows	
#include<editline/readline.h>
#include<editline/history.h>
#endif

int main(int argc, char** argv){
	
		puts("YMT version 0.1");
		puts("Press Ctrl+c to exit\n");
		while(1){
			char* input=readline("YMT> ");
			add_history(input);
			
			printf("Testing YMT version 0.1 : %s \n",input);
			free(input);
		}
	return 0;
}


/*
this file is made so that it can be an REPL in all operating systems
since there are some differences for example the appearing of characters while moving right left while 
entering commands in linux/mac but in windows it's fine
since we want our REPL to be able to run in all compilers we need to create some fake readline dunctions 
using preprocessor 
*/
