#include "scheme.h"

value_t eval(context_p ctxt, value_t v) {
 /* tailcall: */

  if (is_nil(ctxt, v)) {
    return v;
  }

  if (is_integer(ctxt, v)) {
    return v;
  }

  if (is_float(ctxt, v)) {
    return v;
  }

  if (is_double(ctxt, v)) {
    return v;
  }

  if (is_string(ctxt, v)) {
    return v;
  }

  if (is_boolean(ctxt, v)) {
    return v;
  }

  if (is_symbol(ctxt, v)) {
    return environment_get(ctxt, v);
  }

  if (is_cons(ctxt, v)) {
    return vnil;
  }

  fprintf(stderr, "this object isn't handled currently\n");
  return vnil;
}
