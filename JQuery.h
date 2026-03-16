/** @file */
#ifndef JQUERY_H
#define JQUERY_H

#include "jd_Node.h"
#include "sarray.h"

typedef void (*jd_search_hit)(jd_Node *node, sarray_handle *qsteps, int step_index);

bool jd_Node_search(jd_Node *origin, jd_search_hit report, sarray_handle *qsteps, int step_index);

bool jd_search(jd_NodeSet *ns, const jd_Node *origin, const char *query);

#endif
