#include <string.h>
#include "scheme.h"

static value_t install_op(context_p ctxt, value_t env, char *name, native_proc_fn fn) {
  value_t sym = make_symbol(ctxt, name, strlen(name));
  value_t ptr = make_native_proc(ctxt, fn);
  
  return environment_set(ctxt, env, sym, ptr);
}

static value_t nullp_proc(context_p ctxt, value_t args, value_t env) {
  return is_nil(ctxt, eval(ctxt, cons_car(ctxt, args), &env)) ? vtrue : vfalse;
}

static value_t boolp_proc(context_p ctxt, value_t args, value_t env) {
  return is_boolean(ctxt, eval(ctxt, cons_car(ctxt, args), &env)) ? vtrue : vfalse;
}

static value_t symbolp_proc(context_p ctxt, value_t args, value_t env) {
  return is_symbol(ctxt, eval(ctxt, cons_car(ctxt, args), &env)) ? vtrue : vfalse;
}

static value_t integerp_proc(context_p ctxt, value_t args, value_t env) {
  return is_integer(ctxt, eval(ctxt, cons_car(ctxt, args), &env)) ? vtrue : vfalse;
}

static value_t floatp_proc(context_p ctxt, value_t args, value_t env) {
  return is_float(ctxt, eval(ctxt, cons_car(ctxt, args), &env)) ? vtrue : vfalse;
}

static value_t charp_proc(context_p ctxt, value_t args, value_t env) {
  return is_character(ctxt, eval(ctxt, cons_car(ctxt, args), &env)) ? vtrue : vfalse;
}

static value_t stringp_proc(context_p ctxt, value_t args, value_t env) {
  return is_string(ctxt, eval(ctxt, cons_car(ctxt, args), &env)) ? vtrue : vfalse;
}

static value_t consp_proc(context_p ctxt, value_t args, value_t env) {
  return is_cons(ctxt, eval(ctxt, cons_car(ctxt, args), &env)) ? vtrue : vfalse;
}

static value_t procp_proc(context_p ctxt, value_t args, value_t env) {
  return is_proc(ctxt, eval(ctxt, cons_car(ctxt, args), &env)) ? vtrue : vfalse;
}

static value_t to_integer_proc(context_p ctxt, value_t args, value_t env) {
  return to_integer(ctxt, eval(ctxt, cons_car(ctxt, args), &env));
}

static value_t to_character_proc(context_p ctxt, value_t args, value_t env) {
  return to_character(ctxt, eval(ctxt, cons_car(ctxt, args), &env));
}

static value_t to_string_proc(context_p ctxt, value_t args, value_t env) {
  return to_string(ctxt, eval(ctxt, cons_car(ctxt, args), &env));
}

static value_t to_symbol_proc(context_p ctxt, value_t args, value_t env) {
  return to_symbol(ctxt, eval(ctxt, cons_car(ctxt, args), &env));
}

value_t enhance_native_environment(context_p ctxt) {
  value_t env = ctxt->curr_env;

  env = install_op(ctxt, env, "null?",          &nullp_proc);
  env = install_op(ctxt, env, "bool?",          &boolp_proc);
  env = install_op(ctxt, env, "symbol?",        &symbolp_proc);
  env = install_op(ctxt, env, "integer?",       &integerp_proc);
  env = install_op(ctxt, env, "float?",         &floatp_proc);
  env = install_op(ctxt, env, "char?",          &charp_proc);
  env = install_op(ctxt, env, "string?",        &stringp_proc);
  env = install_op(ctxt, env, "pair?",          &consp_proc);
  env = install_op(ctxt, env, "procedure?",     &procp_proc);

  env = install_op(ctxt, env, "char->integer",  &to_integer_proc);
  env = install_op(ctxt, env, "integer->char",  &to_character_proc);
  env = install_op(ctxt, env, "number->string", &to_string_proc);
  env = install_op(ctxt, env, "string->number", &to_integer_proc);
  env = install_op(ctxt, env, "symbol->string", &to_string_proc);
  env = install_op(ctxt, env, "string->symbol", &to_symbol_proc);

  env = install_op(ctxt, env, "+",              &nullp_proc);
  env = install_op(ctxt, env, "-",              &nullp_proc);
  env = install_op(ctxt, env, "*",              &nullp_proc);
  env = install_op(ctxt, env, "quotient",       &nullp_proc);
  env = install_op(ctxt, env, "remainder",      &nullp_proc);
  env = install_op(ctxt, env, "=",              &nullp_proc);
  env = install_op(ctxt, env, "<",              &nullp_proc);
  env = install_op(ctxt, env, ">",              &nullp_proc);
  env = install_op(ctxt, env, ">=",             &nullp_proc);
  env = install_op(ctxt, env, "<=",             &nullp_proc);

  env = install_op(ctxt, env, "cons",           &nullp_proc);
  env = install_op(ctxt, env, "car",            &nullp_proc);
  env = install_op(ctxt, env, "cdr",            &nullp_proc);
  env = install_op(ctxt, env, "set-car!",       &nullp_proc);
  env = install_op(ctxt, env, "set-cdr!",       &nullp_proc);
  env = install_op(ctxt, env, "list",           &nullp_proc);

  env = install_op(ctxt, env, "eq?",            &nullp_proc);
  return env;
}
