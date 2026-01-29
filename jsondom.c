/** @file jsondom.c */

#include "JNode.h"
#include "JParser.h"
#include "jsondom.h"
#include <string.h>  // for strlen
#include <assert.h>

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
EXPORT bool jd_parse_file(int fh, jd_Node **new_tree, jd_ParseError *pe)
{
   *new_tree = NULL;

   JNode *node = NULL;
   bool retval = JParser(fh, NULL, &node, 0, NULL, pe);
   if (retval)
   {
      if (confirm_no_further_file_content(fh))
         *new_tree = (jd_Node*)node;
      else
      {
         report_parse_error(pe, fh,
                            "forbidden characters following singleton root object");
         JNode_destroy((jd_Node*)node);
         retval = false;
      }
   }

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
 * @brief Get pointer to a relation
 * @param node     Starting point
 * @param relation relation direction to return
 * @return Requested relation of starting node, otherwise
 *         NULL if the relation is not represented.
 */
EXPORT jd_Node* jd_get_relation(jd_Node *node, jd_Relation relation)
{
   if ( node && (unsigned int)relation <= JD_LAST )
      return (jd_Node*)((JNode**)node)[relation];
   else
      return NULL;
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

EXPORT int jd_get_value_length(const jd_Node *node)
{
   JNode *jnode = (JNode*)node;

   int len_required = 0;
   switch(jnode->type)
   {
      case JD_NULL:
      case JD_TRUE:
         len_required = 5;
         break;
      case JD_FALSE:
         len_required = 6;
         break;
      case JD_STRING:
      case JD_INTEGER:
      case JD_FLOAT:
         len_required = 1 + strlen((char*)jnode->payload);
         break;
      case JD_ARRAY:
         len_required = 8;  // *array*\0
         break;
      case JD_OBJECT:
         len_required = 9;  // *object*\0
         break;
      case JD_PROPERTY:
         len_required = jd_get_value_length(jnode->firstChild)
            + jd_get_value_length(jnode->lastChild)
            + 2;    // colon between, \0 after
         break;
      default:
         // We shouldn't fall through to here:
         assert(0);
         break;
   }

   return len_required;
}

EXPORT int jd_stringify_value(const jd_Node *node, char *buffer, int bufflen)
{
   JNode *jnode = (JNode*)node;
   int len_required = jd_get_value_length(node);
   char *bptr;
   int tlen;
   if (bufflen >= len_required)
   {
      switch(jnode->type)
      {
         case JD_NULL:
            memcpy(buffer, "null", len_required);
            break;
         case JD_TRUE:
            memcpy(buffer, "true", len_required);
            break;
         case JD_FALSE:
            memcpy(buffer, "false", len_required);
            break;
         case JD_STRING:
         case JD_INTEGER:
         case JD_FLOAT:
            memcpy(buffer, jnode->payload, len_required);
            break;
         case JD_ARRAY:
            memcpy(buffer, "*array*", len_required);
            break;
         case JD_OBJECT:
            memcpy(buffer, "*object*", len_required);
            break;
         case JD_PROPERTY:
            bptr = buffer;
            tlen = jd_get_value_length(jnode->firstChild);
            --tlen;  // don't count or print the '\0'
            memcpy(bptr, jnode->firstChild->payload, tlen);
            bptr += tlen;
            *bptr++ = ':';
            jd_stringify_value(jnode->lastChild, bptr, len_required - tlen - 1);
            break;
         default:
            // We shouldn't fall through to here:
            assert(0);
            break;
      }
   }

   return len_required;
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




