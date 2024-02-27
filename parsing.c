#include "mpc.h"
#include <math.h>
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

// lval struct
typedef struct {
  int type;
  long num;
  int err;
} lval;

// Enum of Possible lval types
enum { LVAL_NUM, LVAL_ERR };

// Enum of possible err types
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

// lval number constructor
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

// lval err constructor
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

// Print an lval
void lval_print(lval v) {
  switch (v.type) {
  case LVAL_NUM:
    printf("%li", v.num);
  case LVAL_ERR:
    if (v.err == LERR_DIV_ZERO) {
      printf("Error: Division by Zero");
    }
    if (v.err == LERR_BAD_OP) {
      printf("Error: Invalid Operator");
    }
    if (v.err == LERR_BAD_NUM) {
      printf("Error: Invalid Number");
    }
    break;
  }
}

// lval println
void lval_println(lval v) {
  lval_print(v);
  putchar('\n');
}

// Function prototypes
int number_of_nodes(mpc_ast_t *t);
int number_of_leaves(mpc_ast_t *t);
int number_of_branches(mpc_ast_t *t);
int most_num_of_nodes(mpc_ast_t *t);
lval eval(mpc_ast_t *t);
lval eval_op(lval x, char *op, lval y);

int main(int argc, char **argv) {
  // Parsers
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Lispy = mpc_new("lispy");

  // Parsers definition
  mpca_lang(MPCA_LANG_DEFAULT, "                                    \
number : /-?[0-9]+/ ;                             \
operator : '+' | '-' | '*' | '/' | '%' | '^';               \
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
      // mpc_ast_print(r.output);

      // Print number of nodes
      // int nodes_num = number_of_nodes(r.output);
      // printf("Number_of_Nodes: %i\n", nodes_num);

      // Print number of leaves
      // int leaves_num = number_of_leaves(r.output);
      // printf("Number of leaves: %i\n", leaves_num);

      // Print number of branches
      // int branches_num = number_of_branches(r.output);
      // printf("Number of branches: %i\n", branches_num);

      // Print highest number of children
      // int most_children = most_num_of_nodes(r.output);
      // printf("Most number of children: %i\n", most_children);

      // Print evaluation result
      lval result = eval(r.output);
      lval_println(result);

      // Delete AST
      mpc_ast_delete(r.output);
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
    for (int i = 0; i < t->children_num; i++) {
      total += number_of_nodes(t->children[i]);
    }
    return total;
  }
  return 0;
}

int number_of_leaves(mpc_ast_t *t) {
  if (t->children_num == 0) {
    if (strstr(t->tag, "number") || strstr(t->tag, "operator")) {
      printf("Cuurent tag: %s\n", t->tag);
      return 1;
    }
  }
  if (t->children_num >= 1) {
    int total = 0;
    for (int i = 0; i < t->children_num; i++) {
      total += number_of_leaves(t->children[i]);
    }
    return total;
  }
  return 0;
}

int number_of_branches(mpc_ast_t *t) {
  if (t->children_num == 0) {
    if (strstr(t->tag, "operator")) {
      printf("Cuurent tag: %s\n", t->tag);
      return 1;
    }
  }
  if (t->children_num >= 1) {
    int total = 0;
    for (int i = 0; i < t->children_num; i++) {
      total += number_of_branches(t->children[i]);
    }
    return total;
  }
  return 0;
}

int most_num_of_nodes(mpc_ast_t *t) {
  if (t->children_num == 0) {
    return 0;
  }
  if (t->children_num >= 1) {
    int highest = 0;
    for (int i = 0; i < t->children_num; i++) {
      if (t->children_num > highest) {
        highest = t->children_num;
      }
      most_num_of_nodes(t->children[i]);
    }
    return highest;
  }
  return 0;
}

lval eval(mpc_ast_t *t) {
  // check err in conversion
  if (strstr(t->tag, "number")) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  char *op = t->children[1]->contents;

  lval x = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  return x;
}

lval eval_op(lval x, char *op, lval y) {
  // if either values are errors return it
  if (x.type == LVAL_ERR) {
    return x;
  }
  if (y.type == LVAL_ERR) {
    return y;
  }

  // Otherwise proceed with math op
  if (strcmp(op, "+") == 0) {
    return lval_num(x.num + y.num);
  }
  if (strcmp(op, "-") == 0) {
    return lval_num(x.num - y.num);
  }
  if (strcmp(op, "*") == 0) {
    return lval_num(x.num * y.num);
  }
  if (strcmp(op, "/") == 0) {
    return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
  }
  return lval_err(LERR_BAD_OP);
}
