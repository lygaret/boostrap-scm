#include "scheme.h"

value_t eval(context_p ctxt, value_t v) {
 tailcall:

  /* atoms */

  if (is_nil(ctxt, v)) {
    return v;
  }

  if (is_atom(ctxt, v)) {
    if (is_symbol(ctxt, v)) {
        return environment_get(ctxt, v);
    }

    if (is_integer(ctxt, v)) { return v; }
    if (is_float(ctxt, v))   { return v; }
    if (is_double(ctxt, v))  { return v; }
    if (is_string(ctxt, v))  { return v; }
    if (is_boolean(ctxt, v)) { return v; }

    fprintf(stderr, "this object isn't handled currently\n");
    return vnil;
  }

  /* pairs */
  else {
    value_t car = cons_car(ctxt, v);

    if (equality_exact(ctxt, symquote, car)) {
      return cons_cadr(ctxt, v);
    }

    if (equality_exact(ctxt, symif, car)) {
      value_t test = eval(ctxt, cons_cadr(ctxt, v));

      v = is_truthy(ctxt, test)
        ? cons_caddr(ctxt, v)
        : cons_cadddr(ctxt, v);
      goto tailcall;
    }

    if (equality_exact(ctxt, symdefine, car)) {
      value_t val = eval(ctxt, cons_caddr(ctxt, v));
      environment_set(ctxt, cons_cadr(ctxt, v), val);
      return vnil;
    }

    if (equality_exact(ctxt, symlambda, car)) {
    }
    
    return vnil;
  }

}
