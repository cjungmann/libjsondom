/** @file jd_Node.c */

#include <stdlib.h>   // malloc/free
#include <string.h>   // memset
#include <stdio.h>    // printf
#include <assert.h>

#include "JNode.h"

/**
 * @brief Array of type names aligned to #JDataType enumeration.
 */
const char* jd_Node_TypeLabels[] = {
   "null",
   "true",
   "false",
   "string",
   "integer",
   "float",
   "array",
   "property",
   "object",
   "INVALID_TYPE"
};

/**
 * @brief Remove a #jd_Node instance from its family.
 *
 * Update links to @b node from its siblings and parent nodes,
 * replacing them with links to appropriate siblings.  Links to
 * children of @b node will be left intact.
 *
 * @param node Pointer to jd_Node instance to be removed from its family.
 */
void jd_Node_emancipate(jd_Node *node)
{
   if (node->parent)
   {
      // Remove/replace parent direct links to node
      if (node->parent->firstChild == node)
         node->parent->firstChild = node->nextSibling;
      if (node->parent->lastChild == node)
         node->parent->lastChild = node->prevSibling;
      // then remove node's link to parent
      node->parent = NULL;

      // Remove/replace sibling direct links to node
      if (node->nextSibling)
         node->nextSibling->prevSibling = node->prevSibling;
      if (node->prevSibling)
         node->prevSibling->nextSibling = node->nextSibling;
      // then remove node's links to siblings
      node->nextSibling = node->prevSibling = NULL;
   }
}

/**
 * @brief
 *    Incorporate @b adoptee node into children of @b parent.
 * @details
 *    The jd_Node @b adoptee will be added as a child of @b parent.
 *    The #jd_Node::parent pointer of @b adoptee will be set to
 *    @b parent.  Then, iff @b before is not NULL, @b adoptee will
 *    be inserted into the child list of @b parent, otherwise
 *    @b adoptee will be added after the last child of @b parent.
 *
 * @param adoptee   The jd_Node instance to be incorporated
 * @param parent    The jd_Node instance to use as the parent of @b adoptee
 * @param before    Optional jd_Node instance of a child of @b parent
 *                  after which @b adoptee will be placed
 */
void jd_Node_adopt(jd_Node *adoptee, jd_Node *parent, jd_Node *before)
{
   assert(parent && adoptee);

   // Previous family connections should have been severed:
   assert(adoptee->parent==NULL);
   assert(adoptee->prevSibling==NULL);
   assert(adoptee->nextSibling==NULL);

   adoptee->parent = parent;

   // Adjust all the links If inserting within children:
   if (before)
   {
      // Don't allow parental confusion or step-children
      assert(before->parent == parent);

      if (before->prevSibling)
      {
         adoptee->prevSibling = before->prevSibling;
         before->prevSibling->nextSibling = adoptee;
      }
      else
         // If no before->prevSibling, adoptee is the new firstChild
         parent->firstChild = adoptee;

      adoptee->nextSibling = before;
      before->prevSibling = adoptee;
   }
   else
   {
      if (parent->lastChild)
      {
         adoptee->prevSibling = parent->lastChild;
         parent->lastChild->nextSibling = adoptee;
         parent->lastChild = adoptee;
      }
      else
         parent->firstChild = parent->lastChild = adoptee;
   }
}


/**
 * @brief
 *    Returns a new jd_Node instance of type JD_NULL.
 * @details
 *    Uses @c malloc to create a new jd_Node instance, using
 *    #jd_Node_adopt to incorporate the new node into an existing
 *    family of nodes.
 *
 * @param new_node  Address of pointer to which the new jd_Node
 *                  instance will be copied
 * @param parent    The jd_Node instance to use as the parent of @b new_node
 * @param before    Optional jd_Node instance of a child of @b parent
 *                  after which @b new_node will be placed
 *
 * @return
 *    True if successful
 *    False if failed to get needed memory
 */
bool jd_Node_create(jd_Node **new_node, jd_Node *parent, jd_Node *before)
{
   jd_Node *node = (jd_Node*)malloc(sizeof(jd_Node));
   if (node)
   {
      memset(node, 0, sizeof(jd_Node));
      node->type = JD_NULL;

      // Adjust family relationships
      if (parent)
         jd_Node_adopt(node, parent, before);

      *new_node = node;
      return true;
   }

   return false;
}

/**
 * @brief
 *    Deletes jd_Node instance @b node after deleting everything
 *    to which it points.
 * @details
 *    Recursively deletes children, then siblings of @b node, then
 *    uses @c free to delete its payload (if appropriate) and finally,
 *    itself.
 *
 * @param node   Instance to be deleted after its pointers are freed.
 *
 * @warning
 *    Use with care: jd_Node_destroy clears the jd_Node pointer in the
 *    calling function.  Avoid attempting to free the memory twice.
 */
