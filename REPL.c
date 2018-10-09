#include "mpc.h" //Local header 
//here's my REPL Version 0.5

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


//an enumeration for possible lisp value types
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR };

typedef struct lval {
  int type;
  long num;
  char* err;
  char* sym;
  int count;
  struct lval** cell;
} lval;

//new number 
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}
//new error
lval* lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}
//new symbol
lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

//new empty sexpr
lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void lval_del(lval* v) {

  switch (v->type) {
    
    case LVAL_NUM: break;
    
   
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;
    
   
    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        lval_del(v->cell[i]);
      }
      
      free(v->cell);
    break;
  }
  
  free(v);
}

lval* lval_add(lval* v, lval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

lval* lval_pop(lval* v, int i) {
  /* Find the item at "i" */
  lval* x = v->cell[i];
  
  memmove(&v->cell[i], &v->cell[i+1],
    sizeof(lval*) * (v->count-i-1));
  
  v->count--;
  
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  return x;
}

lval* lval_take(lval* v, int i) {
  lval* x = lval_pop(v, i);
  lval_del(v);
  return x;
}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    
    lval_print(v->cell[i]);
    
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval* v) {
  switch (v->type) {
    case LVAL_NUM:   printf("%li", v->num); break;
    case LVAL_ERR:   printf("Error: %s", v->err); break;
    case LVAL_SYM:   printf("%s", v->sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
  }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

lval* builtin_op(lval* a, char* op) {
  
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("Cannot operate on non-number!");
    }
  }
  
  lval* x = lval_pop(a, 0);
  
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }
  
  while (a->count > 0) {
  
    lval* y = lval_pop(a, 0);
   
    //opearations    
    if (strcmp(op, "+") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0) { x->num *= y->num; }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        lval_del(x); lval_del(y);
        x = lval_err("You can't Divide By Zero.");
        break;
      }
      x->num /= y->num;
    }
    
    lval_del(y);
  }
  
  lval_del(a);
  return x;
}

lval* lval_eval(lval* v);

lval* lval_eval_sexpr(lval* v) {
  
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(v->cell[i]);
  }
  
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }
  
  // Empty Expression 
  if (v->count == 0) { return v; }
  
  // Single Expression 
  if (v->count == 1) { return lval_take(v, 0); }
  
  //first element must be symbol
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_SYM) {
    lval_del(f); lval_del(v);
    return lval_err("Your S-expression Does not start with symbol.");
  }
  
  //perform operations by calling builtin 
  lval* result = builtin_op(v, f->sym);
  lval_del(f);
  return result;
}

lval* lval_eval(lval* v) {
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
  return v;
}

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
  
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
  
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); } 
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
  
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }
  
  return x;
}

int main(int argc, char** argv){
		//creating parsers
		mpc_parser_t* Number = mpc_new("number");
		mpc_parser_t* Symbol = mpc_new("symbol");
		mpc_parser_t* Sexpr  = mpc_new("sexpr");
		mpc_parser_t* Expr   = mpc_new("expr");
		mpc_parser_t* Lispy  = mpc_new("lispy");
		
		//defining them with the following language	
		mpca_lang(MPCA_LANG_DEFAULT,
			"                                          \
			  number : /-?[0-9]+/ ;                    \
			  symbol : '+' | '-' | '*' | '/' ;         \
			  sexpr  : '(' <expr>* ')' ;               \
			  expr   : <number> | <symbol> | <sexpr> ; \
			  lispy  : /^/ <expr>* /$/ ;               \
			",
		Number, Symbol, Sexpr, Expr, Lispy);

		puts("YMT version 0.5");
		puts("Done By @moadmmh");
		puts("Press Ctrl+c to exit\n");
		while(1){
			char* input=readline("YMT> ");
			add_history(input);
			//Parsing the user's input 
			mpc_result_t r;
			if(mpc_parse("<stdin>",input,Lispy,&r)){ //calling mpc parse fct using Lispy parser
				//accepted and perform the Abstracted segment tree (AST)
				lval* x=lval_eval(lval_read(r.output));
				lval_println(x);
				lval_del(x);
				mpc_ast_delete(r.output);
			}
			else{
				//the input doesn't start as it should be in Lispy
				mpc_err_print(r.error);
				mpc_err_delete(r.error);
			}
			free(input);
		}
				mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy); //deleting the parser

	return 0;
}

/*
this file is made so that it can be an REPL in all operating systems
since there are some differences for example the appearing of characters while moving right left while 
entering commands in linux/mac but in windows it's fine
since we want our REPL to be able to run in all compilers we need to create some fake readline dunctions 
using preprocessor 
*/
