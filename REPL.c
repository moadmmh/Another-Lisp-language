#include "mpc.h" //Local header 
//here's my REPL Version 0.4

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

//an enumeration for possible errors
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

//an enumeration for possible lisp value types
enum{LVAL_NUM, LVAL_ERR };

//the new structure lisp value (lval)
typedef struct {
	int type;
	long num;
	int err;
}lval;

//a number type lval
lval lval_num(long x){
	lval v;
	v.type=LVAL_NUM;
	v.num=x;
	return v;
}

//an error type lval
lval lval_err (int x){
	lval v;
	v.type=LVAL_ERR;
	v.err=x;
	return v;
}

//printing the lval 
void lval_print(lval v){
	switch(v.type){
		case LVAL_NUM: 
			printf("%li",v.num);
			break;
		case LVAL_ERR:
			if(v.err==LERR_DIV_ZERO) {
				printf("Error: Division by Zero !!");
			}
			else if(v.err== LERR_BAD_OP) {
				printf("Error: Invalid Operator !!");
			}
			else if(v.err ==LERR_BAD_NUM) {
				printf("Error :Invalid Number !!");
			}
			break;
	}
}

// printing the lval followed by  a new line char 
void lval_println(lval v){lval_print(v); putchar('\n');}

//a function to check which operation to perform
lval eval_op(lval x,char* op,lval y){
	
	//if the error in the numbers 
	if(x.type== LVAL_ERR) return x;
	if(y.type== LVAL_ERR) return y;
	
	//evaluating operations 
	if (strcmp(op, "+") == 0) { return lval_num(y.num + x.num); }
	if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
	if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
	if (strcmp(op, "%") == 0) { return lval_num(x.num % y.num); }
	if (strcmp(op, "/") == 0) { return (y.num==0 ? lval_err(LERR_DIV_ZERO): lval_num(x.num / y.num)); }
	
	//if the error belongs to the type of the operation entered by the user 
	return lval_err(LERR_BAD_OP);
}

//getting the tree elements to evaluate the operations written 
lval eval(mpc_ast_t* t) {
	
	if(strstr(t->tag,"number")){
		errno=0;
		long x=strtol(t->contents, NULL,10);
		return errno!=ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}
	
	char* op=t->children[1]->contents;
	lval x = eval(t->children[2]);
	
	int idx=3;
	while(strstr(t->children[idx]->tag,"expression")){
		x= eval_op(x,op,eval(t->children[idx]));
		idx++;
	}
	 
	return x;
}

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
				operator : '+' | '-' | '*' | '/' | '%' |'^'|\
			  	\"min\" | \"max\";\
				expression     : <number> | '(' <operator> <expression>+ ')' ;  	\
				lispy    : /^/ <operator> <expression>+ /$/ ;             		\
	    		",
			Nbr,Operator,Expr,Lispy);

		puts("YMT version 0.4");
		puts("Done By @moadmmh");
		puts("Press Ctrl+c to exit\n");
		while(1){
			char* input=readline("YMT> ");
			add_history(input);
			//Parsing the user's input 
			mpc_result_t r;
			if(mpc_parse("<stdin>",input,Lispy,&r)){ //calling mpc parse fct using Lispy parser
				//accepted and perform the Abstracted segment tree (AST)
				lval result=eval(r.output);
				lval_println(result);
				mpc_ast_delete(r.output);
			}
			else{
				//the input doesn't start as it should be in Lispy
				mpc_err_print(r.error);
				mpc_err_delete(r.error);
			}
			free(input);
		}
		mpc_cleanup(4, Nbr, Operator, Expr, Lispy); //deleting the parser
	return 0;
}


/*
this file is made so that it can be an REPL in all operating systems
since there are some differences for example the appearing of characters while moving right left while 
entering commands in linux/mac but in windows it's fine
since we want our REPL to be able to run in all compilers we need to create some fake readline dunctions 
using preprocessor 
*/