void jd_Node_destroy(jd_Node **node)
{
   if (*node)
   {
      // The child will destroy siblings, so we won't have to:
      if ((*node)->firstChild)
         jd_Node_destroy(&(*node)->firstChild);

      if ((*node)->nextSibling)
         jd_Node_destroy(&(*node)->nextSibling);

      if ((*node)->payload)
         free((void*)(*node)->payload);

      free(*node);
      *node = NULL;
   }
}

/**
 * @brief Intelligently free memory of payload member
 */
bool jd_Node_discard_payload(jd_Node *node)
{
   if (node->payload)
   {
      free((void*)node->payload);
      node->payload = NULL;
   }

   return true;
}

/**
 * @brief Safely converts an initialized jd_Node of any type to a JD_NULL jd_Node.
 * @param node   jd_Node to be converted
 * @return true for success, false for failure
 */
bool jd_Node_set_null(jd_Node *node)
{
   jd_Node_discard_payload(node);
   node->type = JD_NULL;
   return true;
}

/**
 * @brief Safely converts an initialized jd_Node of any type to a JD_TRUE jd_Node.
 * @param node   jd_Node to be converted
 * @return true for success, false for failure
 */
bool jd_Node_set_true(jd_Node *node)
{
   jd_Node_discard_payload(node);
   node->type = JD_TRUE;
   return true;
}

/**
 * @brief Safely converts an initialized jd_Node of any type to a JD_FALSE jd_Node.
 * @param node   jd_Node to be converted
 * @return true for success, false for failure
 */
bool jd_Node_set_false(jd_Node *node)
{
   jd_Node_discard_payload(node);
   node->type = JD_FALSE;
   return true;
}

/**
 * @brief Safely converts an initialized jd_Node of any type to a JD_INTEGER jd_Node.
 * @param node   jd_Node to be converted
 * @param value  value to be set in the payload
 * @return true for success, false for failure
 */
bool jd_Node_set_integer(jd_Node *node, const char *value)
{
   if (jd_Node_copy_string(node, value))
   {
      node->type = JD_INTEGER;
      return true;
   }

   return false;
}

/**
 * @brief Safely converts an initialized jd_Node of any type to a JD_FLOAT jd_Node.
 * @param node   jd_Node to be converted
 * @param value  value to be set in the payload
 * @return true for success, false for failure
 */
bool jd_Node_set_float(jd_Node *node, const char *value)
{
   if (jd_Node_copy_string(node, value))
   {
      node->type = JD_FLOAT;
      return true;
   }

   return false;
}

/**
 * @brief Use supplied string as node's payload.
 *
 * jd_Node_destroy will take responsibility for deleting
 * the string argument when the jd_Node is destroyed.
 */
bool jd_Node_take_string(jd_Node *node, const char *str)
{
   jd_Node_discard_payload(node);
   node->payload = (void*)str;

   node->type = JD_STRING;

   return true;
}

/**
 * @brief Allocate new payload memory into which 'str' will be copied
 */
bool jd_Node_copy_string(jd_Node *node, const char *str)
{
   jd_Node_discard_payload(node);
   int len = strlen(str);
   char *new_payload = (char*)malloc(len+1);
   if (new_payload)
   {
      memcpy(new_payload, str, len);
      new_payload[len] = '\0';
      node->payload = (void*)new_payload;

      node->type = JD_STRING;

      return true;
   }

   return false;
}

/**
 * @brief Discards all subordinate memory and values
 */
bool jd_Node_make_null_property(jd_Node *node, const char *label)
{
   jd_Node_discard_payload(node);
   if (node->firstChild)
      jd_Node_destroy(&(node->firstChild));
   node->type = JD_PROPERTY;

   jd_Node *label_node, *value_node;
   if (jd_Node_create(&label_node, node, NULL))
   {
      if (jd_Node_create(&value_node, node, NULL))
      {
         jd_Node_copy_string(label_node, label);
         jd_Node_set_null(value_node);

         return true;
      }
      else
         jd_Node_destroy(&(node->firstChild));
   }

   return false;
}

/**
 * @brief Safely converts an initialized jd_Node of any type to an empty JD_ARRAY jd_Node.
 * @param node   jd_Node to be converted
 * @return true for success, false for failure
 */
bool jd_Node_make_array(jd_Node *node)
{
   jd_Node_discard_payload(node);
   node->type = JD_ARRAY;

   return true;
}

