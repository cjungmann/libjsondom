/** @file jsondom.c */

#include "JNode.h"
#include "JParser.h"
#include "jsondom.h"

#define EXPORT __attribute((visibility("default")))

/**
 * @brief Array of names for jd_id_type
 */
const char *TypeNames[] = {
   "null",
   "true",
   "false",
   "string",
   "integer",
   "float",
   "array",
   "property",
   "object"
};

/**
 * @brief Parse the file into new_tree.
 * @param fh        handle to an open file
 * @param new_tree  address of pointer to which the result will be written
 * @return True for success, false for failure
 */
EXPORT bool jd_parse_file(int fh, jd_Node **new_tree)
{
   JNode *node = NULL;
   bool retval = JParser(fh, NULL, &node, 0, NULL);
   if (retval)
      *new_tree = (jd_Node*)node;
   else
      *new_tree = NULL;

   return retval;
}

/**
 * @brief Free memory in the memory tree
 * @param node   Pointer to node to be destroyed
 */
EXPORT void jd_destroy(jd_Node *node)
{
   JNode_destroy((JNode**)&node);
}

/**
 * @brief returns parent of jd_Node
 * @param node   node from which to get the relative
 * @return pointer to relative, NULL if node or relative is NULL
 */
EXPORT jd_Node* parent(jd_Node *node)
{
   JNode *jnode = (JNode*)node;
   if (node && jnode->parent)
      return jnode->parent;
   else
      return NULL;
}

/**
 * @brief returns nextSibling of jd_Node
 * @param node   node from which to get the relative
 * @return pointer to relative, NULL if node or relative is NULL
 */
EXPORT jd_Node* nextSibling(jd_Node *node)
{
   JNode *jnode = (JNode*)node;
   if (node && jnode->nextSibling)
      return jnode->nextSibling;
   else
      return NULL;
}

/**
 * @brief returns prevSibling of jd_Node
 * @param node   node from which to get the relative
 * @return pointer to relative, NULL if node or relative is NULL
 */
EXPORT jd_Node* prevSibling(jd_Node *node)
{
   JNode *jnode = (JNode*)node;
   if (node && jnode->prevSibling)
      return jnode->prevSibling;
   else
      return NULL;
}

/**
 * @brief returns firstChild of jd_Node
 * @param node   node from which to get the relative
 * @return pointer to relative, NULL if node or relative is NULL
 */
EXPORT jd_Node* firstChild(jd_Node *node)
{
   JNode *jnode = (JNode*)node;
   if (node && jnode->firstChild)
      return jnode->firstChild;
   else
      return NULL;
}

/**
 * @brief returns lastChild of jd_Node
 * @param node   node from which to get the relative
 * @return pointer to relative, NULL if node or relative is NULL
 */
EXPORT jd_Node* lastChild(jd_Node *node)
{
   JNode *jnode = (JNode*)node;
   if (node && jnode->lastChild)
      return jnode->lastChild;
   else
      return NULL;
}

/**
 * @brief Identify a node with a type name string
 * @param node  Node to be identified
 * @return Type name of node, "nonode" if node is NULL
 */
EXPORT jd_Type jd_id_type(const jd_Node *node)
{
   if (node)
      return ((JNode*)node)->type;

   return (jd_Type)-1;
}

/**
 * @brief Identify a node with a type name string
 * @param node  Node to be identified
 * @return Type name of node, "nonode" if node is NULL
 */
EXPORT const char *jd_id_name(const jd_Node *node)
{
   if (node)
      return TypeNames[((JNode*)node)->type];
   else
      return "nonode";
}

EXPORT const void *jd_generic_value(const jd_Node *node)
{
   if (node)
      return ((JNode*)node)->payload;
   else
      return NULL;
}

EXPORT int jd_stringify_null(const jd_Node *node, char *buffer, int bufflen)
{
   return JNode_stringify_null((JNode*)node, buffer, bufflen);
}

EXPORT int jd_stringify_true(const jd_Node *node, char *buffer, int bufflen)
{
   return JNode_stringify_true((JNode*)node, buffer, bufflen);
}

EXPORT int jd_stringify_false(const jd_Node *node, char *buffer, int bufflen)
{
   return JNode_stringify_false((JNode*)node, buffer, bufflen);
}

EXPORT int jd_stringify_integer(const jd_Node *node, char *buffer, int bufflen)
{
   return JNode_stringify_integer((JNode*)node, buffer, bufflen);
}

EXPORT int jd_stringify_float(const jd_Node *node, char *buffer, int bufflen)
{
   return JNode_stringify_float((JNode*)node, buffer, bufflen);
}



EXPORT void jd_serialize(int jd_out, const jd_Node *node)
{
   JNode_serialize((JNode*)node, 0);
}








EXPORT bool jd_make_integer_node(jd_Node **node, long value)
{
   JNode *new_node = NULL;
   if (JNode_create(&new_node, NULL, NULL))
   {
      JNode_set_integer(new_node, value);
      *node = (jd_Node*)new_node;
      return true;
   }

   return false;
}
