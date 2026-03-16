/** @file */

#include "JQuery.h"
#include "sarray.h"
#include <assert.h>
#include <ctype.h>
#include <alloca.h>

static char STEP_DELIM = '/';

void query_string_walker(const char *query, put_string_func func, void *data)
{
   const char *str = query;
   const char *end = str;

   int nest_level = 0;

   while (*end)
   {
      if (*end == '[' && *(end-1)!='\\')
         ++nest_level;
      else if (*end == ']' && *(end-1)!='\\')
      {
         --nest_level;
         assert(nest_level>=0);
      }
      else if (*end == STEP_DELIM && nest_level == 0)
      {
         (*func)(str, end-str, data);
         str = end+1;
      }

      ++end;
   }

   // Save any remainder
   if (end > str)
      (*func)(str, end-str, data);
}

void step_string_walker(const char *step, put_string_func func, void *data)
{
   const char *ptr = step;
   const char *end = step;

   int nest_level = 0;

   while (*end)
   {
      if (*end == '[' && *(end-1)!='\\')
      {
         // If we're entering a substep with some text on, save it
         if (nest_level==0 && end > ptr)
         {
            (*func)(ptr, end-ptr, data);

            // reset for continuing search
            ptr = end+1;
            end = ptr;
         }

         ++nest_level;
      }
      else if (*end == ']' && *(end-1)!='\\')
      {
         --nest_level;
         assert(nest_level>=0);

         // save step at this scope
         if (nest_level == 0)
         {
            assert(*ptr == '[' && *end == ']');
            (*func)(ptr+1, end-ptr-1, data);

            // reset for continuing search
            ptr = end + 1;
         }
      }

      ++end;
   }

   // Save any appropriate remainder
   if (end > ptr && nest_level==0)
      (*func)(ptr, end-ptr, data);
}

#include <stdio.h>
bool jd_Node_matches_step(const jd_Node *node, const char *step)
{
   int len = sarray_measure(step, step_string_walker);
   if (len)
   {
      char *buffer = (char*)alloca(len);
      if (buffer)
      {
         sarray_handle sh = { 0 };
         sarray_build(&sh, step, step_string_walker, buffer, len);

         if (sh.count > 0)
         {
            printf("step '%s' parsed to:\n", step);
            for (int i=0; i<sh.count; ++i)
               printf("   \"%s\"\n", sh.strings[i]);
         }
         else
            printf("not steps found.\n");
      }
   }

   return true;
}

bool jd_Node_search(jd_Node *origin, jd_search_hit report, sarray_handle *qsteps, int step_index)
{
   const char *step = sarray_element_by_index(qsteps, step_index);
   if (step)
   {
      jd_Node *ptr = origin->firstChild;
      while (ptr)
      {
         if (jd_Node_matches_step(ptr, step))
            (*report)(ptr, qsteps, step_index);
      }
   }

   return true;
}

#ifdef JQUERY_MAIN
#include <stdio.h>
#include "sarray.c"

void print_array(const sarray_handle *sa)
{
   const char **str = sa->strings;
   const char **end = str + sa->count;

   while (str < end)
   {
      printf("\"%s\"\n", *str);
      ++str;
   }
}

void parse_generic(const char *str, walker_func func)
{
   int len = sarray_measure(str, func);
   if (len)
   {
      char *buffer = (char*)alloca(len);

      sarray_handle sa = { 0 };
      sarray_build(&sa, str, func, buffer, len);

      printf("\nfor string '%s'\n", str);
      print_array(&sa);
   }
}

void parse_query(const char *str)
{
   parse_generic(str, query_string_walker);
}

void parse_step(const char *str)
{
   parse_generic(str, step_string_walker);
}

int main(int argc, const char **argv)
{
   const char **ptr= argv;
   const char **end = argv + argc;

   // Skip command name
   ++ptr;

   while (ptr < end)
   {
      if (**ptr == '[')
         parse_step(*ptr);
      else
         parse_query(*ptr);
      ++ptr;
   }

   return 0;
}

#endif

/* Local Variables:               */
/* compile-command: "gcc         \*/
/* -Wall -Werror -std=c99 -ggdb  \*/
/* -DJQUERY_MAIN                 \*/
/* -fsanitize=address            \*/
/* -o JQuery JQuery.c"           \*/
/* End:                           */
