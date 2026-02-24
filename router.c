/* router.h */
#ifndef CPROXY_ROUTER_H
#define CPROXY_ROUTER_H

#include "config.h"

/* Trova la route con il prefix più lungo che fa match con path.
   Ritorna NULL se non trovata. */
const Route *router_match(const Config *cfg, const char *path);

#endif