/**
 * @brief Add an initialized jd_Node into an existing jd_Node array
 * @param array           Array into which the new jd_Node is to be inserted
 * @param new_element     element to be inserted
 * @param element_before  if not NULL, the new element will be placed before this element.
 * @return True for success, false for failure
 */
bool jd_Node_array_insert_element(jd_Node *array, jd_Node *new_element, jd_Node *element_before)
{
   assert(array->type == JD_ARRAY);
   jd_Node_adopt(new_element, array, element_before);
   return true;
}

/**
 * @brief Safely converts an initialized jd_Node of any type to an empty JD_OBJECT jd_Node.
 * @param node   jd_Node to be converted
 * @return true for success, false for failure
 */
bool jd_Node_make_object(jd_Node *node)
{
   jd_Node_discard_payload(node);
   node->type = JD_OBJECT;

   return true;
}

/**
 * @brief
 *    Array of pointers to jd_Node printer functions
 * @details
 *    This array of pointers is aligned to enum #JDataType.
 **/
jd_Node_printer jNode_printers[] = {
   jd_Node_print_null,
   jd_Node_print_true,
   jd_Node_print_false,
   jd_Node_print_string,
   jd_Node_print_integer,
   jd_Node_print_float,
   jd_Node_print_array,
   jd_Node_print_property,
   jd_Node_print_object
};

/**
 * @brief JD_NULL jd_Node printing function for jNode_printers array
 * @param node   jd_Node to be printed
 * @param indent multiple of indents to print before value
 */
void jd_Node_print_null(const jd_Node *node, int indent)
{
   assert(node && node->type==JD_NULL);
   if (indent<0)
      printf("null");
   else
      printf("\n%*cnull", indent, ' ');
}

/**
 * @brief JD_TRUE jd_Node printing function for jNode_printers array
 * @param node   jd_Node to be printed
 * @param indent multiple of indents to print before value
 */
void jd_Node_print_true(const jd_Node *node, int indent)
{
   assert(node && node->type==JD_TRUE);
   if (indent<0)
      printf("true");
   else
      printf("\n%*ctrue", indent, ' ');
}

/**
 * @brief JD_FALSE jd_Node printing function for jNode_printers array
 * @param node   jd_Node to be printed
 * @param indent multiple of indents to print before value
 */
void jd_Node_print_false(const jd_Node *node, int indent)
{
   assert(node && node->type==JD_FALSE);
   if (indent<0)
      printf("false");
   else
      printf("\n%*cfalse", indent, ' ');
}

/**
 * @brief JD_STRING jd_Node printing function for jNode_printers array
 * @param node   jd_Node to be printed
 * @param indent multiple of indents to print before value
 */
void jd_Node_print_string(const jd_Node *node, int indent)
{
   assert(node && node->type==JD_STRING);
   if (indent<0)
      printf("\"%s\"", (char*)node->payload);
   else
      printf("\n%*c\"%s\"", indent, ' ', (char*)node->payload);
}

/**
 * @brief JD_INTEGER jd_Node printing function for jNode_printers array
 * @param node   jd_Node to be printed
 * @param indent multiple of indents to print before value
 */
void jd_Node_print_integer(const jd_Node *node, int indent)
{
   assert(node && node->type==JD_INTEGER);
   if (indent<0)
      printf("%s", (char*)node->payload);
   else
      printf("\n%*c%s", indent, ' ', (char*)node->payload);
}

/**
 * @brief JD_FLOAT jd_Node printing function for jNode_printers array
 * @param node   jd_Node to be printed
 * @param indent multiple of indents to print before value
 */
void jd_Node_print_float(const jd_Node *node, int indent)
{
   assert(node && node->type==JD_FLOAT);
   if (indent<0)
      printf("%s", (char*)node->payload);
   else
      printf("\n%*c%s", indent, ' ', (char*)node->payload);
}

/**
 * @brief JD_ARRAY jd_Node printing function for jNode_printers array
 * @details Recursively-prints child nodes with incremented indent.
 * @param node   jd_Node to be printed
 * @param indent multiple of indents to print before value
 */
void jd_Node_print_array(const jd_Node *node, int indent)
{
   assert(node && node->type==JD_ARRAY);

   jd_Node *child;
   int subindent = indent;
   if (indent<0)
      printf("[");
   else
   {
      printf("\n%*c[", indent, ' ');
      subindent += 4;
   }

   child = node->firstChild;
   while (child)
   {
      (*jNode_printers[child->type])(child, subindent);
      child = child->nextSibling;
      if (child)
         printf(",");
   }

   if (indent<0)
      printf("]");
   else
      printf("\n%*c]", indent, ' ');
}

