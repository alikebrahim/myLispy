#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>

// Preprocessor directive to handle windows/unix compilation
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

// Add fake readline for windows compilation
char *readline(char *prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char *cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}

// Add fake add_history for windows compilation
void add_history(char *unused) {}

// Else include libedit/editline for unix
#else
#include <editline/history.h>
#include <editline/readline.h>
#endif
int number_of_nodes(mpc_ast_t *t);

int main(int argc, char **argv) {
  // Parsers
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Lispy = mpc_new("lispy");

  // Parsers definition
  mpca_lang(MPCA_LANG_DEFAULT, "                                    \
number : /-?[0-9]+/ ;                             \
operator : '+' | '-' | '*' | '/' ;               \
expr : <number> | '(' <operator> <expr>+ ')' ;   \
lispy : /^/ <operator> <expr>+ /$/ ;             \
                               ",
            Number, Operator, Expr, Lispy);

  puts("Lispy Version 0.0.1\n");
  puts("Press Ctrl+c to Exit\n");
  while (1) {
    char *input = readline("lispy> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      // On Success print AST
      mpc_ast_print(r.output);
      mpc_ast_delete(r.output);
      int nodes_num = number_of_nodes(r.output);
      printf("Number_of_Nodes: %i", nodes_num);
    } else {
      // Otherwise print error
      mpc_err_print(r.output);
      mpc_err_delete(r.output);
    }
    free(input);
  }

  // Delete language
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}

int number_of_nodes(mpc_ast_t *t) {
  if (t->children_num == 0) {
    return 1;
  }
  if (t->children_num >= 1) {
    int total = 1;
    for (int i = 0; i <= t->children_num; i++) {
      total += number_of_nodes(t->children[i]);
    }
    return total;
  }
  return 0;
}
