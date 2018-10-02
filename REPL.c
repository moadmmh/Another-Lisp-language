#include "mpc.h" //Local header 
//here's my REPL Version 0.2

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
		//creating parsers
		mpc_parser_t* Nbr = mpc_new("number");
		mpc_parser_t* Operator = mpc_new("operator");
		mpc_parser_t* Expr= mpc_new("expression");
		mpc_parser_t* Lispy = mpc_new("lispy");

		//Defining them with this language.
		mpca_lang(MPCA_LANG_DEFAULT,
			"                                                       			\
				number   : /-?[0-9]+/ ;                             			\
				operator : '+' | '-' | '*' | '/' | '%' |'a''d''d' | 'm''u''l' | 's''u''b' | 'd''e''v';\
				expression     : <number> | '(' <operator> <expression>+ ')' ;  	\
				lispy    : /^/ <operator> <expression>+ /$/ ;             		\
	    		",
			Nbr,Operator,Expr,Lispy);

		puts("YMT version 0.2");
		puts("Press Ctrl+c to exit\n");
		while(1){
			char* input=readline("YMT> ");
			add_history(input);
			//Parsing the user's input 
			mpc_result_t r;
			if(mpc_parse("<stdin>",input,Lispy,&r)){ //calling mpc parse fct using Lispy parser
				//accepted and perform the Abstracted segment tree (AST)
				mpc_ast_print(r.output);
				mpc_ast_delete(r.output);
			}
			else{
				//the input doesn't start as it should be in Lispy
				mpc_err_print(r.error);
				mpc_err_delete(r.error);
			}
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