/**
 * @brief JD_PROPERTY jd_Node printing function for jNode_printers array
 * @param node   jd_Node to be printed
 * @param indent multiple of indents to print before value
 */
void jd_Node_print_property(const jd_Node *node, int indent)
{
   assert(node && node->type==JD_PROPERTY);
   assert(node->firstChild && node->firstChild->type == JD_STRING);
   assert(node->firstChild->nextSibling == node->lastChild);

   const char *label = (char*)node->firstChild->payload;
   jd_Node *value = node->lastChild;
   bool is_collection = value->type >= JD_ARRAY;

   if (indent < 0)
   {
      printf("\"%s\":",  label);
      (*jNode_printers[value->type])(value, indent);
   }
   else
   {
      printf("\n%*c\"%s\":", indent, ' ', label);
      if (is_collection)
         indent += 4;
      else
         indent = -1;   // print value on same line with indent value
      (*jNode_printers[value->type])(value, indent);
   }
}

/**
 * @brief JD_OBJECT jd_Node printing function for jNode_printers array
 * @details Recursively-prints child nodes with incremented indent.
 * @param node   jd_Node to be printed
 * @param indent multiple of indents to print before value
 */
void jd_Node_print_object(const jd_Node *node, int indent)
{
   assert(node && node->type==JD_OBJECT);

   jd_Node *child;
   int subindent = indent;
   if (indent<0)
      printf("{");
   else
   {
      printf("\n%*c{", indent, ' ');
      subindent += 4;
   }

   child = node->firstChild;
   while (child)
   {
      (*jNode_printers[child->type])(child, subindent);
      child = child->nextSibling;
      if (child)
         printf(",");
   }

   if (indent<0)
      printf("}");
   else
      printf("\n%*c}", indent, ' ');
}

/**
 * @brief Recursive function prints tree to stdout
 */
void jd_Node_serialize(const jd_Node *node, int indent)
{
   (*jNode_printers[node->type])(node, indent);
   printf("\n");
}

#ifdef JNODE_MAIN

void populate_simple_array(jd_Node *parent)
{
   jd_Node *child;
   jd_Node_create(&child, parent, NULL);
   jd_Node_create(&child, parent, NULL);
   jd_Node_set_true(child);
   jd_Node_create(&child, parent, NULL);
   jd_Node_set_false(child);
   jd_Node_create(&child, parent, NULL);
   jd_Node_copy_string(child, "This is a string");
}

void populate_simple_object(jd_Node *parent)
{
   jd_Node *child;

   jd_Node_create(&child, parent, NULL);
   jd_Node_make_null_property(child, "one_array");
   populate_simple_array(child->lastChild);

   jd_Node_create(&child, parent, NULL);
   jd_Node_make_null_property(child, "two_true");
   jd_Node_set_true(child->lastChild);

   jd_Node_create(&child, parent, NULL);
   jd_Node_make_null_property(child, "three_false");
   jd_Node_set_false(child->lastChild);

   jd_Node_create(&child, parent, NULL);
   jd_Node_make_null_property(child, "four_string");
   jd_Node_copy_string(child->lastChild, "String value");

   jd_Node_create(&child, parent, NULL);
   jd_Node_make_null_property(child, "five_integer");
   jd_Node_set_integer(child->lastChild, "1000");

   jd_Node_create(&child, parent, NULL);
   jd_Node_make_null_property(child, "six_float");
   jd_Node_set_float(child->lastChild, "3.141592653589");
}

void test_array_of_arrays(void)
{
   jd_Node *root;
   if (jd_Node_create(&root, NULL, NULL))
   {
      jd_Node_make_array(root);

      jd_Node *array;
      jd_Node_create(&array, root, NULL);
      jd_Node_make_array(array);
      populate_simple_array(array);

      jd_Node_create(&array, root, NULL);
      jd_Node_make_array(array);
      populate_simple_array(array);

      jd_Node_create(&array, root, NULL);
      jd_Node_make_object(array);
      populate_simple_object(array);

      jd_Node_create(&array, root, NULL);
      jd_Node_make_array(array);
      populate_simple_array(array);

      jd_Node_serialize(root, 0);
      jd_Node_destroy(&root);
   }
}

int main(int argc, const char **argv)
{
   test_array_of_arrays();
   return 0;
}


#endif // JNODE_MAIN




/* Local Variables:               */
/* compile-command: "b=jd_Node;    \*/
/*   gcc -std=c99 -Wall -Werror  \*/
/*       -ggdb -pedantic         \*/
/*       -fsanitize=leak,address \*/
/*       -D${b^^}_MAIN           \*/
/*       -o $b ${b}.c"            */
/* End:                           */


