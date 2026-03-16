/** @file jsondom.c */

#include "JParser.h"
#include "jsondom.h"
#include <string.h>  // for strlen
#include <assert.h>

#define EXPORT __attribute((visibility("default")))

/**
 * @brief Parse the file into new_tree.
 * @param fh        handle to an open file
 * @param new_tree  address of pointer to which the result will be written
 * @return True for success, false for failure
 */
EXPORT bool jd_parse_file(int fh, jd_Node **new_tree, jd_ParseError *pe)
{
   *new_tree = NULL;

   jd_Node *node = NULL;
   bool retval = JParser(fh, NULL, &node, 0, NULL, pe);
   if (retval)
   {
      if (confirm_no_further_file_content(fh))
         *new_tree = (jd_Node*)node;
      else
      {
         report_parse_error(pe, fh,
                            "forbidden characters following singleton root object");
         jd_Node_destroy(&node);
         retval = false;
      }
   }

   return retval;
}

/**
 * @brief Free memory in the memory tree
 * @param node   Pointer to node to be destroyed
 */
EXPORT void jd_destroy(jd_Node **node)
{
   jd_Node_destroy(node);
}

EXPORT bool js_search(jd_NodeSet *ns, const jd_Node *origin, const char *query)
{
   return true;
}

EXPORT void jd_serialize(int jd_out, const jd_Node *node, int indent)
{
   jd_Node_serialize(jd_out, node, indent);
}




